# WIFI概述

## 简介

- wifi模块基于RK平台开发，实现如下功能：建立/关闭WLAN热点、扫描、关联WLAN热点。

- wifi模块支持2种模式: STA/AP:

  -   AP模式: Access Point，提供无线接入服务，允许其它无线设备接入，提供数据访问，一般的无线路由/网桥工作在该模式下。AP和AP之间允许相互连接
-   Sta模式: Station, 类似于无线终端，sta本身并不接受无线的接入，它可以连接到AP，一般无线网卡即工作在该模式。
  
  


## 接口说明

### 包含头文件： 

```c
#include "lz_hardware.h
```

#### 1. WIFI模块使能接口

```c
unsigned int WifiEnable(void);
1) 参数说明：
	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 2. WIFI模块关闭接口

```c
unsigned int WifiDisable(void);
1) 参数说明：
	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 3. WIFI模块启动STA模式：

```c
unsigned int WifiStartStation(void);
1) 参数说明：
   	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 4. WIFI模块连接热点：

```c
unsigned int WifiConnect(unsigned char *ssid, unsigned char *passphrase);
1) 参数说明：
   ssid：  热点名称
   passphrase:  热点密码
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 5. WIFI模块获取连接信息：

```c
unsigned int WifiGetConnectInfo(WifiConnInfo *info);
1) 参数说明：
   info：  连接信息包括ip,连接热点信息等，详见头文件wifi.h
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 6. WIFI模块断开连接：

```c
unsigned int WifiDisconnect(unsigned short reasonCode);
1) 参数说明：
   reasonCode：断开连接代码-可使用 0
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 7. WIFI模块扫描热点：

```c
unsigned int WifiStartScan(void);
1) 参数说明：
   	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 8. WIFI模块停止扫描热点：

```c
unsigned int WifiAbortScan(void);
1) 参数说明：
   	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 9. WIFI模块获取扫描结果：

```c
unsigned int WifiGetScanResult(WifiScanResult **result, unsigned int *size);
1) 参数说明：
   result： 扫描结果-包含热点名称，mac地址等等
   size:   热点数量
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 10. WIFI模块AP模式下配置接口：

```c
unsigned int WifiConfigAP(unsigned char *ssid, unsigned char *preSharedKey, unsigned short channel);
1) 参数说明：
   ssid：  共享热点名称
   preSharedKey:  共享热点密码
   channel： 通道id
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 11. WIFI模块启动AP模式：

```c
unsigned int WifiStartAP(void);
1) 参数说明：
   	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```

#### 12. WIFI模块关闭AP模式：

```c
unsigned int WifiStopAP(void);
1) 参数说明：
   	无
2) 返回值：
   成功返回LZ_HARDWARE_SUCCESS, 出错返回错误码
```



## 使用实例

```c
#include "lz_hardware.h"
#define LOG_FACTORY "WIFI_TEST"

unsigned int sta_sample()
{
    uint32_t ret = LZ_HARDWARE_SUCCESS;
    unsigned int size = 0;
    WifiConnInfo connInfo;
    unsigned short reasonCode = LZ_HARDWARE_SUCCESS;
    WifiScanResult *pResult;
    int retry = 10;
    
    //使能wifi
    if (EnableWifi() != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "EnableWifi  error");
        return LZ_HARDWARE_FAILURE;
    }

	//扫描热点
    if (WifiStartScan() != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "Scan  error");
        return LZ_HARDWARE_FAILURE;
    }

    //获取扫描结果
    retry = 10;
    pResult  = (WifiScanResult *)malloc(sizeof(WifiScanResult) * WIFI_SCAN_HOTSPOT_LIMIT);
    while (retry) {
        if (WifiGetScanResult(&pResult, &size) == LZ_HARDWARE_SUCCESS) {
            break;
        }

        ToyMsleep(1000);
        retry--;
    }

    if (size <= 0) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "Scan nothing error (size:%d)", size);
        return LZ_HARDWARE_FAILURE;
    }


    LZ_HARDWARE_LOGI(LOG_FACTORY, "GetScanResult done");

    if (size > 0) {
        LZ_HARDWARE_LOGI(LOG_FACTORY, "BSSID    SSID    RSSI    Channel");
        LZ_HARDWARE_LOGI(LOG_FACTORY, "========================================");
        for (int i = 0; i < size; i++) {
            char bssid[18];
            char ssid[33];
            sprintf(bssid, "%02x:%02x:%02x:%02x:%02x:%02x",
                pResult[i].bssid[0], pResult[i].bssid[1], pResult[i].bssid[2],
                pResult[i].bssid[3], pResult[i].bssid[4], pResult[i].bssid[5]);
            sprintf(ssid, "%-32s", pResult[i].ssid);
            LZ_HARDWARE_LOGI(LOG_FACTORY, "%s    %s    %d    %u",
                    bssid,
                    ssid,
                    pResult[i].rssi,
                    pResult[i].frequency);
        }
        LZ_HARDWARE_LOGI(LOG_FACTORY, "========================================");
    }
	
    //连接热点
    WifiConnect(WIFI_SSID, WIFI_PSW);
    if (WifiConnect(WIFI_SSID, WIFI_PSW) != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "WifiConnect  error");
        return LZ_HARDWARE_FAILURE;
    }


    LZ_HARDWARE_LOGI(LOG_FACTORY, "ConnectTo (%s) done", WIFI_SSID);

    memset(&connInfo, 0, sizeof(WifiConnInfo));
    retry = 15;
    while (retry) {
        if (WifiGetConnectInfo(&connInfo) == LZ_HARDWARE_SUCCESS) {
            if (connInfo.status == LZ_HARDWARE_WIFI_CONNECTED) {
                goto connect_done;
            }
    	}

        ToyMsleep(1);
        retry--;
    }

    LZ_HARDWARE_LOGI(LOG_FACTORY, "Connect timeout (%s) ", WIFI_SSID);
    return LZ_HARDWARE_FAILURE;

connect_done:
    //断开连接
    if (WifiDisconnect(reasonCode) != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "Disconnect error");
        return LZ_HARDWARE_FAILURE;
    }

    LZ_HARDWARE_LOGI(LOG_FACTORY, "Disconnect done");
	
    //关闭wifi
    if (WifiDisable() != LZ_HARDWARE_SUCCESS) {
        LZ_HARDWARE_LOGE(LOG_FACTORY, "DisableWifi error");
        return LZ_HARDWARE_FAILURE;
    }

    LZ_HARDWARE_LOGI(LOG_FACTORY, "DisableWifi done");
    return ret;
}



```

