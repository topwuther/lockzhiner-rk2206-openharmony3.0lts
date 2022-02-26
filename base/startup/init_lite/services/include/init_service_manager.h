/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASE_STARTUP_INITLITE_SERVICEMANAGER_H
#define BASE_STARTUP_INITLITE_SERVICEMANAGER_H

#include "init_service.h"
#include "cJSON.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UID_STR_IN_CFG        "uid"
#define GID_STR_IN_CFG        "gid"
#define ONCE_STR_IN_CFG       "once"
#define IMPORTANT_STR_IN_CFG  "importance"
#define BIN_SH_NOT_ALLOWED    "/bin/sh"
#define CRITICAL_STR_IN_CFG   "critical"
#define DISABLED_STR_IN_CFG   "disabled"
#define CONSOLE_STR_IN_CFG   "console"

#define MAX_SERVICES_CNT_IN_FILE 100

void RegisterServices(Service* services, int servicesCnt);
void StartServiceByName(const char* serviceName);
void StopServiceByName(const char* serviceName);
void StopAllServices();
void StopAllServicesBeforeReboot();
void ReapServiceByPID(int pid);
void ParseAllServices(const cJSON* fileRoot);
#ifdef OHOS_SERVICE_DUMP
void DumpAllServices();
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_SERVICEMANAGER_H
