/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Copyright (c) 2021 Lockzhiner Electronics Co., Ltd.
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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "cJSON.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_compiler.h"

#include "config_network.h"
#include "lz_hardware.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define OS_SLEEP_S(val)         ((val) * 1000)
#define OS_TASK_STACK_SIZE      0x2000
#define OS_PORT                 9997
#define RKNC_HEAD               "rknc"
#define RKBC_HEAD               "rkbc"

#define LOG_TAG                 "coonfig_network"

#define SSID                    "凌智电子"
#define PASSWORD                "88888888"

STATIC RKWifiConfig g_wificonfig = {0};

STATIC UINT32 g_wifiTask;
STATIC UINT32 g_softbusliteTask;
STATIC UINT32 g_apTask;

////////////////////////////////////////////////////////////////

static WifiErrorCode SetApModeOn()
{
    WifiErrorCode error;
    HotspotConfig config = {0};
    
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetApModeOn start ...\n");

    memcpy_s(config.ssid, WIFI_MAX_SSID_LEN, SSID, sizeof(SSID));
    memcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, PASSWORD, sizeof(PASSWORD));
    config.channelNum = 1;
    error = SetHotspotConfig(&config);
    if (error != WIFI_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork SetHotspotConfig ...error: %d\n", error);
        return error;
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork EnableHotspot ...\n");

    error = EnableHotspot();
    if (error != WIFI_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork EnableHotspot ...error: %d\n", error);
        return error;
    }
    
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork EnableHotspot done");
    return error;
}

static WifiErrorCode SetApModeOff()
{
    WifiErrorCode error;

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetApModeOff start ...\n");
    // check AP stat
    error = IsHotspotActive();
    if (error == WIFI_HOTSPOT_ACTIVE) {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork AP is active, disable now...\n");
        error = DisableHotspot();
        if (error == WIFI_SUCCESS) {
            LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork disable AP success\n");
        } else {
            LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork disable AP fail, please disable ap, error: %d\n", error);
            return error;
        }
    }
    else {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork AP is inactive\n");
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetApModeOff end ...\n");
    return error;

}

static WifiErrorCode SetWifiModeOff()
{
    WifiErrorCode error;
    // check wifi stat
    int ret = IsWifiActive();
    if (ret == WIFI_STATE_AVALIABLE) {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork wifi is active, disable now...\n");
        error = DisableWifi();
        if (error == WIFI_SUCCESS) {
            LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork disable wifi success\n");
        } else {
            LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork disable wifi fail, please disable wifi, error: %d\n", error);
            return error;
        }
    }
    else {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork wifi is inactive\n");
    }

    return error;
}

static WifiErrorCode SetWifiScan()
{
#if WIFI_SCAN_ON
    WifiErrorCode error;
    WifiScanInfo *infoList;
    unsigned int retry = 15;
    unsigned int size = WIFI_SCAN_HOTSPOT_LIMIT;

    error = Scan();
    if (error != WIFI_SUCCESS)
    {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork Scan error: %d\n", error);
        return error;
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork Scan done\n");

    infoList = malloc(sizeof(WifiScanInfo) * WIFI_SCAN_HOTSPOT_LIMIT);
    while (retry) {
        error = GetScanInfoList(infoList, &size);
        if (error != WIFI_SUCCESS || size == 0) {
            LOS_TaskDelay(1000);
            retry--;
            continue;
        }

        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork GetScanResult done\n");
        goto scan_done;
    }

    free(infoList);
    return;

    scan_done:
    if (size > 0) {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork BSSID    SSID    RSSI    Channel\n");
        LZ_HARDWARE_LOGD(LOG_TAG, "========================================\n");
        for (int i = 0; i < size; i++) {
            char bssid[18];
            char ssid[33];
            sprintf(bssid, "%02x:%02x:%02x:%02x:%02x:%02x",
                    infoList[i].bssid[0],
                    infoList[i].bssid[1],
                    infoList[i].bssid[2],
                    infoList[i].bssid[3],
                    infoList[i].bssid[4],
                    infoList[i].bssid[5]);
            sprintf(ssid, "%-32s", infoList[i].ssid);
            LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork %s    %s    %d    %u\n",
                   bssid,
                   ssid,
                   infoList[i].rssi,
                   infoList[i].frequency);
            if (strncmp(ssid, g_wificonfig.ssid) == 0) {
                snprintf(g_wificonfig.bssid, sizeof(g_wificonfig.bssid), "%s", bssid);
                LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork g_wificonfig.bssid: %s\n", g_wificonfig.bssid);
            }
        }
        LZ_HARDWARE_LOGD(LOG_TAG, "========================================\n");
    }
    free(infoList);
    return error;
#endif  /*WIFI_SCAN_ON*/
    return WIFI_SUCCESS;
}

WifiErrorCode SetWifiModeOn()
{
    WifiErrorCode error;
    int netId = 0;
    WifiDeviceConfig config = {0};

    memcpy_s(g_wificonfig.ssid, sizeof(g_wificonfig.ssid), SSID, sizeof(SSID));
    memcpy_s(g_wificonfig.psk, sizeof(g_wificonfig.psk), PASSWORD, sizeof(PASSWORD));

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetWifiModeOn\n");
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork g_wificonfig.ssid %s\n", g_wificonfig.ssid);
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork g_wificonfig.psk %s\n", g_wificonfig.psk);
    error = EnableWifi();
    if (error != WIFI_SUCCESS)
    {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork EnableWifi error: %d\n", error);
        return error;
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork EnableWifi done\n");

    SetWifiScan();
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetWifiScan after g_wificonfig.bssid: %s\n", g_wificonfig.bssid);

    config.freq = 1;
    config.securityType = WIFI_SEC_TYPE_PSK;
    config.wapiPskType = WIFI_PSK_TYPE_ASCII;

    memcpy_s(config.ssid, WIFI_MAX_SSID_LEN, g_wificonfig.ssid, sizeof(g_wificonfig.ssid));
    memcpy_s(config.bssid, WIFI_MAC_LEN, g_wificonfig.bssid, sizeof(g_wificonfig.bssid));
    memcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, g_wificonfig.psk, sizeof(g_wificonfig.psk));

    if (WifiConnect(config.ssid, config.preSharedKey) != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_TAG, "WifiConnect  error");
        return LZ_HARDWARE_FAILURE;
    }

    LZ_HARDWARE_LOGI(LOG_TAG, "ConnectTo (%s) done", config.ssid);

    WifiLinkedInfo info;
    int gw, netmask;
    memset(&info, 0, sizeof(WifiLinkedInfo));
    unsigned int retry = 15;
    while (retry) {
        if (GetLinkedInfo(&info) == WIFI_SUCCESS) {
            if (info.connState == WIFI_CONNECTED) {
                if (info.ipAddress != 0) {
                    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork IP (%s)", inet_ntoa(info.ipAddress));
                    if (WIFI_SUCCESS == GetLocalWifiGw(&gw)) {
                        LZ_HARDWARE_LOGD(LOG_TAG, "network GW (%s)", inet_ntoa(gw));
                    }
                    if (WIFI_SUCCESS == GetLocalWifiNetmask(&netmask)) {
                        LZ_HARDWARE_LOGD(LOG_TAG, "network NETMASK (%s)", inet_ntoa(netmask));
                    }
                    if (WIFI_SUCCESS == SetLocalWifiGw()) {
                        LZ_HARDWARE_LOGD(LOG_TAG, "set network GW");
                    }
                    if (WIFI_SUCCESS == GetLocalWifiGw(&gw)) {
                        LZ_HARDWARE_LOGD(LOG_TAG, "network GW (%s)", inet_ntoa(gw));
                    }
                    if (WIFI_SUCCESS == GetLocalWifiNetmask(&netmask)) {
                        LZ_HARDWARE_LOGD(LOG_TAG, "network NETMASK (%s)", inet_ntoa(netmask));
                    }
                    goto connect_done;
                }
            }
        }

        LOS_TaskDelay(1000);
        retry--;
    }

connect_done:
    return error;
}


static void TaskConfigApModeEntry()
{
    SetWifiModeOff();
    SetApModeOn();
}

static void TaskConfigWifiModeEntry()
{
    SetApModeOff();
    SetWifiModeOn();
}


UINT32 TaskConfigApMode(VOID)
{
    UINT32  ret;
    TSK_INIT_PARAM_S task = { 0 };


    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskConfigApModeEntry;
    task.uwStackSize  = OS_TASK_STACK_SIZE;
    task.pcName       = "taskConfigApModeEntry";
    task.usTaskPrio   = 8;
    ret = LOS_TaskCreate(&g_apTask, &task);
    if (ret != LOS_OK) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork LOS_TaskCreate taskConfigApModeEntry error: %d\n", ret);
        return ret;
    }

    return LOS_OK;
}


UINT32 TaskConfigWifiMode(VOID)
{
    UINT32  ret;
    TSK_INIT_PARAM_S task = { 0 };


    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskConfigWifiModeEntry;
    task.uwStackSize  = OS_TASK_STACK_SIZE;
    task.pcName       = "taskConfigWifiModeEntry";
    task.usTaskPrio   = 8;
    ret = LOS_TaskCreate(&g_wifiTask, &task);
    if (ret != LOS_OK) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork LOS_TaskCreate taskConfigWifiModeEntry error: %d\n", ret);
        return ret;
    }

    return LOS_OK;
}


UINT32 ExternalTaskConfigNetwork(VOID)
{
    UINT32  ret;

    //ret = TaskConfigApMode();
    ret = TaskConfigWifiMode();

    return ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
