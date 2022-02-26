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

#ifndef PERMISSION_INNERKITS_INCLUDE_IDISRIBUTED_PERMISSION_H
#define PERMISSION_INNERKITS_INCLUDE_IDISRIBUTED_PERMISSION_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "on_permission_used_record.h"
#include "on_using_permission_reminder.h"
#include "on_request_permissions_result.h"
#include "query_permission_used_request.h"

namespace OHOS {
namespace Security {
namespace Permission {
class IDistributedPermission : public IRemoteBroker {
public:
    static const int32_t SA_ID_DISTRIBUTED_PERMISSION_MANAGER_SERVICE = 3502;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedpermission.IDistributedPermission");

    virtual int32_t AllocateDuid(const std::string &nodeId, const int32_t rUid) = 0;

    virtual int32_t QueryDuid(const std::string &deviceId, const int32_t rUid) = 0;

    virtual int32_t CheckDPermission(int32_t dUid, const std::string &permissionName) = 0;

    virtual int32_t CheckPermission(
        const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid) = 0;

    virtual int32_t CheckSelfPermission(const std::string &permissionName) = 0;

    virtual int32_t CheckCallingPermission(const std::string &permissionName) = 0;

    virtual int32_t CheckCallingOrSelfPermission(const std::string &permissionName) = 0;

    virtual int32_t CheckCallerPermission(const std::string &permissionName) = 0;

    virtual bool IsRestrictedPermission(const std::string &permissionName) = 0;

    virtual int32_t VerifyPermissionFromRemote(
        const std::string &permission, const std::string &nodeId, int32_t pid, int32_t uid) = 0;

    virtual int32_t VerifySelfPermissionFromRemote(const std::string &permissionName, const std::string &nodeId) = 0;

    virtual bool CanRequestPermissionFromRemote(const std::string &permissionName, const std::string &nodeId) = 0;

    virtual void RequestPermissionsFromRemote(const std::vector<std::string> permissions,
        const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
        int32_t reasonResId) = 0;
    virtual void GrantSensitivePermissionToRemoteApp(
        const std::string &permissionName, const std::string &nodeId, int32_t ruid) = 0;

    virtual int32_t RegisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback) = 0;

    virtual int32_t UnregisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback) = 0;

    virtual int32_t CheckPermissionAndStartUsing(
        const std::string &permissionName, int32_t pid, int32_t uid, const std::string &deviceId) = 0;

    virtual void StartUsingPermission(
        const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId) = 0;

    virtual void StopUsingPermission(
        const std::string &permName, int32_t pid, int32_t uid, const std::string &deviceId) = 0;

    virtual void AddPermissionsRecord(const std::string &permissionName, const std::string &deviceId, const int32_t uid,
        const int32_t sucCount, const int32_t failCount) = 0;

    virtual int32_t GetPermissionRecords(
        const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen, std::string &resultStr) = 0;

    virtual int32_t GetPermissionRecords(const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen,
        const sptr<OnPermissionUsedRecord> &callback) = 0;

    enum class MessageCode {
        ALLOCATE_DUID = 0xf001,
        QUERY_DUID = 0xf002,
        DISTRIBUTED_CHECK_D_PERMISSION = 0xf006,
        DISTRIBUTED_CHECK_PERMISSION = 0xf007,
        CHECK_SELF_PERMISSION = 0xf008,
        CHECK_CALLING_PERMISSION = 0xf009,
        CHECK_CALLING_OR_SELF_PERMISSION = 0xf010,
        CHECK_CALLER_PERMISSION = 0xf011,
        IS_RESTRICTED_PERMISSION = 0xf012,
        VERIFY_PERMISSION_FROM_REMOTE = 0xf013,
        VERIFY_SELF_PERMISSION_FROM_REMOTE = 0xf014,
        CAN_REQUEST_PERMISSION_FROM_REMOTE = 0xf015,
        GRANT_SENSITIVE_PERMISSION_TO_REMOTEAPP = 0xf016,
        REGISTER_USINH_PERMISSION_REMINDER = 0xf017,
        UNREGISTER_USINH_PERMISSION_REMINDER = 0xf018,
        CHECK_PERMISSION_AND_START_USING = 0xf019,
        START_USING_PERMISSION = 0xf020,
        STOP_USING_PERMISSION = 0xf021,
        CHECK_CALLER_PERMISSION_AND_STARTUSING = 0xf022,
        ADD_PERMISSION_RECORD = 0xf023,
        GET_PERMISSION_RECORDS = 0xf024,
        GET_PERMISSION_RECORDS_ASYNCH = 0xf025,
        REQUEST_PERMISSION_FROM_REMOTE = 0xf026,
    };
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_INNERKITS_INCLUDE_IDISRIBUTED_PERMISSION_H