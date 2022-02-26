/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SENSOR_HALL_DRIVER_H
#define SENSOR_HALL_DRIVER_H

#include "hdf_workqueue.h" 
#include "sensor_config_parser.h"
#include "sensor_platform_if.h"

#define HALL_CHIP_NAME_AK8789    "ak8789"

enum HallGpioIrq {
    HALL_NORTH_POLARITY_GPIO = 0,
    HALL_SOUTH_POLARITY_GPIO = 1,
    HALL_POLARITY_GPIO_MAX = 2,
};

struct HallOpsCall {
    int32_t (*Init)(struct SensorCfgData *data);
    int32_t (*ReadData)(struct SensorCfgData *data);
};

struct HallDrvData {
    struct IDeviceIoService ioService;
    struct HdfDeviceObject *device;
    HdfWorkQueue hallWorkQueue;
    HdfWork hallWork;
    bool detectFlag;
    bool enable;
    int64_t interval;
    struct SensorCfgData *hallCfg;
    struct HallOpsCall ops;
    int32_t GpioIrq[HALL_POLARITY_GPIO_MAX];
};

int32_t HallRegisterChipOps(const struct HallOpsCall *ops);
struct SensorCfgData *HallCreateCfgData(const struct DeviceResourceNode *node);
void HallReleaseCfgData(struct SensorCfgData *sensorCfgData);

#endif /* SENSOR_HALL_DRIVER_H */