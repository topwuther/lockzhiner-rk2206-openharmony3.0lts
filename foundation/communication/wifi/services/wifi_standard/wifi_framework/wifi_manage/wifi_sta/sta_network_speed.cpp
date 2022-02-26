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
#include "sta_network_speed.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_LABEL("StaNetWorkSpeed");

namespace OHOS {
namespace Wifi {
StaNetWorkSpeed::StaNetWorkSpeed()
{}

StaNetWorkSpeed::~StaNetWorkSpeed()
{
    WIFI_LOGI("StaNetWorkSpeed::~StaNetWorkSpeed enter");
}

void StaNetWorkSpeed::GetNetSpeed(std::string &strRx, std::string &strTx)
{
    WIFI_LOGE("enter GetNetSpeed\n");
    long rxBytesPre = 0;
    long txBytesPre = 0;
    long rxBytesNext = 0;
    long txBytesNext = 0;
    GetTxAndRxBytes(rxBytesPre, txBytesPre);
    sleep(1);
    GetTxAndRxBytes(rxBytesNext, txBytesNext);

    if ((rxBytesNext - rxBytesPre) < MAXIMUM_BYTE) {
        strRx = std::to_string(rxBytesNext - rxBytesPre);
        strRx += "B/s";
    } else if ((rxBytesNext - rxBytesPre) < MAXIMUM_KILOBYTE) {
        strRx = std::to_string((rxBytesNext - rxBytesPre) / MAXIMUM_KILOBYTE);
        strRx += "M/s";
    } else {
        strRx = std::to_string((rxBytesNext - rxBytesPre) / MAXIMUM_BYTE);
        strRx += "KB/s";
    }

    if ((txBytesNext - txBytesPre) < MAXIMUM_BYTE) {
        strTx = std::to_string(txBytesNext - txBytesPre);
        strTx += "B/s";
    } else if ((txBytesNext - txBytesPre) < MAXIMUM_KILOBYTE) {
        strTx = std::to_string((txBytesNext - txBytesPre) / MAXIMUM_KILOBYTE);
        strTx += "M/s";
    } else {
        strTx = std::to_string((txBytesNext - txBytesPre) / MAXIMUM_BYTE);
        strTx += "KB/s";
    }
    WIFI_LOGI("GetNetSpeed strRx =  %{public}s\n", strRx.c_str());
    WIFI_LOGI("GetNetSpeed strTx =  %{public}s\n", strTx.c_str());
}

std::vector<std::string> StaNetWorkSpeed::SplitString(std::string source, const std::string delim)
{
    std::vector<std::string> res;
    if (source.empty()) {
        return res;
    }

    if (delim.empty()) {
        res.push_back(source);
        return res;
    }
    std::string::size_type begPos = 0;
    std::string::size_type endPos = 0;
    std::string tmpStr;
    while ((endPos = source.find(delim, begPos)) != std::string::npos) {
        if (endPos > begPos) {
            tmpStr = source.substr(begPos, endPos - begPos);
            res.push_back(tmpStr);
        }
        begPos = endPos + delim.size();
    }
    tmpStr = source.substr(begPos);
    if (!tmpStr.empty()) {
        res.push_back(tmpStr);
    }
    return res;
}

ErrCode StaNetWorkSpeed::GetTxAndRxBytes(long &rxBytes, long &txBytes)
{
    std::ifstream fs(WLAN0_NETSPEED_FILE.c_str());
    if (!fs.is_open()) {
        return ErrCode::WIFI_OPT_FAILED;
    }

    std::string line;
    while (std::getline(fs, line)) {
        if (line.empty()) {
            continue;
        }

        if (line.find(IF_NAME) == std::string::npos) {
            continue;
        }

        std::vector<std::string> splitDataList;
        splitDataList = SplitString(line.substr(line.find(":")), " ");
        rxBytes = std::atol(splitDataList[UP_TRAFFIC_INDEX].c_str());
        txBytes = std::atol(splitDataList[REV_TRAFFIC_INDEX].c_str());
    }
    fs.close();
    return ErrCode::WIFI_OPT_SUCCESS;
}
}  // namespace Wifi
}  // namespace OHOS