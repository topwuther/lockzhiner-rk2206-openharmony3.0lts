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

#include "permission_state_cache.h"

#include <iostream>

#include "permission.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionStateCache"};
}
int PermissionStateCache::AddReqPermission(
    const int userId, const std::string& bundleName, const std::string& permissionName)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    BundleInfo& bundleInfo = Root::GetRoot().GetOrCreateUserInfo(userId).GetOrCreateBundleInfo(bundleName);
    return bundleInfo.AddReqPermission(permissionName);
}

int PermissionStateCache::AddReqPermission(
    const int userId, const std::string& bundleName, const std::vector<std::string>& permList)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    BundleInfo& bundleInfo = Root::GetRoot().GetOrCreateUserInfo(userId).GetOrCreateBundleInfo(bundleName);
    for (const auto& permissionName : permList) {
        bundleInfo.AddReqPermission(permissionName);
    }
    return RET_SUCCESS;
}

int PermissionStateCache::ReplaceReqPermission(
    int userId, const std::string& bundleName, const PermissionReq& permissionReq)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    return Root::GetRoot().GetOrCreateUserInfo(userId).ReplaceReqPermission(bundleName, permissionReq);
}

int PermissionStateCache::RemoveReqPermission(const int userId, const std::string& bundleName)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    if (!Root::GetRoot().HasUserInfo(userId)) {
        return RET_FAILED;
    }
    return Root::GetRoot().GetOrCreateUserInfo(userId).RemoveReqPermission(bundleName);
}

int PermissionStateCache::UpdatePermissionState(
    int userId, const std::string& bundleName, const std::string& permissionName, bool isGranted)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    if (!Root::GetRoot().HasUserInfo(userId)) {
        return RET_FAILED;
    }
    return Root::GetRoot().GetOrCreateUserInfo(userId).UpdatePermissionState(bundleName, permissionName, isGranted);
}

int PermissionStateCache::UpdatePermissionFlags(
    int userId, const std::string& bundleName, const std::string& permissionName, int flags)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    if (!Root::GetRoot().HasUserInfo(userId)) {
        return RET_FAILED;
    }
    return Root::GetRoot().GetOrCreateUserInfo(userId).UpdatePermissionFlags(bundleName, permissionName, flags);
}

bool PermissionStateCache::IsGranted(const int userId, const std::string& bundleName, const std::string& permissionName)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    if (!Root::GetRoot().HasUserInfo(userId)) {
        return false;
    }
    return Root::GetRoot().GetOrCreateUserInfo(userId).IsGranted(bundleName, permissionName);
}

int PermissionStateCache::GetPermissionFlags(
    const int userId, const std::string& bundleName, const std::string& permissionName)
{
    if (!Root::GetRoot().HasUserInfo(userId)) {
        return DEFAULT_PERMISSION_FLAGS;
    }
    return Root::GetRoot().GetOrCreateUserInfo(userId).GetPermissionFlags(bundleName, permissionName);
}

std::string PermissionStateCache::ToString()
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    return Root::GetRoot().ToString();
}

void PermissionStateCache::QueryCurrentCache(bool isSystemGranted, std::vector<GenericValues>& valueList)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);

    if (isSystemGranted) {
        if (!Root::GetRoot().HasUserInfo(SYSTEM_GRANTED_USER_ID)) {
            return;
        }
        UserInfo userInfo = Root::GetRoot().GetOrCreateUserInfo(SYSTEM_GRANTED_USER_ID);
        QueryForUser(userInfo, valueList);
        return;
    }

    std::vector<UserInfo> userInfoList;
    Root::GetRoot().GetUserGrantedUserInfo(userInfoList);
    for (auto userInfo : userInfoList) {
        QueryForUser(userInfo, valueList);
    }
}

void PermissionStateCache::QueryForUser(const UserInfo& userInfo, std::vector<GenericValues>& valueList) const
{
    std::vector<BundleInfo> bundleInfoList;
    userInfo.GetAllBundleInfo(bundleInfoList);
    for (auto bundleInfo : bundleInfoList) {
        QueryForBundle(userInfo.userId_, bundleInfo, valueList);
    }
}

void PermissionStateCache::QueryForBundle(
    const int userId, const BundleInfo& bundleInfo, std::vector<GenericValues>& valueList) const
{
    std::vector<PermissionReq> permissionReqList;
    bundleInfo.GetAllPermissionReq(permissionReqList);
    for (auto permissionReq : permissionReqList) {
        GenericValues genericValues;
        genericValues.Put(FIELD_BUNDLE_NAME, bundleInfo.bundleName_);
        DataTranslator::TranslationIntoGenericValues(permissionReq, genericValues);
        if (userId == SYSTEM_GRANTED_USER_ID) {
            valueList.emplace_back(genericValues);
        } else {
            genericValues.Put(FIELD_USER_ID, userId);
            valueList.emplace_back(genericValues);
        }
    }
}

PermissionStateCache& PermissionStateCache::GetInstance()
{
    static PermissionStateCache instance;
    return instance;
}

Root& Root::GetRoot()
{
    static Root root;
    return root;
}

bool Root::HasUserInfo(int userId) const
{
    return userInfos_.count(userId) == 1;
}

UserInfo& Root::GetOrCreateUserInfo(const int userId)
{
    if (!HasUserInfo(userId)) {
        UserInfo userInfo(userId);
        userInfos_[userId] = userInfo;
        PERMISSION_LOG_DEBUG(
            LABEL, "%{public}s: userInfos_[userId]: %{public}s", __func__, userInfos_[userId].ToString().c_str());
    }
    return userInfos_[userId];
}

void Root::GetUserGrantedUserInfo(std::vector<UserInfo>& bundleInfoList) const
{
    for (auto it = userInfos_.begin(); it != userInfos_.end(); ++it) {
        if (it->first != PermissionStateCache::SYSTEM_GRANTED_USER_ID) {
            bundleInfoList.emplace_back(it->second);
        }
    }
}

std::string Root::ToString() const
{
    std::string infos = R"({"Root": "Default", "UserInfos": [)";
    for (auto userInfo : userInfos_) {
        infos.append(userInfo.second.ToString() + ", ");
    }
    infos = infos + "]}";
    return infos;
}

UserInfo::UserInfo(int userId)
{
    userId_ = userId;
}

bool UserInfo::HasBundleInfo(const std::string& bundleName) const
{
    return bundleInfos_.count(bundleName) == 1;
}

BundleInfo& UserInfo::GetOrCreateBundleInfo(const std::string& bundleName)
{
    if (!HasBundleInfo(bundleName)) {
        BundleInfo bundleInfo(bundleName);
        bundleInfos_[bundleName] = bundleInfo;
    }
    return bundleInfos_[bundleName];
}

void UserInfo::GetAllBundleInfo(std::vector<BundleInfo>& bundleInfoList) const
{
    for (auto it = bundleInfos_.begin(); it != bundleInfos_.end(); ++it) {
        bundleInfoList.emplace_back(it->second);
    }
}

std::string UserInfo::ToString() const
{
    std::string infos = R"({"userId": )" + std::to_string(userId_) + R"(, "BundleInfos": [)";
    for (auto bundleInfo : bundleInfos_) {
        infos.append(bundleInfo.second.ToString() + ", ");
    }
    infos.append("]}");
    return infos;
}

int UserInfo::ReplaceReqPermission(const std::string& bundleName, const PermissionReq& permissionReq)
{
    return GetOrCreateBundleInfo(bundleName).ReplaceReqPermission(permissionReq);
}

int UserInfo::RemoveReqPermission(const std::string& bundleName)
{
    if (!HasBundleInfo(bundleName)) {
        return RET_SUCCESS;
    }
    bundleInfos_[bundleName].RemoveReqPermissions();
    return RET_SUCCESS;
}

int UserInfo::UpdatePermissionState(const std::string& bundleName, const std::string& permissionName, bool isGranted)
{
    if (!HasBundleInfo(bundleName)) {
        return RET_FAILED;
    }
    return bundleInfos_[bundleName].UpdateGrantedState(permissionName, isGranted);
}

int UserInfo::UpdatePermissionFlags(const std::string& bundleName, const std::string& permissionName, int flags)
{
    if (!HasBundleInfo(bundleName)) {
        return RET_FAILED;
    }
    return bundleInfos_[bundleName].UpdatePermissionFlags(permissionName, flags);
}

bool UserInfo::IsGranted(const std::string& bundleName, const std::string& permissionName)
{
    if (!HasBundleInfo(bundleName)) {
        return false;
    }
    return bundleInfos_[bundleName].IsGranted(permissionName);
}

int UserInfo::GetPermissionFlags(const std::string& bundleName, const std::string& permissionName)
{
    if (!HasBundleInfo(bundleName)) {
        return DEFAULT_PERMISSION_FLAGS;
    }
    return bundleInfos_[bundleName].GetPermissionFlags(permissionName);
}

BundleInfo::BundleInfo(std::string bundleName)
{
    bundleName_ = std::move(bundleName);
}

int BundleInfo::AddReqPermission(const std::string& permissionName)
{
    if (HasReqPermission(permissionName)) {
        PERMISSION_LOG_DEBUG(
            LABEL, "%{public}s failed, reqPermission is exist, name: %{public}s", __func__, permissionName.c_str());
        return RET_FAILED;
    }
    PermissionReq permissionReq = {permissionName, false, DEFAULT_PERMISSION_FLAGS};
    permissionRequests_[permissionName] = permissionReq;
    return RET_SUCCESS;
}

int BundleInfo::ReplaceReqPermission(const PermissionReq& permissionReq)
{
    permissionRequests_[permissionReq.reqPermissionName] = permissionReq;
    return RET_SUCCESS;
}

PermissionReq& BundleInfo::GetReqPermission(const std::string& permissionName)
{
    return permissionRequests_[permissionName];
}

bool BundleInfo::HasReqPermission(const std::string& permissionName) const
{
    return permissionRequests_.count(permissionName) == 1;
}

void BundleInfo::RemoveReqPermissions()
{
    permissionRequests_.clear();
}

int BundleInfo::UpdateGrantedState(const std::string& permissionName, bool isGranted)
{
    if (!HasReqPermission(permissionName)) {
        return RET_FAILED;
    }
    GetReqPermission(permissionName).isGranted = isGranted;
    return RET_SUCCESS;
}

int BundleInfo::UpdatePermissionFlags(const std::string& permissionName, int flags)
{
    if (!HasReqPermission(permissionName)) {
        return RET_FAILED;
    }
    GetReqPermission(permissionName).flags = flags;
    return RET_SUCCESS;
}

bool BundleInfo::IsGranted(const std::string& permissionName)
{
    if (!HasReqPermission(permissionName)) {
        return false;
    }
    return GetReqPermission(permissionName).isGranted;
}

int BundleInfo::GetPermissionFlags(const std::string& permissionName)
{
    if (!HasReqPermission(permissionName)) {
        return DEFAULT_PERMISSION_FLAGS;
    }
    return GetReqPermission(permissionName).flags;
}

void BundleInfo::GetAllPermissionReq(std::vector<PermissionReq>& permissionReqList) const
{
    for (auto it = permissionRequests_.begin(); it != permissionRequests_.end(); ++it) {
        permissionReqList.emplace_back(it->second);
    }
}

std::string BundleInfo::ToString() const
{
    std::string infos = R"({"bundleName": ")" + bundleName_ + R"(", "ReqPermissions": [)";
    for (const auto& request : permissionRequests_) {
        PermissionReq permissionReq = request.second;
        infos.append(R"({"permissionName": ")" + permissionReq.reqPermissionName + R"(")");
        infos.append(R"(, "isGranted": )" + std::to_string(permissionReq.isGranted));
        infos.append(R"(, "flags": )" + std::to_string(permissionReq.flags));
        infos.append("}, ");
    }
    infos = infos + "]}";
    return infos;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
