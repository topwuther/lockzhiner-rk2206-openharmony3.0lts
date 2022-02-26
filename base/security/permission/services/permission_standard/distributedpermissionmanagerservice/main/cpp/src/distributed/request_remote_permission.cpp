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
#include "request_remote_permission.h"
#include "distributed_permission_manager_service.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RequestRemotePermission"};
}
RequestRemotePermission &RequestRemotePermission ::GetInstance()
{
    static RequestRemotePermission instance;
    return instance;
}
bool RequestRemotePermission::CanRequestPermissionFromRemote(
    const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid)
{
    if (!DistributedDataValidator::IsPermissionNameValid(permission) ||
        !DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "CanRequestPermissionFromRemote:permission or deviceId is invalid");
        return false;
    }
    if (!BaseRemoteCommand::IsSensitiveResource(permission)) {
        PERMISSION_LOG_ERROR(LABEL,
            "CanRequestPermissionFromRemote:permission %{public}s is not sensitive permission",
            permission.c_str());
        return false;
    }
    if (!DeviceInfoManager::GetInstance().ExistDeviceInfo(deviceId, DeviceIdType::UNIQUE_DISABILITY_ID)) {
        PERMISSION_LOG_ERROR(LABEL,
            "CanRequestPermissionFromRemote:device %{public}s is not online",
            Constant::EncryptDevId(deviceId).c_str());
        return false;
    }
    if (ObjectDevicePermissionManager::GetInstance().VerifyPermissionFromRemote(permission, deviceId, pid, uid) ==
        Constant::PERMISSION_GRANTED) {
        PERMISSION_LOG_ERROR(LABEL,
            "CanRequestPermissionFromRemote:permission %{public}s ihave been granted before",
            permission.c_str());
        return false;
    }
    //  need add  func   isUidForeground
    if (PermissionBmsManager::GetInstance().IsSystemSignatureUid(uid)) {
        PERMISSION_LOG_INFO(
            LABEL, "CanRequestPermissionFromRemote:current  uid %{public}d is system or system signature uid", uid);
        return true;
    }
    // when the  switch code is complete, need add func  CheckThirdAppMore(deviceId, permission)
    return true;
}
bool RequestRemotePermission::CheckThirdAppMore(const std::string deviceId, const std::string permission)
{
    return ResourceSwitch::GetInstance().GetRemoteSensitiveResourceSwitch(deviceId, permission) ==
           Constant::RESOURCE_SWITCH_STATUS_ALLOWED;
}
void RequestRemotePermission::GrantSensitivePermissionToRemoteApp(
    const std::string &permission, const std::string &deviceId, int32_t ruid)
{
    SubjectDevicePermissionManager::GetInstance().GrantSensitivePermissionToRemoteApp(permission, deviceId, ruid);
}
/**
 * Dynamically applies for permissions from a specified device (the application result is returned asynchronously).
 *
 * @param permissions The permission names will request from remote.
 * @param callback The callback of the request operation.
 * @param nodeId The remote device udid.
 * @param bundleName The package name that request the permissions.
 * @param reasonResId The reason that request the permissions.
 */
void RequestRemotePermission::RequestPermissionsFromRemote(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
{
    if (callback == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "RequestPermissionsFromRemote callback is error, nullptr");
        return;
    }
    std::shared_ptr<RequestPermissionsInfo> request =
        std::make_shared<RequestPermissionsInfo>(permissions, callback, nodeId, bundleName, reasonResId);
    PERMISSION_LOG_INFO(LABEL, "begin requestPermissionsFromRemote:%{public}s,", request->ToString().c_str());
    if (request->NeedStopProcess()) {
        PERMISSION_LOG_ERROR(LABEL, "parameters is error, stop to process the request");
        return;
    }
    if (requestingDevices_.count(request->deviceId_) != 0) {
        PERMISSION_LOG_ERROR(LABEL, "request is now executing on device:%{public}s", request->deviceId_.c_str());
        RequestResultForError(permissions, nodeId, callback, Constant::PERMISSION_REQUEST_NOT_FINISH);
        return;
    }
    requestInfos_.insert(std::pair<std::string, std::shared_ptr<RequestPermissionsInfo>>(request->requestId_, request));
    requestingDevices_.insert(std::pair<std::string, std::string>(request->deviceId_, request->deviceId_));
    auto taskTimeout = [this, request]() {
        auto sleepTime = std::chrono::milliseconds(RequestRemotePermission::REQUEST_DELAY);
        std::this_thread::sleep_for(sleepTime);
        RequestResultForTimeout(request->requestId_);
    };
    std::thread delayThread(taskTimeout);
    delayThread.detach();
    PERMISSION_LOG_DEBUG(LABEL, "Success to add the timeout :%{public}s", request->requestId_.c_str());
    if (CheckRequestResultForResult(request)) {
        return;
    }
    auto task = [request]() {
        std::shared_ptr<RequestRemotePermissionCommand> command =
            RemoteCommandFactory::GetInstance().NewRequestRemotePermissionCommand(
                Constant::GetLocalDeviceId(), request->deviceId_, request->requestId_);
        command->SetRequestPermissionInfo(
            request->uid_, request->permissions_, request->bundleName_, request->reasonString_);
        int commandResult = RemoteCommandManager::GetInstance().ExecuteCommand(request->deviceId_, command);
        PERMISSION_LOG_INFO(LABEL, " command   result:%{public}d", commandResult);
    };
    std::thread commandThread(task);
    commandThread.detach();
    PERMISSION_LOG_DEBUG(LABEL, " Add  command   to TaskExecutor success:%{public}s", request->requestId_.c_str());
}
bool RequestRemotePermission::CheckRequestResultForResult(std::shared_ptr<RequestPermissionsInfo> request)
{
    if (request == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "CheckRequestResultForResult request err:nullptr");
        return true;
    }
    if (UserHandleManager::IsRootOrSystemUid(request->uid_)) {
        PERMISSION_LOG_ERROR(LABEL, "request is  root or system usr :%{public}s", request->deviceId_.c_str());
        RequestResultForResult(request->requestId_);
        return true;
    }
    if (!request->CanRequestPermissions() ||
        !HaveSensitivePermissionToRequest(request->deviceId_, request->pid_, request->uid_, request->permissions_)) {
        PERMISSION_LOG_ERROR(LABEL, "request can not be execute on device :%{public}s", request->deviceId_.c_str());
        RequestResultForResult(request->requestId_);
        return true;
    }
    return false;
}
void RequestRemotePermission::RequestResultForTimeout(const std::string &requestId)
{
    auto iter = requestInfos_.find(requestId);
    if (iter == requestInfos_.end()) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForTimeout requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    const std::shared_ptr<RequestPermissionsInfo> originRequest = requestInfos_.at(requestId);
    requestInfos_.erase(requestId);
    if (originRequest == nullptr) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForTimeout requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    if (originRequest->callback_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL,
            "failed to do requestResultForTimeout callback = null from requestId :%{public}s",
            requestId.c_str());
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "requestResultForTimeout requestId %{public}s is timeout", requestId.c_str());
    requestingDevices_.erase(originRequest->deviceId_);
    originRequest->callback_->OnTimeOut(originRequest->nodeId_, originRequest->permissions_);
}
void RequestRemotePermission::RequestResultForResult(const std::string &requestId)
{
    auto iter = requestInfos_.find(requestId);
    if (iter == requestInfos_.end()) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForResult requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    const std::shared_ptr<RequestPermissionsInfo> originRequest = requestInfos_.at(requestId);
    requestInfos_.erase(requestId);
    if (originRequest == nullptr) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForResult requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    if (originRequest->callback_ == nullptr) {
        PERMISSION_LOG_ERROR(
            LABEL, "failed to do requestResultForResult callback = null from requestId :%{public}s", requestId.c_str());
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "requestResultForResult requestId %{public}s is grant by user", requestId.c_str());
    requestingDevices_.erase(originRequest->deviceId_);

    std::string taskName("RequestResultForResult");

    auto task = [originRequest]() {
        std::vector<int> result = originRequest->GetGrantResult();
        std::stringstream permissionsStr;
        std::copy(originRequest->permissions_.begin(),
            originRequest->permissions_.end(),
            std::ostream_iterator<std::string>(permissionsStr, ","));
        std::stringstream resultStr;
        std::copy(result.begin(), result.end(), std::ostream_iterator<int>(resultStr, " "));
        PERMISSION_LOG_INFO(LABEL,
            "requestResultForResult(permissions : %{public}s  ;nodeId : %{public}s  ;result : %{public}s)",
            permissionsStr.str().c_str(),
            originRequest->nodeId_.c_str(),
            resultStr.str().c_str());
        originRequest->callback_->OnResult(originRequest->nodeId_, originRequest->permissions_, result);
    };
    std::thread resultThread(task);
    resultThread.detach();
}
void RequestRemotePermission::RequestResultForError(const std::vector<std::string> permissions,
    const std::string &nodeId, const sptr<OnRequestPermissionsResult> &callback, const int32_t errorCode)
{
    PERMISSION_LOG_ERROR(LABEL, "requestResultForError by errorCode %{public}d", errorCode);
    if (callback == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "requestResultForError callback is null");
        return;
    }
    int length = permissions.empty() ? 0 : permissions.size();
    std::vector<int> result(length);
    result.assign(length, errorCode);
    std::string permissionsStr = accumulate(permissions.begin(), permissions.end(), std::string(","));
    PERMISSION_LOG_INFO(LABEL,
        "requestResultForResult(permissions : %{public}s  ;nodeId : %{public}s  ;errorCode : %{public}d)",
        permissionsStr.c_str(),
        nodeId.c_str(),
        errorCode);
    auto task = [callback, nodeId, permissions, result]() { callback->OnResult(nodeId, permissions, result); };
    std::thread errThread(task);
    errThread.detach();
}
bool RequestRemotePermission::HaveSensitivePermissionToRequest(
    const std::string deviceId, const int pid, const int uid, const std::vector<std::string> permissions)
{
    for (auto permission : permissions) {
        if (permission.empty()) {
            continue;
        }
        if (RequestRemotePermission::GetInstance().CanRequestPermissionFromRemote(permission, deviceId, pid, uid)) {
            PERMISSION_LOG_INFO(LABEL,
                " current request have sensitive permission %{public}s need to request from object device",
                permission.c_str());
            return true;
        } else {
            PERMISSION_LOG_INFO(LABEL,
                " current request permission  %{public}s CAN NOT request from object device",
                permission.c_str());
        }
    }
    PERMISSION_LOG_ERROR(LABEL, "current request do not have any permission to request from remote");
    return false;
}
void RequestRemotePermission::RequestResultForCancel(const std::string &requestId)
{
    auto iter = requestInfos_.find(requestId);
    if (iter == requestInfos_.end()) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForCancel requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    const std::shared_ptr<RequestPermissionsInfo> originRequest = requestInfos_.at(requestId);
    requestInfos_.erase(requestId);
    if (originRequest == nullptr) {
        PERMISSION_LOG_ERROR(LABEL,
            "requestResultForCancel requestId %{public}s is not exist and have been processed",
            requestId.c_str());
        return;
    }
    if (originRequest->callback_ == nullptr) {
        PERMISSION_LOG_ERROR(
            LABEL, "failed to do requestResultForCancel callback = null from requestId :%{public}s", requestId.c_str());
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "requestResultForCancel requestId %{public}s is timeout", requestId.c_str());
    requestingDevices_.erase(originRequest->deviceId_);
    std::string taskName("RequestResultForCancel Execute");
    auto task = [originRequest]() {
        originRequest->callback_->OnCancel(originRequest->nodeId_, originRequest->permissions_);
    };
    std::thread cancelThread(task);
    cancelThread.detach();
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS