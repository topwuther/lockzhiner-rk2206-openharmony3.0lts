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

#ifndef OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_PROXY_H
#define OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_PROXY_H

#include "ability_connect_callback_interface.h"

#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedSchedule {
class AbilityConnectionWrapperProxy : public IRemoteProxy<AAFwk::IAbilityConnection> {
public:
    explicit AbilityConnectionWrapperProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<AAFwk::IAbilityConnection>(impl) {}
    virtual ~AbilityConnectionWrapperProxy() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject,
        int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int32_t resultCode) override;

private:
    static inline BrokerDelegator<AbilityConnectionWrapperProxy> delegator_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_CONNECTION_WRAPPER_PROXY_H