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

#include "app_pipe_mgr.h"
#include "reporter.h"

#undef LOG_TAG
#define LOG_TAG "AppPipeMgr"

namespace OHOS {
namespace AppDistributedKv {
using namespace OHOS::DistributedKv;
Status AppPipeMgr::StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    ZLOGI("begin");
    if (observer == nullptr || pipeInfo.pipeId.empty()) {
        ZLOGE("argument invalid");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lock(dataBusMapMutex_);
    auto it = dataBusMap_.find(pipeInfo.pipeId);
    if (it == dataBusMap_.end()) {
        ZLOGE("pipeid not found");
        return Status::ERROR;
    }
    ZLOGI("end");
    return it->second->StartWatchDataChange(observer, pipeInfo);
}

// stop DataChangeListener to watch data change;
Status AppPipeMgr::StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    ZLOGI("begin");
    if (observer == nullptr || pipeInfo.pipeId.empty()) {
        ZLOGE("argument invalid");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lock(dataBusMapMutex_);
    auto it = dataBusMap_.find(pipeInfo.pipeId);
    if (it == dataBusMap_.end()) {
        ZLOGE("pipeid not found");
        return Status::ERROR;
    }
    ZLOGI("end");
    return it->second->StopWatchDataChange(observer, pipeInfo);
}

// Send data to other device, function will be called back after sent to notify send result.
Status AppPipeMgr::SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size,
                            const MessageInfo &info)
{
    if (size > MAX_TRANSFER_SIZE || size <= 0 || ptr == nullptr ||
        pipeInfo.pipeId.empty() || deviceId.deviceId.empty()) {
        ZLOGW("Input is invalid, maxSize:%d, current size:%d", MAX_TRANSFER_SIZE, size);
        return Status::ERROR;
    }
    ZLOGD("pipeInfo:%s ,size:%d", pipeInfo.pipeId.c_str(), size);
    std::shared_ptr<AppPipeHandler> appPipeHandler;
    {
        std::lock_guard<std::mutex> lock(dataBusMapMutex_);
        auto it = dataBusMap_.find(pipeInfo.pipeId);
        if (it == dataBusMap_.end()) {
            ZLOGW("pipeInfo:%s not found", pipeInfo.pipeId.c_str());
            return Status::KEY_NOT_FOUND;
        }
        appPipeHandler = it->second;
    }
    return appPipeHandler->SendData(pipeInfo, deviceId, ptr, size, info);
}

// start server
Status AppPipeMgr::Start(const PipeInfo &pipeInfo)
{
    if (pipeInfo.pipeId.empty()) {
        ZLOGW("Start Failed, pipeInfo is empty.");
        return Status::INVALID_ARGUMENT;
    }
    std::lock_guard<std::mutex> lock(dataBusMapMutex_);
    auto it = dataBusMap_.find(pipeInfo.pipeId);
    if (it != dataBusMap_.end()) {
        ZLOGW("repeated start, pipeInfo:%{public}s.", pipeInfo.pipeId.c_str());
        return Status::REPEATED_REGISTER;
    }
    ZLOGD("Start pipeInfo:%{public}s ", pipeInfo.pipeId.c_str());
    auto handler = std::make_shared<AppPipeHandler>(pipeInfo);
    int ret = handler->CreateSessionServer(pipeInfo.pipeId);
    if (ret != 0) {
        ZLOGW("Start pipeInfo:%{public}s, failed ret:%{public}d.", pipeInfo.pipeId.c_str(), ret);
        return Status::ILLEGAL_STATE;
    }

    dataBusMap_.insert(std::pair<std::string, std::shared_ptr<AppPipeHandler>>(pipeInfo.pipeId, handler));
    return Status::SUCCESS;
}

// stop server
Status AppPipeMgr::Stop(const PipeInfo &pipeInfo)
{
    std::shared_ptr<AppPipeHandler> appPipeHandler;
    {
        std::lock_guard<std::mutex> lock(dataBusMapMutex_);
        auto it = dataBusMap_.find(pipeInfo.pipeId);
        if (it == dataBusMap_.end()) {
            ZLOGW("pipeInfo:%s not found", pipeInfo.pipeId.c_str());
            return Status::KEY_NOT_FOUND;
        }
        appPipeHandler = it->second;
        int ret = appPipeHandler->RemoveSessionServer(pipeInfo.pipeId);
        if (ret != 0) {
            ZLOGW("Stop pipeInfo:%s ret:%d.", pipeInfo.pipeId.c_str(), ret);
            return Status::ERROR;
        }
        dataBusMap_.erase(pipeInfo.pipeId);
        return Status::SUCCESS;
    }
    return Status::KEY_NOT_FOUND;
}

bool AppPipeMgr::IsSameStartedOnPeer(const struct PipeInfo &pipeInfo, const struct DeviceId &peer)
{
    ZLOGI("start");
    if (pipeInfo.pipeId.empty() || peer.deviceId.empty()) {
        ZLOGE("pipeId or deviceId is empty. Return false.");
        return false;
    }
    ZLOGI("pipeInfo == [%s]", pipeInfo.pipeId.c_str());
    std::shared_ptr<AppPipeHandler> appPipeHandler;
    {
        std::lock_guard<std::mutex> lock(dataBusMapMutex_);
        auto it = dataBusMap_.find(pipeInfo.pipeId);
        if (it == dataBusMap_.end()) {
            ZLOGE("pipeInfo:%s not found. Return false.", pipeInfo.pipeId.c_str());
            return false;
        }
        appPipeHandler = it->second;
    }
    return appPipeHandler->IsSameStartedOnPeer(pipeInfo, peer);
}

void AppPipeMgr::SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag)
{
    if (pipeInfo.pipeId.empty()) {
        return;
    }
    std::shared_ptr<AppPipeHandler> appPipeHandler;
    {
        std::lock_guard<std::mutex> lock(dataBusMapMutex_);
        auto it = dataBusMap_.find(pipeInfo.pipeId);
        if (it == dataBusMap_.end()) {
            ZLOGW("pipeInfo:%s not found", pipeInfo.pipeId.c_str());
            return;
        }
        appPipeHandler = it->second;
    }
    appPipeHandler->SetMessageTransFlag(pipeInfo, flag);
}
}  // namespace AppDistributedKv
}  // namespace OHOS
