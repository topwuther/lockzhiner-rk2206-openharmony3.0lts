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
#include "wifi_p2p_service_impl.h"
#include "wifi_permission_utils.h"
#include "wifi_auth_center.h"
#include "wifi_config_center.h"
#include "wifi_manager.h"
#include "wifi_service_manager.h"
#include "wifi_internal_event_dispatcher.h"
#include "wifi_logger.h"
#include "define.h"

DEFINE_WIFILOG_P2P_LABEL("WifiP2pServiceImpl");

namespace OHOS {
namespace Wifi {
std::mutex WifiP2pServiceImpl::instanceLock;
sptr<WifiP2pServiceImpl> WifiP2pServiceImpl::instance;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(WifiP2pServiceImpl::GetInstance().GetRefPtr());

sptr<WifiP2pServiceImpl> WifiP2pServiceImpl::GetInstance()
{
    if (instance == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock);
        if (instance == nullptr) {
            auto service = new (std::nothrow) WifiP2pServiceImpl;
            instance = service;
        }
    }
    return instance;
}

WifiP2pServiceImpl::WifiP2pServiceImpl()
    : SystemAbility(WIFI_P2P_ABILITY_ID, true), mPublishFlag(false), mState(ServiceRunningState::STATE_NOT_START)
{}

WifiP2pServiceImpl::~WifiP2pServiceImpl()
{}

void WifiP2pServiceImpl::OnStart()
{
    if (mState == ServiceRunningState::STATE_RUNNING) {
        WIFI_LOGD("P2p service has already started.");
        return;
    }
    if (!Init()) {
        WIFI_LOGE("Failed to init p2p service");
        OnStop();
        return;
    }
    mState = ServiceRunningState::STATE_RUNNING;
    WIFI_LOGI("Start p2p service!");
    WifiManager::GetInstance().AddSupportedFeatures(WifiFeatures::WIFI_FEATURE_P2P);
}

void WifiP2pServiceImpl::OnStop()
{
    mState = ServiceRunningState::STATE_NOT_START;
    mPublishFlag = false;
    WIFI_LOGI("Stop p2p service!");
}

bool WifiP2pServiceImpl::Init()
{
    if (!mPublishFlag) {
        bool ret = Publish(WifiP2pServiceImpl::GetInstance());
        if (!ret) {
            WIFI_LOGE("Failed to publish p2p service!");
            return false;
        }
        mPublishFlag = true;
    }
    return true;
}

ErrCode WifiP2pServiceImpl::CheckCanEnableP2p(void)
{
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("EnableP2p:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    if (WifiConfigCenter::GetInstance().GetAirplaneModeState() == 1) {
        WIFI_LOGD("current airplane mode and can not use p2p, open failed!");
        return WIFI_OPT_FORBID_AIRPLANE;
    }
    if (WifiConfigCenter::GetInstance().GetPowerSavingModeState() == 1) {
        WIFI_LOGD("current power saving mode and can not use p2p, open failed!");
        return WIFI_OPT_FORBID_POWSAVING;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiP2pServiceImpl::EnableP2p(void)
{
    WIFI_LOGI("EnableP2p");
    ErrCode errCode = CheckCanEnableP2p();
    if (errCode != WIFI_OPT_SUCCESS) {
        return errCode;
    }

    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetP2pMidState();
    if (curState != WifiOprMidState::CLOSED) {
        WIFI_LOGD("current p2p state is %{public}d", static_cast<int>(curState));
        if (curState == WifiOprMidState::CLOSING) {
            return WIFI_OPT_OPEN_FAIL_WHEN_CLOSING;
        } else {
            return WIFI_OPT_OPEN_SUCC_WHEN_OPENED;
        }
    }
    if (!WifiConfigCenter::GetInstance().SetP2pMidState(curState, WifiOprMidState::OPENING)) {
        WIFI_LOGD("set p2p mid state opening failed! may be other activity has been operated");
        return WIFI_OPT_OPEN_SUCC_WHEN_OPENED;
    }
    ErrCode ret = WIFI_OPT_FAILED;
    do {
        if (WifiServiceManager::GetInstance().CheckAndEnforceService(WIFI_SERVICE_P2P) < 0) {
            WIFI_LOGE("Load %{public}s service failed!", WIFI_SERVICE_P2P);
            break;
        }
        IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
        if (pService == nullptr) {
            WIFI_LOGE("Create %{public}s service failed!", WIFI_SERVICE_P2P);
            break;
        }
        ret = pService->RegisterP2pServiceCallbacks(WifiManager::GetInstance().GetP2pCallback());
        if (ret != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("Register p2p service callback failed!");
            break;
        }
        ret = pService->EnableP2p();
        if (ret != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("service EnableP2p failed, ret %{public}d!", static_cast<int>(ret));
            break;
        }
    } while (false);
    if (ret != WIFI_OPT_SUCCESS) {
        WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::OPENING, WifiOprMidState::CLOSED);
        WifiServiceManager::GetInstance().UnloadService(WIFI_SERVICE_P2P);
    }
    return ret;
}

ErrCode WifiP2pServiceImpl::DisableP2p(void)
{
    WIFI_LOGI("DisableP2p");
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DisableP2p:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetP2pMidState();
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGD("current p2p state is %{public}d", static_cast<int>(curState));
        if (curState == WifiOprMidState::OPENING) {
            return WIFI_OPT_CLOSE_FAIL_WHEN_OPENING;
        } else {
            return WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED;
        }
    }
    if (!WifiConfigCenter::GetInstance().SetP2pMidState(curState, WifiOprMidState::CLOSING)) {
        WIFI_LOGD("set p2p mid state opening failed! may be other activity has been operated");
        return WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED;
    }
    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::CLOSED);
        WifiServiceManager::GetInstance().UnloadService(WIFI_SERVICE_P2P);
        return WIFI_OPT_SUCCESS;
    }
    ErrCode ret = pService->DisableP2p();
    if (ret != WIFI_OPT_SUCCESS) {
        WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::CLOSING, WifiOprMidState::RUNNING);
    }
    return ret;
}

ErrCode WifiP2pServiceImpl::DiscoverDevices(void)
{
    WIFI_LOGI("DiscoverDevices");
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DiscoverDevices:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->DiscoverDevices();
}

ErrCode WifiP2pServiceImpl::StopDiscoverDevices(void)
{
    WIFI_LOGI("StopDiscoverDevices");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->StopDiscoverDevices();
}

ErrCode WifiP2pServiceImpl::DiscoverServices(void)
{
    WIFI_LOGI("DiscoverServices");
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DiscoverServices:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->DiscoverServices();
}

ErrCode WifiP2pServiceImpl::StopDiscoverServices(void)
{
    WIFI_LOGI("StopDiscoverServices");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->StopDiscoverServices();
}

ErrCode WifiP2pServiceImpl::RequestService(const WifiP2pDevice &device, const WifiP2pServiceRequest &request)
{
    WIFI_LOGI("RequestService");
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RequestService:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->RequestService(device, request);
}

ErrCode WifiP2pServiceImpl::PutLocalP2pService(const WifiP2pServiceInfo &srvInfo)
{
    WIFI_LOGI("PutLocalP2pService, service name is [%{public}s]", srvInfo.GetServiceName().c_str());
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->PutLocalP2pService(srvInfo);
}

ErrCode WifiP2pServiceImpl::DeleteLocalP2pService(const WifiP2pServiceInfo &srvInfo)
{
    WIFI_LOGI("DeleteLocalP2pService, service name is [%{public}s]", srvInfo.GetServiceName().c_str());
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->DeleteLocalP2pService(srvInfo);
}

ErrCode WifiP2pServiceImpl::StartP2pListen(int period, int interval)
{
    WIFI_LOGI("StartP2pListen, period %{public}d, interval %{public}d", period, interval);
    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("StartP2pListen:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->StartP2pListen(period, interval);
}

ErrCode WifiP2pServiceImpl::StopP2pListen()
{
    WIFI_LOGI("StopP2pListen");
    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("StopP2pListen:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->StopP2pListen();
}

ErrCode WifiP2pServiceImpl::FormGroup(const WifiP2pConfig &config)
{
    WIFI_LOGI("FormGroup, network name is [%{public}s]", config.GetNetworkName().c_str());
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("FormGroup:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->FormGroup(config);
}

ErrCode WifiP2pServiceImpl::RemoveGroup()
{
    WIFI_LOGI("RemoveGroup");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->RemoveGroup();
}

ErrCode WifiP2pServiceImpl::DeleteGroup(const WifiP2pGroupInfo &group)
{
    WIFI_LOGI("DeleteGroup, group name [%{public}s]", group.GetGroupName().c_str());
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->DeleteGroup(group);
}

ErrCode WifiP2pServiceImpl::P2pConnect(const WifiP2pConfig &config)
{
    WIFI_LOGI("P2pConnect device address [%{private}s]", config.GetDeviceAddress().c_str());
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("P2pConnect:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->P2pConnect(config);
}

ErrCode WifiP2pServiceImpl::P2pDisConnect()
{
    WIFI_LOGI("P2pDisConnect");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->P2pDisConnect();
}

ErrCode WifiP2pServiceImpl::QueryP2pInfo(WifiP2pInfo &connInfo)
{
    WIFI_LOGI("QueryP2pInfo group owner address [%{private}s]", connInfo.GetGroupOwnerAddress().c_str());
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->QueryP2pInfo(connInfo);
}

ErrCode WifiP2pServiceImpl::GetCurrentGroup(WifiP2pGroupInfo &group)
{
    WIFI_LOGI("GetCurrentGroup");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->GetCurrentGroup(group);
}

ErrCode WifiP2pServiceImpl::GetP2pEnableStatus(int &status)
{
    WIFI_LOGI("GetP2pEnableStatus");
    status = WifiConfigCenter::GetInstance().GetP2pState();
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiP2pServiceImpl::GetP2pDiscoverStatus(int &status)
{
    WIFI_LOGI("GetP2pDiscoverStatus");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->GetP2pDiscoverStatus(status);
}

ErrCode WifiP2pServiceImpl::GetP2pConnectedStatus(int &status)
{
    WIFI_LOGI("GetP2pConnectedStatus");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->GetP2pConnectedStatus(status);
}

ErrCode WifiP2pServiceImpl::QueryP2pDevices(std::vector<WifiP2pDevice> &devives)
{
    WIFI_LOGI("QueryP2pDevices");
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("QueryP2pDevices:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->QueryP2pDevices(devives);
}

ErrCode WifiP2pServiceImpl::QueryP2pGroups(std::vector<WifiP2pGroupInfo> &groups)
{
    WIFI_LOGI("QueryP2pGroups");
    if (WifiPermissionUtils::VerifyGetWifiDirectDevicePermission() == PERMISSION_DENIED) {
        WIFI_LOGE("QueryP2pGroups:VerifyGetWifiDirectDevicePermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->QueryP2pGroups(groups);
}

ErrCode WifiP2pServiceImpl::QueryP2pServices(std::vector<WifiP2pServiceInfo> &services)
{
    WIFI_LOGI("QueryP2pServices");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }

    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->QueryP2pServices(services);
}

ErrCode WifiP2pServiceImpl::RegisterCallBack(const sptr<IWifiP2pCallback> &callback)
{
    WIFI_LOGI("WifiP2pServiceImpl::RegisterCallBack!");
    WifiInternalEventDispatcher::GetInstance().SetSingleP2pCallback(callback);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiP2pServiceImpl::GetSupportedFeatures(long &features)
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

bool WifiP2pServiceImpl::IsP2pServiceRunning()
{
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetP2pMidState();
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGD("p2p service does not started!");
        return false;
    }
    return true;
}

ErrCode WifiP2pServiceImpl::SetP2pDeviceName(const std::string &deviceName)
{
    WIFI_LOGI("SetDeviceName:%s", deviceName.c_str());
    int length = deviceName.length();
    if (length > DEVICE_NAME_LENGTH || length < 0) {
        return WIFI_OPT_INVALID_PARAM;
    }
    WifiConfigCenter::GetInstance().SetP2pDeviceName(deviceName);
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_SUCCESS;
    }
    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->SetP2pDeviceName(deviceName);
}
ErrCode WifiP2pServiceImpl::SetP2pWfdInfo(const WifiP2pWfdInfo &wfdInfo)
{
    WIFI_LOGI("SetP2pWfdInfo");
    if (!IsP2pServiceRunning()) {
        WIFI_LOGE("P2pService is not runing!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return WIFI_OPT_P2P_NOT_OPENED;
    }
    return pService->SetP2pWfdInfo(wfdInfo);
}
}  // namespace Wifi
}  // namespace OHOS