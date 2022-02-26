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

#ifndef PERMISSION_STANDARD_PERMISSION_STATE_CACHE_H
#define PERMISSION_STANDARD_PERMISSION_STATE_CACHE_H

#include <string>
#include <map>
#include <vector>

#include "permission_req.h"
#include "generic_values.h"
#include "data_translator.h"
#include "field_const.h"

#include "rwlock.h"

namespace OHOS {
namespace Security {
namespace Permission {
static const int DEFAULT_PERMISSION_FLAGS = 0;
class BundleInfo final {
public:
    std::string bundleName_;
    BundleInfo() = default;
    virtual ~BundleInfo() = default;
    explicit BundleInfo(std::string bundleName);
    int AddReqPermission(const std::string& permissionName);
    int ReplaceReqPermission(const PermissionReq& permissionReq);
    PermissionReq& GetReqPermission(const std::string& permissionName);
    void RemoveReqPermissions();
    bool HasReqPermission(const std::string& permissionName) const;
    int UpdateGrantedState(const std::string& permissionName, bool isGranted);
    int UpdatePermissionFlags(const std::string& permissionName, int flags);
    bool IsGranted(const std::string& permissionName);
    int GetPermissionFlags(const std::string& permissionName);
    void GetAllPermissionReq(std::vector<PermissionReq>& permissionReqList) const;
    std::string ToString() const;

private:
    std::map<std::string, PermissionReq> permissionRequests_;
};

class UserInfo final {
public:
    int userId_;
    UserInfo() : userId_(0)
    {}
    virtual ~UserInfo() = default;
    explicit UserInfo(int userId);
    bool HasBundleInfo(const std::string& bundleName) const;
    BundleInfo& GetOrCreateBundleInfo(const std::string& bundleName);
    int ReplaceReqPermission(const std::string& bundleName, const PermissionReq& permissionReq);
    int RemoveReqPermission(const std::string& bundleName);
    int UpdatePermissionState(const std::string& bundleName, const std::string& permissionName, bool isGranted);
    int UpdatePermissionFlags(const std::string& bundleName, const std::string& permissionName, int flags);
    bool IsGranted(const std::string& bundleName, const std::string& permissionName);
    int GetPermissionFlags(const std::string& bundleName, const std::string& permissionName);
    void GetAllBundleInfo(std::vector<BundleInfo>& bundleInfoList) const;
    std::string ToString() const;

private:
    std::map<std::string, BundleInfo> bundleInfos_;
};

class Root final {
public:
    static Root& GetRoot();
    bool HasUserInfo(int userId) const;
    UserInfo& GetOrCreateUserInfo(int userId);
    void GetUserGrantedUserInfo(std::vector<UserInfo>& userInfoList) const;
    std::string ToString() const;

private:
    std::map<int, UserInfo> userInfos_;
};

class PermissionStateCache final {
public:
    static const int SYSTEM_GRANTED_USER_ID = -1;
    static PermissionStateCache& GetInstance();
    virtual ~PermissionStateCache() = default;
    std::string ToString();
    int AddReqPermission(int userId, const std::string& bundleName, const std::string& permissionName);
    int AddReqPermission(int userId, const std::string& bundleName, const std::vector<std::string>& permList);
    int ReplaceReqPermission(int userId, const std::string& bundleName, const PermissionReq& permissionReq);
    int RemoveReqPermission(int userId, const std::string& bundleName);
    int UpdatePermissionState(
        int userId, const std::string& bundleName, const std::string& permissionName, bool isGranted);
    int UpdatePermissionFlags(int userId, const std::string& bundleName, const std::string& permissionName, int flags);
    bool IsGranted(int userId, const std::string& bundleName, const std::string& permissionName);
    int GetPermissionFlags(int userId, const std::string& bundleName, const std::string& permissionName);
    void QueryCurrentCache(bool isSystemGranted, std::vector<GenericValues>& valueList);

private:
    PermissionStateCache() = default;

    DISALLOW_COPY_AND_MOVE(PermissionStateCache);

    void QueryForUser(const UserInfo& userInfo, std::vector<GenericValues>& valueList) const;

    void QueryForBundle(const int userId, const BundleInfo& bundleInfo, std::vector<GenericValues>& valueList) const;

    OHOS::Utils::RWLock cacheLock_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_STANDARD_PERMISSION_STATE_CACHE_H
