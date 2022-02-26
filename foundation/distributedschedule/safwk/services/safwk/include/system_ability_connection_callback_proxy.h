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

#ifndef SYSTEM_ABILITY_CONNECTION_CALLBACK_PROXY_H_
#define SYSTEM_ABILITY_CONNECTION_CALLBACK_PROXY_H_

#include "if_system_ability_connection_callback.h"
#include "iremote_proxy.h"
#include "ipc_skeleton.h"

namespace OHOS {
class SystemAbilityConnectionCallbackProxy : public IRemoteProxy<ISystemAbilityConnectionCallback> {
public:
    explicit SystemAbilityConnectionCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityConnectionCallback>(impl) {}
    ~SystemAbilityConnectionCallbackProxy() = default;

    virtual void OnConnectedSystemAbility(const sptr<IRemoteObject>& connectionCallback);

    virtual void OnDisConnectedSystemAbility(int32_t systemAbilityId);

private:
    static inline BrokerDelegator<SystemAbilityConnectionCallbackProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(SYSTEM_ABILITY_CONNECTION_CALLBACK_PROXY_H_)