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

#ifndef DEVICE_ATTRIBUTE_FULL_H
#define DEVICE_ATTRIBUTE_FULL_H

#include "hdf_device_info.h"

struct HdfDeviceInfoFull {
    struct HdfDeviceInfo super;
    void *deviceHandle;
};

struct HdfDeviceInfoFull *HdfDeviceInfoFullReinterpretCast(const struct HdfDeviceInfo *attribute);
struct HdfDeviceInfoFull *HdfDeviceInfoFullNewInstance(void);
void HdfDeviceInfoFullFreeInstance(struct HdfDeviceInfoFull *attribute);
void HdfDeviceInfoFullDelete(struct HdfSListNode *listEntry);

#endif /* DEVICE_ATTRIBUTE_FULL_H */
