/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_ATTRIBUTE_MANAGER_H
#define HDF_ATTRIBUTE_MANAGER_H

#include "hdf_slist.h"

const struct DeviceResourceNode *HdfGetRootNode(void);
bool HdfAttributeManagerGetHostList(struct HdfSList *hostList);
struct HdfSList *HdfAttributeManagerGetDeviceList(uint16_t hostId, const char *hostName);
bool HdfDeviceListAdd(const char *moduleName, const char *serviceName, const void *privateData);
void HdfDeviceListDel(const char *moduleName, const char *serviceName);

#endif /* HDF_ATTRIBUTE_MANAGER_H */
