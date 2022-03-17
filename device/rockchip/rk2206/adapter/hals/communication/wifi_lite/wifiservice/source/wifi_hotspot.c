/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

#include "wifi_hotspot.h"
#include <securec.h>
#include "lwip/netifapi.h"
#include "wifi_device_util.h"

#define RSSI_LEVEL_4_2_G (-65)
#define RSSI_LEVEL_3_2_G (-75)
#define RSSI_LEVEL_2_2_G (-82)
#define RSSI_LEVEL_1_2_G (-88)

#define RSSI_LEVEL_4_5_G (-65)
#define RSSI_LEVEL_3_5_G (-72)
#define RSSI_LEVEL_2_5_G (-79)
#define RSSI_LEVEL_1_5_G (-85)

#define IP_AP_ADDR0 192
#define IP_AP_ADDR1 168
#define IP_AP_ADDR2 5
#define IP_AP_ADDR3 1
#define GW_AP_ADDR0 192
#define GW_AP_ADDR1 168
#define GW_AP_ADDR2 5
#define GW_AP_ADDR3 1
#define NETMSK_ADDR0 255
#define NETMSK_ADDR1 255
#define NETMSK_ADDR2 255
#define NETMSK_ADDR3 0

#define WIFI_TPC_MAX_POWER 20
#define WIFI_TPC_ID 35
#define WIFI_TPC_LEN 2
#define WLAN_AP_NAME "ap0"

static int g_apStatus = WIFI_HOTSPOT_NOT_ACTIVE;
static HotspotConfig g_apConfig = {0};

WifiErrorCode EnableHotspot(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_apStatus == WIFI_HOTSPOT_ACTIVE) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiStartAP() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    g_apStatus = WIFI_HOTSPOT_ACTIVE;
    return WIFI_SUCCESS;
}

WifiErrorCode DisableHotspot(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_apStatus == WIFI_HOTSPOT_NOT_ACTIVE) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiStopAP() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    g_apStatus = WIFI_HOTSPOT_NOT_ACTIVE;
    return WIFI_SUCCESS;
}

WifiErrorCode SetHotspotConfig(const HotspotConfig* config)
{
    if (config == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    int err = memcpy_s(&g_apConfig, sizeof(HotspotConfig), config, sizeof(HotspotConfig));
    if (err != EOK) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiConfigAP(config->ssid, config->preSharedKey, config->channelNum) != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    return WIFI_SUCCESS;
}

WifiErrorCode GetHotspotConfig(HotspotConfig* result)
{
    if (result == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    int err = memcpy_s(result, sizeof(HotspotConfig), &g_apConfig, sizeof(HotspotConfig));
    if (err != EOK) {
        return ERROR_WIFI_UNKNOWN;
    }

    return WIFI_SUCCESS;
}

int IsHotspotActive(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    int ret = g_apStatus;
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    return ret;
}

WifiErrorCode GetStationList(StationInfo* result, unsigned int* size)
{
    if (result == NULL || size == NULL || *size == 0) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    return WIFI_SUCCESS;
}

WifiErrorCode SetBand(int band)
{
    if (band != HOTSPOT_BAND_TYPE_2G) {
        return ERROR_WIFI_NOT_SUPPORTED;
    }
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    g_apConfig.band = band;
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

WifiErrorCode GetBand(int* result)
{
    if (result == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_apConfig.band == 0) {
        if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
            return ERROR_WIFI_UNKNOWN;
        }
        return ERROR_WIFI_NOT_AVAILABLE;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    *result = HOTSPOT_BAND_TYPE_2G;

    return WIFI_SUCCESS;
}

int GetSignalLevel(int rssi, int band)
{
    if (band == HOTSPOT_BAND_TYPE_2G) {
        if (rssi >= RSSI_LEVEL_4_2_G) {
            return RSSI_LEVEL_4;
        }
        if (rssi >= RSSI_LEVEL_3_2_G) {
            return RSSI_LEVEL_3;
        }
        if (rssi >= RSSI_LEVEL_2_2_G) {
            return RSSI_LEVEL_2;
        }
        if (rssi >= RSSI_LEVEL_1_2_G) {
            return RSSI_LEVEL_1;
        }
    }

    if (band == HOTSPOT_BAND_TYPE_5G) {
        if (rssi >= RSSI_LEVEL_4_5_G) {
            return RSSI_LEVEL_4;
        }
        if (rssi >= RSSI_LEVEL_3_5_G) {
            return RSSI_LEVEL_3;
        }
        if (rssi >= RSSI_LEVEL_2_5_G) {
            return RSSI_LEVEL_2;
        }
        if (rssi >= RSSI_LEVEL_1_5_G) {
            return RSSI_LEVEL_1;
        }
    }

    return ERROR_WIFI_INVALID_ARGS;
}

int GetHotspotChannel(void)
{
    return 0;
}

WifiErrorCode GetHotspotInterfaceName(char* result, int size)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

WifiErrorCode DisassociateSta(unsigned char* mac, int macLen)
{
    return WIFI_SUCCESS;
}

WifiErrorCode AddTxPowerInfo(int power)
{
    return WIFI_SUCCESS;
}
