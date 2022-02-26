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

#include "bundle_mgr_proxy.h"

#include "ipc_types.h"
#include "parcel.h"
#include "string_ex.h"

#include "appexecfwk_errors.h"
#include "bundle_constants.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrProxy::BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
{}

BundleMgrProxy::~BundleMgrProxy()
{}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return true;
}

bool BundleMgrProxy::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
{
    return true;
}

bool BundleMgrProxy::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}

bool BundleMgrProxy::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}

int BundleMgrProxy::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return 1;
}

std::string BundleMgrProxy::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    return "appId";
}

bool BundleMgrProxy::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    bundleName = "bundleName";
    return true;
}

bool BundleMgrProxy::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    return true;
}

bool BundleMgrProxy::GetNameForUid(const int uid, std::string &name)
{
    return true;
}

bool BundleMgrProxy::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return true;
}

std::string BundleMgrProxy::GetAppType(const std::string &bundleName)
{
    return "Constants::EMPTY_STRING";
}

bool BundleMgrProxy::CheckIsSystemAppByUid(const int uid)
{
    return true;
}

bool BundleMgrProxy::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}

bool BundleMgrProxy::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrProxy::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    return true;
}

std::string BundleMgrProxy::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    return "reply.ReadString()";
}

bool BundleMgrProxy::GetBundleArchiveInfo(const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    return true;
}

bool BundleMgrProxy::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    return true;
}

int BundleMgrProxy::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    return 1;
}

int BundleMgrProxy::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    return 1;
}

bool BundleMgrProxy::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    return true;
}

bool BundleMgrProxy::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return true;
}

bool BundleMgrProxy::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return true;
}

bool BundleMgrProxy::HasSystemCapability(const std::string &capName)
{
    return true;
}

bool BundleMgrProxy::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return true;
}

bool BundleMgrProxy::IsSafeMode()
{
    return true;
}

bool BundleMgrProxy::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
{
    return true;
}

bool BundleMgrProxy::CleanBundleDataFiles(const std::string &bundleName)
{
    return true;
}

bool BundleMgrProxy::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}

bool BundleMgrProxy::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}

bool BundleMgrProxy::UnregisterBundleStatusCallback()
{
    return true;
}

bool BundleMgrProxy::DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result)
{
    return true;
}

bool BundleMgrProxy::IsApplicationEnabled(const std::string &bundleName)
{
    return true;
}

bool BundleMgrProxy::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    return true;
}

bool BundleMgrProxy::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrProxy::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    return true;
}

std::string BundleMgrProxy::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    return "AbilityIcon";
}

bool BundleMgrProxy::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    return true;
}

bool BundleMgrProxy::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    return true;
}

bool BundleMgrProxy::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}

bool BundleMgrProxy::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}

bool BundleMgrProxy::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}

template<typename T>
bool BundleMgrProxy::GetParcelableInfo(IBundleMgr::Message code, MessageParcel &data, T &parcelableInfo)
{
    return true;
}

template<typename T>
bool BundleMgrProxy::GetParcelableInfos(IBundleMgr::Message code, MessageParcel &data, std::vector<T> &parcelableInfos)
{
    return true;
}

bool BundleMgrProxy::SendTransactCmd(IBundleMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
