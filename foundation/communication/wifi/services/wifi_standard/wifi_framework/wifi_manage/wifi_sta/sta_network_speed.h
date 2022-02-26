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

#ifndef OHOS_WIFI_NET_SPEED_H
#define OHOS_WIFI_NET_SPEED_H

#include <unistd.h>
#include <cstring>
#include <fstream>
#include <vector>
#include "wifi_log.h"
#include "sta_define.h"
#include "wifi_errcode.h"

static const std::string IF_NAME = "wlan0";
static const std::string WLAN0_NETSPEED_FILE = "/proc/net/dev";

static const int MAXIMUM_BYTE = 1024;
static const int MAXIMUM_KILOBYTE = 1048576;

static const int UP_TRAFFIC_INDEX = 1;
static const int REV_TRAFFIC_INDEX = 9;

namespace OHOS {
namespace Wifi {
class StaNetWorkSpeed {
public:
    StaNetWorkSpeed();
    ~StaNetWorkSpeed();
    /**
     * @Description : Get internet speed
     *
     * @param strRx - download speed string[out]
     * @param strTx - Upload speed string [out]
     */
    void GetNetSpeed(std::string &strRx, std::string &strTx);

private:
    /**
     * @Description : Get upload and download speed.
     *
     * @param rxBytes - download speed long[out]
     * @param txBytes - Upload speed long [out]
     * @Return success:WIFI_OPT_SUCCESS  failed: WIFI_OPT_FAILED
     */
    ErrCode GetTxAndRxBytes(long &rxBytes, long &txBytes);
    /**
     * @Description : Split string according to delimiter.
     *
     * @param source - Source string[in]
     * @param delim - Delimiter string [in]
     * @Return : Separated string vector
     */
    std::vector<std::string> SplitString(std::string source, const std::string delim);
};
}  // namespace Wifi
}  // namespace OHOS
#endif