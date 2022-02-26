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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H

#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "bundle_mgr_host.h"
#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "ohos/aafwk/content/want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockBundleMgrService : public BundleMgrHost {
public:
    MockBundleMgrService()
    {
        HILOG_ERROR("BundleMgrService::BundleMgrService");
    }

    ~MockBundleMgrService()
    {}

    /**
     * @brief Obtains the ApplicationInfo based on a given bundle name.
     * @param appName Indicates the application bundle name to be queried.
     * @param flag Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     * @return Returns true if the application is successfully obtained; returns false otherwise.
     */
    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override
    {
        return true;
    }
    /**
     * @brief Obtains information about all installed applications of a specified user.
     * @param flag Indicates the flag used to specify information contained
     *             in the ApplicationInfo objects that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfos Indicates all of the obtained ApplicationInfo objects.
     * @return Returns true if the application is successfully obtained; returns false otherwise.
     */
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) override
    {
        return true;
    }
    /**
     * @brief Obtains the BundleInfo based on a given bundle name.
     * @param bundleName Indicates the application bundle name to be queried.
     * @param flag Indicates the information contained in the BundleInfo object to be returned.
     * @param bundleInfo Indicates the obtained BundleInfo object.
     * @return Returns true if the BundleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    }
    /**
     * @brief Obtains BundleInfo of all bundles available in the system.
     * @param flag Indicates the flag used to specify information contained in the BundleInfo that will be returned.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    /**
     * @brief Obtains the application UID based on the given bundle name and user ID.
     * @param bundleName Indicates the bundle name of the application.
     * @param userId Indicates the user ID.
     * @return Returns the uid if successfully obtained; returns -1 otherwise.
     */
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) override
    {
        return 0;
    }
    /**
     * @brief Obtains the bundle name of a specified application based on the given UID.
     * @param uid Indicates the uid.
     * @param bundleName Indicates the obtained bundle name.
     * @return Returns true if the bundle name is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override
    {
        bundleName = "hello";
        return true;
    }
    /**
     * @brief Obtains an array of all group IDs associated with a specified bundle.
     * @param bundleName Indicates the bundle name.
     * @param gids Indicates the group IDs associated with the specified bundle.
     * @return Returns true if the gids is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) override
    {
        return true;
    }
    /**
     * @brief Obtains the type of a specified application based on the given bundle name.
     * @param bundleName Indicates the bundle name.
     * @return Returns "system" if the bundle is a system application; returns "third-party" otherwise.
     */
    virtual std::string GetAppType(const std::string &bundleName) override
    {
        return "";
    }
    /**
     * @brief Check whether the app is system app by it's UID.
     * @param uid Indicates the uid.
     * @return Returns true if the bundle is a system application; returns false otherwise.
     */
    virtual bool CheckIsSystemAppByUid(const int uid) override;

    /**
     * @brief Obtains the BundleInfo of application bundles based on the specified metaData.
     * @param metaData Indicates the metadata to get in the bundle.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    }
    /**
     * @brief Query the AbilityInfo by the given Want.
     * @param want Indicates the infomation of the ability.
     * @param abilityInfo Indicates the obtained AbilityInfo object.
     * @return Returns true if the AbilityInfo is successfully obtained; returns false otherwise.
     */
    virtual bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override
    {
        return true;
    }
    /**
     * @brief Query the AbilityInfo of list by the given Want.
     * @param want Indicates the information of the ability.
     * @param abilityInfos Indicates the obtained AbilityInfos object.
     * @return Returns true if the AbilityInfos is successfully obtained; returns false otherwise.
     */
    virtual bool QueryAbilityInfos(const Want &want, std::vector<AbilityInfo> &abilityInfos) override
    {
        return true;
    }
    /**
     * @brief Query the AbilityInfo by ability.uri in config.json.
     * @param abilityUri Indicates the uri of the ability.
     * @param abilityInfo Indicates the obtained AbilityInfo object.
     * @return Returns true if the AbilityInfo is successfully obtained; returns false otherwise.
     */
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) override
    {
        return true;
    }
    /**
     * @brief Obtains the BundleInfo of all keep-alive applications in the system.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) override
    {
        return true;
    };
    /**
     * @brief Obtains the label of a specified ability.
     * @param bundleName Indicates the bundle name.
     * @param className Indicates the ability class name.
     * @return Returns the label of the ability if exist; returns empty string otherwise.
     */
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) override
    {
        return "";
    }
    /**
     * @brief Obtains information about an application bundle contained in a ohos Ability Package (HAP).
     * @param hapFilePath Indicates the absolute file path of the HAP.
     * @param flag Indicates the information contained in the BundleInfo object to be returned.
     * @param bundleInfo Indicates the obtained BundleInfo object.
     * @return Returns true if the BundleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) override
    {
        return true;
    }
    /**
     * @brief Obtain the HAP module info of a specific ability.
     * @param abilityInfo Indicates the ability.
     * @param hapModuleInfo Indicates the obtained HapModuleInfo object.
     * @return Returns true if the HapModuleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) override
    {
        return true;
    }
    /**
     * @brief Obtains the Want for starting the main ability of an application based on the given bundle name.
     * @param bundleName Indicates the bundle name.
     * @param want Indicates the obtained launch Want object.
     * @return Returns true if the launch Want object is successfully obtained; returns false otherwise.
     */
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) override
    {
        return true;
    }
    /**
     * @brief Confirms with the permission management module to check whether a request prompt is required for granting
     * a certain permission.
     * @param bundleName Indicates the name of the bundle to check.
     * @param permission Indicates the permission to check.
     * @param userId Indicates the user id.
     * @return Returns true if the current application does not have the permission and the user does not turn off
     * further requests; returns false if the current application already has the permission, the permission is rejected
     * by the system, or the permission is denied by the user and the user has turned off further requests.
     */
    virtual bool CanRequestPermission(
        const std::string &bundleName, const std::string &permissionName, const int userId) override
    {
        return true;
    }
    /**
     * @brief Checks whether the publickeys of two bundles are the same.
     * @param firstBundleName Indicates the first bundle name.
     * @param secondBundleName Indicates the second bundle name.
     * @return Returns SIGNATURE_UNKNOWN_BUNDLE if at least one of the given bundles is not found;
     *         returns SIGNATURE_NOT_MATCHED if their publickeys are different;
     *         returns SIGNATURE_MATCHED if their publickeys are the same.
     */
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) override
    {
        return 0;
    }
    /**
     * @brief Checks whether a specified bundle has been granted a specific permission.
     * @param bundleName Indicates the name of the bundle to check.
     * @param permission Indicates the permission to check.
     * @return Returns 0 if the bundle has the permission; returns -1 otherwise.
     */
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission) override;
    /**
     * @brief Obtains detailed information about a specified permission.
     * @param permissionName Indicates the name of the ohos permission.
     * @param permissionDef Indicates the object containing detailed information about the given ohos permission.
     * @return Returns true if the PermissionDef object is successfully obtained; returns false otherwise.
     */
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) override
    {
        return true;
    }
    /**
     * @brief Obtains all known permission groups in the system.
     * @param permissionDefs Indicates the list of objects containing the permission group information.
     * @return Returns true if the PermissionDef objects is successfully obtained; returns false otherwise.
     */
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) override
    {
        return true;
    }
    /**
     * @brief Obtains all known permission groups in the system.
     * @param permissions Indicates the permission array.
     * @param appNames Indicates the list of application names that have the specified permissions.
     * @return Returns true if the application names is successfully obtained; returns false otherwise.
     */
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) override
    {
        return true;
    }
    /**
     * @brief Requests a certain permission from user.
     * @param bundleName Indicates the name of the bundle to request permission.
     * @param permission Indicates the permission to request permission.
     * @param userId Indicates the user id.
     * @return Returns true if the permission request successfully; returns false otherwise.
     */
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId) override
    {
        return true;
    }
    /**
     * @brief Checks whether the system has a specified capability.
     * @param capName Indicates the name of the system feature to check.
     * @return Returns true if the given feature specified by name is available in the system; returns false otherwise.
     */
    virtual bool HasSystemCapability(const std::string &capName) override
    {
        return true;
    }
    /**
     * @brief Obtains the capabilities that are available in the system.
     * @param systemCaps Indicates the list of capabilities available in the system.
     * @return Returns true if capabilities in the system are successfully obtained; returns false otherwise.
     */
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) override
    {
        return true;
    }
    /**
     * @brief Checks whether the current device has been started in safe mode.
     * @return Returns true if the device is in safe mode; returns false otherwise.
     */
    virtual bool IsSafeMode() override
    {
        return true;
    }
    /**
     * @brief Clears cache data of a specified application.
     * @param bundleName Indicates the bundle name of the application whose cache data is to be cleared.
     * @param cleanCacheCallback Indicates the callback to be invoked for returning the operation result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) override
    {
        return true;
    }
    /**
     * @brief Clears application running data of a specified application.
     * @param bundleName Indicates the bundle name of the application whose data is to be cleared.
     * @return Returns true if the data cleared successfully; returns false otherwise.
     */
    virtual bool CleanBundleDataFiles(const std::string &bundleName) override
    {
        return true;
    }
    /**
     * @brief Register the specific bundle status callback.
     * @param bundleStatusCallback Indicates the callback to be invoked for returning the bundle status changed result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    }
    /**
     * @brief Clear the specific bundle status callback.
     * @param bundleStatusCallback Indicates the callback to be cleared.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) override
    {
        return true;
    }
    /**
     * @brief Unregister all the callbacks of status changed.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool UnregisterBundleStatusCallback() override
    {
        return true;
    }
    /**
     * @brief Dump the bundle informations with specifc flags.
     * @param flag Indicates the information contained in the dump result.
     * @param bundleName Indicates the bundle name if needed.
     * @param result Indicates the dump infomation result.
     * @return Returns true if the dump result is successfully obtained; returns false otherwise.
     */
    virtual bool DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result) override
    {
        return true;
    }
    /**
     * @brief Checks whether a specified application is enabled.
     * @param bundleName Indicates the bundle name of the application.
     * @return Returns true if the application is enabled; returns false otherwise.
     */
    virtual bool IsApplicationEnabled(const std::string &bundleName) override
    {
        return true;
    }
    /**
     * @brief Sets whether to enable a specified application.
     * @param bundleName Indicates the bundle name of the application.
     * @param isEnable Specifies whether to enable the application.
     *                 The value true means to enable it, and the value false means to disable it.
     * @return Returns true if the application is enabled; returns false otherwise.
     */
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) override
    {
        return true;
    }
    /**
     * @brief Obtains the interface used to install and uninstall bundles.
     * @return Returns a pointer to IBundleInstaller class if exist; returns nullptr otherwise.
     */
    virtual sptr<IBundleInstaller> GetBundleInstaller() override
    {
        return nullptr;
    }
    /**
     * @brief Sets whether to enable a specified ability.
     * @param abilityInfo Indicates information about the ability to check.
     * @return Returns true if the ability is enabled; returns false otherwise.
     */
    virtual bool IsAbilityEnabled(const AbilityInfo &abilityInfo) override
    {
        return true;
    }
    /**
     * @brief Sets whether to enable a specified ability.
     * @param abilityInfo Indicates information about the ability.
     * @param isEnabled Specifies whether to enable the ability.
     *                 The value true means to enable it, and the value false means to disable it.
     * @return Returns true if the ability is enabled; returns false otherwise.
     */
    virtual bool SetAbilityEnabled(const AbilityInfo &abilityInfo, bool isEnabled) override
    {
        return true;
    }
    /**
     * @brief Obtains the icon of a specified ability.
     * @param bundleName Indicates the bundle name.
     * @param className Indicates the ability class name.
     * @return Returns the icon resource string of the ability if exist; returns empty string otherwise.
     */
    virtual std::string GetAbilityIcon(const std::string &bundleName, const std::string &className) override
    {
        return "";
    }
    /**
     * @brief Obtains all bundle names of a specified application based on the given application UID.
     * @param uid Indicates the uid.
     * @param bundleNames Indicates the obtained bundle names.
     * @return Returns true if the bundle names is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundlesForUid(const int uid, std::vector<std::string> &bundleNames) override
    {
        return true;
    }
    /**
     * @brief Obtains the formal name associated with the given UID.
     * @param uid Indicates the uid.
     * @param name Indicates the obtained formal name.
     * @return Returns true if the formal name is successfully obtained; returns false otherwise.
     */
    virtual bool GetNameForUid(const int uid, std::string &name) override
    {
        return true;
    }
    /**
     * @brief Registers a callback for listening for permission changes of all UIDs.
     * @param callback Indicates the callback method to register.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool RegisterAllPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override
    {
        return true;
    }
    /**
     * @brief Registers a callback for listening for permission changes of specified UIDs.
     * @param uids Indicates the list of UIDs whose permission changes will be monitored.
     * @param callback Indicates the callback method to register.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool RegisterPermissionsChanged(
        const std::vector<int> &uids, const sptr<OnPermissionChangedCallback> &callback) override
    {
        return true;
    }
    /**
     * @brief Unregisters a specified callback for listening for permission changes.
     * @param callback Indicates the callback method to register.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool UnregisterPermissionsChanged(const sptr<OnPermissionChangedCallback> &callback) override
    {
        return true;
    }
    /**
     * @brief Obtains the application ID based on the given bundle name and user ID.
     * @param bundleName Indicates the bundle name of the application.
     * @param userId Indicates the user ID.
     * @return Returns the application ID if successfully obtained; returns empty string otherwise.
     */
    virtual std::string GetAppIdByBundleName(const std::string &bundleName, const int userId) override
    {
        return "";
    }
        /**
     * @brief  Obtains the FormInfo objects provided by all applications on the device.
     * @param  formInfo List of FormInfo objects if obtained; returns an empty List if no FormInfo is available on the
     * device.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool GetAllFormsInfo(std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    /**
     * @brief  Obtains the FormInfo objects provided by a specified application on the device.
     * @param  bundleName Indicates the bundle name of the application.
     * @param  formInfo List of FormInfo objects if obtained; returns an empty List if no FormInfo is available on the
     * device.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool GetFormsInfoByApp(const std::string &bundleName, std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    /**
     * @brief  Obtains the FormInfo objects provided by a specified module name.
     * @param  formInfo List of FormInfo objects if obtained; returns an empty List if no FormInfo is available on the
     * device.
     * @param  moduleName Indicates the module name of the application.
     * @param  bundleName Indicates the bundle name of the application.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool GetFormsInfoByModule(
        const std::string &bundleName, const std::string &moduleName, std::vector<FormInfo> &formInfos) override
    {
        return true;
    }
    /**
     * @brief Obtains the ShortcutInfo objects provided by a specified application on the device.
     * @param bundleName Indicates the bundle name of the application.
     * @param shortcutInfos List of ShortcutInfo objects if obtained.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool GetShortcutInfos(const std::string &bundleName, std::vector<ShortcutInfo> &shortcutInfos) override
    {
        return true;
    }
    void MockSetIsSystemApp(bool isSystemApp);
    /**
     * @brief Get module usage record list in descending order of lastLaunchTime.
     * @param maxNum the return size of the records, must be in range of 1 to 1000.
     * @param moduleUsageRecords List of ModuleUsageRecord objects if obtained.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool GetModuleUsageRecords(
        const int32_t number, std::vector<ModuleUsageRecord> &moduleUsageRecords) override
    {
        return true;
    }
    /**
     * @brief Notify a specified ability for activity.
     * @param bundleName Indicates the bundle name of the ability to activity.
     * @param abilityName Indicates the name of the ability to activity.
     * @param launchTime Indicates the ability launchTime.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool NotifyActivityLifeStatus(
        const std::string &bundleName, const std::string &abilityName, const int64_t launchTime) override
    {
        return true;
    }
private:
    bool isSystemApp_ = false;
    bool isSystemAppMock_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_TEST_UNITTEST_MOCK_INCLUDE_MOCK_BUNDLE_MANAGER_H