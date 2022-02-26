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

#include "bundle/bundle_manager_internal.h"

#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
IMPLEMENT_SINGLE_INSTANCE(BundleManagerInternal);

bool BundleManagerInternal::GetCallerAppIdFromBms(int32_t callingUid, std::string& appId)
{
    std::vector<std::string> bundleNameList;
    int32_t ret = DistributedSchedAdapter::GetInstance().GetBundleNameListFromBms(callingUid, bundleNameList);
    if (ret != ERR_OK || bundleNameList.empty()) {
        HILOGE("BundleManagerInternal::GetCallerAppIdFromBms error");
        return false;
    }

    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("BundleManagerInternal::GetCallerAppIdFromBms failed to get bms");
        return false;
    }

    // getting an arbitrary bundlename for they sharing a same appId, here we get the first one
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    appId = bundleMgr->GetAppIdByBundleName(bundleNameList.front(), callingUid);
    IPCSkeleton::SetCallingIdentity(identity);
    HILOGD("BundleManagerInternal::GetCallerAppIdFromBms appId:%s", appId.c_str());
    return true;
}

sptr<AppExecFwk::IBundleMgr> BundleManagerInternal::GetBundleManager()
{
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient != nullptr) {
        return iface_cast<AppExecFwk::IBundleMgr>(
            samgrClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
    } else {
        HILOGE("DistributedSchedAdapter::GetBundleManager failed to get samgr");
        return nullptr;
    }
}
} // namespace DistributedSchedule
} // namespace OHOS