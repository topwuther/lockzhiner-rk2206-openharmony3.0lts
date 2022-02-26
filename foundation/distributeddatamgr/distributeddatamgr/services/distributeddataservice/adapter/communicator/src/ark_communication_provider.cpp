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

#include "ark_communication_provider.h"
#include "log_print.h"

#undef LOG_TAG
#define LOG_TAG "ArkCommunicationProvider"

namespace OHOS {
namespace AppDistributedKv {
CommunicationProvider &ArkCommunicationProvider::Init()
{
    static ArkCommunicationProvider instance;
    if (instance.isInited) {
        return instance;
    }
    ZLOGI("begin");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!instance.isInited) {
        instance.Initialize();
    }
    instance.isInited = true;
    ZLOGI("normal end");
    return instance;
}
ArkCommunicationProvider::ArkCommunicationProvider()
    : CommunicationProviderImpl(appPipeMgrImpl_, appDeviceHandlerImpl_)
{
}

DeviceInfo ArkCommunicationProvider::GetLocalDevice() const
{
    if (deviceQuery_ == nullptr) {
        return CommunicationProviderImpl::GetLocalDevice();
    }
    return deviceQuery_->GetLocalDevice();
}

std::vector<DeviceInfo> ArkCommunicationProvider::GetDeviceList() const
{
    if (deviceQuery_ == nullptr) {
        return CommunicationProviderImpl::GetDeviceList();
    }
    return deviceQuery_->GetDeviceList();
}

void ArkCommunicationProvider::SetDeviceQuery(std::shared_ptr<IDeviceQuery> deviceQuery)
{
    ZLOGI("set device query");
    deviceQuery_ = deviceQuery;
}
}  // namespace AppDistributedKv
}  // namespace OHOS
