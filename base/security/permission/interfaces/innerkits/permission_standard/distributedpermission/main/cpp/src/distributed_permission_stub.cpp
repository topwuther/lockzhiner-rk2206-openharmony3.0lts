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

#include "distributed_permission_stub.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionStub"};
}
const int32_t ERROR = -1;
DistributedPermissionStub::DistributedPermissionStub()
{}

DistributedPermissionStub::~DistributedPermissionStub()
{}

int32_t DistributedPermissionStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PERMISSION_LOG_INFO(LABEL, "code = %{public}d", code);

    switch (code) {
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::ALLOCATE_DUID): {
            std::string nodeId = data.ReadString();
            int32_t rUid = data.ReadInt32();
            int32_t ret = AllocateDuid(nodeId, rUid);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::QUERY_DUID): {
            std::string deviceId = data.ReadString();
            int32_t rUid = data.ReadInt32();
            int32_t ret = QueryDuid(deviceId, rUid);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::DISTRIBUTED_CHECK_D_PERMISSION): {
            int32_t dUid = data.ReadInt32();
            std::string permissionName = data.ReadString();
            int32_t ret = CheckDPermission(dUid, permissionName);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::DISTRIBUTED_CHECK_PERMISSION): {
            std::string permissionName = data.ReadString();
            std::string nodeId = data.ReadString();
            int32_t pid = data.ReadInt32();
            int32_t uid = data.ReadInt32();
            int32_t ret = CheckPermission(permissionName, nodeId, pid, uid);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CHECK_SELF_PERMISSION): {
            std::string permissionName = data.ReadString();
            int32_t ret = CheckSelfPermission(permissionName);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CHECK_CALLING_PERMISSION): {
            std::string permissionName = data.ReadString();
            int32_t ret = CheckCallingPermission(permissionName);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CHECK_CALLING_OR_SELF_PERMISSION): {
            std::string permissionName = data.ReadString();
            int32_t ret = CheckCallingOrSelfPermission(permissionName);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CHECK_CALLER_PERMISSION): {
            std::string permissionName = data.ReadString();
            int32_t ret = CheckCallerPermission(permissionName);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::IS_RESTRICTED_PERMISSION): {
            std::string permissionName = data.ReadString();
            bool ret = IsRestrictedPermission(permissionName);
            if (!reply.WriteBool(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteBool(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::VERIFY_PERMISSION_FROM_REMOTE): {
            std::string permission = data.ReadString();
            std::string nodeId = data.ReadString();
            int32_t pid = data.ReadInt32();
            int32_t uid = data.ReadInt32();
            int32_t ret = VerifyPermissionFromRemote(permission, nodeId, pid, uid);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::VERIFY_SELF_PERMISSION_FROM_REMOTE): {
            std::string permissionName = data.ReadString();
            std::string nodeId = data.ReadString();
            int32_t ret = VerifySelfPermissionFromRemote(permissionName, nodeId);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CAN_REQUEST_PERMISSION_FROM_REMOTE): {
            std::string permissionName = data.ReadString();
            std::string nodeId = data.ReadString();
            bool ret = CanRequestPermissionFromRemote(permissionName, nodeId);
            if (!reply.WriteBool(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteBool(ret)");
                return false;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::REQUEST_PERMISSION_FROM_REMOTE): {
            std::vector<std::string> permissions;
            data.ReadStringVector(&permissions);
            sptr<OnRequestPermissionsResult> callback = iface_cast<OnRequestPermissionsResult>(data.ReadRemoteObject());
            if (callback == nullptr) {
                return ERROR;
            }
            std::string nodeId = data.ReadString();
            std::string bundleName = data.ReadString();
            int32_t reasonResId = data.ReadInt32();
            RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
            reply.WriteInt32(0);
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::GRANT_SENSITIVE_PERMISSION_TO_REMOTEAPP): {
            std::string permissionName = data.ReadString();
            std::string nodeId = data.ReadString();
            int32_t ruid = data.ReadInt32();
            GrantSensitivePermissionToRemoteApp(permissionName, nodeId, ruid);
            reply.WriteInt32(0);
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::REGISTER_USINH_PERMISSION_REMINDER): {
            sptr<OnUsingPermissionReminder> callback = iface_cast<OnUsingPermissionReminder>(data.ReadRemoteObject());
            if (callback == nullptr) {
                return ERROR;
            }
            int32_t ret = RegisterUsingPermissionReminder(callback);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::UNREGISTER_USINH_PERMISSION_REMINDER): {
            sptr<OnUsingPermissionReminder> callback = iface_cast<OnUsingPermissionReminder>(data.ReadRemoteObject());
            if (callback == nullptr) {
                return ERROR;
            }
            int32_t ret = UnregisterUsingPermissionReminder(callback);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::CHECK_PERMISSION_AND_START_USING): {
            std::string permissionName = data.ReadString();
            int32_t pid = data.ReadInt32();
            int32_t uid = data.ReadInt32();
            std::string deviceId = data.ReadString();
            int32_t ret = CheckPermissionAndStartUsing(permissionName, pid, uid, deviceId);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::START_USING_PERMISSION): {
            std::string permName = data.ReadString();
            int32_t pid = data.ReadInt32();
            int32_t uid = data.ReadInt32();
            std::string deviceId = data.ReadString();
            StartUsingPermission(permName, pid, uid, deviceId);
            reply.WriteInt32(0);
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::STOP_USING_PERMISSION): {
            std::string permName = data.ReadString();
            int32_t pid = data.ReadInt32();
            int32_t uid = data.ReadInt32();
            std::string deviceId = data.ReadString();
            StopUsingPermission(permName, pid, uid, deviceId);
            reply.WriteInt32(0);
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::ADD_PERMISSION_RECORD): {
            std::string permissionName = data.ReadString();
            std::string deviceId = data.ReadString();
            int32_t uid = data.ReadInt32();
            int32_t sucCount = data.ReadInt32();
            int32_t failCount = data.ReadInt32();
            AddPermissionsRecord(permissionName, deviceId, uid, sucCount, failCount);
            reply.WriteInt32(0);
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::GET_PERMISSION_RECORDS): {
            std::string queryGzipStr = data.ReadString();
            unsigned long codeLen = data.ReadUint64();
            unsigned long zipLen = data.ReadUint64();
            std::string resultStr;
            int ret = GetPermissionRecords(queryGzipStr, codeLen, zipLen, resultStr);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            if (!reply.WriteString(resultStr)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(resultStr)");
                return ERROR;
            }
            if (!reply.WriteUint64(codeLen)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(codeLen)");
                return ERROR;
            }
            if (!reply.WriteUint64(zipLen)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(zipLen)");
                return ERROR;
            }
            break;
        }
        case static_cast<uint32_t>(IDistributedPermission::MessageCode::GET_PERMISSION_RECORDS_ASYNCH): {
            std::string queryGzipStr = data.ReadString();
            unsigned long codeLen = data.ReadUint64();
            unsigned long zipLen = data.ReadUint64();
            sptr<OnPermissionUsedRecord> callback = iface_cast<OnPermissionUsedRecord>(data.ReadRemoteObject());
            if (callback == nullptr) {
                return ERROR;
            }
            int32_t ret = GetPermissionRecords(queryGzipStr, codeLen, zipLen, callback);
            if (!reply.WriteInt32(ret)) {
                PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ret)");
                return ERROR;
            }
            break;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS