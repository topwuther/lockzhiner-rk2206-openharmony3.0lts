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

/**
 * @addtogroup Lockzhiner
 *
 * @file wifi.h
 */

#ifndef LZ_HARDWARE_WIFI_H
#define LZ_HARDWARE_WIFI_H

#define WIFI_MAX_SSID_LEN    33 // 32 + \0
#define WIFI_MAC_LEN         6
#define WIFI_MAX_KEY_LEN     65 // 64 + \0

typedef struct _WifiScanResult {
    /** Service set ID (SSID). For its length. */
    char ssid[WIFI_MAX_SSID_LEN];
    /** Basic service set ID (BSSID). For its length. */
    unsigned char bssid[WIFI_MAC_LEN];
    /** Security type. For details. */
    int securityType;
    /** Received signal strength indicator (RSSI) */
    int rssi;
    /** Frequency band */
    int band;
    /** Frequency in MHz */
    int frequency;
} WifiScanResult;

typedef enum {
    LZ_HARDWARE_WIFI_DISCONNECTED,
    LZ_HARDWARE_WIFI_CONNECTED,
} WifiConnStatus;

typedef struct _WifiConnInfo {
    char ssid[WIFI_MAX_SSID_LEN];
    unsigned char bssid[WIFI_MAC_LEN];
    unsigned int channel;
    WifiConnStatus status;
} WifiConnInfo;

unsigned int WifiEnable(void);
unsigned int WifiDisable(void);
unsigned int WifiStartStation(void);
unsigned int WifiConnect(unsigned char *ssid, unsigned char *passphrase);
unsigned int WifiGetConnectInfo(WifiConnInfo *info);
unsigned int WifiDisconnect(unsigned short reasonCode);
unsigned int WifiStartScan(void);
unsigned int WifiAbortScan(void);
unsigned int WifiGetScanResult(WifiScanResult **result, unsigned int *size);
unsigned int WifiRegisterEvent(void);
unsigned int WifiUnregisterEvent(void);
unsigned int WifiConfigAP(unsigned char *ssid, unsigned char *preSharedKey, unsigned short channel);
unsigned int WifiStartAP(void);
unsigned int WifiStopAP(void);
unsigned int WifiResidentSet(int enter);

#endif
/** @} */
