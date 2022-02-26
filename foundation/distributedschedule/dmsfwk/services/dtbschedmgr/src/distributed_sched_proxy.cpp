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

#include "distributed_sched_proxy.h"

#include "dtbschedmgr_log.h"

#include "ipc_types.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;

namespace {
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}

int32_t DistributedSchedProxy::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Parcelable, &abilityInfo);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    AppExecFwk::CompatibleAbilityInfo compatibleAbilityInfo;
    abilityInfo.ConvertToCompatiableAbilityInfo(compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Parcelable, &compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, accountInfo.accountType);
    PARCEL_WRITE_HELPER(data, StringVector, accountInfo.groupIdList);
    PARCEL_WRITE_HELPER(data, String, callerInfo.callerAppId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::StartContinuation(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr) {
        HILOGE("StartContinuation abilityToken null!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("StartContinuation remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Parcelable, &abilityInfo);
    PARCEL_WRITE_HELPER(data, RemoteObject, abilityToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_CONTINUATION, data, reply);
}

void DistributedSchedProxy::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyCompleteContinuation remote service null");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return;
    }
    PARCEL_WRITE_HELPER_NORET(data, String16, devId);
    PARCEL_WRITE_HELPER_NORET(data, Int32, sessionId);
    PARCEL_WRITE_HELPER_NORET(data, Bool, isSuccess);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_NORET(remote, NOTIFY_COMPLETE_CONTINUATION, data, reply);
}

int32_t DistributedSchedProxy::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyContinuationResultFromRemote remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, sessionId);
    PARCEL_WRITE_HELPER(data, Bool, isSuccess);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, NOTIFY_CONTINUATION_RESULT_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    if (abilityToken == nullptr || continuationCallback == nullptr) {
        HILOGE("RegisterAbilityToken paramter null!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("RegisterAbilityToken remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, abilityToken);
    PARCEL_WRITE_HELPER(data, RemoteObject, continuationCallback);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_ABILITY_TOKEN, data, reply);
}

int32_t DistributedSchedProxy::UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    if (abilityToken == nullptr || continuationCallback == nullptr) {
        HILOGE("UnregisterAbilityToken paramter null!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("UnregisterAbilityToken remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, abilityToken);
    PARCEL_WRITE_HELPER(data, RemoteObject, continuationCallback);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UNREGISTER_ABILITY_TOKEN, data, reply);
}

int32_t DistributedSchedProxy::ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("ConnectRemoteAbility connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ConnectRemoteAbility remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Parcelable, &abilityInfo);
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("DisconnectRemoteAbility connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("DisconnectRemoteAbility remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DISCONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    if (connect == nullptr) {
        HILOGE("ConnectAbilityFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ConnectAbilityFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    AppExecFwk::CompatibleAbilityInfo compatibleAbilityInfo;
    abilityInfo.ConvertToCompatiableAbilityInfo(compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Parcelable, &compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, accountInfo.accountType);
    PARCEL_WRITE_HELPER(data, StringVector, accountInfo.groupIdList);
    PARCEL_WRITE_HELPER(data, String, callerInfo.callerAppId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    if (connect == nullptr) {
        HILOGE("DisconnectAbilityFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("DisconnectAbilityFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, uid);
    PARCEL_WRITE_HELPER(data, String, sourceDeviceId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DISCONNECT_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    HILOGD("DistributedSchedProxy::NotifyProcessDiedFromRemote called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyProcessDiedFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, NOTIFY_PROCESS_DIED_FROM_REMOTE, data, reply);
}
} // namespace DistributedSchedule
} // namespace OHOS

