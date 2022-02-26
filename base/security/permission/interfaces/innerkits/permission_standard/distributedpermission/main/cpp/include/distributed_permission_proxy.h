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

#ifndef PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_PROXY_H
#define PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_PROXY_H

#include "constant.h"
#include "idistributed_permission.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DistributedPermissionProxy : public IRemoteProxy<IDistributedPermission> {
public:
    explicit DistributedPermissionProxy(const sptr<IRemoteObject> &object);
    virtual ~DistributedPermissionProxy() override;

    virtual int32_t AllocateDuid(const std::string &nodeId, const int32_t rUid) override;

    virtual int32_t QueryDuid(const std::string &deviceId, const int32_t rUid) override;

    virtual int32_t CheckDPermission(int32_t dUid, const std::string &permissionName) override;

    virtual int32_t CheckPermission(
        const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid) override;

    virtual int32_t CheckSelfPermission(const std::string &permissionName) override;

    virtual int32_t CheckCallingPermission(const std::string &permissionName) override;

    virtual int32_t CheckCallingOrSelfPermission(const std::string &permissionName) override;

    virtual int32_t CheckCallerPermission(const std::string &permissionName) override;

    virtual bool IsRestrictedPermission(const std::string &permissionName) override;

    virtual int32_t VerifyPermissionFromRemote(
        const std::string &permission, const std::string &nodeId, int32_t pid, int32_t uid) override;

    virtual int32_t VerifySelfPermissionFromRemote(
        const std::string &permissionName, const std::string &nodeId) override;

    virtual bool CanRequestPermissionFromRemote(const std::string &permissionName, const std::string &nodeId) override;

    virtual void RequestPermissionsFromRemote(const std::vector<std::string> permissions,
        const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
        int32_t reasonResId) override;

    virtual void GrantSensitivePermissionToRemoteApp(
        const std::string &permissionName, const std::string &nodeId, int32_t ruid) override;

    virtual int32_t RegisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback) override;

    virtual int32_t UnregisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback) override;

    virtual int32_t CheckPermissionAndStartUsing(
        const std::string &permissionName, int32_t pid, int32_t uid, const std::string &deviceId) override;

    virtual void StartUsingPermission(
        const std::string &permName, int32_t pid, int32_t, const std::string &deviceId) override;

    virtual void StopUsingPermission(
        const std::string &permName, int32_t, int32_t, const std::string &deviceId) override;

    virtual void AddPermissionsRecord(const std::string &permissionName, const std::string &deviceId, const int32_t uid,
        const int32_t sucCount, const int32_t failCount) override;

    virtual int32_t GetPermissionRecords(const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen,
        std::string &resultStr) override;

    virtual int32_t GetPermissionRecords(const std::string &queryGzipStr, unsigned long &codeLen, unsigned long &zipLen,
        const sptr<OnPermissionUsedRecord> &callback) override;

private:
    bool SendRequest(IDistributedPermission::MessageCode code, MessageParcel &data, MessageParcel &reply);

private:
    static inline BrokerDelegator<DistributedPermissionProxy> delegator_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_PROXY_H