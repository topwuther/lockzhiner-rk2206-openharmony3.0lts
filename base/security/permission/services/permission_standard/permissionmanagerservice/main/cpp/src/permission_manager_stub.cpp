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

#include "permission_manager_stub.h"

#include "permission.h"
#include "permission_log.h"

#include "ipc_skeleton.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionManagerStub"};
}

int32_t PermissionManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, code: %{public}d", __func__, code);
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != IPermissionManager::GetDescriptor()) {
        PERMISSION_LOG_ERROR(LABEL, "get unexpect descriptor: %{public}s", Str16ToStr8(descriptor).c_str());
        return RET_FAILED;
    }
    switch (code) {
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::VERIFY_PERMISSION):
            VerifyPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::CAN_REQUEST_PERMISSION):
            CanRequestPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::GRANT_USER_GRANTED_PERMISSION):
            GrantUserGrantedPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::GRANT_SYSTEM_GRANTED_PERMISSION):
            GrantSystemGrantedPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::REVOKE_USER_GRANTED_PERMISSION):
            RevokeUserGrantedPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::REVOKE_SYSTEM_GRANTED_PERMISSION):
            RevokeSystemGrantedPermissionInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_USER_GRANTED_REQ_PERMISSIONS):
            AddUserGrantedReqPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_SYSTEM_GRANTED_REQ_PERMISSIONS):
            AddSystemGrantedReqPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_USER_GRANTED_REQ_PERMISSIONS):
            RemoveUserGrantedReqPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_SYSTEM_GRANTED_REQ_PERMISSIONS):
            RemoveSystemGrantedReqPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::ADD_DEF_PERMISSIONS):
            AddDefPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::REMOVE_DEF_PERMISSIONS):
            RemoveDefPermissionsInner(data, reply);
            break;
        case static_cast<uint32_t>(IPermissionManager::InterfaceCode::GET_DEF_PERMISSION):
            GetDefPermissionInner(data, reply);
            break;
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}

void PermissionManagerStub::VerifyPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int userId = data.ReadInt32();
    int result = this->VerifyPermission(bundleName, permissionName, userId);
    reply.WriteInt32(result);
}

void PermissionManagerStub::CanRequestPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int userId = data.ReadInt32();
    int result = this->CanRequestPermission(bundleName, permissionName, userId);
    reply.WriteBool(result);
}

void PermissionManagerStub::GrantUserGrantedPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int userId = data.ReadInt32();
    int result = this->GrantUserGrantedPermission(bundleName, permissionName, userId);
    reply.WriteInt32(result);
}

void PermissionManagerStub::GrantSystemGrantedPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int result = this->GrantSystemGrantedPermission(bundleName, permissionName);
    reply.WriteInt32(result);
}

void PermissionManagerStub::RevokeUserGrantedPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int userId = data.ReadInt32();
    int result = this->RevokeUserGrantedPermission(bundleName, permissionName, userId);
    reply.WriteInt32(result);
}

void PermissionManagerStub::RevokeSystemGrantedPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::string permissionName = data.ReadString();
    int result = this->RevokeSystemGrantedPermission(bundleName, permissionName);
    reply.WriteInt32(result);
}

void PermissionManagerStub::AddUserGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::vector<std::string> permList;
    data.ReadStringVector(&permList);
    int userId = data.ReadInt32();
    int result = this->AddUserGrantedReqPermissions(bundleName, permList, userId);
    reply.WriteInt32(result);
}

void PermissionManagerStub::AddSystemGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    std::vector<std::string> permList;
    data.ReadStringVector(&permList);
    int result = this->AddSystemGrantedReqPermissions(bundleName, permList);
    reply.WriteInt32(result);
}

void PermissionManagerStub::RemoveUserGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    int userId = data.ReadInt32();
    int result = this->RemoveUserGrantedReqPermissions(bundleName, userId);
    reply.WriteInt32(result);
}

void PermissionManagerStub::RemoveSystemGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    int result = this->RemoveSystemGrantedReqPermissions(bundleName);
    reply.WriteInt32(result);
}

void PermissionManagerStub::AddDefPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    int size = 0;
    data.ReadInt32(size);
    std::vector<PermissionDefParcel> permList;
    for (int i = 0; i < size; i++) {
        sptr<PermissionDefParcel> permissionDef = data.ReadParcelable<PermissionDefParcel>();
        if (permissionDef != nullptr) {
            permList.emplace_back(*permissionDef);
        }
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, permList size: %{public}d", __func__, (int) permList.size());
    int result = this->AddDefPermissions(permList);
    reply.WriteInt32(result);
}

void PermissionManagerStub::RemoveDefPermissionsInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        reply.WriteInt32(RET_FAILED);
        return;
    }
    std::string bundleName = data.ReadString();
    int result = this->RemoveDefPermissions(bundleName);
    reply.WriteInt32(result);
}

void PermissionManagerStub::GetDefPermissionInner(MessageParcel& data, MessageParcel& reply)
{
    if (!IsAuthorizedCalling()) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s called, permission denied", __func__);
        return;
    }
    std::string permissionName = data.ReadString();
    PermissionDefParcel permissionDefParcel;
    int result = this->GetDefPermission(permissionName, permissionDefParcel);
    reply.WriteParcelable(&permissionDefParcel);
    reply.WriteInt32(result);
}

bool PermissionManagerStub::IsAuthorizedCalling() const
{
    int callingUid = IPCSkeleton::GetCallingUid();
    PERMISSION_LOG_INFO(LABEL, "Calling uid: %{public}d", callingUid);
    return callingUid == SYSTEM_UID || callingUid == ROOT_UID;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS