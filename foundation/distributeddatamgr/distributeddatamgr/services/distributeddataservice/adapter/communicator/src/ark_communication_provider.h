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

#ifndef DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H
#define DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H

#include "communication_provider_impl.h"
#include "idevice_query.h"
#include "app_device_handler.h"
#include "app_pipe_mgr.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppDistributedKv {
class ArkCommunicationProvider : public CommunicationProviderImpl {
public:
    static CommunicationProvider &Init();
    void SetDeviceQuery(std::shared_ptr<IDeviceQuery> deviceQuery) override;
    // Get online deviceList
    std::vector<DeviceInfo> GetDeviceList() const override;
    // Get local device information
    DeviceInfo GetLocalDevice() const override;

    ~ArkCommunicationProvider() override {};
private:
    DISALLOW_COPY_AND_MOVE(ArkCommunicationProvider);
    ArkCommunicationProvider();
    std::shared_ptr<IDeviceQuery> deviceQuery_ {};
    AppPipeMgr appPipeMgrImpl_ {};
    AppDeviceHandler appDeviceHandlerImpl_ {};
    bool isInited = false;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // DISTRIBUTEDDATAFWK_ARK_COMMUNICATION_PROVIDER_H
