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

#include "permission_manager_proxy.h"

#include "permission/permission.h"
#include "permission_log.h"

#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionManagerProxy"};
}

PermissionManagerProxy::PermissionManagerProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IPermissionManager>(impl)
{}

PermissionManagerProxy::~PermissionManagerProxy()
{}

int PermissionManagerProxy::VerifyPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return PERMISSION_NOT_GRANTED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::VERIFY_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return PERMISSION_NOT_GRANTED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

bool PermissionManagerProxy::CanRequestPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return false;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return false;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return false;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::CAN_REQUEST_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return false;
    }

    bool result = reply.ReadBool();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::GrantUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::GRANT_USER_GRANTED_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::GrantSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::GRANT_SYSTEM_GRANTED_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::RevokeUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::REVOKE_USER_GRANTED_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::RevokeSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult =
        remote->SendRequest(static_cast<uint32_t>(IPermissionManager::InterfaceCode::REVOKE_SYSTEM_GRANTED_PERMISSION),
            data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::AddUserGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteStringVector(permList)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permList", __func__);
        return RET_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult =
        remote->SendRequest(static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_USER_GRANTED_REQ_PERMISSIONS),
            data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::AddSystemGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteStringVector(permList)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permList", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_SYSTEM_GRANTED_REQ_PERMISSIONS), data, reply,
        option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }
    if (!data.WriteInt32(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write userId", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_USER_GRANTED_REQ_PERMISSIONS), data, reply,
        option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::RemoveSystemGrantedReqPermissions(const std::string& bundleName)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_SYSTEM_GRANTED_REQ_PERMISSIONS), data, reply,
        option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::AddDefPermissions(const std::vector<PermissionDefParcel>& permDefList)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteInt32(permDefList.size())) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permDefList's size", __func__);
        return RET_FAILED;
    }
    for (auto permissionDef : permDefList) {
        if (!data.WriteParcelable(&permissionDef)) {
            return RET_FAILED;
        }
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_DEF_PERMISSIONS), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::RemoveDefPermissions(const std::string& bundleName)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write bundleName", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_DEF_PERMISSIONS), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }

    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int PermissionManagerProxy::GetDefPermission(
    const std::string& permissionName, PermissionDefParcel& permissionDefResult)
{
    MessageParcel data;
    data.WriteInterfaceToken(IPermissionManager::GetDescriptor());
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: Failed to write permissionName", __func__);
        return RET_FAILED;
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: remote service null.", __func__);
        return RET_FAILED;
    }
    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(IPermissionManager::InterfaceCode::GET_DEF_PERMISSION), data, reply, option);
    if (requestResult != NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s send request fail, result: %{public}d", __func__, requestResult);
        return RET_FAILED;
    }
    sptr<PermissionDefParcel> resultSptr = reply.ReadParcelable<PermissionDefParcel>();
    if (resultSptr == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s ReadParcelable fail", __func__);
        return RET_FAILED;
    }
    permissionDefResult = *resultSptr;
    int32_t result = reply.ReadInt32();
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS