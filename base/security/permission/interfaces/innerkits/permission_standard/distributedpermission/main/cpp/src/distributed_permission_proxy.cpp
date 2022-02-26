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

#include "distributed_permission_proxy.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionProxy"};
}
const int32_t PERMISSION_NOT_GRANTED = -1;
const int32_t ERROR = -1;

DistributedPermissionProxy::DistributedPermissionProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IDistributedPermission>(object)
{}

DistributedPermissionProxy::~DistributedPermissionProxy()
{}

int32_t DistributedPermissionProxy::AllocateDuid(const std::string &nodeId, const int32_t rUid)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "nodeId = %{public}s, rUid = %{public}d", Constant::EncryptDevId(nodeId).c_str(), rUid);
    MessageParcel data;
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(rUid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(rUid).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::ALLOCATE_DUID, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::QueryDuid(const std::string &deviceId, const int32_t rUid)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(
        LABEL, "deviceId = %{public}s, rUid = %{public}d", Constant::EncryptDevId(deviceId).c_str(), rUid);
    MessageParcel data;
    if (!data.WriteString(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(deviceId).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(rUid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(rUid).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::QUERY_DUID, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::CheckDPermission(int32_t dUid, const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "dUid = %{public}d, permissionName = %{public}s", dUid, permissionName.c_str());
    MessageParcel data;
    if (!data.WriteInt32(dUid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(dUid).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::DISTRIBUTED_CHECK_D_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}
int32_t DistributedPermissionProxy::CheckPermission(
    const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s, pid = %{public}d, uid = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        pid,
        uid);
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(pid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(pid).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::DISTRIBUTED_CHECK_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::CheckSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CHECK_SELF_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::CheckCallingPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CHECK_CALLING_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::CheckCallingOrSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CHECK_CALLING_OR_SELF_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

int32_t DistributedPermissionProxy::CheckCallerPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CHECK_CALLER_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

bool DistributedPermissionProxy::IsRestrictedPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return false;
    }
    MessageParcel reply;
    bool result = false;
    bool ret = SendRequest(IDistributedPermission::MessageCode::IS_RESTRICTED_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadBool();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}
int32_t DistributedPermissionProxy::VerifyPermissionFromRemote(
    const std::string &permission, const std::string &nodeId, int32_t pid, int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permission = %{public}s, nodeId = %{public}s, pid = %{public}d, uid = %{public}d",
        permission.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        pid,
        uid);
    MessageParcel data;
    if (!data.WriteString(permission)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(pid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(pid).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::VERIFY_PERMISSION_FROM_REMOTE, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}
int32_t DistributedPermissionProxy::VerifySelfPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::VERIFY_SELF_PERMISSION_FROM_REMOTE, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}
bool DistributedPermissionProxy::CanRequestPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return false;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return false;
    }
    MessageParcel reply;
    bool result = false;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CAN_REQUEST_PERMISSION_FROM_REMOTE, data, reply);
    if (ret) {
        result = reply.ReadBool();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}
void DistributedPermissionProxy::RequestPermissionsFromRemote(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "bundleName = %{public}s, nodeId = %{public}s,",
        bundleName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    MessageParcel data;
    if (!data.WriteStringVector(permissions)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteStringVector(permissions).");
        return;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WirteRomoteObject(callback->AsObject()).");
        return;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return;
    }
    if (!data.WriteString(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(bundleName).");
        return;
    }
    if (!data.WriteInt32(reasonResId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(reasonResId).");
        return;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::REQUEST_PERMISSION_FROM_REMOTE, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
}

void DistributedPermissionProxy::GrantSensitivePermissionToRemoteApp(
    const std::string &permissionName, const std::string &nodeId, int32_t ruid)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s, ruid = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        ruid);
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return;
    }
    if (!data.WriteString(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(nodeId).");
        return;
    }
    if (!data.WriteInt32(ruid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(ruid).");
        return;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::GRANT_SENSITIVE_PERMISSION_TO_REMOTEAPP, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
}

/**
 * Register using permission reminder to dpms at ohos side.
 *
 * @param callback instance of {@link ohos.security.permission.OnUsingPermissionReminder}.
 * @return result code indicates callback if register successfully.
 */
int32_t DistributedPermissionProxy::RegisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "callback = OnUsingPermissionReminder");

    MessageParcel data;
    if (!data.WriteRemoteObject(callback->AsObject())) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WirteRomoteObject(callback->AsObject()).");
        return PERMISSION_NOT_GRANTED;
    }

    MessageParcel reply;
    int32_t result = ERROR;

    bool ret = SendRequest(IDistributedPermission::MessageCode::REGISTER_USINH_PERMISSION_REMINDER, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

/**
 * unregister using permission reminder to dpms at ohos side.
 *
 * @param callback instance of {@link ohos.security.permission.OnUsingPermissionReminder} has been registered.
 * @return result code indicates callback if unregister successfully.
 */
int32_t DistributedPermissionProxy::UnregisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "callback = OnUsingPermissionReminder");
    MessageParcel data;
    if (!data.WriteRemoteObject(callback->AsObject())) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WirteRomoteObject(callback->AsObject()).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::UNREGISTER_USINH_PERMISSION_REMINDER, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
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
int32_t DistributedPermissionProxy::CheckPermissionAndStartUsing(
    const std::string &permissionName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, pid = %{public}d, uid = %{public}d, deviceId = %{public}s",
        permissionName.c_str(),
        pid,
        uid,
        Constant::EncryptDevId(deviceId).c_str());
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(pid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(pid).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return PERMISSION_NOT_GRANTED;
    }
    if (!data.WriteString(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(deviceId).");
        return PERMISSION_NOT_GRANTED;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::CHECK_PERMISSION_AND_START_USING, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    return result;
}

/**
 * Used to permission using remind when app start using permission continuously.
 *
 * @param permName the permission name which app start using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 */
void DistributedPermissionProxy::StartUsingPermission(
    const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL,
        "permName = %{public}s, pid = %{public}d, uid = %{public}d, deviceId = %{public}s",
        permName.c_str(),
        pid,
        uid,
        Constant::EncryptDevId(deviceId).c_str());
    MessageParcel data;
    if (!data.WriteString(permName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permName).");
        return;
    }
    if (!data.WriteInt32(pid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(pid).");
        return;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return;
    }
    if (!data.WriteString(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(deviceId).");
        return;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::START_USING_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
}

/**
 * Used to permission using remind when app stop using permission continuously.
 *
 * @param permName the permission name which app stop using.
 * @param pid the pid from pid json string indicates app information
 * @param uid the uid from pid json string indicates app information
 * @param deviceId thd deviceId from pid json string indicates app information
 */
void DistributedPermissionProxy::StopUsingPermission(
    const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL,
        "permName = %{public}s, pid = %{public}d, uid = %{public}d, deviceId = %{public}s",
        permName.c_str(),
        pid,
        uid,
        Constant::EncryptDevId(deviceId).c_str());
    MessageParcel data;
    if (!data.WriteString(permName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permName).");
        return;
    }
    if (!data.WriteInt32(pid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(pid).");
        return;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return;
    }
    if (!data.WriteString(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(deviceId).");
        return;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::STOP_USING_PERMISSION, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
}

void DistributedPermissionProxy::AddPermissionsRecord(const std::string &permissionName, const std::string &deviceId,
    const int32_t uid, const int32_t sucCount, const int32_t failCount)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, deviceId = %{public}s, uid = %{public}d, sucCount = "
        "%{public}d, failCount = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        uid,
        sucCount,
        failCount);
    MessageParcel data;
    if (!data.WriteString(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(permissionName).");
        return;
    }
    if (!data.WriteString(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(deviceId).");
        return;
    }
    if (!data.WriteInt32(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(uid).");
        return;
    }
    if (!data.WriteInt32(sucCount)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(sucCount).");
        return;
    }
    if (!data.WriteInt32(failCount)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteInt32(failCount).");
        return;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::ADD_PERMISSION_RECORD, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "add result = %{public}d", result);
    }
}

int32_t DistributedPermissionProxy::GetPermissionRecords(
    const std::string &request, unsigned long &codeLen, unsigned long &zipLen, std::string &resultStr)
{
    PERMISSION_LOG_INFO(LABEL, "request = %{public}s, resultStr = %{public}s", request.c_str(), resultStr.c_str());
    MessageParcel data;
    if (!data.WriteString(request)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(request).");
        return false;
    }
    if (!data.WriteUint64(codeLen)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(codeLen).");
        return false;
    }
    if (!data.WriteUint64(zipLen)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(zipLen).");
        return false;
    }
    MessageParcel reply;
    int32_t result = 0;
    bool ret = SendRequest(IDistributedPermission::MessageCode::GET_PERMISSION_RECORDS, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
        resultStr = reply.ReadString();
        codeLen = reply.ReadUint64();
        zipLen = reply.ReadUint64();
    }
    return result;
}

int32_t DistributedPermissionProxy::GetPermissionRecords(const std::string &request, unsigned long &codeLen,
    unsigned long &zipLen, const sptr<OnPermissionUsedRecord> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "queryGzipStr = %{public}s, callback = OnPermissionUsedRecord", request.c_str());
    MessageParcel data;
    if (!data.WriteString(request)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteString(request).");
        return false;
    }
    if (!data.WriteUint64(codeLen)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(codeLen).");
        return false;
    }
    if (!data.WriteUint64(zipLen)) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteUint64(zipLen).");
        return false;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        PERMISSION_LOG_ERROR(LABEL, "failed to WriteRemoteObject(callback).");
        return false;
    }
    MessageParcel reply;
    int32_t result = ERROR;
    bool ret = SendRequest(IDistributedPermission::MessageCode::GET_PERMISSION_RECORDS_ASYNCH, data, reply);
    if (ret) {
        result = reply.ReadInt32();
        PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    }
    PERMISSION_LOG_INFO(LABEL, "result = %{public}d", result);
    return result;
}

bool DistributedPermissionProxy::SendRequest(
    IDistributedPermission::MessageCode code, MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "failed to get remote.");
        return false;
    }
    MessageOption option(MessageOption::TF_SYNC);
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        PERMISSION_LOG_ERROR(LABEL, "failed to SendRequest.result = %{public}d", result);
        return false;
    }
    return true;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS