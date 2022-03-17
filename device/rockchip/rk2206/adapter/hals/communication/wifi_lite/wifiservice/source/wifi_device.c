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

#include <securec.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwip/if_api.h"
#include "lwip/netifapi.h"
#include "wifi_device.h"
#include "wifi_device_util.h"
#include "wifi_hotspot_config.h"
#include "utils_file.h"

#include "lz_hardware.h"

#define WIFI_DEVICE_TAG "WIFI_DEVICE"

static WifiDeviceConfig g_wifiConfigs[WIFI_MAX_CONFIG_SIZE] = {
    {0}, {0}, {0}, 0, WIFI_CONFIG_INVALID, 0, 0, UNKNOWN, {0, 0, {0, 0}, 0}
};
static int g_networkId = -1;
static int g_staStatus = WIFI_STA_NOT_ACTIVE;
static int g_scan_flag = 0;
static WifiEvent *g_event = NULL;

WifiErrorCode EnableWifi(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_staStatus == WIFI_STA_ACTIVE) {
        return ERROR_WIFI_BUSY;
    }

    if (WifiEnable() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiStartStation() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    g_staStatus = WIFI_STA_ACTIVE;
    return WIFI_SUCCESS;
}

WifiErrorCode DisableWifi(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_staStatus == WIFI_STA_NOT_ACTIVE) {
        return ERROR_WIFI_NOT_STARTED;
    }

    if (WifiDisable() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    g_staStatus = WIFI_STA_NOT_ACTIVE;
    return WIFI_SUCCESS;
}

int IsWifiActive(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    int ret = g_staStatus;
    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    return ret;
}

WifiErrorCode Scan(void)
{
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiStartScan() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    g_scan_flag = 1;
    sleep(5);
    if(g_event)
    {
        g_event->OnWifiScanStateChanged(1,0);
    }
    return WIFI_SUCCESS;
}

WifiErrorCode AdvanceScan(WifiScanParams *params)
{
    if (params == NULL) {
        return ERROR_WIFI_UNKNOWN;
    }

    // if (params->scanType == 0) {
    //     return ERROR_WIFI_UNKNOWN;
    // }

    if (params->scanType == WIFI_BSSID_SCAN) {
        if (params->bssid[0] == 0) {
            return ERROR_WIFI_UNKNOWN;
        }
    }

    if (params->scanType == WIFI_SSID_SCAN) {
        if (params->ssidLen == 0) {
            return ERROR_WIFI_UNKNOWN;
        }
    }

    if (params->scanType == WIFI_FREQ_SCAN) {
        if (params->freqs == 0) {
            return ERROR_WIFI_UNKNOWN;
        }
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (LockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiStartScan() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    g_scan_flag = 1;
    sleep(5);
    if(g_event)
    {
        g_event->OnWifiScanStateChanged(1,0);
    }

    return WIFI_SUCCESS;
}

WifiErrorCode GetScanInfoList(WifiScanInfo* result, unsigned int* size)
{
    WifiScanResult *scanResult;
    unsigned int scanSize;

    if (result == NULL || size == NULL || *size == 0) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (WifiGetScanResult(&scanResult, &scanSize) != WIFI_SUCCESS) {
        if(g_scan_flag == 0)
        {
            *size = 0;
            return WIFI_SUCCESS;
        }

        return ERROR_WIFI_UNKNOWN;
    }

    if(g_scan_flag)
    {
        g_scan_flag = 0;
    }

    for (int i = 0; i < scanSize; i++) {
        memcpy(result[i].ssid, scanResult[i].ssid, WIFI_MAX_SSID_LEN);
        memcpy(result[i].bssid, scanResult[i].bssid, WIFI_MAC_LEN);
        result[i].securityType = scanResult[i].securityType;
        result[i].rssi = scanResult[i].rssi;
        result[i].band = scanResult[i].band;
        result[i].frequency = scanResult[i].frequency;
    }

    *size = scanSize;
    return WIFI_SUCCESS;
}

WifiErrorCode AddDeviceConfig(const WifiDeviceConfig* config, int* result)
{
    if (config == NULL || result == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    int netId = WIFI_CONFIG_INVALID;
    for (int i = 0; i < WIFI_MAX_CONFIG_SIZE; i++) {
        if (g_wifiConfigs[i].netId != i) {
            netId = i;
            break;
        }
    }

    if (netId == WIFI_CONFIG_INVALID) {
        LZ_HARDWARE_LOGD(WIFI_DEVICE_TAG, "AddDeviceConfig: wifi config is full, delete one first");
        if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
            return ERROR_WIFI_UNKNOWN;
        }
        return ERROR_WIFI_BUSY;
    }

    int err = memcpy_s(&g_wifiConfigs[netId], sizeof(WifiDeviceConfig), config, sizeof(WifiDeviceConfig));
    if (err != EOK) {
        if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
            return ERROR_WIFI_UNKNOWN;
        }
        LZ_HARDWARE_LOGD(WIFI_DEVICE_TAG, "AddDeviceConfig: memcpy failed, err = %d", err);
        return ERROR_WIFI_UNKNOWN;
    }

    g_wifiConfigs[netId].netId = netId;

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    *result = netId;
    return WIFI_SUCCESS;
}

WifiErrorCode GetDeviceConfigs(WifiDeviceConfig* result, unsigned int* size)
{
    if (result == NULL || size == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    unsigned int retIndex = 0;

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    for (int i = 0; i < WIFI_MAX_CONFIG_SIZE; i++) {
        if (g_wifiConfigs[i].netId != i) {
            continue;
        }

        int err = memcpy_s(&result[retIndex], sizeof(WifiDeviceConfig), &g_wifiConfigs[i], sizeof(WifiDeviceConfig));
        if (err != EOK) {
            if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
                return ERROR_WIFI_UNKNOWN;
            }
            LZ_HARDWARE_LOGD(WIFI_DEVICE_TAG, "GetDeviceConfig: memcpy failed, err = %d", err);
            return ERROR_WIFI_UNKNOWN;
        }

        retIndex++;
        if (*size < retIndex) {
            if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
                return ERROR_WIFI_UNKNOWN;
            }
            return ERROR_WIFI_INVALID_ARGS;
        }
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (retIndex == 0) {
        return ERROR_WIFI_NOT_AVAILABLE;
    }

    *size = retIndex;
    return WIFI_SUCCESS;
}

WifiErrorCode ConnectTo(int networkId)
{
    if (networkId >= WIFI_MAX_CONFIG_SIZE || networkId < 0) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_staStatus != WIFI_STA_ACTIVE) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (g_wifiConfigs[networkId].netId != networkId) {
        if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
            return ERROR_WIFI_UNKNOWN;
        }
        return ERROR_WIFI_NOT_AVAILABLE;
    }

    g_networkId = networkId;

    if (WifiConnect(g_wifiConfigs[networkId].ssid,
            g_wifiConfigs[networkId].preSharedKey) != WIFI_SUCCESS) {
        if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
            return ERROR_WIFI_UNKNOWN;
        }
        return ERROR_WIFI_UNKNOWN;
    }

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (LockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    if (UnlockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

WifiErrorCode Disconnect(void)
{
    if (g_staStatus != WIFI_STA_ACTIVE) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (WifiDisconnect(0) != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

WifiErrorCode RemoveDevice(int networkId)
{
    if (networkId >= WIFI_MAX_CONFIG_SIZE || networkId < 0) {
        return ERROR_WIFI_INVALID_ARGS;
    }
    if (LockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (memset_s(&g_wifiConfigs[networkId], sizeof(WifiDeviceConfig),
            0, sizeof(WifiDeviceConfig)) != EOK) {
        LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "RemoveDevice: memset failed");
    }
    g_wifiConfigs[networkId].netId = WIFI_CONFIG_INVALID;

    if (UnlockWifiGlobalLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

int GetLocalWifiGw(int *const gw)
{
    struct netif *netif = netif_find("wlan0");
    if (netif == NULL) {
        LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "GetLocalWifiGw: netif get fail\n");
        return ERROR_WIFI_UNKNOWN;
    }

    *gw = ip_addr_get_ip4_u32(&netif->gw);
    return WIFI_SUCCESS;
}

int SetLocalWifiGw()
{
    ip4_addr_t gw;
    int i;
    struct netif *netif;
    for (i = 0; i < 10; i++) {
        netif = netif_get_by_index(i);
        if (netif != NULL) {
            LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "%c %c num:%d %s", netif->name[0], netif->name[1], netif->num, inet_ntoa(ip_addr_get_ip4_u32(&netif->ip_addr)));
        }
    }

    netif = netif_find("wlan0");
    if (netif == NULL) {
        LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "SetLocalWifiGw: netif get fail\n");
        return ERROR_WIFI_UNKNOWN;
    }
    IP4_ADDR(&gw, 0, 0, 0, 0);
    netifapi_netif_set_default(netif);
    return WIFI_SUCCESS;
}
int GetLocalWifiNetmask(int *const netmask)
{
    struct netif *netif = netif_find("wlan0");
    if (netif == NULL) {
        LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "GetLocalWifiNetmask: netif get fail\n");
        return ERROR_WIFI_UNKNOWN;
    }

    *netmask = ip_addr_get_ip4_u32(&netif->netmask);
    return WIFI_SUCCESS;
}

static int GetLocalWifiIp(int * const ip)
{
    struct netif *netif = netif_find("wlan0");
    if (netif == NULL) {
        LZ_HARDWARE_LOGE(WIFI_DEVICE_TAG, "GetLocalWifiIp: netif get fail\n");
        return ERROR_WIFI_UNKNOWN;
    }

    *ip = ip_addr_get_ip4_u32(&netif->ip_addr);
    return WIFI_SUCCESS;
}

WifiErrorCode GetLinkedInfo(WifiLinkedInfo* result)
{
    WifiConnInfo connInfo;

    if (result == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (WifiGetConnectInfo(&connInfo) != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }

    if (connInfo.status == LZ_HARDWARE_WIFI_CONNECTED) {
        result->connState = WIFI_CONNECTED;
        result->rssi = 0;
    } else {
        result->connState = WIFI_DISCONNECTED;
    }

    if (GetLocalWifiIp(&(result->ipAddress)) != EOK) {
        return ERROR_WIFI_UNKNOWN;
    }

    return WIFI_SUCCESS;
}

WifiErrorCode RegisterWifiEvent(WifiEvent* event)
{
    if (event == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }
    g_event = event;
    g_event->OnWifiScanStateChanged(1,0);
    return WIFI_SUCCESS;
}

WifiErrorCode UnRegisterWifiEvent(const WifiEvent* event)
{
    if (event == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    if (LockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    g_event = NULL;
    if (UnlockWifiEventLock() != WIFI_SUCCESS) {
        return ERROR_WIFI_UNKNOWN;
    }
    return WIFI_SUCCESS;
}

WifiErrorCode GetDeviceMacAddress(unsigned char* result)
{
    if (result == NULL) {
        return ERROR_WIFI_INVALID_ARGS;
    }

    return WIFI_SUCCESS;
}
