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

#ifndef OHOS_DISTRIBUTED_SCHED_ADAPTER_H
#define OHOS_DISTRIBUTED_SCHED_ADAPTER_H

#include "ability_info.h"
#include "ability_manager_client.h"
#include "caller_info.h"
#include "event_handler.h"
#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "single_instance.h"
#include "uri.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedSchedAdapter {
    DECLARE_SINGLE_INSTANCE(DistributedSchedAdapter);

public:
    void Init();
    void UnInit();

    int32_t ConnectAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const sptr<IRemoteObject>& callerToken);
    int32_t DisconnectAbility(const sptr<IRemoteObject>& connect);
    void DeviceOnline(const std::string& deviceId);
    void DeviceOffline(const std::string& deviceId);
    bool QueryAbilityInfo(const OHOS::AAFwk::Want& want, AppExecFwk::AbilityInfo& abilityInfo);
    void ProcessConnectDied(const sptr<IRemoteObject>& connect);
    int32_t GetBundleNameListFromBms(int32_t uid, std::vector<std::u16string>& u16BundleNameList);
    int32_t GetBundleNameListFromBms(int32_t uid, std::vector<std::string>& bundleNameList);

private:
    void ProcessDeviceOffline(const std::string& deviceId);

    std::shared_ptr<AppExecFwk::EventHandler> dmsAdapterHandler_;
    friend class BundleManagerInternal;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_ADAPTER_H