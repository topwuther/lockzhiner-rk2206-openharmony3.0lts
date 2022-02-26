/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_WPA_WORK_H
#define OHOS_WPA_WORK_H

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "parameter.h"
#include "pthread.h"
#include "securec.h"
#include "wpa_ctrl.h"
#include <sys/select.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
    #define HIDDEN_CLOSE 0
    #define HIDDEN_OPEN 1
    void ExitWpaScan(void);
    void ExitWpa(void);
    int wpa_main(int argc, char *argv[]);
    int GetCurrentConnInfo(char *ssid, int len);
    void DeinitWifiService();
    void ResetSSIDBuff(void);
    int GetIdNum(void);
    int GetAndResetScanStat(void);
    char *GetSsid(int ssidNum);
    void WpaScanReconnect(const char *gSsid, const char *gPassWord, const int hiddenSwitch);
    void WpaClientStart(void);
    void LockWifiData(void);
    void UnLockWifiData(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
