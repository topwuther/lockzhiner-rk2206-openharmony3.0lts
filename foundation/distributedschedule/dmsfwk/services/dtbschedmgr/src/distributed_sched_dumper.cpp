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

#include "distributed_sched_dumper.h"

#include "distributed_sched_service.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
constexpr size_t MIN_ARGS_SIZE = 1;
const std::string ARGS_HELP = "-h";
const std::string ARGS_CONNECT_REMOTE_ABILITY = "-connect";
constexpr int32_t UID_ROOT = 0;
}

bool DistributedSchedDumper::DumpDefault(std::string& result)
{
    result.append("DistributedSched Dump\n");
    result.append("\n");
    ShowConnectRemoteAbility(result);
    return true;
}

bool DistributedSchedDumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    result.clear();
    if (!CanDump()) {
        result.append("Dump failed, not allowed");
        return false;
    }

    if (args.size() < MIN_ARGS_SIZE) {
        return DumpDefault(result);
    }

    if (args.size() == MIN_ARGS_SIZE) {
        // -h
        if (args[0] == ARGS_HELP) {
            ShowHelp(result);
            return true;
        }
        // -connect
        if (args[0] == ARGS_CONNECT_REMOTE_ABILITY) {
            ShowConnectRemoteAbility(result);
            return true;
        }
    }
    IllegalInput(result);
    return false;
}

void DistributedSchedDumper::ShowHelp(std::string& result)
{
    result.append("DistributedSched Dump options:\n")
        .append("  [-h] [cmd]...\n")
        .append("cmd maybe one of:\n")
        .append("  -connect: show all connected remote abilities.\n");
}

void DistributedSchedDumper::IllegalInput(std::string& result)
{
    result.append("The arguments are illegal and you can enter '-h' for help.\n");
}

void DistributedSchedDumper::ShowConnectRemoteAbility(std::string& result)
{
    DistributedSchedService::GetInstance().DumpConnectInfo(result);
}

bool DistributedSchedDumper::CanDump()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    HILOGI("calling uid = %u", callingUid);
    if (callingUid != UID_ROOT) {
        return false;
    }
    return true;
}
} // namespace DistributedSchedule
} // namespace OHOS