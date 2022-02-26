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
#ifndef SERVICE_FS_MOUNT_TABLE_H
#define SERVICE_FS_MOUNT_TABLE_H
#include <string>
#include <unistd.h>
#include <vector>

namespace OHOS {
namespace FsMountTab {
enum {
    // MOUNT_CMD = 0,
    MOUNT_BLKDEV,
    MOUNT_MOUNTPOINT,
    MOUNT_FSTYPE,
    MOUNT_MOUNTFLAGS,
    MOUNT_FSMGRFLAGS,
    MOUNT_OTHER,
    MOUNT_INVALID
};
struct FsMountEntry {
    std::string blkDev;
    std::string logicPartition;
    std::string mountPoint;
    std::string fsType;
    unsigned long mountFlags = 0;
    std::string encryptOptions;
    std::string mountExtra;
    std::string fsOptions;
    struct FsMgrFlags {
        bool wait : 1;
        bool check : 1;
        bool crypt : 1;
        bool nonremovable : 1;
        bool onlyRecovery : 1;
        bool noEmulatedSd : 1; // No emulated sdcard daemon; sd card is the only external
        bool noTrim : 1;
        bool fileEncryption : 1;
        bool forceCrypt : 1;
        bool formattable : 1;
        bool forceFdeFbe : 1;
        bool lateMount : 1;
        bool noFail : 1;
        bool quota : 1;
        bool logical : 1;
        bool checkpointBlk : 1;
        bool checkpointFs : 1;
        bool wrappedKey : 1;
    } fsMgrFlags = {};
    bool IsEncryptable() const
    {
        return fsMgrFlags.forceCrypt | fsMgrFlags.forceFdeFbe | fsMgrFlags.crypt;
    }
    bool IsValid() const;
};

bool ParseMountEntry(const std::string &strEnt, FsMountEntry &entry);
bool ParseMountFlags(const std::string &strMount, FsMountEntry &entry);
bool ParseFsMgrFlags(const std::string &strFsmgr, FsMountEntry &entry);
void WaitForFile(const std::string &source, unsigned int maxCount);
int Split(std::string source, const std::string &delim, std::vector<std::string> &items);
inline bool StartsWith(std::string s, std::string preFix)
{
    return s.substr(0, preFix.size()) == preFix;
}
} // namespace FsMountTab
} // namespace OHOS

#endif // SERVICE_FS_MOUNT_TABLE_H