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
#ifndef STORAGE_SERVICES_FILESYSTEMMANAGER_H
#define STORAGE_SERVICES_FILESYSTEMMANAGER_H

#include <string>
#include <vector>
#include <unistd.h>
#include <string_view>
#include "fs_mount_tab.h"
#if !defined(_WIN32) && !defined(O_BINARY)
#define O_BINARY 0
#endif
namespace OHOS {
enum MountStatus : int {
    MOUNT_ERROR = -1,
    MOUNT_UMOUNTED = 0,
    MOUNT_MOUNTED = 1,
};

enum Figures : int {
    MOUNT_ZERO = 0,
    MOUNT_ONE = 1,
    MOUNT_TWO = 2,
    MOUNT_THREE = 3,
    MOUNT_FOUR = 4,
    MOUNT_FIVE = 5,
    MOUNT_SIX = 6,
};

class FileSystemManager {
public:
    static int DoMount(const std::string &mountAttr);
    static int DoUMount(const std::string &mountAttr);
    static int DoFormat(const std::string &formatAttr);

    struct FormatInfo {
        std::string source;
        std::string type;
        std::string cmd;
    };

private:
    static int FsMount(const FsMountTab::FsMountEntry &entry);
    static int FsUMount(const std::string &target);
    static int FormatCommandPatch(FormatInfo &stFormatAttr);
    static int GetFormatParameters(const FormatInfo &stFormatInfo,
                                   std::vector<std::string> &formatParameters);
    static int PerformFormatting(std::vector<std::string> &formatParameters);
    static int FsIsSupport(const std::string &fsType);
    static int ParseFormatAttr(const std::string &formatAttr, FormatInfo &stFormatAttr);
    static bool SendMountOption(const std::string &mountOption);
};
} // namespace OHOS

#endif // STORAGE_SERVICES_FILESYSTEMMANAGER_H