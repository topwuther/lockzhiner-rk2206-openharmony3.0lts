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

#include "distributed_permission_manager_service.h"
#include "permission_log.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "soft_bus_manager.h"
#include "object_device_permission_manager.h"
#include "subject_device_permission_manager.h"
#include "bundle_mgr_interface.h"
#include "permission_kit.h"
#include "init_log.h"
#include "iservice_registry.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionManagerService"};
}
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DistributedPermissionManagerService>::GetInstance().get());

DistributedPermissionManagerService::DistributedPermissionManagerService()
    : SystemAbility(SA_ID_DISTRIBUTED_PERMISSION_MANAGER_SERVICE, true),
      serviceRunningState_(ServiceRunningState::STATE_NOT_START),
      runner_(nullptr)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
}

DistributedPermissionManagerService::~DistributedPermissionManagerService()
{
    PERMISSION_LOG_INFO(LABEL, "exit");
}

bool DistributedPermissionManagerService::Init()
{
    runner_ = AppExecFwk::EventRunner::Create(true);
    if (!runner_) {
        PERMISSION_LOG_ERROR(LABEL, "failed to create a runner.");
        return false;
    }

    handler_ = std::make_shared<DistributedPermissionEventHandler>(runner_);
    if (!handler_) {
        PERMISSION_LOG_ERROR(LABEL, "handler_ is nullpter.");
        return false;
    }

    // init
    ObjectDevicePermissionManager::GetInstance().Init();
    PermissionBmsManager::GetInstance().Init();
    RemoteCommandManager::GetInstance().Init();
    SoftBusManager::GetInstance().Initialize();

    return true;
}

void DistributedPermissionManagerService::OnStart()
{
    INIT_LOGI("DistributedPermissionManagerService::OnStart()");
    int32_t intFlag = 2;
    int32_t sleepTime = 50;
    while (true) {
        int32_t flag = 0;
        std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
        if (externalDeps != nullptr) {
            sptr<AppExecFwk::IBundleMgr> iBundleManager;
            iBundleManager = externalDeps->GetBundleManager(iBundleManager);
            if (iBundleManager != nullptr) {
                INIT_LOGI("iBundleManager is not nullptr.");
                flag++;
            } else {
                INIT_LOGI("iBundleManager is nullptr.");
            }
            auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (samgr == nullptr) {
                INIT_LOGI("Get SystemAbilityManager Failed");
            }
            auto object = samgr->CheckSystemAbility(Constant::DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
            if (object == nullptr) {
                INIT_LOGI("Get DeviceManager SystemAbility Failed");
            } else {
                INIT_LOGI("iDeviceManager is not nullptr.");
                flag++;
            }
        }
        if (flag == intFlag) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    if (serviceRunningState_ == ServiceRunningState::STATE_RUNNING) {
        PERMISSION_LOG_ERROR(LABEL, "Distributed Manager Service has already started.");
        return;
    }

    PERMISSION_LOG_INFO(LABEL, "start");

    if (!Init()) {
        PERMISSION_LOG_ERROR(LABEL, "failed to Init service.");
        return;
    }

    runner_->Run();

    if (!Publish(this)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to publish the service.");
        return;
    }

    serviceRunningState_ = ServiceRunningState::STATE_RUNNING;
}

void DistributedPermissionManagerService::OnStop()
{
    PERMISSION_LOG_INFO(LABEL, "stop");

    serviceRunningState_ = ServiceRunningState::STATE_NOT_START;
}

int32_t DistributedPermissionManagerService::AllocateDuid(const std::string &nodeId, const int32_t rUid)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, nodeId: %{public}s, rUid: %{public}d",
        __func__,
        Constant::EncryptDevId(nodeId).c_str(),
        rUid);
    const std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return convert(Constant::INVALID_DEVICE_ID);
    }
    int32_t duid = SubjectDevicePermissionManager::GetInstance().GetDistributedUid(deviceId, rUid);
    if (DistributedDataValidator::IsDuidValid(duid)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}d: returned: ", duid);
        return convert(duid);
    }

    duid = SubjectDevicePermissionManager::GetInstance().AllocateDistributedUid(deviceId, rUid);
    PERMISSION_LOG_INFO(LABEL, "%{public}d returned: ", duid);
    return convert(duid);
}

int32_t DistributedPermissionManagerService::QueryDuid(const std::string &nodeId, const int32_t rUid)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, nodeId: %{public}s, rUid: %{public}d",
        __func__,
        Constant::EncryptDevId(nodeId).c_str(),
        rUid);
    const std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: nodeId cannot convert to deviceId, failure.", __func__);
        return convert(Constant::INVALID_DEVICE_ID);
    }
    const int32_t duid = SubjectDevicePermissionManager::GetInstance().GetDistributedUid(deviceId, rUid);
    PERMISSION_LOG_INFO(LABEL, "%{public}d returned: ", duid);
    return convert(duid);
}

int32_t DistributedPermissionManagerService::WaitDuidReady(
    const std::string &nodeId, const int32_t rUid, const int32_t timeout)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, nodeId: %{public}s, rUid: %{public}d, timeout: %{public}d",
        __func__,
        Constant::EncryptDevId(nodeId).c_str(),
        rUid,
        timeout);

    const std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: nodeId cannot convert to deviceId, failure.", __func__);
        return Constant::INVALID_DEVICE_ID;
    }
    return SubjectDevicePermissionManager::GetInstance().WaitDuidReady(deviceId, rUid, timeout);
}

int32_t DistributedPermissionManagerService::NotifySyncPermission(
    std::string &nodeId, int uid, std::string &packageName)
{
    PERMISSION_LOG_INFO(LABEL,
        "nodeId = %{public}s, uid = %{public}d, packageName = %{public}s",
        Constant::EncryptDevId(nodeId).c_str(),
        uid,
        packageName.c_str());

    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_DEBUG(LABEL,
            "notifySyncPermission: nodeId [%{public}s] cannot convert to deviceId, failure.",
            Constant::EncryptDevId(nodeId).c_str());
        return Constant::INVALID_DEVICE_ID;
    }
    return ObjectDevicePermissionManager::GetInstance().NotifySyncPermission(deviceId, uid, packageName);
}

int32_t DistributedPermissionManagerService::CheckDPermission(int dUid, const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, dUid: %{public}d, permissionName: %{public}s",
        __func__,
        dUid,
        permissionName.c_str());
    int32_t checkResult =
        SubjectDevicePermissionManager::GetInstance().CheckDistributedPermission(dUid, permissionName);
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s returned: %{public}d", __func__, checkResult);
    return checkResult;
}
int32_t DistributedPermissionManagerService::CheckLocalPermission(int32_t uid, const std::string &permissionName)
{
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    if (iBundleManager_ == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: iBundleManager_ is nullptr.", __func__);
        return Constant::PERMISSION_DENIED;
    }
    std::vector<std::string> bundleNames;
    bool result = iBundleManager_->GetBundlesForUid(uid, bundleNames);
    if (!result) {
        PERMISSION_LOG_ERROR(LABEL, "cannot get bundle name by uid %{public}d", uid);
        return Constant::PERMISSION_DENIED;
    }
    for (auto bundleName : bundleNames) {
        int32_t iGranted =
            Permission::PermissionKit::VerifyPermission(bundleName, permissionName, Constant::DEFAULT_USERID);
        PERMISSION_LOG_INFO(LABEL, "iGranted : %{public}d", iGranted);
        if (iGranted == Constant::PERMISSION_DENIED) {
            return Constant::PERMISSION_DENIED;
        }
    }
    return Constant::PERMISSION_GRANTED;
}
int32_t DistributedPermissionManagerService::CheckPermission(
    const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s, pid = %{public}d, uid = %{public}d, ",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        pid,
        uid);
    if (!DistributedDataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "PermissionName data invalid");
        return Constant::PERMISSION_DENIED;
    }
    if (DistributedDataValidator::IsDuidValidAndNotSpecial(uid)) {
        return SubjectDevicePermissionManager::GetInstance().CheckDistributedPermission(uid, permissionName);
    }
    // If nodeId is null, check the local permission.
    if (nodeId.empty()) {
        PERMISSION_LOG_INFO(LABEL, "checkPermission: empty nodeId.");
        return CheckLocalPermission(uid, permissionName);
    }
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "checkPermission: nodeId cannot convert to deviceId, failure.");
        return Constant::PERMISSION_DENIED;
    }
    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);
    if (deviceId == localDeviceId) {
        PERMISSION_LOG_INFO(LABEL, "checkPermission: check by pms because of local device.");
        return CheckLocalPermission(uid, permissionName);
    }
    // If the duid is not present, then get the duid by deviceId and uid.
    int32_t distributedUid = SubjectDevicePermissionManager::GetInstance().AllocateDistributedUid(deviceId, uid);
    return SubjectDevicePermissionManager::GetInstance().CheckDistributedPermission(distributedUid, permissionName);
}

int32_t DistributedPermissionManagerService::CheckSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!DistributedDataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "CheckSelfPermission::permissionName is not valid");
        return Constant::PERMISSION_DENIED;
    }
    // CheckSelfPermission is used by applications to check whether they have certain permissions,
    // so the IPC package is used to get the process id and uid of the call source.
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    return CheckPermission(permissionName, "", pid, uid);
}

int32_t DistributedPermissionManagerService::CheckCallingPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!DistributedDataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "CheckSelfPermission::permissionName is not valid");
        return Constant::PERMISSION_DENIED;
    }
    // Only used to check the permissions of the caller.
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    std::string deviceId = IPCSkeleton::GetCallingDeviceID();
    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);
    if (pid == getpid() && uid == (pid_t)getuid() && deviceId == localDeviceId) {
        return Constant::PERMISSION_DENIED;
    }
    return CheckPermission(permissionName, deviceId, pid, uid);
}

int32_t DistributedPermissionManagerService::CheckCallingOrSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!DistributedDataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "CheckSelfPermission::permissionName is not valid");
        return Constant::PERMISSION_DENIED;
    }
    // Check the permission of its own application or caller application.
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    std::string deviceID = IPCSkeleton::GetCallingDeviceID();
    return CheckPermission(permissionName, deviceID, pid, uid);
}

int32_t DistributedPermissionManagerService::CheckCallerPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    return CheckCallingOrSelfPermission(permissionName);
}

bool DistributedPermissionManagerService::IsRestrictedPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    return PermissionDefinition::GetInstance().IsRestrictedPermission(permissionName);
}

int32_t DistributedPermissionManagerService::VerifyPermissionFromRemote(
    const std::string &permission, const std::string &nodeId, int32_t pid, int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "verifyPermissionFromRemote() called with: permission= %{public}s, nodeId= %{public}s, "
        " pid= %{public}d  , uid= %{public}d",
        permission.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        pid,
        uid);
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL,
            "VerifyPermissionFromRemote: nodeId %{public}s cannot convert to deviceId, failure.",
            Constant::EncryptDevId(nodeId).c_str());
        return Constant::PERMISSION_DENIED;
    }
    int result =
        ObjectDevicePermissionManager::GetInstance().VerifyPermissionFromRemote(permission, deviceId, pid, uid);
    PERMISSION_LOG_DEBUG(LABEL, "verifyPermissionFromRemote() returned: %{public}d.", result);
    return result;
}

int32_t DistributedPermissionManagerService::VerifySelfPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "verifyPermissionFromRemote() called with: permission= %{public}s, nodeId= %{public}s ",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    const pid_t pid = IPCSkeleton::GetCallingPid();
    const pid_t uid = IPCSkeleton::GetCallingUid();
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL,
            "VerifySelfPermissionFromRemote: nodeId %{public}s cannot convert to deviceId, failure.",
            Constant::EncryptDevId(nodeId).c_str());
        return Constant::PERMISSION_DENIED;
    }
    int result =
        ObjectDevicePermissionManager::GetInstance().VerifyPermissionFromRemote(permissionName, deviceId, pid, uid);
    PERMISSION_LOG_DEBUG(LABEL, "verifySelfPermissionFromRemote() returned: %{public}d.", result);
    return result;
}

bool DistributedPermissionManagerService::CanRequestPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "CanRequestPermissionFromRemote() called with: permission= %{public}s, nodeId= %{public}s ",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    const pid_t pid = IPCSkeleton::GetCallingPid();
    const pid_t uid = IPCSkeleton::GetCallingUid();
    const std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL,
            "CanRequestPermissionFromRemote: nodeId %{public}s cannot convert to deviceId, failure.",
            Constant::EncryptDevId(nodeId).c_str());
        return false;
    }
    bool result =
        RequestRemotePermission::GetInstance().CanRequestPermissionFromRemote(permissionName, deviceId, pid, uid);
    PERMISSION_LOG_ERROR(LABEL, "canRequestPermissionFromRemote() returned: %{public}s .", result ? "true" : "false");
    return result;
}
void DistributedPermissionManagerService::GrantSensitivePermissionToRemoteApp(
    const std::string &permissionName, const std::string &nodeId, int ruid)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "GrantSensitivePermissionToRemoteApp() called with: permission= %{public}s, nodeId= %{public}s, "
        " ruid= %{public}d ",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        ruid);
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL,
            "GrantSensitivePermissionToRemoteApp: nodeId %{public}s cannot convert to deviceId, failure.",
            Constant::EncryptDevId(nodeId).c_str());
        return;
    }
    RequestRemotePermission::GetInstance().GrantSensitivePermissionToRemoteApp(permissionName, deviceId, ruid);
}

void DistributedPermissionManagerService::RequestPermissionsFromRemote(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "RequestPermissionsFromRemote() called with: nodeId= %{public}s, "
        " bundleName= %{public}s , reasonResId= %{public}d",
        Constant::EncryptDevId(nodeId).c_str(),
        bundleName.c_str(),
        reasonResId);
    // RequestRemotePermission need the node Id, please do not convert it to deviceId
    RequestRemotePermission::GetInstance().RequestPermissionsFromRemote(
        permissions, callback, nodeId, bundleName, reasonResId);
}

/**
 * Register using permission reminder to dpms at ohos side.
 *
 * @param callback instance of {@link ohos.security.permission.OnUsingPermissionReminder}.
 * @return result code indicates callback if register successfully.
 */
int32_t DistributedPermissionManagerService::RegisterUsingPermissionReminder(
    const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (callback == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: callback is nullptr", __func__);
        return Constant::FAILURE_DPMS;
    }

    if (reminderSet_.find(callback) == reminderSet_.end()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: regist callback", __func__);
        reminderSet_.insert(callback);
    }
    return Constant::SUCCESS;
}

/**
 * unregister using permission reminder to dpms at ohos side.
 *
 * @param callback instance of {@link ohos.security.permission.OnUsingPermissionReminder} has been registered.
 * @return result code indicates callback if unregister successfully.
 */
int32_t DistributedPermissionManagerService::UnregisterUsingPermissionReminder(
    const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (callback == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: callback is nullptr", __func__);
        return Constant::FAILURE_DPMS;
    }

    if (reminderSet_.find(callback) != reminderSet_.end()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: remove callback", __func__);
        reminderSet_.erase(callback);
    }
    return Constant::SUCCESS;
}

/**
 * Check permission and start permission using reminder if permission granted.
 *
 * @param permissionName permission name.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @return Permission checked result, {@link #GRANTED} indicates permission granted, otherwise {@link #DENIED}.
 */
int32_t DistributedPermissionManagerService::CheckPermissionAndStartUsing(
    const std::string &permissionName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);
    if (permissionName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: deviceId is null.", __func__);
        return Constant::FAILURE;
    }

    if (deviceId.empty()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: deviceId is null.", __func__);
        return Constant::FAILURE;
    }

    bool isSevsitive = BaseRemoteCommand::IsSensitiveResource(permissionName);
    if (!isSevsitive) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: The permissionName is not sevsitive.", __func__);
        return Constant::FAILURE;
    }

    int32_t iGranted = Constant::FAILURE;
    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);

    if (deviceId == localDeviceId) {
        std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
        if (externalDeps == nullptr) {
            PERMISSION_LOG_INFO(LABEL, "%{public}s: The permissionName is not sevsitive.", __func__);
            return Constant::FAILURE;
        }
        iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
        if (iBundleManager_ == nullptr) {
            PERMISSION_LOG_INFO(LABEL, "%{public}s: iBundleManager_ is nullptr.", __func__);
            return Constant::FAILURE;
        }
        std::string bundleName;
        bool result = iBundleManager_->GetBundleNameForUid(uid, bundleName);
        if (!result) {
            PERMISSION_LOG_INFO(LABEL, "%{public}s cannot get bundle name by uid %{public}d", __func__, uid);
            return Constant::FAILURE;
        }

        iGranted = Permission::PermissionKit::VerifyPermission(bundleName, permissionName, Constant::DEFAULT_USERID);
        PERMISSION_LOG_INFO(LABEL, "iGranted : %{public}d", iGranted);
    } else {
        int32_t distributedUid = SubjectDevicePermissionManager::GetInstance().AllocateDistributedUid(deviceId, uid);
        iGranted =
            SubjectDevicePermissionManager::GetInstance().CheckDistributedPermission(distributedUid, permissionName);
    }

    bool isRootOrSystem = DistributedDataValidator::IsSpecRuid(uid);
    if (GRANTED == iGranted && !isRootOrSystem) {
        StartUsingPermission(permissionName, pid, uid, deviceId);
    }

    if (GRANTED != iGranted) {
        AddPermissionsRecord(permissionName, deviceId, uid, 0, 1);
    }

    return iGranted;
}

/**
 * Used to permission using remind when app start using permission continuously.
 *
 * @param permName the permission name which app start using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 */
void DistributedPermissionManagerService::StartUsingPermission(
    const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "invalid appIdInfo json");
        return;
    }

    PermissionReminderInfo permReminderInfo;

    permReminderInfo.Init();

    GetPermissionReminderInfo(permName, pid, uid, deviceId, permReminderInfo);

    CloseAns();

    InsertPermissionReminderInfo(permName, pid, uid, deviceId, permReminderInfo);

    CreatAns();

    AddPermissionsRecord(permName, deviceId, uid, 1, 0);
    return;
}

/**
 * Used to permission using remind when app stop using permission continuously.
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 */
void DistributedPermissionManagerService::StopUsingPermission(
    const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "invalid appIdInfo json");
        return;
    }

    PermissionReminderInfo permReminderInfo;

    permReminderInfo.Init();

    GetPermissionReminderInfo(permName, pid, uid, deviceId, permReminderInfo);

    CloseAns();

    RemovePermissionReminderInfo(permName, pid, uid, deviceId, permReminderInfo);

    CreatAns();

    AddPermissionsRecord(permName, deviceId, uid, 1, 0);
    return;
}

/**
 * Get permission reminder infomation for the callback
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @param permReminderInfo permmission reminder infomation
 */
void DistributedPermissionManagerService::GetPermissionReminderInfo(const std::string &permName, int32_t pid,
    int32_t uid, const std::string &deviceId, PermissionReminderInfo &permReminderInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    // set deviceId
    permReminderInfo.SetDeviceId(deviceId);

    // set deviceLabel
    DeviceInfo deviceInfo;
    if (!deviceId.empty() &&
        DeviceInfoManager::GetInstance().GetDeviceInfo(deviceId, DeviceIdType::UNKNOWN, deviceInfo) &&
        !deviceInfo.deviceName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: set deviceLabel.", __func__);
        permReminderInfo.SetDeviceLabel(deviceInfo.deviceName);
    }

    // get bundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    if (!GetBundleInfo(bundleInfo, uid)) {
        return;
    }
    // set bundleName
    permReminderInfo.SetBundleName(bundleInfo.name);

    // set bundleLabel
    int indexS = bundleInfo.label.find("$");
    int indexE = bundleInfo.label.find(":");
    if (indexS == 0 && indexE > 0) {
        bundleInfo.label = bundleInfo.label.erase(indexS, ++indexE);
    }
    permReminderInfo.SetBundleLabel(bundleInfo.label);

    // set permName
    permReminderInfo.SetPermName(permName);

    // set appName
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: create resourceManager failed!", __func__);
        return;
    }

    std::string appName = "";
    for (auto resPath = bundleInfo.moduleResPaths.begin(); resPath != bundleInfo.moduleResPaths.end(); resPath++) {
        if (resPath->empty()) {
            continue;
        }

        if (!resourceManager->AddResource(resPath->c_str())) {
            continue;
        }

        if (Constant::SUCCESS != resourceManager->GetStringByName(bundleInfo.label.c_str(), appName)) {
            continue;
        }

        if (appName.empty()) {
            continue;
        }
        break;
    }
    permReminderInfo.appName = appName;
}

/**
 * Get bundle info by uid
 *
 * @param bundleInfo the bundle info of app.
 * @param uid the uid from app.
 * @return result true if get successfully.
 */
bool DistributedPermissionManagerService::GetBundleInfo(AppExecFwk::BundleInfo &bundleInfo, int32_t uid)
{
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    std::string bundleName;
    bool result = iBundleManager_->GetBundleNameForUid(uid, bundleName);
    if (!result) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s cannot get bundle name by uid %{public}d", __func__, uid);
        return false;
    }
    result = iBundleManager_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!result) {
        PERMISSION_LOG_INFO(
            LABEL, "%{public}s cannot get bundleInfo by bundleName %{public}s", __func__, bundleName.c_str());
        return false;
    }
    return true;
}

/**
 * Insert permission reminder infomation for Ans
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @param permReminderInfo permmission reminder infomation
 */
void DistributedPermissionManagerService::InsertPermissionReminderInfo(const std::string &permName, int32_t pid,
    int32_t uid, const std::string &deviceId, PermissionReminderInfo &permReminderInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    bool isSevsitive = BaseRemoteCommand::IsSensitiveResource(permName);
    if (!isSevsitive) {
        PERMISSION_LOG_INFO(LABEL, "The permName is not sevsitive.");
        return;
    }

    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);
    PERMISSION_LOG_INFO(LABEL, "localDeviceId :%{public}s", localDeviceId);

    std::vector<PermissionRemindInfo>::iterator item;
    bool permission_is_exist = false;
    for (item = RemindInfo::GetInstance().permRemindInfos_.begin();
         item != RemindInfo::GetInstance().permRemindInfos_.end();
         item++) {
        if (!(deviceId != localDeviceId && item->deviceId == deviceId) &&
            !(deviceId == localDeviceId && item->pid == pid && item->uid == uid)) {
            continue;
        }

        permission_is_exist = permission_is_exist || CheckPermissionExist(item, permName);
        if (permName == Constant::LOCATION || permName == Constant::LOCATION_IN_BACKGROUND) {
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::LOCATION);
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::LOCATION_IN_BACKGROUND);
        }

        if (permName == Constant::READ_CALENDAR || permName == Constant::WRITE_CALENDAR) {
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::READ_CALENDAR);
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::WRITE_CALENDAR);
        }

        if (permName == Constant::DISTRIBUTED_DATASYNC || permName == Constant::DISTRIBUTED_DATA) {
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::DISTRIBUTED_DATASYNC);
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::DISTRIBUTED_DATA);
        }

        if (permName == Constant::MEDIA_LOCATION || permName == Constant::READ_MEDIA ||
            permName == Constant::WRITE_MEDIA) {
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::MEDIA_LOCATION);
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::READ_MEDIA);
            permission_is_exist = permission_is_exist || CheckPermissionExist(item, Constant::WRITE_MEDIA);
        }
        if (!permission_is_exist) {
            item->permissions.push_back(permName);
        }
        return;
    }
    CreateNewPermissionReminderInfo(permName, pid, uid, deviceId, permReminderInfo);
}

/**
 * check permisson is exist
 *
 * @param item permission info vector
 * @param permissionName  permission name.
 * @return result true if check successfully.
 */
bool DistributedPermissionManagerService::CheckPermissionExist(
    const std::vector<PermissionRemindInfo>::iterator item, const std::string permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);
    std::vector<std::string>::iterator it = find(item->permissions.begin(), item->permissions.end(), permissionName);
    if (it != item->permissions.end()) {
        PERMISSION_LOG_INFO(LABEL, "The permission is haved. permssionName :%{public}s", permissionName.c_str());
        return true;
    }
    return false;
}

/**
 * create new permission reminder info
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @param permReminderInfo permmission reminder infomation
 */
void DistributedPermissionManagerService::CreateNewPermissionReminderInfo(const std::string &permName, int32_t pid,
    int32_t uid, const std::string &deviceId, PermissionReminderInfo &permReminderInfo)
{
    PermissionRemindInfo permItem;
    permItem.permissions.push_back(permName);
    permItem.pid = pid;
    permItem.uid = uid;
    permItem.deviceId = deviceId;
    permItem.notificationId = uid;
    permItem.bundleName = Str16ToStr8(permReminderInfo.bundleName);
    permItem.bundleLabel = Str16ToStr8(permReminderInfo.bundleLabel);
    permItem.deviceName = Str16ToStr8(permReminderInfo.deviceLabel);
    permItem.appName = permReminderInfo.appName;
    RemindInfo::GetInstance().permRemindInfos_.push_back(permItem);
}

/**
 * Remove permission reminder infomation for Ans
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @param permReminderInfo permmission reminder infomation
 */
void DistributedPermissionManagerService::RemovePermissionReminderInfo(const std::string &permName, int32_t pid,
    int32_t uid, const std::string &deviceId, PermissionReminderInfo &permReminderInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called", __func__);

    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);
    PERMISSION_LOG_INFO(LABEL, "localDeviceId :%{public}s", localDeviceId);

    std::vector<PermissionRemindInfo>::iterator item;
    for (item = RemindInfo::GetInstance().permRemindInfos_.begin();
         item != RemindInfo::GetInstance().permRemindInfos_.end();
         item++) {
        if (!(deviceId != localDeviceId && item->deviceId == deviceId) &&
            !(deviceId == localDeviceId && item->pid == pid && item->uid == uid)) {
            continue;
        }

        ErasePermission(item, permName);
        if (permName == Constant::LOCATION || permName == Constant::LOCATION_IN_BACKGROUND) {
            ErasePermission(item, Constant::LOCATION);
            ErasePermission(item, Constant::LOCATION_IN_BACKGROUND);
        }

        if (permName == Constant::READ_CALENDAR || permName == Constant::WRITE_CALENDAR) {
            ErasePermission(item, Constant::READ_CALENDAR);
            ErasePermission(item, Constant::WRITE_CALENDAR);
        }

        if (permName == Constant::DISTRIBUTED_DATASYNC || permName == Constant::DISTRIBUTED_DATA) {
            ErasePermission(item, Constant::DISTRIBUTED_DATASYNC);
            ErasePermission(item, Constant::DISTRIBUTED_DATA);
        }

        if (permName == Constant::MEDIA_LOCATION || permName == Constant::READ_MEDIA ||
            permName == Constant::WRITE_MEDIA) {
            ErasePermission(item, Constant::MEDIA_LOCATION);
            ErasePermission(item, Constant::READ_MEDIA);
            ErasePermission(item, Constant::WRITE_MEDIA);
        }

        if (item->permissions.size() <= 0) {
            RemindInfo::GetInstance().permRemindInfos_.erase(item);
        }
        return;
    }
}

/**
 * delete permission
 *
 * @param item permission info vector
 * @param permissionName  permission name.
 */
void DistributedPermissionManagerService::ErasePermission(
    std::vector<PermissionRemindInfo>::iterator &item, const std::string permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);
    std::vector<std::string>::iterator it = find(item->permissions.begin(), item->permissions.end(), permissionName);
    if (it != item->permissions.end()) {
        item->permissions.erase(it);
    }
}

/**
 * creat ans, get resource manager
 */
void DistributedPermissionManagerService::CreatAns()
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s: create resourceManager failed!", __func__);
        return;
    }

    if (RemindInfo::GetInstance().permRemindInfos_.size() <= 0) {
        PERMISSION_LOG_INFO(LABEL, "RemindInfo::GetInstance().permRemindInfos_ is nullptr.");
        return;
    }
    bool ret = true;
    ret = resourceManager->AddResource(g_resFilePath);
    if (!ret) {
        PERMISSION_LOG_INFO(LABEL, "failed to AddResource");
    }

    int permCount = 0;
    std::string label = "";
    std::string settingtext = "";
    std::string settingtextDistributed = "";
    std::string permName = "";
    std::string permUsing = "";
    char localDeviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, Constant::DEVICE_UUID_LENGTH);

    if (ret) {
        resourceManager->GetStringByName("perm_using_remind_label", label);
        resourceManager->GetStringByName("perm_using_remind_setting1", settingtext);
        resourceManager->GetStringByName("perm_using_remind_setting2", settingtextDistributed);
    }

    if (label.empty() || settingtext.empty() || settingtextDistributed.empty()) {
        ret = false;
        label = perm_using_remind_label_;
        settingtext = perm_using_remind_setting1_;
        settingtextDistributed = perm_using_remind_setting2_;
    }

    std::vector<PermissionRemindInfo>::iterator item;
    std::vector<std::string>::iterator perm;

    for (item = RemindInfo::GetInstance().permRemindInfos_.begin();
         item != RemindInfo::GetInstance().permRemindInfos_.end();
         item++) {
        if (item->permissions.size() <= 0) {
            continue;
        }

        item->label = label;

        Notification::NotificationRequest notification(item->notificationId);

        notification.SetLabel(item->label);
        std::shared_ptr<Notification::NotificationLongTextContent> longTextContent =
            std::make_shared<Notification::NotificationLongTextContent>();

        // The using permName
        permCount = item->permissions.size();
        if (ret) {
            resourceManager->GetPluralStringByName("perm_using_remind_text", permCount, permUsing);
            if (permUsing.empty()) {
                permUsing = perm_using_remind_texts_.at(permCount);
            }
        } else {
            permUsing = perm_using_remind_texts_.at(permCount);
        }

        for (perm = item->permissions.begin(); perm != item->permissions.end(); perm++) {
            if (ret) {
                resourceManager->GetStringByName(perm->c_str(), permName);
                if (permName.empty()) {
                    permName = perm->c_str();
                    permName = perm_using_names_.find(permName)->second;
                }
            } else {
                permName = perm->c_str();
                permName = perm_using_names_.find(permName)->second;
            }

            permUsing.replace(permUsing.find("@", 0), 1, permName.c_str());
        }

        longTextContent->SetLongText(permUsing);
        std::shared_ptr<Notification::NotificationContent> content =
            std::make_shared<Notification::NotificationContent>(longTextContent);
        notification.SetCreatorPid(item->pid);
        notification.SetCreatorUid(item->uid);
        notification.SetContent(content);

        if (item->deviceId.empty() || item->deviceId == localDeviceId) {
            if (item->appName.empty()) {
                longTextContent->SetExpandedTitle(item->bundleName);
            } else {
                longTextContent->SetExpandedTitle(item->appName);
            }
            notification.SetSettingsText(settingtext);
            PERMISSION_LOG_INFO(LABEL, "label     : %{public}s", label.c_str());
            PERMISSION_LOG_INFO(LABEL, "appName   : %{public}s", longTextContent->GetExpandedTitle().c_str());
            PERMISSION_LOG_INFO(LABEL, "permUsing : %{public}s", permUsing.c_str());
            PERMISSION_LOG_INFO(LABEL, "setting   : %{public}s", settingtext.c_str());
            PERMISSION_LOG_INFO(LABEL, "checkresult : CreatAns Success");
        } else {
            longTextContent->SetExpandedTitle(item->deviceName);
            notification.SetSettingsText(settingtextDistributed);
            PERMISSION_LOG_INFO(LABEL, "label     : %{public}s", label.c_str());
            PERMISSION_LOG_INFO(LABEL, "appName   : %{public}s", item->deviceName.c_str());
            PERMISSION_LOG_INFO(LABEL, "permUsing : %{public}s", permUsing.c_str());
            PERMISSION_LOG_INFO(LABEL, "setting   : %{public}s", settingtextDistributed.c_str());
            PERMISSION_LOG_INFO(LABEL, "checkresult : CreatAns Success");
        }
        Notification::NotificationHelper::PublishNotification(item->label, notification);
    }
}

/**
 * Close the Ans
 */
void DistributedPermissionManagerService::CloseAns()
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    std::vector<PermissionRemindInfo>::iterator item;

    for (item = RemindInfo::GetInstance().permRemindInfos_.begin();
         item != RemindInfo::GetInstance().permRemindInfos_.end();
         item++) {
        Notification::NotificationHelper::CancelNotification(item->label, item->notificationId);
    }
}

/**
 * add permission record
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 * @param failCount fail number
 */
void DistributedPermissionManagerService::AddPermissionsRecord(const std::string &permissionName,
    const std::string &deviceId, const int32_t uid, const int32_t sucCount, const int32_t failCount)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, deviceId = %{public}s, uid = %{public}d, sucCount = "
        "%{public}d, failCount = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        uid,
        sucCount,
        failCount);
    PermissionRecordManager::GetInstance().AddPermissionsRecord(permissionName, deviceId, uid, sucCount, failCount);
}

/**
 * get permission record
 *
 * @param queryGzipStr the query string zip.
 * @param codeLen code length.
 * @param zipLen query string length.
 * @param resultStr query result string.
 * @return result code if get successfully.
 */
int32_t DistributedPermissionManagerService::GetPermissionRecords(
    const std::string &queryGzipStr, uLong &codeLen, uLong &zipLen, std::string &resultStr)
{
    PERMISSION_LOG_INFO(
        LABEL, "queryGzipStr = %{public}s, resultStr = %{public}s", queryGzipStr.c_str(), resultStr.c_str());
    return PermissionRecordManager::GetInstance().GetPermissionRecordsBase(queryGzipStr, codeLen, zipLen, resultStr);
}

/**
 * get permission record with callback
 *
 * @param request the query string.
 * @param codeLen code length.
 * @param zipLen query string length.
 * @param callback query result callback.
 * @return result code if get successfully.
 */
int32_t DistributedPermissionManagerService::GetPermissionRecords(const std::string &request, unsigned long &codeLen,
    unsigned long &zipLen, const sptr<OnPermissionUsedRecord> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "request = %{public}s", request.c_str());
    if (callback == nullptr) {
        return false;
    }
    return PermissionRecordManager::GetInstance().GetPermissionRecordsAsync(request, codeLen, zipLen, callback);
}

/**
 * duid convert
 *
 * @param duid  duid.
 * @return result code if get successfully.
 */
int32_t DistributedPermissionManagerService::convert(int32_t duid)
{
    return duid > Constant::SUCCESS ? duid : Constant::FAILURE_DPMS;
}

/**
 *get event handler
 *
 * @return event handler object.
 */
std::shared_ptr<DistributedPermissionEventHandler> DistributedPermissionManagerService::GetEventHandler()
{
    return handler_;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS