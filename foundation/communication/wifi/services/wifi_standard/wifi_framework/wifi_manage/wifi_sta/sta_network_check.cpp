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
#include "sta_network_check.h"
#include "if_config.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_LABEL("StaNetworkCheck");

namespace OHOS {
namespace Wifi {
StaNetworkCheck::StaNetworkCheck(NetStateHandler handle)
{
    pDealNetCheckThread = nullptr;
    netStateHandler = handle;
    lastNetState = NETWORK_STATE_UNKNOW;
    isStopNetCheck = true;
    isExitNetCheckThread = false;
}

StaNetworkCheck::~StaNetworkCheck()
{
    WIFI_LOGI("StaNetworkCheck::~StaNetworkCheck enter\n");
    ExitNetCheckThread();
    WIFI_LOGI("StaNetworkCheck::~StaNetworkCheck complete\n");
}

void StaNetworkCheck::HttpDetection()
{
    WIFI_LOGI("Enter httpDetection");
    /* Detect portal hotspot and send message to InterfaceSeervice if result is yes. */
    HttpRequest httpRequest;
    std::string httpReturn;
    std::string httpMsg(DEFAULT_PORTAL_HTTPS_URL);

    if (httpRequest.HttpGet(httpMsg, httpReturn) == 0) {
        if (httpReturn.find("204") != std::string::npos) {
            WIFI_LOGE("This network is normal!");
            if ((lastNetState != NETWORK_STATE_WORKING) && (isExitNetCheckThread == false) &&
                (isStopNetCheck == false)) {
                netStateHandler(StaNetState::NETWORK_STATE_WORKING);
            }
            lastNetState = NETWORK_STATE_WORKING;
            return;
        } else {
            /* Callback result to InterfaceService. */
            WIFI_LOGI("This network is portal AP, need certification!");
            return;
        }
    }
    WIFI_LOGE("This network cant online!");
    if ((lastNetState != NETWORK_STATE_NOWORKING) && (isExitNetCheckThread == false) && (isStopNetCheck == false)) {
        netStateHandler(StaNetState::NETWORK_STATE_NOWORKING);
    }
    lastNetState = NETWORK_STATE_NOWORKING;
}

void StaNetworkCheck::RunNetCheckThreadFunc()
{
    WIFI_LOGI("enter RunNetCheckThreadFunc!\n");
    for (;;) {
        std::unique_lock<std::mutex> lck(mMutex);
        while (isStopNetCheck) {
            WIFI_LOGI("waiting for sigal\n");
            mCondition.wait(lck);
        }

        if (isExitNetCheckThread) {
            WIFI_LOGI("break the loop\n");
            break;
        }
        lck.unlock();
        HttpDetection();
        sleep(1);
    }
}

ErrCode StaNetworkCheck::InitNetCheckThread()
{
    pDealNetCheckThread = new (std::nothrow) std::thread(&StaNetworkCheck::RunNetCheckThreadFunc, this);
    if (pDealNetCheckThread == nullptr) {
        WIFI_LOGE("In StaNetworkCheck start NetCheck thread failed!\n");
        return ErrCode::WIFI_OPT_FAILED;
    }
    return ErrCode::WIFI_OPT_SUCCESS;
}

void StaNetworkCheck::StopNetCheckThread()
{
    WIFI_LOGI("enter StopNetCheckThread!\n");
    std::unique_lock<std::mutex> lck(mMutex);
    isStopNetCheck = true;
}

void StaNetworkCheck::SignalNetCheckThread()
{
    WIFI_LOGI("enter SignalNetCheckThread!\n");
    lastNetState = NETWORK_STATE_UNKNOW;
    isStopNetCheck = false;
    std::unique_lock<std::mutex> lck(mMutex);
    mCondition.notify_one();
}

void StaNetworkCheck::ExitNetCheckThread()
{
    {
        isStopNetCheck = false;
        isExitNetCheckThread = true;
        std::unique_lock<std::mutex> lck(mMutex);
        mCondition.notify_one();
    }

    if (pDealNetCheckThread != nullptr) {
        pDealNetCheckThread->join();
        delete pDealNetCheckThread;
        pDealNetCheckThread = nullptr;
    }
}
}  // namespace Wifi
}  // namespace OHOS