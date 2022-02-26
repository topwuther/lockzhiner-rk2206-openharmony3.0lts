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
#include "scan_interface.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_SCAN_LABEL("ScanStateMachine");

namespace OHOS {
namespace Wifi {
ScanInterface::ScanInterface() : pScanService(nullptr)
{}

ScanInterface::~ScanInterface()
{
    WIFI_LOGI("Enter ScanInterface::~ScanInterface.");
    if (pScanService != nullptr) {
        delete pScanService;
        pScanService = nullptr;
    }
}

extern "C" IScanService *Create(void)
{
    return new ScanInterface();
}
extern "C" void Destroy(ScanInterface *scanInterface)
{
    delete scanInterface;
}

ErrCode ScanInterface::Init()
{
    WIFI_LOGI("Enter ScanInterface::Init.\n");

    pScanService = new (std::nothrow)ScanService();
    if (pScanService == nullptr) {
        WIFI_LOGE("New ScanService failed.\n");
        return WIFI_OPT_INVALID_PARAM;
    }

    if (!(pScanService->InitScanService(mScanSerivceCallbacks))) {
        WIFI_LOGE("InitScanService failed.\n");
        delete pScanService;
        pScanService = nullptr;
        return WIFI_OPT_INVALID_PARAM;
    }

    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::UnInit()
{
    WIFI_LOGI("Enter ScanInterface::UnInit.\n");

    pScanService->UnInitScanService();

    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::Scan(bool externFlag)
{
    WIFI_LOGI("Enter ScanInterface::Scan\n");

    return pScanService->Scan(externFlag);
}

ErrCode ScanInterface::ScanWithParam(const WifiScanParams &wifiScanParams)
{
    WIFI_LOGI("Enter ScanInterface::ScanWithParam\n");

    return pScanService->ScanWithParam(wifiScanParams);
}

ErrCode ScanInterface::OnScreenStateChanged(int screenState)
{
    WIFI_LOGI("Enter ScanInterface::OnScreenStateChanged\n");

    if (screenState != STATE_OPEN && screenState != STATE_CLOSE) {
        WIFI_LOGE("screenState param is error");
        return WIFI_OPT_INVALID_PARAM;
    }
    bool screenOn = true;
    if (screenState == STATE_CLOSE) {
        screenOn = false;
    }
    pScanService->HandleScreenStatusChanged(screenOn);
    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::OnClientModeStatusChanged(int staStatus)
{
    WIFI_LOGI("Enter ScanInterface::OnClientModeStatusChanged\n");

    pScanService->HandleStaStatusChanged(staStatus);
    pScanService->SetStaCurrentTime();
    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::OnAppRunningModeChanged(int appMode)
{
    WIFI_LOGI("Enter ScanInterface::OnAppRunningModeChanged\n");

    pScanService->SetOperateAppMode(appMode);
    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::OnCustomControlStateChanged(int customScene, int customSceneStatus)
{
    WIFI_LOGI("Enter ScanInterface::OnCustomControlStateChanged\n");

    if (customSceneStatus != STATE_OPEN && customSceneStatus != STATE_CLOSE) {
        WIFI_LOGE("screenState param is error");
        return WIFI_OPT_INVALID_PARAM;
    }
    pScanService->HandleCustomStatusChanged(customScene, customSceneStatus);
    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::OnControlStrategyChanged()
{
    WIFI_LOGI("Enter ScanInterface::OnControlStrategyChanged\n");

    pScanService->ClearScanControlValue();
    pScanService->GetScanControlInfo();
    pScanService->SystemScanProcess(true);
    return WIFI_OPT_SUCCESS;
}

ErrCode ScanInterface::RegisterScanCallbacks(const IScanSerivceCallbacks &scanSerivceCallbacks)
{
    mScanSerivceCallbacks = scanSerivceCallbacks;
    return WIFI_OPT_SUCCESS;
}
}  // namespace Wifi
}  // namespace OHOS