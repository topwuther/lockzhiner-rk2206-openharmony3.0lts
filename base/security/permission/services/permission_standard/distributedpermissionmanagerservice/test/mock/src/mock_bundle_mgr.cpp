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

#include "mock_bundle_mgr.h"
#include "application_info.h"

namespace OHOS {
namespace AppExecFwk {

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return true;
}
bool BundleMgrService::GetApplicationInfos(
    const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos)
{
    return true;
}
bool BundleMgrService::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo)
{
    bundleInfo.name = "bundleInfo.name";
    bundleInfo.label = "bundleInfo.label";
    bundleInfo.description = "bundleInfo.description";
    bundleInfo.vendor = "bundleInfo.vendor";
    bundleInfo.versionCode = 0;
    bundleInfo.versionName = "bundleInfo.versionName";
    bundleInfo.jointUserId = "bundleInfo.jointUserId";
    bundleInfo.minSdkVersion = -1;
    bundleInfo.maxSdkVersion = -1;
    bundleInfo.mainEntry = "bundleInfo.mainEntry";
    bundleInfo.cpuAbi = "bundleInfo.cpuAbi";
    bundleInfo.appId = "bundleInfo.appId";
    bundleInfo.compatibleVersion = 0;
    bundleInfo.targetVersion = 0;
    bundleInfo.releaseType = "bundleInfo.releaseType";
    bundleInfo.uid = -1;
    bundleInfo.gid = -1;
    bundleInfo.seInfo = "bundleInfo.seInfo";
    bundleInfo.entryModuleName = "bundleInfo.entryModuleName";
    bundleInfo.isKeepAlive = false;
    bundleInfo.isNativeApp = false;
    bundleInfo.isDifferentName = false;
    bundleInfo.installTime = 0;
    bundleInfo.updateTime = 0;
    ApplicationInfo applicationInfo;
    applicationInfo.name = "applicationInfo.name";
    applicationInfo.bundleName = "applicationInfo.bundleName";
    applicationInfo.description = "applicationInfo.description";
    applicationInfo.iconPath = "applicationInfo.iconPath";
    applicationInfo.label = "applicationInfo.label";
    applicationInfo.labelId = 0;
    applicationInfo.iconId = 0;
    applicationInfo.descriptionId = 0;
    applicationInfo.deviceId = "applicationInfo.deviceId";
    applicationInfo.signatureKey = "applicationInfo.signatureKey";
    applicationInfo.isSystemApp = false;
    applicationInfo.isLauncherApp = true;
    applicationInfo.supportedModes = 0;
    applicationInfo.process = "applicationInfo.process";
    applicationInfo.entryDir = "applicationInfo.entryDir";
    applicationInfo.codePath = "applicationInfo.codePath";
    applicationInfo.dataDir = "applicationInfo.dataDir";
    applicationInfo.dataBaseDir = "applicationInfo.dataBaseDir";
    applicationInfo.cacheDir = "applicationInfo.cacheDir";
    applicationInfo.flags = 0;
    applicationInfo.enabled = false;

    bundleInfo.applicationInfo = applicationInfo;
    std::vector<std::string> reqPermissions;
    if (bundleName == "bundleName1024") {
        for (int i = 1; i <= 1025; i++) {
            reqPermissions.push_back("reqPermissions" + std::to_string(i));
        }
    } else {
        for (int i = 1; i <= 5; i++) {
            reqPermissions.push_back("reqPermissions" + std::to_string(i));
        }
    }

    bundleInfo.reqPermissions = reqPermissions;
    std::vector<std::string> defPermissions;
    for (int i = 1; i <= 5; i++) {
        defPermissions.push_back("defPermissions" + std::to_string(i));
    }
    bundleInfo.defPermissions = defPermissions;
    return true;
}
bool BundleMgrService::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
int BundleMgrService::GetUidByBundleName(const std::string &bundleName, const int userId)
{
    return 0;
}
std::string BundleMgrService::GetAppIdByBundleName(const std::string &bundleName, const int userId)
{
    return "BundleName";
}
bool BundleMgrService::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    if (uid == 1024) {
        bundleName = "bundleName1024";
        return true;
    }
    if (uid >= 0) {
        bundleName = "bundleName";
        return true;
    }
    return false;
}
bool BundleMgrService::GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames)
{
    if (uid == 1024) {
        bundleNames.push_back("bundleName1024");
        return true;
    }
    if (uid >= 0) {
        bundleNames.push_back("bundleName");
        return true;
    }
    return false;
}
bool BundleMgrService::GetNameForUid(const int uid, std::string &name)
{
    return true;
}
bool BundleMgrService::GetBundleGids(const std::string &bundleName, std::vector<int> &gids)
{
    return true;
}
std::string BundleMgrService::GetAppType(const std::string &bundleName)
{
    return "launcher";
}
bool BundleMgrService::CheckIsSystemAppByUid(const int uid)
{
    return true;
}
bool BundleMgrService::GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
bool BundleMgrService::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    return true;
}
bool BundleMgrService::QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo)
{
    return true;
}
bool BundleMgrService::QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos)
{
    return true;
}
std::string BundleMgrService::GetAbilityLabel(const std::string &bundleName, const std::string &className)
{
    return "BundleName";
}
bool BundleMgrService::GetBundleArchiveInfo(
    const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo)
{
    return true;
}
bool BundleMgrService::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    return true;
}
bool BundleMgrService::GetLaunchWantForBundle(const std::string &bundleName, Want &want)
{
    return true;
}
int BundleMgrService::CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName)
{
    return 0;
}
int BundleMgrService::CheckPermission(const std::string &bundleName, const std::string &permission)
{
    return 0;
}
bool BundleMgrService::GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef)
{
    return true;
}
bool BundleMgrService::GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs)
{
    return true;
}
bool BundleMgrService::GetAppsGrantedPermissions(
    const std::vector<std::string> &permissions, std::vector<std::string> &appNames)
{
    return true;
}
bool BundleMgrService::HasSystemCapability(const std::string &capName)
{
    return true;
}
bool BundleMgrService::GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps)
{
    return true;
}
bool BundleMgrService::IsSafeMode()
{
    return true;
}
bool BundleMgrService::CleanBundleCacheFiles(
    const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback)
{
    return true;
}
bool BundleMgrService::CleanBundleDataFiles(const std::string &bundleName)
{
    return true;
}
bool BundleMgrService::RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrService::ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback)
{
    return true;
}
bool BundleMgrService::UnregisterBundleStatusCallback()
{
    return true;
}
bool BundleMgrService::DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result)
{
    return true;
}
bool BundleMgrService::IsApplicationEnabled(const std::string &bundleName)
{
    return true;
}
bool BundleMgrService::SetApplicationEnabled(const std::string &bundleName, bool isEnable)
{
    return true;
}
bool BundleMgrService::IsAbilityEnabled(const AbilityInfo &abilityInfo)
{
    return true;
}
bool BundleMgrService::SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled)
{
    return true;
}
std::string BundleMgrService::GetAbilityIcon(const std::string &bundleName, const std::string &className)
{
    return "BundleName";
}
bool BundleMgrService::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, const int userId)
{
    return true;
}
bool BundleMgrService::RequestPermissionFromUser(
    const std::string &bundleName, const std::string &permission, const int userId)
{
    return true;
}
bool BundleMgrService::RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrService::RegisterPermissionsChanged(
    const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}
bool BundleMgrService::UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback)
{
    return true;
}

sptr<IBundleInstaller> BundleMgrService::GetBundleInstaller()
{
    return nullptr;
}

bool BundleMgrService::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetFormsInfoByModule(
    const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    return true;
}
bool BundleMgrService::GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos)
{
    return true;
}

// bool BundleMgrService::GetModuleUsageRecords(const int32_t number, std::vector<ModuleUsageRecord>
// &moduleUsageRecords)
// {
//     return true;
// }

// bool BundleMgrService::NotifyActivityLifeStatus(
//     const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
// {
//     return true;
// }
// bool BundleMgrService::QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos)
// {
//     return true;
// }

}  // namespace AppExecFwk
}  // namespace OHOS
