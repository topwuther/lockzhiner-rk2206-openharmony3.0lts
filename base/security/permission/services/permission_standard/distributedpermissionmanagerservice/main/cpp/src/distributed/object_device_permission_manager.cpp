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

#include "object_device_permission_manager.h"
#include "distributed_permission_manager_service.h"
#include "permission_kit.h"
#include "permission_callback.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ObjectDevicePermissionManager"};
}
ObjectDevicePermissionManager::ObjectDevicePermissionManager() : recovered_(false), registered_(false)
{}

ObjectDevicePermissionManager::~ObjectDevicePermissionManager()
{}

ObjectDevicePermissionManager &ObjectDevicePermissionManager::GetInstance()
{
    static ObjectDevicePermissionManager instance;
    return instance;
}

/**
 * If not successfully recovered, recover from file.
 */
void ObjectDevicePermissionManager::Init()
{
    if (!recovered_.load()) {
        bool recoverResult = ObjectDevicePermissionRepository::GetInstance().RecoverFromFile();
        PERMISSION_LOG_INFO(LABEL, "init: recoverResult:%{public}s", recoverResult ? "true" : "false");
        recovered_.store(recoverResult);
    }
    bool falseFlag = false;
    if (registered_.compare_exchange_strong(falseFlag, true)) {
        PERMISSION_LOG_INFO(LABEL, "Register uid permission and state changed callback.");
        MonitorManager::GetInstance().AddOnPermissionChangedListener("ObjectDevicePermissionManager",
            std::make_shared<std::function<void(int32_t uid, const std::string &packageName)>>(
                [&](int32_t uid, const std::string &packageName) { this->NotifyPermissionChanged(uid); }));
    }
}

/**
 * Operation function. Ask RemoteExecutorManager to execute SyncUidPermissionCommand or DeleteUidPermissionCommand
 * for all deviceIds that has a relationship with given uid.
 *
 * @param uid The operation uid in local device.
 * @return SUCCESS: 0; FAILURE: -1.
 */
int32_t ObjectDevicePermissionManager::NotifyPermissionChanged(const int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL, "notifyPermissionChanged: operation start with uid:%{public}d", uid);
    // Check uid is negative, do remove operation instead.
    if (uid == SPECIAL_INVALID_UID) {
        PERMISSION_LOG_DEBUG(LABEL, "notifyPermissionChanged: uid: -1 is SPECIAL_INVALID_UID, return SUCCESS");
        return Constant::SUCCESS;
    }
    if (uid < 0) {
        PERMISSION_LOG_INFO(
            LABEL, "notifyPermissionChanged: uid: %{public}d is negative, call removeNotifyPermissionMonitorUid", uid);
        return RemoveNotifyPermissionMonitorUid(-uid);
    }

    // For special uid, do not need to notify.
    if (DistributedDataValidator::IsSpecRuid(uid)) {
        PERMISSION_LOG_DEBUG(LABEL, "notifyPermissionChanged: uid is special, return success");
        return Constant::SUCCESS;
    }

    // Find all deviceIds and build command, push to RemoteExecutorManager.
    const std::set<std::string> toSyncDevices = ObjectDevicePermissionRepository::GetInstance().ListDeviceId(uid);
    for (const std::string &deviceId : toSyncDevices) {
        const std::shared_ptr<SyncUidPermissionCommand> command =
            RemoteCommandFactory::GetInstance().NewSyncUidPermissionCommand(
                uid, Constant::GetLocalDeviceId(), deviceId);
        const int32_t resultCode = RemoteCommandManager::GetInstance().AddCommand(deviceId, command);
        if (resultCode != Constant::SUCCESS) {
            PERMISSION_LOG_WARN(LABEL,
                "notifyPermissionChanged: SyncUidPermissionCommand resultCode is not SUCCESS:  %{public}d",
                resultCode);
        }
    }

    if (toSyncDevices.empty()) {
        PERMISSION_LOG_DEBUG(
            LABEL, "notifyPermissionChanged: devicesToSync is empty, no need to notify, return success");
        return Constant::SUCCESS;
    }

    // Start a new thread to wait 1000 milliseconds, and return the result.
    return ProcessDeviceCommandImmediately(uid, toSyncDevices);
}

int32_t ObjectDevicePermissionManager::RemoveNotifyPermissionMonitorUid(const int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL, "rremoveNotifyPermissionMonitorUid: operation start with uid: %{public}d", uid);

    // For special uid, do not need to notify.
    if (DistributedDataValidator::IsSpecRuid(uid)) {
        PERMISSION_LOG_INFO(LABEL, "removeNotifyPermissionMonitorUid: uid is special, return success");
        return Constant::SUCCESS;
    }
    // Find all deviceIds and build command.
    const std::set<std::string> toDeleteDevices = ObjectDevicePermissionRepository::GetInstance().ListDeviceId(uid);

    for (const std::string &deviceId : toDeleteDevices) {
        const std::shared_ptr<DeleteUidPermissionCommand> command =
            RemoteCommandFactory::GetInstance().NewDeleteUidPermissionCommand(
                uid, Constant::GetLocalDeviceId(), deviceId);
        RemoteCommandManager::GetInstance().AddCommand(deviceId, command);
    }
    // Remove uid to deviceId relationship anyway.
    ObjectDevicePermissionRepository::GetInstance().RemoveUid(uid);
    ObjectDevicePermissionRepository::GetInstance().SaveToFile();

    // Ask RemoteExecutorManager to process command
    return RemoteCommandManager::GetInstance().Loop();
}

int32_t ObjectDevicePermissionManager::ProcessDeviceCommandImmediately(
    const int32_t uid, const std::set<std::string> &devicesToSync)
{
    const std::shared_ptr<CountDownLatch> latch = std::make_shared<CountDownLatch>(1);
    std::string taskName("ProcessDeviceCommandImmediately");
    auto task = [devicesToSync, uid, latch]() {
        for (std::string deviceId : devicesToSync) {
            RemoteCommandManager::GetInstance().ProcessDeviceCommandImmediately(deviceId);
        }
        PERMISSION_LOG_INFO(LABEL,
            "processDeviceCommandImmediately: all thread for NotifyPermissionChanged finish "
            "for uid: %{public}d finished",
            uid);
        latch->CountDown();
    };
    std::thread thread(task);
    thread.detach();
    latch->Await(MAX_NOTIFY_PERMISSION_CHANGED_TIMEOUT);
    if (latch->GetCount() != 0L) {
        PERMISSION_LOG_ERROR(
            LABEL, "processDeviceCommandImmediately: NotifyPermissionChanged time out, return FAILURE");
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "processDeviceCommandImmediately: NotifyPermissionChanged succeed, return SUCCESS");
    return Constant::SUCCESS;
}

int32_t ObjectDevicePermissionManager::AddNotifyPermissionMonitorUid(const std::string &deviceId, const int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL,
        "addNotifyPermissionMonitorUid: operation start with deviceId: %{public}s",
        Constant::EncryptDevId(deviceId).c_str());

    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) || !DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL,
            "addNotifyPermissionMonitorUid: addNotifyPermissionMonitorUid: deviceId or uid "
            "is invalid, return FAILURE");
        return Constant::FAILURE;
    }

    if (DistributedDataValidator::IsSpecRuid(uid)) {
        PERMISSION_LOG_INFO(
            LABEL, "addNotifyPermissionMonitorUid: addNotifyPermissionMonitorUid: uid is special, return SUCCESS");
        return Constant::SUCCESS;
    }

    ObjectDevicePermissionRepository::GetInstance().PutDeviceIdUidPair(deviceId, uid);
    ObjectDevicePermissionRepository::GetInstance().SaveToFile();

    return Constant::SUCCESS;
}

int32_t ObjectDevicePermissionManager::NotifySyncPermission(
    std::string &deviceId, int32_t uid, std::string &packageName)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(
            LABEL, "notifySyncPermission: invalid deviceId: %{public}s", Constant::EncryptDevId(deviceId).c_str());
        return Constant::INVALID_DEVICE_ID;
    }
    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "notifySyncPermission: invalid uid:%{public}d", uid);
        return Constant::FAILURE;
    }
    if (packageName.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "notifySyncPermission: invalid pkgName:%{public}s", packageName.c_str());
        return Constant::FAILURE;
    }
    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
    } else {
        std::string taskName("RequestPermissionsFromRemoteTimeout");
        auto task = [&deviceId, &uid, this]() {
            PERMISSION_LOG_INFO(LABEL, "notifySyncPermission: invalid uid:%{public}d", uid);
            std::shared_ptr<SyncUidPermissionCommand> syncUidPermissionCommand =
                std::make_shared<SyncUidPermissionCommand>(uid, Constant::GetLocalDeviceId(), deviceId);
            int32_t resultCode = RemoteCommandManager::GetInstance().ExecuteCommand(deviceId, syncUidPermissionCommand);
            if (resultCode == Constant::SUCCESS) {
                AddNotifyPermissionMonitorUid(deviceId, uid);
            }
            PERMISSION_LOG_INFO(LABEL, "notifySyncPermission: get resultCode:%{public}d", resultCode);
        };
        handler->PostTask(task, taskName);
    }

    return Constant::SUCCESS;
}
int32_t ObjectDevicePermissionManager::VerifyPermissionFromRemote(
    const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "verifyPermissionFromRemote: permission:  = %{public}s, deviceId = %{public}s, pid = "
        "%{public}d ,uid = %{public}d ",
        permission.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        pid,
        uid);
    int32_t result = VerifyPermissionFromRemoteInner(permission, deviceId, pid, uid);
    PERMISSION_LOG_DEBUG(LABEL,
        "verifyPermissionFromRemote: permission:  = %{public}s, deviceId = %{public}s, pid = "
        "%{public}d ,uid = %{public}d ",
        permission.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        pid,
        uid);
    return result;
}
int32_t ObjectDevicePermissionManager::VerifyPermissionFromRemoteInner(
    const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "VerifyPermissionFromRemoteInner: start");

    if (!IsValidVerificationParams(permission, deviceId, uid)) {
        PERMISSION_LOG_ERROR(LABEL, "VerifyPermissionFromRemoteInner: with invalid param");
        return Constant::PERMISSION_DENIED;
    }
    if (!DeviceInfoManager::GetInstance().ExistDeviceInfo(deviceId, DeviceIdType::UNIQUE_DISABILITY_ID)) {
        PERMISSION_LOG_ERROR(LABEL,
            "VerifyPermissionFromRemoteInner:device is ot online: %{public}s",
            Constant::EncryptDevId(deviceId).c_str());
        return Constant::PERMISSION_DENIED;
    }
    if (UserHandleManager::IsRootOrSystemUid(uid)) {
        return Constant::PERMISSION_GRANTED;
    }
    if (PermissionBmsManager::GetInstance().IsSystemSignatureUid(uid)) {
        // For system program,check if self have the permissin ,the permission will sync to object
        std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
        iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
        iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);

        std::vector<std::string> bundleNames;
        iBundleManager_->GetBundlesForUid(uid, bundleNames);
        bool allPermissionFlag = true;
        for (auto bundleName : bundleNames) {
            if (Permission::PermissionKit::VerifyPermission(bundleName, permission, Constant::DEFAULT_USERID) !=
                Constant::PERMISSION_GRANTED) {
                allPermissionFlag = false;
                break;
            }
        }
        if (allPermissionFlag && bundleNames.size() > 0) {
            return Constant::PERMISSION_GRANTED;
        }
    }
    return ObjectDevicePermissionRepository::GetInstance().VerifyPermissionFromRemote(permission, deviceId, pid, uid);
}
bool ObjectDevicePermissionManager::IsValidVerificationParams(
    const std::string &permission, const std::string &deviceId, const int32_t uid)
{
    if (!DistributedDataValidator::IsPermissionNameValid(permission)) {
        PERMISSION_LOG_ERROR(LABEL, "invalid permission: %{public}s", permission.c_str());
        return false;
    }
    if (!BaseRemoteCommand::IsSensitiveResource(permission)) {
        PERMISSION_LOG_ERROR(LABEL, " permission is not sensitive permission: %{public}s", permission.c_str());
        return false;
    }

    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "invalid deviceId: %{public}s", Constant::EncryptDevId(deviceId).c_str());
        return false;
    }
    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "uid is invalid: %{public}d", uid);
        return false;
    }
    return true;
}
void ObjectDevicePermissionManager::ResetGrantSensitivePermission(
    std::string &deviceId, int32_t uid, std::set<std::string> &permissions)
{
    ObjectDevicePermissionRepository::GetInstance().GetOrCreateObjectDevice(deviceId)->ResetGrantSensitivePermission(
        uid, permissions);
}
void ObjectDevicePermissionManager::RemoveObject(const std::string &deviceId)
{
    PERMISSION_LOG_INFO(
        LABEL, "removeObject: operation start with deviceId: %{public}s", Constant::EncryptDevId(deviceId).c_str());
    ObjectDevicePermissionRepository::GetInstance().RemoveDeviceId(deviceId);
    ObjectDevicePermissionRepository::GetInstance().SaveToFile();
};
int32_t ObjectDevicePermissionManager::RemoveNotifyPermissionMonitorUserId(const int32_t userId)
{
    std::lock_guard<std::recursive_mutex> guard(remove_notify_permission_monitor_user_id_locl_);
    PERMISSION_LOG_INFO(LABEL, "removeNotifyPermissionMonitorUserId: operation start with uid: %{public}d", userId);
    std::set<int32_t> uidList = ObjectDevicePermissionRepository::GetInstance().ListAllUid();
    std::set<int32_t>::iterator iter;
    int32_t resultCode = Constant::FAILURE;
    for (iter = uidList.begin(); iter != uidList.end(); iter++) {
        if (userId == (*iter / UserHandleManager::PER_USER_RANGE)) {
            int32_t result = RemoveNotifyPermissionMonitorUid(*iter);
            if (result == Constant::SUCCESS) {
                resultCode = Constant::SUCCESS;
            }
        }
    }
    return resultCode;
};

void ObjectDevicePermissionManager::Clear()
{
    PERMISSION_LOG_INFO(LABEL, "clear: remove caches related to all object devices");
    ObjectDevicePermissionRepository::GetInstance().Clear();
    ObjectDevicePermissionRepository::GetInstance().SaveToFile();
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
