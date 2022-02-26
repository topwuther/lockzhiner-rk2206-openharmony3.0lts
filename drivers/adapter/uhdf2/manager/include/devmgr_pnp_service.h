/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DEVMGR_PNP_SERVICE_H
#define DEVMGR_PNP_SERVICE_H

#include "devmgr_service.h"
#include "hdf_slist.h"

#define PNP_HOST_NAME "pnp_host"

struct HdfSList *DevmgrServiceGetPnpDeviceInfo();

struct DevHostServiceClnt *DevmgrServiceGetPnpHostClnt(struct DevmgrService *inst);

int32_t DevmgrServiceStartPnpHost(struct DevmgrService *inst);

int32_t DevmgrServiceRegPnpDevice(
    struct IDevmgrService *devmgrSvc, const char *moduleName, const char *serviceName,
    const char *deviceMatchAttr, const void *privateData);

int32_t DevmgrServiceUnRegPnpDevice(
    struct IDevmgrService *devmgrSvc, const char *moduleName, const char *serviceName);

#endif
