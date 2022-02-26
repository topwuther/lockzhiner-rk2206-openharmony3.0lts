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

#include "distributed_sched_adapter.h"

#include "bundle/bundle_manager_internal.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_log.h"

#include "ability_manager_client.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;
using namespace AppExecFwk;

namespace {
// set a non-zero value on need later
constexpr int64_t DEVICE_OFFLINE_DELAY_TIME = 0;
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedAdapter);

void DistributedSchedAdapter::Init()
{
    if (dmsAdapterHandler_ == nullptr) {
        shared_ptr<EventRunner> runner = EventRunner::Create("dmsAdapter");
        if (runner == nullptr) {
            HILOGE("DistributedSchedAdapter create runner failed");
            return;
        }
        dmsAdapterHandler_ = make_shared<EventHandler>(runner);
    }
}

void DistributedSchedAdapter::UnInit()
{
    dmsAdapterHandler_ = nullptr;
}

int32_t DistributedSchedAdapter::ConnectAbility(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const sptr<IRemoteObject>& callerToken)
{
    HILOGD("DistributedSchedAdapter ConnectAbility");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("DistributedSchedAdapter connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want,
        iface_cast<AAFwk::IAbilityConnection>(connect), callerToken);
    return ret;
}

int32_t DistributedSchedAdapter::DisconnectAbility(const sptr<IRemoteObject>& connect)
{
    HILOGD("DistributedSchedAdapter DisconnectAbility");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("DistributedSchedAdapter connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(
        iface_cast<AAFwk::IAbilityConnection>(connect));
    return ret;
}

void DistributedSchedAdapter::DeviceOnline(const std::string& deviceId)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("DeviceOnline dmsAdapterHandler is null");
        return;
    }

    if (deviceId.empty()) {
        HILOGW("DeviceOnline deviceId is empty");
        return;
    }
    HILOGD("process DeviceOnline deviceId is %s", deviceId.c_str());
    dmsAdapterHandler_->RemoveTask(deviceId);
}

void DistributedSchedAdapter::DeviceOffline(const std::string& deviceId)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("DeviceOffline dmsAdapterHandler is null");
        return;
    }

    if (deviceId.empty()) {
        HILOGW("DeviceOffline deviceId is empty");
        return;
    }
    HILOGD("process DeviceOffline deviceId is %s", deviceId.c_str());
    auto callback = [deviceId, this] () {
        ProcessDeviceOffline(deviceId);
    };
    if (!dmsAdapterHandler_->PostTask(callback, deviceId, DEVICE_OFFLINE_DELAY_TIME)) {
        HILOGW("DeviceOffline PostTask failed");
    }
}

bool DistributedSchedAdapter::QueryAbilityInfo(const OHOS::AAFwk::Want& want, AppExecFwk::AbilityInfo& abilityInfo)
{
    auto bundleMgr = BundleManagerInternal::GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("QueryAbilityInfo failed to get bms");
        return false;
    }
    bool result = bundleMgr->QueryAbilityInfo(want, abilityInfo);
    if (!result) {
        HILOGE("QueryAbilityInfo fail");
        return false;
    }
    return true;
}

void DistributedSchedAdapter::ProcessDeviceOffline(const std::string& deviceId)
{
    HILOGD("ProcessDeviceOffline");
    DistributedSchedService::GetInstance().ProcessDeviceOffline(deviceId);
}

void DistributedSchedAdapter::ProcessConnectDied(const sptr<IRemoteObject>& connect)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("ProcessConnectDied dmsAdapterHandler is null");
        return;
    }

    if (connect == nullptr) {
        HILOGE("ProcessConnectDied connect is null");
        return;
    }
    HILOGD("process connect died");
    auto callback = [connect] () {
        DistributedSchedService::GetInstance().ProcessConnectDied(connect);
    };
    if (!dmsAdapterHandler_->PostTask(callback)) {
        HILOGW("ProcessConnectDied PostTask failed");
    }
}

int32_t DistributedSchedAdapter::GetBundleNameListFromBms(int32_t uid,
    std::vector<std::u16string>& u16BundleNameList)
{
    vector<string> bundleNameList;
    int32_t ret = GetBundleNameListFromBms(uid, bundleNameList);
    if (ret != ERR_OK) {
        HILOGE("DistributedSchedAdapter::GetBundleNameListFromBms failed");
        return ret;
    }
    for (const string& bundleName : bundleNameList) {
        u16BundleNameList.emplace_back(Str8ToStr16(bundleName));
    }
    return ERR_OK;
}

int32_t DistributedSchedAdapter::GetBundleNameListFromBms(int32_t uid, std::vector<std::string>& bundleNameList)
{
    auto bundleMgr = BundleManagerInternal::GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("DistributedSchedAdapter::GetBundleNameListFromBms failed to get bms");
        return OBJECT_NULL;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool result = bundleMgr->GetBundlesForUid(uid, bundleNameList);
    IPCSkeleton::SetCallingIdentity(identity);
    HILOGD("DistributedSchedAdapter::GetBundleNameListFromBms %{public}d", result);
    return result ? ERR_OK : BUNDLE_MANAGER_SERVICE_ERR;
}
} // namespace DistributedSchedule
} // namespace OHOS