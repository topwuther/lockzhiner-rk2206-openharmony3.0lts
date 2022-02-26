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

#include "ability_connection_wrapper_stub.h"

#include "ability_connection_wrapper_proxy.h"
#include "dtbschedmgr_log.h"
#include "ipc_types.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;

AbilityConnectionWrapperStub::AbilityConnectionWrapperStub(sptr<IRemoteObject> connection)
{
    distributedConnection_ = connection;
}

int32_t AbilityConnectionWrapperStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGD("AbilityConnectionWrapperStub::OnRemoteRequest code = %{public}d", code);
    std::u16string descriptor = IAbilityConnection::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("AbilityConnectionWrapperStub local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    sptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOGE("AbilityConnectionWrapperStub element is null");
        return ERR_INVALID_VALUE;
    }
    int32_t resultCode = ERR_NONE;
    switch (code) {
        case IAbilityConnection::ON_ABILITY_CONNECT_DONE: {
            if (auto remoteObject = data.ReadParcelable<IRemoteObject>()) {
                resultCode = data.ReadInt32();
                OnAbilityConnectDone(*element, remoteObject, resultCode);
                return ERR_NONE;
            }
            HILOGE("AbilityConnectionWrapperStub remoteObject is null");
            return ERR_INVALID_DATA;
        }
        case IAbilityConnection::ON_ABILITY_DISCONNECT_DONE: {
            resultCode = data.ReadInt32();
            OnAbilityDisconnectDone(*element, resultCode);
            return ERR_NONE;
        }
        default: {
            HILOGE("AbilityConnectionWrapperStub unknown code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AbilityConnectionWrapperStub::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    auto proxy = std::make_unique<AbilityConnectionWrapperProxy>(distributedConnection_);
    proxy->OnAbilityConnectDone(element, remoteObject, resultCode);
}

void AbilityConnectionWrapperStub::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
    auto proxy = std::make_unique<AbilityConnectionWrapperProxy>(distributedConnection_);
    proxy->OnAbilityDisconnectDone(element, resultCode);
}
} // namespace DistributedSchedule
} // namespace OHOS