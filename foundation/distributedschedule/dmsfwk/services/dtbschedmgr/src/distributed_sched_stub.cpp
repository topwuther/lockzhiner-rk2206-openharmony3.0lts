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

#include "distributed_sched_stub.h"

#include "ability_info.h"
#include "caller_info.h"
#include "distributed_sched_ability_shell.h"
#include "dtbschedmgr_log.h"

#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;
using namespace AppExecFwk;

namespace {
constexpr int32_t HID_HAP = 10000; /* first hap user */
const std::u16string DMS_STUB_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}

DistributedSchedStub::DistributedSchedStub()
{
    localFuncsMap_[START_REMOTE_ABILITY] = &DistributedSchedStub::StartRemoteAbilityInner;
    localFuncsMap_[START_CONTINUATION] = &DistributedSchedStub::StartContinuationInner;
    localFuncsMap_[NOTIFY_COMPLETE_CONTINUATION] = &DistributedSchedStub::NotifyCompleteContinuationInner;
    localFuncsMap_[REGISTER_ABILITY_TOKEN] = &DistributedSchedStub::RegisterAbilityTokenInner;
    localFuncsMap_[UNREGISTER_ABILITY_TOKEN] = &DistributedSchedStub::UnregisterAbilityTokenInner;
    localFuncsMap_[CONNECT_REMOTE_ABILITY] = &DistributedSchedStub::ConnectRemoteAbilityInner;
    localFuncsMap_[DISCONNECT_REMOTE_ABILITY] = &DistributedSchedStub::DisconnectRemoteAbilityInner;

    remoteFuncsMap_[START_ABILITY_FROM_REMOTE] = &DistributedSchedStub::StartAbilityFromRemoteInner;
    remoteFuncsMap_[NOTIFY_CONTINUATION_RESULT_FROM_REMOTE] =
        &DistributedSchedStub::NotifyContinuationResultFromRemoteInner;
    remoteFuncsMap_[CONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::ConnectAbilityFromRemoteInner;
    remoteFuncsMap_[DISCONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::DisconnectAbilityFromRemoteInner;
    remoteFuncsMap_[NOTIFY_PROCESS_DIED_FROM_REMOTE] = &DistributedSchedStub::NotifyProcessDiedFromRemoteInner;
}

DistributedSchedStub::~DistributedSchedStub()
{
    remoteFuncsMap_.clear();
    localFuncsMap_.clear();
}

int32_t DistributedSchedStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    bool IsLocalCalling = IPCSkeleton::IsLocalCalling();
    HILOGI("DistributedSchedStub::OnRemoteRequest, code = %{public}d, flags = %{public}d, IsLocalCalling = %{public}d.",
        code, option.GetFlags(), IsLocalCalling);

    const auto& funcsMap = IsLocalCalling ? localFuncsMap_ : remoteFuncsMap_;
    auto iter = funcsMap.find(code);
    if (iter != funcsMap.end()) {
        auto func = iter->second;
        if (!EnforceInterfaceToken(data)) {
            HILOGW("DistributedSchedStub::OnRemoteRequest interface token check failed!");
            return DMS_PERMISSION_DENIED;
        }
        if (func != nullptr) {
            return (this->*func)(data, reply);
        }
    }

    HILOGW("DistributedSchedStub::OnRemoteRequest default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DistributedSchedStub::StartRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("DistributedSchedStub:: START_ABILITY want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<AbilityInfo> spAbilityInfo(data.ReadParcelable<AbilityInfo>());
    if (spAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub: StartRemoteAbilityInner AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    int32_t result = StartRemoteAbility(*want, *spAbilityInfo, requestCode);
    HILOGI("DistributedSchedStub:: StartRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("DistributedSchedStub:: StartAbilityFromRemoteInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<CompatibleAbilityInfo> cmpAbilityInfo(data.ReadParcelable<CompatibleAbilityInfo>());
    if (cmpAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub: StartAbilityFromRemoteInner AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    AbilityInfo abilityInfo;
    cmpAbilityInfo->ConvertToAbilityInfo(abilityInfo);
    std::string package = abilityInfo.bundleName;
    std::string deviceId = abilityInfo.deviceId;
    int64_t begin = GetTickCount();
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    AccountInfo accountInfo;
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    int32_t result = StartAbilityFromRemote(*want, abilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("DistributedSchedStub:: StartAbilityFromRemoteInner result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    PARCEL_WRITE_HELPER(reply, String, package);
    PARCEL_WRITE_HELPER(reply, String, deviceId);
    return ERR_NONE;
}

int32_t DistributedSchedStub::StartContinuationInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("DistributedSchedStub: StartContinuationInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<AbilityInfo> spAbilityInfo(data.ReadParcelable<AbilityInfo>());
    if (spAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub: StartContinuationInner AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    int32_t result = StartContinuation(*want, *spAbilityInfo, abilityToken);
    HILOGI("DistributedSchedStub: StartContinuationInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::NotifyCompleteContinuationInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGE("DistributedSchedStub: NotifyCompleteContinuationInner devId empty!");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t sessionId = 0;
    PARCEL_READ_HELPER(data, Int32, sessionId);
    bool continuationResult = false;
    PARCEL_READ_HELPER(data, Bool, continuationResult);
    NotifyCompleteContinuation(devId, sessionId, continuationResult);
    return ERR_OK;
}

int32_t DistributedSchedStub::NotifyContinuationResultFromRemoteInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("DistributedSchedStub: NotifyContinuationResultFromRemoteInner request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    int32_t sessionId = 0;
    PARCEL_READ_HELPER(data, Int32, sessionId);
    bool continuationResult = false;
    PARCEL_READ_HELPER(data, Bool, continuationResult);
    return NotifyContinuationResultFromRemote(sessionId, continuationResult);
}

int32_t DistributedSchedStub::RegisterAbilityTokenInner(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    sptr<IRemoteObject> continuationCallback = data.ReadRemoteObject();
    int32_t result = RegisterAbilityToken(abilityToken, continuationCallback);
    HILOGI("DistributedSchedStub: RegisterAbilityTokenInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::UnregisterAbilityTokenInner(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    sptr<IRemoteObject> continuationCallback = data.ReadRemoteObject();
    int32_t result = UnregisterAbilityToken(abilityToken, continuationCallback);
    HILOGI("DistributedSchedStub: UnregisterAbilityTokenInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::ConnectRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("DistributedSchedStub::ConnectRemoteAbilityInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<AppExecFwk::AbilityInfo> abilityInfo(data.ReadParcelable<AppExecFwk::AbilityInfo>());
    if (abilityInfo == nullptr) {
        HILOGW("DistributedSchedStub::ConnectRemoteAbilityInner abilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t result = ConnectRemoteAbility(*want, *abilityInfo, connect);
    HILOGI("DistributedSchedStub::ConnectRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::DisconnectRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t result = DisconnectRemoteAbility(connect);
    HILOGI("DistributedSchedStub::DisconnectRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::ConnectAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("DistributedSchedStub::ConnectAbilityFromRemoteInner request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("DistributedSchedStub::ConnectAbilityFromRemoteInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<CompatibleAbilityInfo> cmpAbilityInfo(data.ReadParcelable<CompatibleAbilityInfo>());
    if (cmpAbilityInfo == nullptr) {
        HILOGW("DistributedSchedStub::ConnectAbilityFromRemoteInner abilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    AbilityInfo abilityInfo;
    cmpAbilityInfo->ConvertToAbilityInfo(abilityInfo);
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, Int32, callerInfo.pid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    AccountInfo accountInfo;
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    std::string package = abilityInfo.bundleName;
    std::string deviceId = abilityInfo.deviceId;
    int64_t begin = GetTickCount();
    int32_t result = ConnectAbilityFromRemote(*want, abilityInfo, connect, callerInfo, accountInfo);
    HILOGW("DistributedSchedStub::ConnectAbilityFromRemoteInner result = %{public}d", result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int32, result);
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    PARCEL_WRITE_HELPER(reply, String, package);
    PARCEL_WRITE_HELPER(reply, String, deviceId);
    return ERR_NONE;
}

int32_t DistributedSchedStub::DisconnectAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("DistributedSchedStub::DisconnectAbilityFromRemoteInner request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t uid = 0;
    PARCEL_READ_HELPER(data, Int32, uid);
    string sourceDeviceId;
    PARCEL_READ_HELPER(data, String, sourceDeviceId);
    int32_t result = DisconnectAbilityFromRemote(connect, uid, sourceDeviceId);
    HILOGI("DistributedSchedStub::DisconnectAbilityFromRemoteInner result %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::NotifyProcessDiedFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("DistributedSchedStub::NotifyProcessDiedFromRemoteInner request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    int32_t uid = 0;
    PARCEL_READ_HELPER(data, Int32, uid);
    int32_t pid = 0;
    PARCEL_READ_HELPER(data, Int32, pid);
    string sourceDeviceId;
    PARCEL_READ_HELPER(data, String, sourceDeviceId);
    CallerInfo callerInfo;
    callerInfo.uid = uid;
    callerInfo.pid = pid;
    callerInfo.sourceDeviceId = sourceDeviceId;
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    int32_t result = NotifyProcessDiedFromRemote(callerInfo);
    HILOGI("DistributedSchedStub::NotifyProcessDiedFromRemoteInner result %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

bool DistributedSchedStub::CheckCallingUid()
{
    // never allow non-system uid for distributed request
    auto callingUid = IPCSkeleton::GetCallingUid();
    return callingUid < HID_HAP;
}

bool DistributedSchedStub::EnforceInterfaceToken(MessageParcel& data)
{
    u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == DMS_STUB_INTERFACE_TOKEN;
}
} // namespace DistributedSchedule
} // namespace OHOS
