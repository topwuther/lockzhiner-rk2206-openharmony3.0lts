/*
 * Copyright (c) 2021 Huawei Device Co., Ltd. 2021-2021.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "filesystem_manager.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "filesystem_log.h"

/*
MS_RDONLY
MS_NOSUID
MS_NODEV
MS_NOEXEC
MS_SYNCHRONOUS
MS_REMOUNT
MS_MANDLOCK
MS_DIRSYNC
MS_NOATIME
MS_NODIRATIME
MS_BIND
MS_MOVE
MS_REC
MS_SILENT
MS_POSIXACL
MS_UNBINDABLE
MS_PRIVATE
MS_SLAVE
MS_SHARED
MS_RELATIME
MS_KERNMOUNT
MS_I_VERSION
MS_STRICTATIME
MS_LAZYTIME
MS_ACTIVE
MS_NOUSER
MS_RMT_MASK
MS_MGC_VAL
MS_MGC_MSK
*/

using namespace OHOS::FsMountTab;
namespace OHOS {
constexpr int MAX_FORMAT_OPTION_COUNT = 2;

int FileSystemManager::FsMount(const FsMountTab::FsMountEntry &entry)
{
    int ret = -1;
    struct stat st {};
    if (!entry.IsValid()) {
        SSLOG_E("Invalid mount entry,exit");
        return -1;
    }
    if (stat(entry.mountPoint.c_str(), &st) != 0 && errno != ENOENT) {
        SSLOG_E("Cannot get %{public}s status: %{public}d", entry.mountPoint.c_str(), errno);
        return -1;
    }
    if ((st.st_mode & S_IFMT) == S_IFLNK) { // link, delete it.
        unlink(entry.mountPoint.c_str());
    }
    mkdir(entry.mountPoint.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    SSLOG_D("cSource:%{public}s", entry.blkDev.c_str());
    SSLOG_D("cTarget:%{public}s", entry.mountPoint.c_str());
    SSLOG_D("cfsType:%{public}s", entry.fsType.c_str());
    SSLOG_D("mountFlag:%{public}lu", entry.mountFlags);
    SSLOG_D("fsOption:%{public}s", entry.mountExtra.c_str());

    if (entry.mountExtra.empty()) {
        ret = mount(entry.blkDev.c_str(), entry.mountPoint.c_str(), entry.fsType.c_str(), entry.mountFlags,
                    nullptr);
        SSLOG_D("The fifth parameter of this mount is NULL");
    } else {
        ret = mount(entry.blkDev.c_str(), entry.mountPoint.c_str(), entry.fsType.c_str(), entry.mountFlags,
                    entry.mountExtra.c_str());
        SSLOG_D("The fifth parameter of this mount is not NULL");
    }
    if (ret < 0) {
        return -1;
    }
    return 0;
}

int FileSystemManager::FsIsSupport(const std::string &fsType)
{
    std::vector<std::string> supportedFilesystems = { "ext4", "f2fs", "vfat", "exfat", "ntfs" };
    for (auto it : supportedFilesystems) {
        if (it == fsType) {
            return 1;
        }
    }
    return 0;
}

int FileSystemManager::FormatCommandPatch(FormatInfo &stFormatAttr)
{
    static std::map<std::string, std::string> formatCommandMap = { { "ext4", "/system/bin/mke2fs" },
                                                                   { "f2fs", "/system/bin/make_f2fs" },
                                                                   { "vfat", "/system/bin/newfs_msdos" },
                                                                   { "exfat", "/system/bin/mkfs.exfat" },
                                                                   { "ntfs", "/system/bin/mkfs.ntfs" } };
    auto it = formatCommandMap.find(stFormatAttr.type);
    if (it == formatCommandMap.end()) {
        return -1;
    }

    if (access(it->second.c_str(), X_OK) != 0) {
        return -1;
    }
    stFormatAttr.cmd = it->second;
    return 0;
}

int FileSystemManager::FsUMount(const std::string &tarGet)
{
    int ret = -1;
    if (tarGet.size() <= 0) {
        SSLOG_E("invalid tarGet");
        return -1;
    }
    ret = umount(tarGet.c_str());
    if (ret < 0) {
        SSLOG_E("call mount failed");
        perror("mount");
        return -1;
    }
    SSLOG_D("umount(%s) successful", tarGet.c_str());
    return 0;
}

int FileSystemManager::ParseFormatAttr(const std::string &formatAttr, FormatInfo &stFormatAttr)
{
    const std::string delim(" ");
    std::vector<std::string> formatAttrVector;
    Split(formatAttr, delim, formatAttrVector);
    if (formatAttrVector.size() != MAX_FORMAT_OPTION_COUNT) {
        return -1;
    }
    stFormatAttr.source = formatAttrVector[0];
    stFormatAttr.type = formatAttrVector[1];
    return 0;
}

int FileSystemManager::GetFormatParameters(const FormatInfo &stFormatInfo,
                                           std::vector<std::string> &formatParameters)
{
    constexpr int blockSize = 4096;

    if (stFormatInfo.type == "ext4") {
        formatParameters.push_back("-F");
        formatParameters.push_back("-f");
        formatParameters.push_back("ext4");
        formatParameters.push_back("-b");
        formatParameters.push_back(std::to_string(blockSize));
    } else if (stFormatInfo.type == "vfat") {
        formatParameters.push_back("-p");
        formatParameters.push_back("-f");
        formatParameters.push_back("-y");
    } else {
        SSLOG_I("No Need add Extra paramter");
    }
    formatParameters.push_back(stFormatInfo.source);
    if (PerformFormatting(formatParameters) < 0) {
        SSLOG_E("call PerformFormatting failed\n");
        return -1;
    }
    return 0;
}

int FileSystemManager::PerformFormatting(std::vector<std::string> &formatParameters)
{
    int formatParaSize = 0;
    std::vector<char *> cmds;
    std::vector<char *> newenviron = {};
    newenviron.push_back(nullptr);

    formatParaSize = formatParameters.size();
    for (int i = 0; i < formatParaSize; i++) {
        cmds.emplace_back(const_cast<char *>(formatParameters[i].c_str()));
    }
    cmds.emplace_back(nullptr);

    if (MOUNT_ERROR == (execve(formatParameters[0].data(), cmds.data(), newenviron.data()))) {
        SSLOG_E("call execve failed\n");
        return -1;
    }
    return 0;
}

int FileSystemManager::DoMount(const std::string &mountAttr)
{
    int mountRet = -1;
    FsMountTab::FsMountEntry entry;
    if (!FsMountTab::ParseMountEntry(mountAttr, entry)) {
        SSLOG_E("call ParseMountAttr failed\n");
        return -1;
    }
    mountRet = FsMount(entry);
    if (mountRet != 0) {
        return -1;
    }
    // SendMountOption(mountAttr);
    return 0;
}

int FileSystemManager::DoUMount(const std::string &umountAttr)
{
    const std::string delim(" ");
    std::vector<std::string> umountAttrVector;
    Split(umountAttr, delim, umountAttrVector);
    size_t umountSize = 0;
    int ret = 0;
    for (umountSize = 0; umountSize < umountAttrVector.size(); umountSize++) {
        ret = FsUMount(umountAttrVector[umountSize]);
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

int FileSystemManager::DoFormat(const std::string &formatAttr)
{
    std::vector<std::string> formatParameters = {};
    FileSystemManager::FormatInfo stFormatInfo;
    if (ParseFormatAttr(formatAttr, stFormatInfo) < 0) {
        SSLOG_E("call ParseMountAttr failed\n");
        return -1;
    }
    if (FormatCommandPatch(stFormatInfo) < 0) {
        SSLOG_E("call ParseFormatCommand failed\n");
        return -1;
    }
    if (GetFormatParameters(stFormatInfo, formatParameters) < 0) {
        SSLOG_E("call GetFormatParameters failed\n");
        return -1;
    }

    return 0;
}

} // namespace OHOS

extern "C" void DoMountTo(const char *mountAttr, int maxArg)
{
    std::string str(mountAttr);
    OHOS::FileSystemManager::DoMount(str);
}

extern "C" int DoUnMountTo(const char *mountAttr, int maxArg)
{
    std::string str(mountAttr);
    int ret = OHOS::FileSystemManager::DoUMount(str);
    return ret;
}