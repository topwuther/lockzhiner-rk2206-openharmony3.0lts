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
#include "sta_interface.h"
#include "sta_service.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_LABEL("StaInterface");

namespace OHOS {
namespace Wifi {
StaInterface::StaInterface() : pStaService(nullptr)
{}

StaInterface::~StaInterface()
{
    WIFI_LOGI("StaInterface::~StaInterface");
    if (pStaService != nullptr) {
        delete pStaService;
    }
}

extern "C" IStaService *Create(void)
{
    return new (std::nothrow)StaInterface();
}

extern "C" void Destroy(IStaService *pservice)
{
    delete pservice;
}

ErrCode StaInterface::EnableWifi()
{
    WIFI_LOGD("Enter StaInterface::EnableWifi.\n");
    if(pStaService == nullptr) {
        pStaService = new (std::nothrow) StaService();
        if (pStaService == nullptr) {
            WIFI_LOGE("New StaService failed.\n");
            return WIFI_OPT_FAILED;
        }
    }

    if (pStaService->InitStaService(staCallback) != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("InitStaService failed.\n");
        delete pStaService;
        pStaService = nullptr;
        return WIFI_OPT_FAILED;
    }

    if (pStaService->EnableWifi() != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("EnableWifi failed.\n");
        DisableWifi();
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DisableWifi()
{
    LOGD("Enter StaInterface::DisableWifi.\n");
    if (pStaService->DisableWifi() != WIFI_OPT_SUCCESS) {
        LOGD("DisableWifi failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectToNetwork(int networkId)
{
    LOGD("Enter StaInterface::Connect.\n");
    if (pStaService->ConnectToNetwork(networkId) != WIFI_OPT_SUCCESS) {
        LOGD("ConnectTo failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectToDevice(const WifiDeviceConfig &config)
{
    LOGD("Enter StaInterface::Connect.\n");
    if (pStaService->ConnectToDevice(config) != WIFI_OPT_SUCCESS) {
        LOGD("ConnectTo failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ReAssociate()
{
    LOGD("Enter StaInterface::ReAssociate.\n");
    if (pStaService->ReAssociate() != WIFI_OPT_SUCCESS) {
        LOGD("ReAssociate failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::Disconnect()
{
    LOGD("Enter StaInterface::Disconnect.\n");
    if (pStaService->Disconnect() != WIFI_OPT_SUCCESS) {
        LOGD("Disconnect failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

int StaInterface::AddDeviceConfig(const WifiDeviceConfig &config)
{
    LOGD("Enter StaInterface::AddDeviceConfig.\n");
    return pStaService->AddDeviceConfig(config);
}

int StaInterface::UpdateDeviceConfig(const WifiDeviceConfig &config)
{
    LOGD("Enter StaInterface::UpdateDeviceConfig.\n");
    return pStaService->UpdateDeviceConfig(config);
}

ErrCode StaInterface::RemoveDevice(int networkId)
{
    LOGD("Enter StaInterface::RemoveDeviceConfig.\n");
    if (pStaService->RemoveDevice(networkId) != WIFI_OPT_SUCCESS) {
        LOGD("RemoveDeviceConfig failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RemoveAllDevice()
{
    WIFI_LOGD("Enter StaInterface::RemoveAllDevice.\n");

    if (pStaService->RemoveAllDevice() != WIFI_OPT_SUCCESS) {
        WIFI_LOGD("RemoveAllDevice failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}
ErrCode StaInterface::EnableDeviceConfig(int networkId, bool attemptEnable)
{
    LOGD("Enter StaInterface::EnableDeviceConfig.\n");
    return pStaService->EnableDeviceConfig(networkId, attemptEnable);
}

ErrCode StaInterface::DisableDeviceConfig(int networkId)
{
    LOGD("Enter StaInterface::DisableDeviceConfig.\n");
    return pStaService->DisableDeviceConfig(networkId);
}

ErrCode StaInterface::StartWps(const WpsConfig &config)
{
    LOGD("Enter StaInterface::StartWps.\n");
    if (pStaService->StartWps(config) != WIFI_OPT_SUCCESS) {
        LOGD("StartWps failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::CancelWps()
{
    LOGD("Enter StaInterface::CancelWps.\n");
    if (pStaService->CancelWps() != WIFI_OPT_SUCCESS) {
        LOGD("CancelWps failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectivityManager(const std::vector<InterScanInfo> &scanInfos)
{
    LOGI("Enter Connection management.\n");
    if (pStaService->AutoConnectService(scanInfos) != WIFI_OPT_SUCCESS) {
        LOGD("ConnectivityManager failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::SetCountryCode(const std::string &countryCode)
{
    LOGD("Enter StaInterface::SetCountryCode.\n");
    if (pStaService->SetCountryCode(countryCode) != WIFI_OPT_SUCCESS) {
        LOGD("SetCountryCode failed.\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RegisterStaServiceCallback(const StaServiceCallback &callbacks)
{
    LOGD("Enter StaInterface::RegisterStaServiceCallback.\n");
    staCallback = callbacks;
    return WIFI_OPT_SUCCESS;
}

}  // namespace Wifi
}  // namespace OHOS
