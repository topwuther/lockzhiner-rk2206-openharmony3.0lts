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
#include "wifi_device.h"
#include "wifi_hotspot.h"
#include "lwip/inet.h"
#include "stdbool.h"
#include "lz_hardware.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define WIFI_SCAN_ON 0

typedef enum RK_BUTTOON_STATUS {
    AP_BUTTON_UP = 0,  /** UP mode */
    AP_BUTTON_DOWN = 1    /** DOWN mode */
} RKButtonStatus;

typedef struct RK_WIFI_CONFIG{
    char ssid[WIFI_MAX_SSID_LEN];
    char psk[WIFI_MAX_KEY_LEN];
    unsigned char bssid[WIFI_MAC_LEN];
} RKWifiConfig;

typedef struct RK_BOARD_CONFIG {
    char rkbc_head[4];    // head
    char rkbc_led_status;      // led status
    char rkbc_key_status[2]; // key status
    char rkbc_buzzer_status; // buzzer status
    char rkbc_light_status; // light status
    char rkbc_human_body_sensor_status; //human body sensor status
    char rkbc_gas_status; // gas status
    char rkbc_temperature_value[2]; // temperature value
    char rkbc_humidity_value[2]; // humidity value
    char rkbc_expand_data[16]; // expand data
} RKBoardConfig;

typedef struct RK_NETWORK_CONFIG {
    char rknc_head[4];    // head
    char rknc_ssid_length;      // ssid len
    char  rknc_psk_length; // password len
    char  rknc_data[]; // ssid data + password data
} RKNetworkConfig;


UINT32 ExternalTaskConfigNetwork(VOID);
int ExternalSyncBCState(RKBoardConfig BCstate);
void ExternalSetButtonStatus(bool isdown);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
