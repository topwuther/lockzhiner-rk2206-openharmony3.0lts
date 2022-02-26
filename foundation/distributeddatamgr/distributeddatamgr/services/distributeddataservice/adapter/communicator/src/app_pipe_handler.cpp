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

#include "app_pipe_handler.h"
#include <thread>
#include <codecvt>
#include <locale>
#include <string>
#include "log_print.h"
#include "reporter.h"
#include "dds_trace.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "AppPipeHandler"

namespace OHOS {
namespace AppDistributedKv {
using namespace std;
using namespace OHOS::DistributedKv;

AppPipeHandler::~AppPipeHandler()
{
    ZLOGI("destructor pipeId: %{public}s", pipeInfo_.pipeId.c_str());
}

AppPipeHandler::AppPipeHandler(const PipeInfo &pipeInfo)
    : pipeInfo_(pipeInfo)
{
    ZLOGI("constructor pipeId: %{public}s", pipeInfo_.pipeId.c_str());
    softbusAdapter_ = SoftBusAdapter::GetInstance();
}

Status AppPipeHandler::SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size,
                                const MessageInfo &info)
{
    return softbusAdapter_->SendData(pipeInfo, deviceId, ptr, size, info);
}

Status AppPipeHandler::StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    return softbusAdapter_->StartWatchDataChange(observer, pipeInfo);
}

Status AppPipeHandler::StopWatchDataChange(__attribute__((unused))const AppDataChangeListener *observer,
                                           const PipeInfo &pipeInfo)
{
    return softbusAdapter_->StopWatchDataChange(observer, pipeInfo);
}

bool AppPipeHandler::IsSameStartedOnPeer(const struct PipeInfo &pipeInfo,
                                         __attribute__((unused))const struct DeviceId &peer)
{
    return softbusAdapter_->IsSameStartedOnPeer(pipeInfo, peer);
}

void AppPipeHandler::SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag)
{
    return softbusAdapter_->SetMessageTransFlag(pipeInfo, flag);
}

int AppPipeHandler::CreateSessionServer(const std::string &sessionName) const
{
    return softbusAdapter_->CreateSessionServerAdapter(sessionName);
}

int AppPipeHandler::RemoveSessionServer(const std::string &sessionName) const
{
    return softbusAdapter_->RemoveSessionServerAdapter(sessionName);
}
}  // namespace AppDistributedKv
}  // namespace OHOS
