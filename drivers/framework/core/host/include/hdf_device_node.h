/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_DEVICE_NODE_H
#define HDF_DEVICE_NODE_H

#include "hdf_device.h"
#include "hdf_device_info.h"
#include "hdf_device_desc.h"
#include "hdf_dlist.h"
#include "hdf_pm.h"

struct HdfDeviceNode;
struct DevHostService;

struct IDeviceNode {
    struct HdfObject object;
    int (*PublishService)(struct HdfDeviceNode *, const char *);
    int (*LaunchNode)(struct HdfDeviceNode *, struct IHdfDevice *);
};

struct HdfDeviceNode {
    struct IDeviceNode super;
    struct DListHead entry;
    struct PowerStateToken *powerToken;
    struct DevHostService *hostService;
    struct HdfDeviceObject deviceObject;
    struct IHdfDeviceToken *token;
    struct HdfDriverEntry *driverEntry;
    const struct HdfDeviceInfo *deviceInfo;
};

int HdfDeviceNodeAddPowerStateListener(
    struct HdfDeviceNode *devNode, const struct IPowerEventListener *listener);
void HdfDeviceNodeRemovePowerStateListener(
    struct HdfDeviceNode *devNode, const struct IPowerEventListener *listener);
void HdfDeviceNodeConstruct(struct HdfDeviceNode *service);
void HdfDeviceNodeDestruct(struct HdfDeviceNode *service);
struct HdfDeviceNode *HdfDeviceNodeNewInstance(void);
void HdfDeviceNodeFreeInstance(struct HdfDeviceNode *service);
void HdfDeviceNodeDelete(struct HdfDeviceNode *devNode);
int HdfDeviceNodePublishPublicService(struct HdfDeviceNode *service, const char *svcName);
void HdfDeviceNodeReclaimService(const char *svcName);

#endif /* HDF_DEVICE_NODE_H */
