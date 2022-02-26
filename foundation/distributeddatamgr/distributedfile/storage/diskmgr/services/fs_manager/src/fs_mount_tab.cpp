/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "fs_mount_tab.h"

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
#include "fs_mount_tab.h"
namespace OHOS {
namespace FsMountTab {
constexpr int WAIT_MAX_COUNT = 10;
bool FsMountEntry::IsValid() const
{
    if (mountPoint.empty() || blkDev.empty() || fsType.empty()) {
        return false;
    }
    if ((mountPoint[0] != '/')) {
        return false;
    }
    return true;
}
bool ParseMountEntry(const std::string &strOpt, FsMountEntry &entry)
{
    bool ret = false;
    const std::string delim(" ");
    std::vector<std::string> attrs;
    Split(strOpt, " ", attrs);

    int MIN_MOUNT_OPTION = 3;
    int MAX_MOUNT_OPTION = 5;
    if (attrs.size() < MIN_MOUNT_OPTION && attrs.size() > MAX_MOUNT_OPTION) {
        SSLOG_E("invalid mount item, mountAttrVector.size()=%{public}d", attrs.size());
        return false;
    }
    entry.blkDev = attrs[MOUNT_BLKDEV];
    entry.mountPoint = attrs[MOUNT_MOUNTPOINT];
    entry.fsType = attrs[MOUNT_FSTYPE];
    if (attrs.size() >= MOUNT_FSMGRFLAGS) {
        ret = ParseMountFlags(attrs[MOUNT_MOUNTFLAGS], entry);
    }
    if (attrs.size() == MOUNT_OTHER) {
        ret = ParseFsMgrFlags(attrs[MOUNT_FSMGRFLAGS], entry);
    }
    return ret;
}
void WaitForFile(const std::string &source, unsigned int maxCount)
{
    int flag = maxCount;
    if (flag > WAIT_MAX_COUNT) {
        SSLOG_E("WaitForFile max time is 5s");
        flag = WAIT_MAX_COUNT;
    }
    struct stat sourceInfo;
    int MAX_WAIT_TIME = 500000;
    unsigned int waitTime = MAX_WAIT_TIME;
    unsigned int count = 0;
    do {
        usleep(waitTime);
        count++;
    } while ((stat(source.c_str(), &sourceInfo) < 0) && (errno == ENOENT) && (count < flag));
    if (count == flag) {
        SSLOG_E("wait for file:%s failed after retry %d. times", source.c_str(), flag);
    }
}

static std::map<std::string, unsigned long> mountFlagMap = {
    { "rdonly", MS_RDONLY },
    { "nosuid", MS_NOSUID },
    { "nodev", MS_NODEV },
    { "noexec", MS_NOEXEC },
    { "synchronous", MS_SYNCHRONOUS },
    { "remount", MS_REMOUNT },
    { "mandlock", MS_MANDLOCK },
    { "dirsync", MS_DIRSYNC },
    { "noatime", MS_NOATIME },
    { "nodiratime", MS_NODIRATIME },
    { "bind", MS_BIND },
    { "move", MS_MOVE },
    { "rec", MS_REC },
    { "silent", MS_SILENT },
    { "posixacl", MS_POSIXACL },
    { "unbindable", MS_UNBINDABLE },
    { "private", MS_PRIVATE },
    { "slave", MS_SLAVE },
    { "shared", MS_SHARED },
    { "relatime", MS_RELATIME },
    { "kernmount", MS_KERNMOUNT },
    { "i_version", MS_I_VERSION },
    { "strictatime", MS_STRICTATIME },
    { "lazytime", MS_LAZYTIME },
    { "active", MS_ACTIVE },
    { "nouser", MS_NOUSER },
    { "rmt_mask", MS_RMT_MASK },
    { "mgc_val", MS_MGC_VAL },
    { "mgc_msk", MS_MGC_MSK },
    { "defaults", 0 },
};

bool ParseMountFlags(const std::string &mountStr, FsMountEntry &entry)
{
    std::vector<std::string> flags;
    Split(mountStr, ",", flags);
    SSLOG_D("mount flasg size is =%{public}u", (unsigned int)flags.size());

    for (auto &item : flags) {
        if (item == "wait") {
            WaitForFile(entry.blkDev, WAIT_MAX_COUNT);
            continue;
        }
        auto it = mountFlagMap.find(item);
        if (it == mountFlagMap.end()) {
            SSLOG_E("%{public}s not find, failed", item.c_str());
            if (!entry.mountExtra.empty()) {
                entry.mountExtra.append(",");
            }
            entry.mountExtra.append(item);
        } else {
            SSLOG_D("find first=%{public}s second=%{public}ld", it->first.c_str(), it->second);
            auto flag = it->second;
            entry.mountFlags |= flag;
            SSLOG_D("mountFlag=%{public}ld", entry.mountFlags);
        }
    }
    return true;
}
bool ParseFsMgrFlags(const std::string &strFsmgr, FsMountEntry &entry)
{
    std::vector<std::string> flags;
    Split(strFsmgr, ",", flags);
    for (auto &item : flags) {
        if (item == "defaults") {
            continue;
        }
        std::string value;
        size_t equalSignPosition = 0;
        equalSignPosition = item.find('=');

        if (item == "checkpoint=block") {
            entry.fsMgrFlags.checkpointBlk = true;
            continue;
        } else if (item == "checkpoint=fs") {
            entry.fsMgrFlags.checkpointFs = true;
            continue;
        }
        if (equalSignPosition == std::string::npos) {
            if (item == "wait") {
                entry.fsMgrFlags.wait = true;
                continue;
            } else if (item == "check") {
                entry.fsMgrFlags.check = true;
                continue;
            } else if (item == "nonremovable") {
                entry.fsMgrFlags.nonremovable = true;
                continue;
            } else if (item == "recoveryonly") {
                entry.fsMgrFlags.onlyRecovery = true;
                continue;
            } else if (item == "noemulatedsd") {
                entry.fsMgrFlags.noEmulatedSd = true;
                continue;
            } else if (item == "notrim") {
                entry.fsMgrFlags.noTrim = true;
                continue;
            } else if (item == "formattable") {
                entry.fsMgrFlags.formattable = true;
                continue;
            } else if (item == "latemount") {
                entry.fsMgrFlags.lateMount = true;
                continue;
            } else if (item == "nofail") {
                entry.fsMgrFlags.noFail = true;
                continue;
            } else if (item == "quota") {
                entry.fsMgrFlags.quota = true;
                continue;
            } else if (item == "wrappedkey") {
                entry.fsMgrFlags.wrappedKey = true;
                continue;
            }
        } else {
            value = item.substr(equalSignPosition + 1);
        }

        if (StartsWith(item, "encryptable=")) {
            entry.fsMgrFlags.crypt = true;
        } else if (StartsWith(item, "forceencrypt=")) {
            entry.fsMgrFlags.forceCrypt = true;
        } else if (StartsWith(item, "fileencryption=")) {
            entry.encryptOptions = value;
            entry.fsMgrFlags.fileEncryption = true;
        } else if (StartsWith(item, "forcefdeorfbe=")) {
            entry.fsMgrFlags.forceFdeFbe = true;
            entry.encryptOptions = "aes-256-xts:aes-256-cts";
        } else {
            std::cout << "Warning: unknown flag: " << item;
        }
    }
    return true;
}
int Split(std::string source, const std::string &delim, std::vector<std::string> &items)
{
    size_t found = std::string::npos;
    size_t start = 0;
    while (true) {
        found = source.find_first_of(delim, start);
        items.push_back(source.substr(start, found - start));
        if (found == std::string::npos) {
            break;
        }
        start = found + 1;
    }
    return 0;
}

} // namespace FsMountTab
} // namespace OHOS