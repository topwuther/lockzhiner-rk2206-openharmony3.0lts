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
#define OS_TASK_STACK_SIZE      0x4000
#define OS_PORT                 9997
#define RKNC_HEAD               "rknc"
#define RKBC_HEAD               "rkbc"

#define LOG_TAG                 "config_network"

// 该处为Wifi模式默认的SSID和密码，即Flash如为空，则填写默认
// 如需要修改，请直接调用set_wifi_config_route_ssid()和set_wifi_config_route_passwd()函数
// 如需要修改，可以TaskConfigWifiModeEntry()启用上述2个函数
#define ROUTE_SSID              "凌智电子"
#define ROUTE_PASSWORD          "88888888"
// 该处为AP模式的默认SSID和密码，即Flash如为空，则填写默认
// 如需要修改，请直接调用set_wifi_config_ssid()和set_wifi_config_passwd()函数
#define AP_SSID                 "凌智电子"
#define AP_PASSWORD             "88888888"


STATIC RKWifiConfig g_wificonfig = {0};

STATIC UINT32 g_wifiTask;
STATIC UINT32 g_softbusliteTask;
STATIC UINT32 g_apTask;

typedef struct 
{
    bool   init;
    bool   ap_on;
    bool   sta_on;
    UINT32 muxlock;
}wifi_mode_lock_t;

wifi_mode_lock_t m_wml = {
    .init   = false,
    .ap_on  = false,
    .sta_on = false,
};

#define MUX_LOCK_TO_TIME  20000


//设置wifi默认配置
void set_default_wifi_config(void)
{
    FlashInit();         
    uint8_t hwaddr[6]  = {0x10, 0xdc, 0xb6, 0x90, 0x00, 0x00};
    uint8_t ip[4]      = {192, 168, 2, 10};
    uint8_t gateway[4] = {192, 168, 2, 1};
    uint8_t mask[4]    = {255, 255, 255, 0};

    VendorSet(VENDOR_ID_SN,                "LZ01",         sizeof("LZ01"));
    VendorSet(VENDOR_ID_PRODUCT,           "小凌派",       sizeof("小凌派"));
    VendorSet(VENDOR_ID_FACTORY,           "凌睿智捷",     sizeof("凌睿智捷"));
    VendorSet(VENDOR_ID_WIFI_MODE,         "STA",          3);
    VendorSet(VENDOR_ID_MAC,               hwaddr,         6);
    VendorSet(VENDOR_ID_NET_IP,            ip,             4);
    VendorSet(VENDOR_ID_NET_GW,            gateway,        4);
    VendorSet(VENDOR_ID_NET_MASK,          mask,           4);
    VendorSet(VENDOR_ID_WIFI_SSID,         AP_SSID,        sizeof(AP_SSID));
    VendorSet(VENDOR_ID_WIFI_PASSWD,       AP_PASSWORD,    sizeof(AP_PASSWORD));
    VendorSet(VENDOR_ID_WIFI_ROUTE_SSID,   ROUTE_SSID,     sizeof(ROUTE_SSID));
    VendorSet(VENDOR_ID_WIFI_ROUTE_PASSWD, ROUTE_PASSWORD, sizeof(ROUTE_PASSWORD));
} 

// 设置wifi 配置 
void set_wifi_config(wifi_config_t wifi_config)
{
    FlashInit();         

    VendorSet(VENDOR_ID_SN,                "LZ01",     sizeof("LZ01"));
    VendorSet(VENDOR_ID_PRODUCT,           "小凌派",   sizeof("小凌派"));
    VendorSet(VENDOR_ID_FACTORY,           "凌睿智捷", sizeof("凌睿智捷"));

    VendorSet(VENDOR_ID_WIFI_MODE,         wifi_config.mode,           3);
    VendorSet(VENDOR_ID_MAC,               wifi_config.hwaddr,         6);
    VendorSet(VENDOR_ID_NET_IP,            wifi_config.ip,             4);
    VendorSet(VENDOR_ID_NET_GW,            wifi_config.gateway,        4);
    VendorSet(VENDOR_ID_NET_MASK,          wifi_config.mask,           4);
    VendorSet(VENDOR_ID_WIFI_SSID,         wifi_config.ssid,           sizeof(wifi_config.ssid));
    VendorSet(VENDOR_ID_WIFI_PASSWD,       wifi_config.password,       sizeof(wifi_config.password));
    VendorSet(VENDOR_ID_WIFI_ROUTE_SSID,   wifi_config.route_ssid,     sizeof(wifi_config.route_ssid));
    VendorSet(VENDOR_ID_WIFI_ROUTE_PASSWD, wifi_config.route_password, sizeof(wifi_config.route_password));
}

//获取wifi配置
void get_wifi_config(printf_fn pfn, wifi_config_t *wifi_config)
{
    if(wifi_config == NULL) return;
    FlashInit();         
    VendorGet(VENDOR_ID_SN,                wifi_config->sn,             sizeof(wifi_config->sn));
    VendorGet(VENDOR_ID_PRODUCT,           wifi_config->product,        sizeof(wifi_config->product));
    VendorGet(VENDOR_ID_FACTORY,           wifi_config->factory,        sizeof(wifi_config->factory));
    VendorGet(VENDOR_ID_WIFI_MODE,         wifi_config->mode,           3);
    VendorGet(VENDOR_ID_MAC,               wifi_config->hwaddr,         6);
    VendorGet(VENDOR_ID_NET_IP,            wifi_config->ip,             4);
    VendorGet(VENDOR_ID_NET_GW,            wifi_config->gateway,        4);
    VendorGet(VENDOR_ID_NET_MASK,          wifi_config->mask,           4);
    VendorGet(VENDOR_ID_WIFI_SSID,         wifi_config->ssid,           sizeof(wifi_config->ssid));
    VendorGet(VENDOR_ID_WIFI_PASSWD,       wifi_config->password,       sizeof(wifi_config->password));
    VendorGet(VENDOR_ID_WIFI_ROUTE_SSID,   wifi_config->route_ssid,     sizeof(wifi_config->route_ssid));
    VendorGet(VENDOR_ID_WIFI_ROUTE_PASSWD, wifi_config->route_password, sizeof(wifi_config->route_password));

    if(pfn == NULL) return;

    pfn("sn:           %s\n", wifi_config->sn);
    pfn("product:      %s\n", wifi_config->product);
    pfn("factory:      %s\n", wifi_config->factory);
    pfn("mode:         %s\n", wifi_config->mode);
    pfn("APssid:       %s\n", wifi_config->ssid);
    pfn("APpasswd:     %s\n", wifi_config->password);
    pfn("route_ssid:   %s\n", wifi_config->route_ssid);
    pfn("route_passwd: %s\n", wifi_config->route_password);
    LOS_Msleep(10);

    pfn("mac:          %02x:%02x:%02x:%02x:%02x:%02x\n", 
        wifi_config->hwaddr[0], wifi_config->hwaddr[1], wifi_config->hwaddr[2], 
        wifi_config->hwaddr[3], wifi_config->hwaddr[4], wifi_config->hwaddr[5]);
    pfn("ip:           %d.%d.%d.%d\n", 
        wifi_config->ip[0], wifi_config->ip[1], wifi_config->ip[2], wifi_config->ip[3]);
    pfn("gateway:      %d.%d.%d.%d\n", 
        wifi_config->gateway[0], wifi_config->gateway[1], wifi_config->gateway[2], wifi_config->gateway[3]);
    pfn("mask:         %d.%d.%d.%d\n", 
        wifi_config->mask[0], wifi_config->mask[1], wifi_config->mask[2], wifi_config->mask[3]);
        
}

//设置wifi模式
void set_wifi_config_mode(printf_fn pfn, uint8_t *md)
{
    uint8_t mode[WIFI_MAX_SN_LEN]  = {0};
    if(md == NULL) return;

    FlashInit();
    if(strcasecmp(md, "AP") == 0) memcpy(mode, "AP", 2);
    else memcpy(mode, "STA", 3);

    VendorSet(VENDOR_ID_WIFI_MODE,         mode, 3);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_WIFI_MODE,         wifi_config.mode,   3);
    if(pfn == NULL) return;
    pfn("mode:         %s\n", wifi_config.mode);
}

//设置wifi MAC地址
void set_wifi_config_mac(printf_fn pfn, uint8_t *mac)
{
    uint8_t hwaddr[6]  = {0};
    if(mac == NULL) return;

    FlashInit();         
    memcpy(hwaddr, mac, 6);
    VendorSet(VENDOR_ID_MAC,         hwaddr, 6);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_MAC,         wifi_config.hwaddr,   6);
    if(pfn == NULL) return;
    pfn("mac:          %02x:%02x:%02x:%02x:%02x:%02x\n", 
        wifi_config.hwaddr[0], wifi_config.hwaddr[1], wifi_config.hwaddr[2], 
        wifi_config.hwaddr[3], wifi_config.hwaddr[4], wifi_config.hwaddr[5]);
}

//设置ip地址
void set_wifi_config_ip(printf_fn pfn, uint8_t *ip)
{
    uint8_t ipaddr[4]  = {0};
    if(ip == NULL) return;
    
    FlashInit();         
    memcpy(ipaddr, ip, 4);
    VendorSet(VENDOR_ID_NET_IP,      ipaddr, 4);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_NET_IP,      wifi_config.ip,       4);
    if(pfn == NULL) return;
    pfn("ip:           %d.%d.%d.%d\n", 
        wifi_config.ip[0], wifi_config.ip[1], wifi_config.ip[2], wifi_config.ip[3]);
}

//设置网关
void set_wifi_config_gw(printf_fn pfn, uint8_t *gw)
{
    uint8_t gateway[4]  = {0};
    if(gw == NULL) return;
    
    FlashInit();         
    memcpy(gateway, gw, 4);
    VendorSet(VENDOR_ID_NET_GW,      gateway, 4);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_NET_GW,      wifi_config.gateway,  4);
    if(pfn == NULL) return;
    pfn("gateway:      %d.%d.%d.%d\n", 
        wifi_config.gateway[0], wifi_config.gateway[1], wifi_config.gateway[2], wifi_config.gateway[3]);
}

//设置子网掩码
void set_wifi_config_mask(printf_fn pfn, uint8_t *m)
{
    uint8_t mask[4]  = {0};
    if(m == NULL) return;
    
    FlashInit();         
    memcpy(mask, m, 4);
    VendorSet(VENDOR_ID_NET_MASK,      mask, 4);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_NET_MASK,    wifi_config.mask,     4);
    if(pfn == NULL) return;
    pfn("mask:         %d.%d.%d.%d\n", 
        wifi_config.mask[0], wifi_config.mask[1], wifi_config.mask[2], wifi_config.mask[3]);
}

//设置ap ssid
void set_wifi_config_ssid(printf_fn pfn, uint8_t *s)
{
    uint8_t ssid[WIFI_MAX_SN_LEN]  = {0};
    if(s == NULL) return;

    int len = strlen(s);
    len = len>WIFI_MAX_SN_LEN? WIFI_MAX_SN_LEN : len;

    FlashInit();         
    memcpy(ssid, s, len);
    VendorSet(VENDOR_ID_WIFI_SSID,      ssid, WIFI_MAX_SN_LEN);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_WIFI_SSID,   wifi_config.ssid,     sizeof(wifi_config.ssid));
    if(pfn == NULL) return;
    pfn("APssid:       %s\n", wifi_config.ssid);
}

//设置ap passwd
void set_wifi_config_passwd(printf_fn pfn, uint8_t *p)
{
    uint8_t passwd[WIFI_MAX_SN_LEN]  = {0};
    if(p == NULL) return;

    int len = strlen(p);
    len = len>WIFI_MAX_SN_LEN? WIFI_MAX_SN_LEN : len;
    
    FlashInit();         
    memcpy(passwd, p, len);
    VendorSet(VENDOR_ID_WIFI_PASSWD,      passwd, WIFI_MAX_SN_LEN);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_WIFI_PASSWD, wifi_config.password, sizeof(wifi_config.password));
    if(pfn == NULL) return;
    pfn("APpasswd:     %s\n", wifi_config.password);
}

//设置路由wifi  ssid
void set_wifi_config_route_ssid(printf_fn pfn, uint8_t *s)
{
    uint8_t ssid[WIFI_MAX_SN_LEN]  = {0};
    if(s == NULL) return;

    int len = strlen(s);
    len = len>WIFI_MAX_SN_LEN? WIFI_MAX_SN_LEN : len;
    
    FlashInit();         
    memcpy(ssid, s, len);
    VendorSet(VENDOR_ID_WIFI_ROUTE_SSID,      ssid, WIFI_MAX_SN_LEN);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_WIFI_ROUTE_SSID,   wifi_config.route_ssid,     sizeof(wifi_config.route_ssid));
    if(pfn == NULL) return;
    pfn("route_ssid:   %s\n", wifi_config.route_ssid);
}

//设置路由wifi passwd
void set_wifi_config_route_passwd(printf_fn pfn, uint8_t *p)
{
    uint8_t passwd[WIFI_MAX_SN_LEN]  = {0};
    if(p == NULL) return;

    int len = strlen(p);
    len = len>WIFI_MAX_SN_LEN? WIFI_MAX_SN_LEN : len;
    
    FlashInit();         
    memcpy(passwd, p, len);
    VendorSet(VENDOR_ID_WIFI_ROUTE_PASSWD,      passwd, WIFI_MAX_SN_LEN);

    wifi_config_t wifi_config = {0};
    VendorGet(VENDOR_ID_WIFI_ROUTE_PASSWD, wifi_config.route_password, sizeof(wifi_config.route_password));
    if(pfn == NULL) return;
    pfn("route_passwd: %s\n", wifi_config.route_password);
}


////////////////////////////////////////////////////////////////

WifiErrorCode SetApModeOn()
{
    if(!m_wml.init)
    {
        m_wml.init = true;
        LOS_MuxCreate(&m_wml.muxlock);
    }
    LOS_MuxPend(m_wml.muxlock, MUX_LOCK_TO_TIME);

    WifiErrorCode error;
    HotspotConfig config = {0};
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetApModeOn start ...\n");

    char temp[WIFI_MAX_SN_LEN] = {0};
    FlashInit();         
    if(LZ_HARDWARE_SUCCESS == VendorGet(VENDOR_ID_SN, temp, WIFI_MAX_SN_LEN))
    {
        wifi_config_t wifi_config = {0};
        if(strcmp(temp, "LZ01") != 0)
        {
            set_default_wifi_config();
            get_wifi_config(printf, &wifi_config);
        }
        memset(temp, 0, WIFI_MAX_SN_LEN);
        VendorGet(VENDOR_ID_WIFI_SSID,   temp, WIFI_MAX_SN_LEN);
        memcpy(config.ssid, temp, sizeof(config.ssid));
        memset(temp, 0, WIFI_MAX_SN_LEN);
        VendorGet(VENDOR_ID_WIFI_PASSWD, temp, WIFI_MAX_SN_LEN);
        memcpy(config.preSharedKey, temp, sizeof(config.preSharedKey));
    }
    else
    {
        memcpy_s(config.ssid, WIFI_MAX_SSID_LEN, AP_SSID, sizeof(AP_SSID));
        memcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, AP_PASSWORD, sizeof(AP_PASSWORD));
    }


    config.channelNum = 1;
    error = SetHotspotConfig(&config);
    if (error != WIFI_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork SetHotspotConfig ...error: %d\n", error);
        goto end;
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork EnableHotspot ...\n");

    error = EnableHotspot();
    if (error != WIFI_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork EnableHotspot ...error: %d\n", error);
        goto end;
    }
    
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork EnableHotspot done");
    FlashSetResidentFlag(1);
    m_wml.ap_on = true;
end:    
    LOS_MuxPost(m_wml.muxlock);
    return error;
}

WifiErrorCode SetApModeOff()
{
    if(!m_wml.init)
    {
        m_wml.init = true;
        LOS_MuxCreate(&m_wml.muxlock);
    }
    LOS_MuxPend(m_wml.muxlock, MUX_LOCK_TO_TIME);

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
            goto end;
        }
    }
    else {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork AP is inactive\n");
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetApModeOff end ...\n");
    m_wml.ap_on = false;
    if(!m_wml.sta_on) FlashSetResidentFlag(0);
    
end:    
    LOS_MuxPost(m_wml.muxlock);
    return error;

}

WifiErrorCode SetWifiModeOff()
{
    if(!m_wml.init)
    {
        m_wml.init = true;
        LOS_MuxCreate(&m_wml.muxlock);
    }
    LOS_MuxPend(m_wml.muxlock, MUX_LOCK_TO_TIME);

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
            goto end;
        }
    }
    else {
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork wifi is inactive\n");
    }

    m_wml.sta_on = false;
    if(!m_wml.ap_on) FlashSetResidentFlag(0);
end:    
    LOS_MuxPost(m_wml.muxlock);

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
        LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork BSSID    ROUTE_SSID    RSSI    Channel\n");
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
    if(!m_wml.init)
    {
        m_wml.init = true;
        LOS_MuxCreate(&m_wml.muxlock);
    }
    LOS_MuxPend(m_wml.muxlock, MUX_LOCK_TO_TIME);

    WifiErrorCode error;
    int netId = 0;
    WifiDeviceConfig config = {0};

    
    char temp[WIFI_MAX_SN_LEN] = {0};
    FlashInit();         

    if(LZ_HARDWARE_SUCCESS == VendorGet(VENDOR_ID_SN, temp, WIFI_MAX_SN_LEN))
    {
        wifi_config_t wifi_config = {0};
        if(strcmp(temp, "LZ01") != 0)
        {
            set_default_wifi_config();
            get_wifi_config(printf, &wifi_config);
        }
        memset(temp, 0, WIFI_MAX_SN_LEN);
        VendorGet(VENDOR_ID_WIFI_ROUTE_SSID,   temp, WIFI_MAX_SN_LEN);
        memcpy(g_wificonfig.ssid, temp, sizeof(g_wificonfig.ssid));
        memset(temp, 0, WIFI_MAX_SN_LEN);
        VendorGet(VENDOR_ID_WIFI_ROUTE_PASSWD, temp, WIFI_MAX_SN_LEN);
        memcpy(g_wificonfig.psk, temp, sizeof(g_wificonfig.psk));
    }
    else
    {
        memcpy_s(g_wificonfig.ssid, sizeof(g_wificonfig.ssid), ROUTE_SSID, sizeof(ROUTE_SSID));
        memcpy_s(g_wificonfig.psk, sizeof(g_wificonfig.psk), ROUTE_PASSWORD, sizeof(ROUTE_PASSWORD));
    }

    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork SetWifiModeOn\n");
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork g_wificonfig.ssid %s\n", g_wificonfig.ssid);
    LZ_HARDWARE_LOGD(LOG_TAG, "rknetwork g_wificonfig.psk %s\n", g_wificonfig.psk);
    error = EnableWifi();
    if (error != WIFI_SUCCESS)
    {
        LZ_HARDWARE_LOGE(LOG_TAG, "rknetwork EnableWifi error: %d\n", error);
        goto connect_done;
    }
    
    FlashSetResidentFlag(1);
    m_wml.sta_on = true;

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
        error = LZ_HARDWARE_FAILURE;
        goto connect_done;
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

        LOS_Msleep(1000);
        // LOS_TaskDelay(1000);
        retry--;
    }

connect_done:
    LOS_MuxPost(m_wml.muxlock);
    return error;
}


static void TaskConfigApModeEntry()
{
    SetWifiModeOff();
    SetApModeOn();
}

static void TaskConfigWifiModeEntry()
{
    // 如果需要修改Wifi的SSID和密码，可以在此启用下述接口
    set_wifi_config_route_ssid(printf,   "凌智电子");     // 路由的WiFi名称
    set_wifi_config_route_passwd(printf, "88888888");    // 路由器WiFi密码

init:
    SetWifiModeOff();
    SetApModeOff();
    SetWifiModeOn();

    while(1)
    {
        if(wifi_get_connect_status_internal() != 1)
        {
            LOS_Msleep(5000);
            printf("%s:%d wifi disconnect, try reconnect...\r\n", __func__, __LINE__);
            goto init;
        }
        LOS_Msleep(1000);
    }
}


UINT32 TaskConfigApMode(VOID)
{
    UINT32  ret;
    TSK_INIT_PARAM_S task = { 0 };

    printf("%s start\r\n", __FUNCTION__);
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

    printf("%s start\r\n", __FUNCTION__);
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskConfigWifiModeEntry;
    task.uwStackSize  = OS_TASK_STACK_SIZE;
    task.pcName       = "taskConfigWifiModeEntry";
    task.usTaskPrio   = 15;
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
    wifi_config_t wifi = {0};
    
    get_wifi_config(printf, &wifi);
    if(strcasecmp(wifi.mode, "ap") == 0)
    {
        ret = TaskConfigApMode();
    }
    else
    {
        ret = TaskConfigWifiMode();
    }

    return ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
