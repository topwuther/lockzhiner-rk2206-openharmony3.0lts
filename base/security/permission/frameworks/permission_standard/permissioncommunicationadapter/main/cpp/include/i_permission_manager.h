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

#ifndef I_PERMISSION_MANAGER_H
#define I_PERMISSION_MANAGER_H

#include <string>

#include "permission_def_parcel.h"

#include "iremote_broker.h"
#include "errors.h"

namespace OHOS {
namespace Security {
namespace Permission {
class IPermissionManager : public IRemoteBroker {
public:
    static const int SA_ID_PERMISSION_MANAGER_SERVICE = 3501;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.permission.IPermissionManager");

    virtual int VerifyPermission(const std::string& bundleName, const std::string& permissionName, int userId) = 0;

    virtual bool CanRequestPermission(const std::string& bundleName, const std::string& permissionName, int userId) = 0;

    virtual int GrantUserGrantedPermission(
        const std::string& bundleName, const std::string& permissionName, int userId) = 0;

    virtual int GrantSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) = 0;

    virtual int RevokeUserGrantedPermission(
        const std::string& bundleName, const std::string& permissionName, int userId) = 0;

    virtual int RevokeSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) = 0;

    virtual int AddUserGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList, int userId) = 0;

    virtual int AddSystemGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList) = 0;

    virtual int RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId) = 0;

    virtual int RemoveSystemGrantedReqPermissions(const std::string& bundleName) = 0;

    virtual int AddDefPermissions(const std::vector<PermissionDefParcel>& permDefList) = 0;

    virtual int RemoveDefPermissions(const std::string& bundleName) = 0;

    virtual int GetDefPermission(const std::string& permissionName, PermissionDefParcel& permissionDefResult) = 0;

    enum class InterfaceCode {
        VERIFY_PERMISSION = 0xff01,
        CAN_REQUEST_PERMISSION = 0xff02,
        GRANT_USER_GRANTED_PERMISSION = 0xff03,
        GRANT_SYSTEM_GRANTED_PERMISSION = 0xff04,
        REVOKE_USER_GRANTED_PERMISSION = 0xff05,
        REVOKE_SYSTEM_GRANTED_PERMISSION = 0xff06,
        ADD_USER_GRANTED_REQ_PERMISSIONS = 0xff07,
        ADD_SYSTEM_GRANTED_REQ_PERMISSIONS = 0xff08,
        REMOVE_USER_GRANTED_REQ_PERMISSIONS = 0xff09,
        REMOVE_SYSTEM_GRANTED_REQ_PERMISSIONS = 0xff10,
        ADD_DEF_PERMISSIONS = 0xff11,
        REMOVE_DEF_PERMISSIONS = 0xff12,
        GET_DEF_PERMISSION = 0xff13,
    };
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // I_PERMISSION_MANAGER_H
