/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "wifi_scan_service_impl.h"
#include "wifi_msg.h"
#include "permission_def.h"
#include "wifi_permission_utils.h"
#include "wifi_auth_center.h"
#include "wifi_config_center.h"
#include "wifi_manager.h"
#include "wifi_service_manager.h"
#include "wifi_internal_event_dispatcher.h"
#include "wifi_internal_msg.h"
#include "wifi_logger.h"
#include "define.h"
#include "wifi_scan_callback_proxy.h"

DEFINE_WIFILOG_SCAN_LABEL("WifiScanServiceImpl");
namespace OHOS {
namespace Wifi {
std::mutex WifiScanServiceImpl::g_instanceLock;
sptr<WifiScanServiceImpl> WifiScanServiceImpl::g_instance;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(WifiScanServiceImpl::GetInstance().GetRefPtr());
sptr<WifiScanServiceImpl> WifiScanServiceImpl::GetInstance()
{
    if (g_instance == nullptr) {
        std::lock_guard<std::mutex> autoLock(g_instanceLock);
        if (g_instance == nullptr) {
            auto service = new (std::nothrow) WifiScanServiceImpl;
            g_instance = service;
        }
    }
    return g_instance;
}

WifiScanServiceImpl::WifiScanServiceImpl()
    : SystemAbility(WIFI_SCAN_ABILITY_ID, true), mPublishFlag(false), mState(ServiceRunningState::STATE_NOT_START)
{}

WifiScanServiceImpl::~WifiScanServiceImpl()
{}

void WifiScanServiceImpl::OnStart()
{
    if (mState == ServiceRunningState::STATE_RUNNING) {
        WIFI_LOGD("Service has already started.");
        return;
    }
    if (!Init()) {
        WIFI_LOGE("Failed to init service");
        OnStop();
        return;
    }
    mState = ServiceRunningState::STATE_RUNNING;
    WIFI_LOGI("Start scan service!");
    WifiManager::GetInstance();
}

void WifiScanServiceImpl::OnStop()
{
    mState = ServiceRunningState::STATE_NOT_START;
    mPublishFlag = false;
    WIFI_LOGI("Stop scan service!");
}

bool WifiScanServiceImpl::Init()
{
    if (!mPublishFlag) {
        bool ret = Publish(WifiScanServiceImpl::GetInstance());
        if (!ret) {
            WIFI_LOGE("Failed to publish scan service!");
            return false;
        }
        mPublishFlag = true;
    }
    return true;
}

ErrCode WifiScanServiceImpl::SetScanControlInfo(const ScanControlInfo &info)
{
    WIFI_LOGI("WifiScanServiceImpl::SetScanControlInfo");
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("SetScanControlInfo:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    WifiConfigCenter::GetInstance().SetScanControlInfo(info);
    if (IsScanServiceRunning()) {
        IScanService *pService = WifiServiceManager::GetInstance().GetScanServiceInst();
        if (pService == nullptr) {
            return WIFI_OPT_SCAN_NOT_OPENED;
        }
        return pService->OnControlStrategyChanged();
    }

    return WIFI_OPT_SUCCESS;
}

ErrCode WifiScanServiceImpl::Scan()
{
    WIFI_LOGI("Scan");
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("Scan:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsScanServiceRunning()) {
        return WIFI_OPT_SCAN_NOT_OPENED;
    }

    IScanService *pService = WifiServiceManager::GetInstance().GetScanServiceInst();
    if (pService == nullptr) {
        return WIFI_OPT_SCAN_NOT_OPENED;
    }
    return pService->Scan(true);
}

ErrCode WifiScanServiceImpl::AdvanceScan(const WifiScanParams &params)
{
    WIFI_LOGI("Scan with WifiScanParams, band %{public}u", params.band);
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("Scan with WifiScanParams:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsScanServiceRunning()) {
        return WIFI_OPT_SCAN_NOT_OPENED;
    }

    IScanService *pService = WifiServiceManager::GetInstance().GetScanServiceInst();
    if (pService == nullptr) {
        return WIFI_OPT_SCAN_NOT_OPENED;
    }
    return pService->ScanWithParam(params);
}

ErrCode WifiScanServiceImpl::IsWifiClosedScan(bool &bOpen)
{
    WIFI_LOGI("IsWifiClosedScan");
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("IsWifiClosedScan:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    bOpen = WifiConfigCenter::GetInstance().IsScanAlwaysActive();
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiScanServiceImpl::GetScanInfoList(std::vector<WifiScanInfo> &result)
{
    WIFI_LOGI("GetScanInfoList");

    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetScanInfoList:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetScanInfoList:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    WifiConfigCenter::GetInstance().GetScanInfoList(result);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiScanServiceImpl::RegisterCallBack(const sptr<IWifiScanCallback> &callback)
{
    WIFI_LOGI("WifiScanServiceImpl::RegisterCallBack!");
    WifiInternalEventDispatcher::GetInstance().SetSingleScanCallback(callback);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiScanServiceImpl::GetSupportedFeatures(long &features)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetSupportedFeatures:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    int ret = WifiManager::GetInstance().GetSupportedFeatures(features);
    if (ret < 0) {
        WIFI_LOGE("Failed to get supported features!");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

bool WifiScanServiceImpl::IsScanServiceRunning()
{
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetScanMidState();
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGD("scan service does not started!");
        return false;
    }
    return true;
}
}  // namespace Wifi
}  // namespace OHOS