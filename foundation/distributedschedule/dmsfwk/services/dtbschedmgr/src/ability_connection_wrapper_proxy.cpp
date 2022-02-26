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

#include "ability_connection_wrapper_proxy.h"

#include "dtbschedmgr_log.h"
#include "ipc_object_proxy.h"
#include "ipc_types.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.abilityshell.DistributedConnection";
}

void AbilityConnectionWrapperProxy::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    HILOGD("AbilityConnectionWrapperProxy::OnAbilityConnectDone called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    PARCEL_WRITE_HELPER_NORET(data, Parcelable, &element);
    PARCEL_WRITE_HELPER_NORET(data, RemoteObject, remoteObject);
    PARCEL_WRITE_HELPER_NORET(data, Int32, resultCode);
    int32_t errCode = Remote()->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    HILOGD("AbilityConnectionWrapperProxy::OnAbilityConnectDone result %{public}d", errCode);
}

void AbilityConnectionWrapperProxy::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int32_t resultCode)
{
    HILOGD("AbilityConnectionWrapperProxy::OnAbilityDisconnectDone called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    PARCEL_WRITE_HELPER_NORET(data, Parcelable, &element);
    PARCEL_WRITE_HELPER_NORET(data, Int32, resultCode);
    int32_t errCode = Remote()->SendRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
    HILOGD("AbilityConnectionWrapperProxy::OnAbilityDisconnectDone result %{public}d", errCode);
}
} // namespace DistributedSchedule
} // namespace OHOS