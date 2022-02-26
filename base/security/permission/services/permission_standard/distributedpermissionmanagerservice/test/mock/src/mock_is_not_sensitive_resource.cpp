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

#define private public
#include "bundle_mgr_interface.h"
#include "external_deps.h"
#include "i_permission_manager.h"
#include "permission_bms_manager.h"
#include "request_remote_permission.h"
#include "subject_device_permission_manager.h"
#include "soft_bus_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusManager"};
}
// SoftBusManager
const std::string SoftBusManager::SESSION_NAME = "ohos.security.dpms_channel";

SoftBusManager::SoftBusManager() : isSoftBusServiceBindSuccess_(false), inited_(false), mutex_(), fulfillMutex_()
{}

SoftBusManager::~SoftBusManager()
{}

SoftBusManager &SoftBusManager::GetInstance()
{
    static SoftBusManager instance;
    return instance;
}

void SoftBusManager::Initialize()
{}

void SoftBusManager::Destroy()
{}

int32_t SoftBusManager::OpenSession(const std::string &deviceId)
{
    isSoftBusServiceBindSuccess_ = true;
    return 0;
}

int SoftBusManager::CloseSession(int sessionId)
{

    return 0;
}

std::string SoftBusManager::GetUniversallyUniqueIdByNodeId(const std::string &deviceNodeId)
{
    return "uuid";
}

std::string SoftBusManager::GetUniqueDisabilityIdByNodeId(const std::string &nodeId)
{
    return "udid";
}

std::string SoftBusManager::GetUuidByNodeId(const std::string &nodeId) const
{
    return "12";
}

std::string SoftBusManager::GetUdidByNodeId(const std::string &nodeId) const
{
    return "12";
}

int SoftBusManager::FulfillLocalDeviceInfo()
{
    return 0;
}

// RemoteCommandManager
RemoteCommandManager::RemoteCommandManager()
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager()");
}

RemoteCommandManager::~RemoteCommandManager()
{
    PERMISSION_LOG_DEBUG(LABEL, "~RemoteCommandManager()");
}

RemoteCommandManager &RemoteCommandManager::GetInstance()
{
    static RemoteCommandManager instance;
    return instance;
}

void RemoteCommandManager::Init()
{
    PERMISSION_LOG_DEBUG(LABEL, "Init()");
}

int RemoteCommandManager::AddCommand(const std::string &deviceId, const std::shared_ptr<BaseRemoteCommand> &command)
{
    return 0;
}

int RemoteCommandManager::ExecuteCommand(const std::string &deviceId, const std::shared_ptr<BaseRemoteCommand> &command)
{
    if (deviceId == "uniqueDisabilityIdSuccess") {
        std::string key = DistributedUidAllocator::GetInstance().Hash("uniqueDisabilityIdSuccess", 1024);
        DistributedUidEntity distributedUidEntity;
        distributedUidEntity.distributedUid = 12610001;
        DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
            std::pair<std::string, DistributedUidEntity>(key, distributedUidEntity));
        return Constant::SUCCESS;
    }
    if (deviceId == "uniqueDisabilityIdCanNotGetPackgeForUid") {
        return Constant::CANNOT_GET_PACKAGE_FOR_UID;
    }
    return Constant::FAILURE;
}

int RemoteCommandManager::ProcessDeviceCommandImmediately(const std::string &deviceId)
{
    return 0;
}

int RemoteCommandManager::Loop()
{
    return Constant::SUCCESS;
}

/**
 * caller: service connection listener
 */
void RemoteCommandManager::Clear()
{}

/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOnline(const std::string &deviceId)
{
    return Constant::SUCCESS;
}

/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOffline(const std::string &deviceId)
{
    return Constant::SUCCESS;
}

std::shared_ptr<RemoteCommandExecutor> RemoteCommandManager::GetOrCreateRemoteCommandExecutor(
    const std::string &deviceId)
{
    return nullptr;
}

/**
 * caller: session listener(onBytesReceived), device listener(offline)
 */
std::shared_ptr<RpcChannel> RemoteCommandManager::GetExecutorChannel(const std::string &nodeId)
{
    return nullptr;
}

// ExternalDeps
void RemoteCommandManager::RemoveCommand(const std::string &deviceId)
{}

// sptr<AppExecFwk::IBundleMgr> ExternalDeps::GetBundleManager(sptr<AppExecFwk::IBundleMgr> iBundleManager_)
// {
//     return nullptr;
// }

// sptr<Permission::IPermissionManager> ExternalDeps::GetPermissionManager(
//     sptr<Permission::IPermissionManager> iPermissionManager_)
// {
//     return nullptr;
// }

// sptr<AAFwk::IAbilityManager> ExternalDeps::GetAbilityManager(sptr<AAFwk::IAbilityManager> iAbilityManager_)
// {
//     return nullptr;
// }

// sptr<IRemoteObject> ExternalDeps::GetSystemAbility(const int32_t systemAbilityId)
// {
//     return nullptr;
// }

// PermissionBmsManager
PermissionBmsManager::PermissionBmsManager()
    : iBundleManager_(nullptr), iPermissionManager_(nullptr), permissionFetcher_(nullptr), appInfoFetcher_(nullptr)
{}

PermissionBmsManager::~PermissionBmsManager()
{}

PermissionBmsManager &PermissionBmsManager::GetInstance()
{
    static PermissionBmsManager instance;
    return instance;
}

int PermissionBmsManager::Init()
{
    return Constant::SUCCESS;
}

int32_t PermissionBmsManager::GetPermissions(int32_t uid, UidBundleBo &info)
{
    return Constant::SUCCESS;
}

int32_t PermissionBmsManager::GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo)
{
    return Constant::SUCCESS;
}

bool PermissionBmsManager::IsSystemSignatureUid(const int32_t &uid)
{
    return true;
}

int PermissionBmsManager::InitSystemDefinedPermissions()
{
    return Constant::SUCCESS;
}

void PermissionBmsManager::ReGrantDuidPermissions(UidBundleBo &uidBundlePermInfo)
{}

// RequestRemotePermission
RequestRemotePermission &RequestRemotePermission ::GetInstance()
{
    static RequestRemotePermission instance;
    return instance;
}
bool RequestRemotePermission::CanRequestPermissionFromRemote(
    const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid)
{
    return true;
}

void RequestRemotePermission::GrantSensitivePermissionToRemoteApp(
    const std::string &permission, const std::string &deviceId, int32_t ruid)
{}

void RequestRemotePermission::RequestPermissionsFromRemote(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
{}
void RequestRemotePermission::RequestResultForTimeout(const std::string &requestId)
{}
void RequestRemotePermission::RequestResultForResult(const std::string &requestId)
{}

void RequestRemotePermission::RequestResultForError(const std::vector<std::string> permissions,
    const std::string &nodeId, const sptr<OnRequestPermissionsResult> &callback, const int32_t errorCode)
{}

bool RequestRemotePermission::HaveSensitivePermissionToRequest(
    const std::string deviceId, const int pid, const int uid, const std::vector<std::string> permissions)
{
    return true;
}

// IPermissionManager
// int IPermissionManager::GetDefPermission(const std::string &permissionName, PermissionDefParcel &permissionDefResult)
// {

//     permissionDefResult.permissionDef.permissionName = permissionName;
//     permissionDefResult.permissionDef.bundleName = permissionName + "bundleName";
//     permissionDefResult.permissionDef.grantMode = 0;
//     permissionDefResult.permissionDef.availableScope = 0;
//     permissionDefResult.permissionDef.label = permissionName + "label";
//     permissionDefResult.permissionDef.labelId = 0;
//     permissionDefResult.permissionDef.description = permissionName + "description";
//     permissionDefResult.permissionDef.descriptionId = 0;

//     return 0;
// }
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
