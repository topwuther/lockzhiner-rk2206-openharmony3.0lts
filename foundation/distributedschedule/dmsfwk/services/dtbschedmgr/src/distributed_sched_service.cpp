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

#include "distributed_sched_service.h"

#include <cinttypes>
#include <unistd.h>

#include "ability_connection_wrapper_stub.h"
#include "adapter/dnetwork_adapter.h"
#include "bundle/bundle_manager_internal.h"
#include "connect_death_recipient.h"
#include "distributed_sched_adapter.h"
#include "distributed_sched_ability_shell.h"
#include "distributed_sched_permission.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "distributed_sched_dumper.h"

#include "ability_manager_client.h"
#include "datetime_ex.h"
#include "element_name.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parcel_helper.h"
#include "string_ex.h"
#include "file_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
using namespace AppExecFwk;

namespace {
const std::u16string CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.abilityshell.DistributedConnection";

constexpr int32_t BIND_CONNECT_RETRY_TIMES = 3;
constexpr int32_t BIND_CONNECT_TIMEOUT = 500; // 500ms
constexpr int32_t MAX_DISTRIBUTED_CONNECT_NUM = 600;
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t INVALID_CALLER_UID = -1;
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedService);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&DistributedSchedService::GetInstance());

DistributedSchedService::DistributedSchedService() : SystemAbility(DISTRIBUTED_SCHED_SA_ID, true)
{
}

void DistributedSchedService::OnStart()
{
    if (!Init()) {
        HILOGE("failed to init DistributedSchedService");
        return;
    }
    FuncContinuationCallback continuationCallback = [this] (const sptr<IRemoteObject>& abilityToken) {
        HILOGW("continuationCallback timeout.");
        NotifyContinuationCallbackResult(abilityToken, CONTINUE_ABILITY_TIMEOUT_ERR);
    };
    dschedContinuation_ = std::make_shared<DSchedContinuation>();
    dschedContinuation_->Init(continuationCallback);
    HILOGI("DistributedSchedService::OnStart start service success.");
}

bool DistributedSchedService::Init()
{
    HILOGD("DistributedSchedService::Init ready to init.");

    bool ret = Publish(this);
    if (!ret) {
        HILOGE("DistributedSchedService::Init Publish failed!");
        return false;
    }

    if (!DtbschedmgrDeviceInfoStorage::GetInstance().Init()) {
        HILOGE("DistributedSchedService::Init DtbschedmgrDeviceInfoStorage init failed.");
    }
    HILOGD("DistributedSchedService::Init init success.");
    DistributedSchedAdapter::GetInstance().Init();
    DnetworkAdapter::GetInstance()->Init();
    connectDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new ConnectDeathRecipient());
    return true;
}

void DistributedSchedService::OnStop()
{
    HILOGD("DistributedSchedService::OnStop ready to stop service.");
}

int32_t DistributedSchedService::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode)
{
    std::string localDeviceId;
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("StartRemoteAbility check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("StartRemoteAbility DMS get remoteDms failed");
        return INVALID_PARAMETERS_ERR;
    }
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = localDeviceId;
    AccountInfo accountInfo;
    HILOGI("[PerformanceTest] DistributedSchedService StartRemoteAbility transact begin");
    int32_t result = remoteDms->StartAbilityFromRemote(want, abilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("[PerformanceTest] DistributedSchedService StartRemoteAbility transact end");
    return result;
}

int32_t DistributedSchedService::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    std::string localDeviceId;
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, deviceId, callerInfo.sourceDeviceId)) {
        HILOGE("StartAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    ErrCode err = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, abilityInfo, deviceId);
    if (err != ERR_OK) {
        HILOGE("StartAbilityFromRemote CheckDPermission denied!!");
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("StartAbilityFromRemote connect ability server failed %{public}d", err);
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode);
    if (err != ERR_OK) {
        HILOGE("StartAbilityFromRemote is failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::StartContinuation(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& abilityToken)
{
    HILOGD("[PerformanceTest] DistributedSchedService StartContinuation begin");
    if (abilityToken == nullptr) {
        HILOGE("StartContinuation abilityToken is null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    auto flags = want.GetFlags();
    if ((flags & AAFwk::Want::FLAG_ABILITY_CONTINUATION) == 0) {
        HILOGE("StartContinuation want continuation flags invalid!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    std::string devId;
    if (!GetLocalDeviceId(devId)) {
        HILOGE("StartContinuation get local deviceId failed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    if (dschedContinuation_ == nullptr) {
        HILOGE("StartContinuation continuation object null!");
        return INVALID_PARAMETERS_ERR;
    }

    int32_t sessionId = dschedContinuation_->GenerateSessionId();
    AAFwk::Want newWant = want;
    newWant.SetParam("sessionId", sessionId);
    newWant.SetParam("deviceId", devId);
    int32_t result = ERR_OK;
    result = StartRemoteAbility(newWant, abilityInfo, 0);
    if (result != ERR_OK) {
        HILOGE("DistributedSchedService:continue ability failed, errorCode = %{public}d", result);
        return result;
    }

    bool ret = dschedContinuation_->PushAbilityToken(sessionId, abilityToken);
    if (!ret) {
        HILOGW("StartContinuation PushAbilityToken failed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    HILOGD("[PerformanceTest] DistributedSchedService StartContinuation end");
    return result;
}

void DistributedSchedService::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
    if (!isSuccess) {
        HILOGE("NotifyCompleteContinuation failed!");
    }
    if (sessionId <= 0) {
        HILOGE("NotifyCompleteContinuation sessionId invalid!");
        return;
    }
    std::string deviceId = Str16ToStr8(devId);
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("NotifyCompleteContinuation get remote dms null!");
        return;
    }
    remoteDms->NotifyContinuationResultFromRemote(sessionId, isSuccess);
}

int32_t DistributedSchedService::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    if (sessionId <= 0) {
        HILOGE("NotifyContinuationResultFromRemote sessionId:%{public}d invalid!", sessionId);
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    if (dschedContinuation_ == nullptr) {
        HILOGE("NotifyContinuationResultFromRemote continuation object null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    auto abilityToken = dschedContinuation_->PopAbilityToken(sessionId);
    if (abilityToken == nullptr) {
        HILOGE("DSchedContinuationCallback NotifyContinuationResultFromRemote abilityToken null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    NotifyContinuationCallbackResult(abilityToken, isSuccess ? 0 : NOTIFYCOMPLETECONTINUATION_FAILED);
    return ERR_OK;
}

void DistributedSchedService::NotifyContinuationCallbackResult(const sptr<IRemoteObject>& abilityToken,
    int32_t isSuccess)
{
    if (isSuccess != ERR_OK) {
        HILOGE("NotifyContinuationCallbackResult failed!");
    }
    HILOGD("NotifyContinuationCallbackResult ContinuationRet result:%{public}d", isSuccess);
    if (abilityToken == nullptr) {
        HILOGE("NotifyContinuationCallbackResult abilityToken null!");
        return;
    }

    int32_t result = DistributedSchedAbilityShell::GetInstance().ScheduleCompleteContinuation(
        abilityToken, isSuccess);
    HILOGD("NotifyContinuationCallbackResult ScheduleCompleteContinuation result:%{public}d", result);
}

int32_t DistributedSchedService::RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return DistributedSchedAbilityShell::GetInstance().RegisterAbilityToken(abilityToken, continuationCallback);
}

int32_t DistributedSchedService::UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return DistributedSchedAbilityShell::GetInstance().UnregisterAbilityToken(abilityToken, continuationCallback);
}

void DistributedSchedService::RemoteConnectAbilityMappingLocked(const sptr<IRemoteObject>& connect,
    const std::string& localDeviceId, const std::string& remoteDeviceId, const AppExecFwk::ElementName& element,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
{
    if (connect == nullptr) {
        return;
    }
    auto itConnect = distributedConnectAbilityMap_.find(connect);
    if (itConnect == distributedConnectAbilityMap_.end()) {
        // add uid's connect number
        uint32_t number = ++trackingUidMap_[callerInfo.uid];
        HILOGD("uid %d has %u connection(s), targetComponent:%d", callerInfo.uid, number, targetComponent);
        // new connect, add death recipient
        connect->AddDeathRecipient(connectDeathRecipient_);
    }
    auto& sessionsList = distributedConnectAbilityMap_[connect];
    for (auto& session : sessionsList) {
        if (remoteDeviceId == session.GetDestinationDeviceId()) {
            session.AddElement(element);
            // already added session for remote device
            return;
        }
    }
    // connect to another remote device, add a new session to list
    auto& session = sessionsList.emplace_back(localDeviceId, remoteDeviceId, callerInfo, targetComponent);
    session.AddElement(element);
    HILOGD("add connection success");
}

int32_t DistributedSchedService::CheckDistributedConnectLocked(const CallerInfo& callerInfo) const
{
    if (callerInfo.uid < 0) {
        HILOGE("uid %d is invalid", callerInfo.uid);
        return BIND_ABILITY_UID_INVALID_ERR;
    }

    if (callerInfo.uid == SYSTEM_UID) {
        return ERR_OK;
    }
    auto it = trackingUidMap_.find(callerInfo.uid);
    if (it != trackingUidMap_.end() && it->second >= MAX_DISTRIBUTED_CONNECT_NUM) {
        HILOGE("uid %{public}d connected too much abilities, it maybe leak", callerInfo.uid);
        return BIND_ABILITY_LEAK_ERR;
    }
    return ERR_OK;
}

void DistributedSchedService::DecreaseConnectLocked(int32_t uid)
{
    if (uid < 0) {
        HILOGE("DecreaseConnectLocked invalid uid %{public}d", uid);
        return;
    }
    auto it = trackingUidMap_.find(uid);
    if (it != trackingUidMap_.end()) {
        auto& conns = it->second;
        if (conns > 0) {
            conns--;
        }
        if (conns == 0) {
            HILOGD("DecreaseConnectLocked uid %{public}d connection(s) is 0", uid);
            trackingUidMap_.erase(it);
        }
    }
}

int32_t DistributedSchedService::GetUidLocked(const std::list<ConnectAbilitySession>& sessionsList)
{
    if (!sessionsList.empty()) {
        return sessionsList.front().GetCallerInfo().uid;
    }
    return INVALID_CALLER_UID;
}

int32_t DistributedSchedService::ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect)
{
    std::string localDeviceId;
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, remoteDeviceId)) {
        HILOGE("ConnectRemoteAbility check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }

    std::lock_guard<std::mutex> autoLock(distributedLock_);
    CallerInfo callerInfo = {
        IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid(), CALLER_TYPE_HARMONY, localDeviceId
    };
    int32_t checkResult = CheckDistributedConnectLocked(callerInfo);
    if (checkResult != ERR_OK) {
        return checkResult;
    }

    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("ConnectRemoteAbility GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = DistributedSchedAdapter::GetInstance().GetBundleNameListFromBms(
        callerInfo.uid, callerInfo.bundleNames);
    if (ret != ERR_OK) {
        HILOGE("ConnectRemoteAbility GetBundleNameListFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }

    HILOGD("[PerformanceTest] DistributedSchedService::ConnectRemoteAbility begin");
    int32_t result = TryConnectRemoteAbility(want, abilityInfo, connect, callerInfo);
    if (result != ERR_OK) {
        HILOGE("ConnectRemoteAbility result is %{public}d", result);
    }
    HILOGD("[PerformanceTest] DistributedSchedService::ConnectRemoteAbility end");
    return result;
}

int32_t DistributedSchedService::TryConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo)
{
    AccountInfo accountInfo;
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr || connect == nullptr) {
        HILOGE("TryConnectRemoteAbility invalid parameters");
        return INVALID_PARAMETERS_ERR;
    }

    int32_t retryTimes = BIND_CONNECT_RETRY_TIMES;
    int32_t result = REMOTE_DEVICE_BIND_ABILITY_ERR;
    while (retryTimes--) {
        int64_t start = GetTickCount();
        HILOGD("[PerformanceTest] ConnectRemoteAbility begin");
        result = remoteDms->ConnectAbilityFromRemote(want, abilityInfo, connect, callerInfo, accountInfo);
        HILOGD("[PerformanceTest] ConnectRemoteAbility end");
        if (result == ERR_OK) {
            RemoteConnectAbilityMappingLocked(connect, callerInfo.sourceDeviceId, remoteDeviceId,
                want.GetElement(), callerInfo, TargetComponent::HARMONY_COMPONENT);
            break;
        }
        if (result == INVALID_REMOTE_PARAMETERS_ERR || result == REMOTE_DEVICE_BIND_ABILITY_ERR) {
            break;
        }
        int64_t elapsedTime = GetTickCount() - start;
        if (elapsedTime > BIND_CONNECT_TIMEOUT) {
            HILOGW("ConnectRemoteAbility timeout, elapsedTime is %{public}" PRId64 " ms", elapsedTime);
            break;
        }
    }
    return result;
}

sptr<IDistributedSched> DistributedSchedService::GetRemoteDms(const std::string& remoteDeviceId)
{
    if (remoteDeviceId.empty()) {
        HILOGE("GetRemoteDms remoteDeviceId is empty");
        return nullptr;
    }
    HILOGD("GetRemoteDms connect deviceid is %s", remoteDeviceId.c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("GetRemoteDms failed to connect to systemAbilityMgr!");
        return nullptr;
    }
    HILOGD("[PerformanceTest] DistributedSchedService GetRemoteDms begin");
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID, remoteDeviceId);
    HILOGD("[PerformanceTest] DistributedSchedService GetRemoteDms end");
    if (object == nullptr) {
        HILOGE("GetRemoteDms failed to get remote DistributedSched %{private}s", remoteDeviceId.c_str());
        return nullptr;
    }
    return iface_cast<IDistributedSched>(object);
}

bool DistributedSchedService::GetLocalDeviceId(std::string& localDeviceId)
{
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        HILOGE("GetLocalDeviceId failed");
        return false;
    }
    return true;
}

bool DistributedSchedService::CheckDeviceId(const std::string& localDeviceId, const std::string& remoteDeviceId)
{
    // remoteDeviceId must not same with localDeviceId
    if (localDeviceId.empty() || remoteDeviceId.empty() || localDeviceId == remoteDeviceId) {
        HILOGE("check deviceId failed");
        return false;
    }
    return true;
}

bool DistributedSchedService::CheckDeviceIdFromRemote(const std::string& localDeviceId,
    const std::string& destinationDeviceId, const std::string& sourceDeviceId)
{
    if (localDeviceId.empty() || destinationDeviceId.empty() || sourceDeviceId.empty()) {
        HILOGE("CheckDeviceIdFromRemote failed");
        return false;
    }
    // destinationDeviceId set by remote must be same with localDeviceId
    if (localDeviceId != destinationDeviceId) {
        HILOGE("destinationDeviceId is not same with localDeviceId");
        return false;
    }
    HILOGD("CheckDeviceIdFromRemote sourceDeviceId %s", sourceDeviceId.c_str());
    HILOGD("CheckDeviceIdFromRemote localDeviceId %s", localDeviceId.c_str());
    HILOGD("CheckDeviceIdFromRemote destinationDeviceId %s", destinationDeviceId.c_str());

    if (sourceDeviceId == destinationDeviceId || sourceDeviceId == localDeviceId) {
        HILOGE("destinationDeviceId is different with localDeviceId and destinationDeviceId");
        return false;
    }
    return true;
}

int32_t DistributedSchedService::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    HILOGD("[PerformanceTest] DistributedSchedService ConnectAbilityFromRemote begin");
    if (connect == nullptr) {
        HILOGE("ConnectAbilityFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    HILOGD("ConnectAbilityFromRemote uid is %d, pid is %d", callerInfo.uid, callerInfo.pid);
    std::string localDeviceId;
    std::string destinationDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, destinationDeviceId, callerInfo.sourceDeviceId)) {
        HILOGE("ConnectAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    int32_t result = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, abilityInfo, localDeviceId);
    if (result != ERR_OK) {
        HILOGE("ConnectAbilityFromRemote CheckDPermission denied!!");
        return result;
    }

    HILOGD("ConnectAbilityFromRemote callerType is %{public}d", callerInfo.callerType);
    sptr<IRemoteObject> callbackWrapper = connect;
    if (callerInfo.callerType == CALLER_TYPE_HARMONY) {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        auto itConnect = connectAbilityMap_.find(connect);
        if (itConnect != connectAbilityMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
        } else {
            callbackWrapper = new AbilityConnectionWrapperStub(connect);
            ConnectInfo connectInfo {callerInfo, callbackWrapper};
            connectAbilityMap_.emplace(connect, connectInfo);
        }
    }
    int32_t errCode = DistributedSchedAdapter::GetInstance().ConnectAbility(want, callbackWrapper, this);
    HILOGD("[PerformanceTest] DistributedSchedService ConnectAbilityFromRemote end");
    return errCode;
}

int32_t DistributedSchedService::DisconnectEachRemoteAbilityLocked(const std::string& localDeviceId,
    const std::string& remoteDeviceId, const sptr<IRemoteObject>& connect)
{
    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("DisconnectRemoteAbility get remote dms failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t result = remoteDms->DisconnectAbilityFromRemote(connect, IPCSkeleton::GetCallingUid(), localDeviceId);
    if (result != ERR_OK) {
        HILOGE("DisconnectEachRemoteAbilityLocked result is %{public}d", result);
    }
    return result;
}

int32_t DistributedSchedService::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("DisconnectRemoteAbility connect is null");
        return INVALID_PARAMETERS_ERR;
    }

    std::lock_guard<std::mutex> autoLock(distributedLock_);
    auto it = distributedConnectAbilityMap_.find(connect);
    if (it != distributedConnectAbilityMap_.end()) {
        std::list<ConnectAbilitySession>& sessionsList = it->second;
        int32_t uid = GetUidLocked(sessionsList);
        for (const auto& session : sessionsList) {
            if (session.GetTargetComponent() == TargetComponent::HARMONY_COMPONENT) {
                DisconnectEachRemoteAbilityLocked(session.GetSourceDeviceId(),
                    session.GetDestinationDeviceId(), connect);
            } else {
                HILOGW("DisconnectRemoteAbility non-harmony component");
            }
        }
        // also decrease number when erase connect
        DecreaseConnectLocked(uid);
        connect->RemoveDeathRecipient(connectDeathRecipient_);
        distributedConnectAbilityMap_.erase(it);
        HILOGI("remove connect sucess");
        return ERR_OK;
    }
    return NO_CONNECT_CALLBACK_ERR;
}

int32_t DistributedSchedService::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    if (connect == nullptr) {
        HILOGE("DisconnectAbilityFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    HILOGD("[PerformanceTest] DistributedSchedService::DisconnectAbilityFromRemote begin");
    std::string localDeviceId;
    AppExecFwk::AbilityInfo abilityInfo;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId.empty() ||
        sourceDeviceId.empty() || localDeviceId == sourceDeviceId) {
        HILOGE("DisconnectAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> callbackWrapper = connect;
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        auto itConnect = connectAbilityMap_.find(connect);
        if (itConnect != connectAbilityMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
            connectAbilityMap_.erase(itConnect);
        } else {
            if (!IPCSkeleton::IsLocalCalling()) {
                HILOGE("DisconnectAbilityFromRemote connect not found");
                return INVALID_REMOTE_PARAMETERS_ERR;
            }
        }
    }
    int32_t result = DistributedSchedAdapter::GetInstance().DisconnectAbility(callbackWrapper);
    HILOGD("[PerformanceTest] DistributedSchedService::DisconnectAbilityFromRemote end");
    return result;
}

int32_t DistributedSchedService::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    HILOGI("DistributedSchedService::NotifyProcessDiedFromRemote called");
    int32_t errCode = ERR_OK;
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        for (auto iter = connectAbilityMap_.begin(); iter != connectAbilityMap_.end();) {
            ConnectInfo& connectInfo = iter->second;
            if (callerInfo.sourceDeviceId == connectInfo.callerInfo.sourceDeviceId
                && callerInfo.uid == connectInfo.callerInfo.uid
                && callerInfo.pid == connectInfo.callerInfo.pid
                && callerInfo.callerType == connectInfo.callerInfo.callerType) {
                HILOGI("NotifyProcessDiedFromRemote erase connection success");
                int32_t ret = DistributedSchedAdapter::GetInstance().DisconnectAbility(connectInfo.callbackWrapper);
                if (ret != ERR_OK) {
                    errCode = ret;
                }
                connectAbilityMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }
    return errCode;
}

void DistributedSchedService::ProcessDeviceOffline(const std::string& deviceId)
{
    HILOGI("DistributedSchedService::ProcessDeviceOffline called");
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("ProcessDeviceOffline check deviceId failed");
        return;
    }

    std::lock_guard<std::mutex> autoLock(distributedLock_);
    for (auto itConnect = distributedConnectAbilityMap_.begin(); itConnect != distributedConnectAbilityMap_.end();) {
        std::list<ConnectAbilitySession>& sessionsList = itConnect->second;
        int32_t uid = GetUidLocked(sessionsList);
        auto itSession = std::find_if(sessionsList.begin(), sessionsList.end(), [&deviceId](const auto& session) {
            return session.GetDestinationDeviceId() == deviceId;
        });
        if (itSession != sessionsList.end()) {
            NotifyDeviceOfflineToAppLocked(itConnect->first, *itSession);
            CallerInfo callerInfo = itSession->GetCallerInfo();
            sessionsList.erase(itSession);
        }

        if (sessionsList.empty()) {
            if (itConnect->first != nullptr) {
                itConnect->first->RemoveDeathRecipient(connectDeathRecipient_);
            }
            DecreaseConnectLocked(uid);
            distributedConnectAbilityMap_.erase(itConnect++);
        } else {
            itConnect++;
        }
    }

    for (auto iter = connectAbilityMap_.begin(); iter != connectAbilityMap_.end();) {
        ConnectInfo& connectInfo = iter->second;
        if (deviceId == connectInfo.callerInfo.sourceDeviceId) {
            DistributedSchedAdapter::GetInstance().DisconnectAbility(connectInfo.callbackWrapper);
            connectAbilityMap_.erase(iter++);
            HILOGI("ProcessDeviceOffline erase connection success");
        } else {
            iter++;
        }
    }
}

void DistributedSchedService::NotifyDeviceOfflineToAppLocked(const sptr<IRemoteObject>& connect,
    const ConnectAbilitySession& session)
{
    std::list<AppExecFwk::ElementName> elementsList = session.GetElementsList();
    for (const auto& element : elementsList) {
        int32_t errCode = NotifyApp(connect, element, DEVICE_OFFLINE_ERR);
        if (errCode != ERR_NONE) {
            HILOGW("ProcessDeviceOffline notify failed, errCode = %{public}d", errCode);
        }
    }
}

int32_t DistributedSchedService::NotifyApp(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName& element, int32_t errCode)
{
    if (connect == nullptr) {
        return OBJECT_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &element);
    PARCEL_WRITE_HELPER(data, Int32, errCode);
    MessageParcel reply;
    MessageOption option;
    return connect->SendRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
}

void DistributedSchedService::ProcessConnectDied(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("ProcessConnectDied connect is null");
        return;
    }

    std::lock_guard<std::mutex> autoLock(distributedLock_);
    auto it = distributedConnectAbilityMap_.find(connect);
    if (it == distributedConnectAbilityMap_.end()) {
        return;
    }
    std::list<ConnectAbilitySession>& connectSessionsList = it->second;
    if (connectSessionsList.empty()) {
        return;
    }
    CallerInfo callerInfo = connectSessionsList.front().GetCallerInfo();
    std::set<std::string> processedDeviceSet;
    // to reduce the number of communications between devices, clean all the died process's connections
    for (auto itConnect = distributedConnectAbilityMap_.begin(); itConnect != distributedConnectAbilityMap_.end();) {
        std::list<ConnectAbilitySession>& sessionsList = itConnect->second;
        if (!sessionsList.empty() && sessionsList.front().IsSameCaller(callerInfo)) {
            for (const auto& session : sessionsList) {
                std::string remoteDeviceId = session.GetDestinationDeviceId();
                TargetComponent targetComponent = session.GetTargetComponent();
                // the same session can connect different types component on the same device
                std::string key = remoteDeviceId + std::to_string(static_cast<int32_t>(targetComponent));
                // just notify one time for same remote device
                auto [_, isSuccess] = processedDeviceSet.emplace(key);
                if (isSuccess) {
                    NotifyProcessDiedLocked(remoteDeviceId, callerInfo, targetComponent);
                }
            }
            DecreaseConnectLocked(callerInfo.uid);
            if (itConnect->first != nullptr) {
                itConnect->first->RemoveDeathRecipient(connectDeathRecipient_);
            }
            distributedConnectAbilityMap_.erase(itConnect++);
        } else {
            itConnect++;
        }
    }
}

void DistributedSchedService::NotifyProcessDiedLocked(const std::string& remoteDeviceId,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
{
    if (targetComponent != TargetComponent::HARMONY_COMPONENT) {
        HILOGD("NotifyProcessDiedLocked not harmony component, no need to notify");
        return;
    }

    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("NotifyProcessDiedLocked get remote dms failed");
        return;
    }
    int32_t result = remoteDms->NotifyProcessDiedFromRemote(callerInfo);
    HILOGI("NotifyProcessDiedLocked result is %{public}d", result);
}

ConnectAbilitySession::ConnectAbilitySession(const std::string& sourceDeviceId, const std::string& destinationDeviceId,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
    : sourceDeviceId_(sourceDeviceId),
      destinationDeviceId_(destinationDeviceId),
      callerInfo_(callerInfo),
      targetComponent_(targetComponent)
{
}

void ConnectAbilitySession::AddElement(const AppExecFwk::ElementName& element)
{
    for (const auto& elementName : elementsList_) {
        if (elementName == element) {
            return;
        }
    }
    elementsList_.emplace_back(element);
}

bool ConnectAbilitySession::IsSameCaller(const CallerInfo& callerInfo)
{
    return (callerInfo.uid == callerInfo_.uid &&
            callerInfo.pid == callerInfo_.pid &&
            callerInfo.sourceDeviceId == callerInfo_.sourceDeviceId &&
            callerInfo.callerType == callerInfo_.callerType);
}

int32_t DistributedSchedService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> argsInStr8;
    for (const auto& arg : args) {
        argsInStr8.emplace_back(Str16ToStr8(arg));
    }

    std::string result;
    DistributedSchedDumper::Dump(argsInStr8, result);

    if (!SaveStringToFd(fd, result)) {
        HILOGE("save to fd failed");
        return DMS_WRITE_FILE_FAILED_ERR;
    }
    return ERR_OK;
}

void DistributedSchedService::DumpConnectInfo(std::string& info)
{
    std::lock_guard<std::mutex> autoLock(distributedLock_);
    info += "connected remote abilities:\n";
    if (!distributedConnectAbilityMap_.empty()) {
        for (const auto& distributedConnect : distributedConnectAbilityMap_) {
            const std::list<ConnectAbilitySession> sessionsList = distributedConnect.second;
            DumpSessionsLocked(sessionsList, info);
        }
    } else {
        info += "  <none info>\n";
    }
}

void DistributedSchedService::DumpSessionsLocked(const std::list<ConnectAbilitySession>& sessionsList,
    std::string& info)
{
    for (const auto& session : sessionsList) {
        info += "  ";
        info += "SourceDeviceId: ";
        info += session.GetSourceDeviceId();
        info += ", ";
        info += "DestinationDeviceId: ";
        info += session.GetDestinationDeviceId();
        info += ", ";
        info += "CallerUid: ";
        info += std::to_string(session.GetCallerInfo().uid);
        info += ", ";
        info += "CallerPid: ";
        info += std::to_string(session.GetCallerInfo().pid);
        info += ", ";
        info += "CallerType: ";
        info += std::to_string(session.GetCallerInfo().callerType);
        DumpElementLocked(session.GetElementsList(), info);
        info += "\n";
    }
}

void DistributedSchedService::DumpElementLocked(const std::list<AppExecFwk::ElementName>& elementsList,
    std::string& info)
{
    for (const auto& element : elementsList) {
        info += ", ";
        info += "BundleName: ";
        info += element.GetBundleName();
        info += ", ";
        info += "AbilityName: ";
        info += element.GetAbilityName();
    }
}
} // namespace DistributedSchedule
} // namespace OHOS