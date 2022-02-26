/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "log_catcher_utils.h"

#include <sstream>
#include <string>

#include "common_utils.h"
#include "file_util.h"
#include "logger.h"
#include "string_util.h"
namespace OHOS {
namespace HiviewDFX {
namespace LogCatcherUtils {
int DumpStacktrace(int fd, int pid)
{
    if (fd < 0) {
        return -1;
    }
    (void)pid;
    return 0;
}

bool IsJavaProcess(pid_t pid)
{
    // read proc/pid/stat and check number after process statu
    // 901234567890123456789012345678
    // 2193 (package) S 624
    std::string readPath = "/proc/" + std::to_string(pid) + "/stat";
    std::string stat;
    FileUtil::LoadStringFromFile(readPath, stat);

    // skip process status
    std::string pidStr = StringUtil::FindMatchSubString(stat, ")", 4, " ");  // 4: offset
    pid_t ppid = 0;
    std::stringstream ss;
    ss << pidStr;
    ss >> ppid;
    // the pidStr can be empty
    if ((ss.fail()) || (ppid <= 0)) {
        return false;
    }

    // 624 (main) S 1
    std::string parentStatPath = "/proc/" + std::to_string(ppid) + "/stat";
    std::string parentStat;
    FileUtil::LoadStringFromFile(parentStatPath, parentStat);
    std::string processName = StringUtil::FindMatchSubString(parentStat, "(", 1, ")"); // 1: offset
    if (processName.find("main") == std::string::npos) {
        return false;
    }
    return true;
}

bool ReadCPUInfo(int fd, int pid)
{
    std::string content;
    bool ret1 = FileUtil::LoadStringFromFile("/proc/cpuinfo", content);
    FileUtil::SaveStringToFd(fd, content);
    return ret1;
}

bool ReadMemoryInfo(int fd, int pid)
{
    std::string content;
    bool ret1 = FileUtil::LoadStringFromFile("/proc/meminfo", content);
    FileUtil::SaveStringToFd(fd, content);
    return ret1;
}
}
} // namespace HiviewDFX
} // namespace OHOS