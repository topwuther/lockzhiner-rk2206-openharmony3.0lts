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

#include "hdf_device_info_full.h"
#include <dlfcn.h>
#include "osal_mem.h"

struct HdfDeviceInfoFull *HdfDeviceInfoFullReinterpretCast(const struct HdfDeviceInfo *attribute)
{
    return (struct HdfDeviceInfoFull *)attribute;
}

void HdfDeviceInfoFullConstruct(struct HdfDeviceInfoFull *attribute)
{
    HdfDeviceInfoConstruct(&attribute->super);
    attribute->deviceHandle = NULL;
}

struct HdfDeviceInfoFull* HdfDeviceInfoFullNewInstance()
{
    struct HdfDeviceInfoFull *fullAttribute =
        (struct HdfDeviceInfoFull *)OsalMemCalloc(sizeof(struct HdfDeviceInfoFull));
    if (fullAttribute != NULL) {
        HdfDeviceInfoFullConstruct(fullAttribute);
    }
    return fullAttribute;
}

void HdfDeviceInfoFullFreeInstance(struct HdfDeviceInfoFull *attribute)
{
    if (attribute != NULL) {
        if (attribute->super.moduleName != NULL) {
            OsalMemFree((void *)attribute->super.moduleName);
        }
        if (attribute->super.svcName != NULL) {
            OsalMemFree((void *)attribute->super.svcName);
        }
        if (attribute->super.deviceMatchAttr != NULL) {
            OsalMemFree((void *)attribute->super.deviceMatchAttr);
        }
        if (attribute->super.private != NULL) {
            OsalMemFree((void *)attribute->super.private);
        }
        if (attribute->deviceHandle != NULL) {
            dlclose(attribute->deviceHandle);
        }
        OsalMemFree(attribute);
    }
}

void HdfDeviceInfoFullDelete(struct HdfSListNode *listEntry)
{
    struct HdfDeviceInfoFull *fullAttribute = (struct HdfDeviceInfoFull *)listEntry;
    if (fullAttribute != NULL) {
        HdfDeviceInfoFullFreeInstance(fullAttribute);
    }
}

