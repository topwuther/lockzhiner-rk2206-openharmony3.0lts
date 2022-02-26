/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "sensor_hall_driver.h"
#include <securec.h>
#include "gpio_if.h"
#include "hall_ak8789.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_irq.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_hall_driver_c
#define HDF_HALL_WORK_QUEUE_NAME    "hdf_hall_work_queue"

static struct HallDrvData *g_hallDrvData = NULL;

static struct HallDrvData *HallGetDrvData(void)
{
    return g_hallDrvData;
}

int32_t HallRegisterChipOps(const struct HallOpsCall *ops)
{
    struct HallDrvData *drvData = HallGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);
    
    drvData->ops.Init = ops->Init;
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void HallDataWorkEntry(void *arg)
{
    struct HallDrvData *drvData = NULL;
    
    drvData = (struct HallDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (drvData->ops.ReadData == NULL) {
        HDF_LOGI("%s: Hall ReadData function NULL", __func__);
        return;
    }
    if (drvData->ops.ReadData(drvData->hallCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: Hall read data failed", __func__);
    }
}

static int32_t HallNorthPolarityIrqFunc(uint16_t gpio, void *data)
{
    (void)gpio;

    struct HallDrvData *drvData = (struct HallDrvData *)data;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!HdfAddWork(&drvData->hallWorkQueue, &drvData->hallWork)) {
        HDF_LOGE("%s: Hall add work queue failed", __func__);
    }

    return HDF_SUCCESS;
}

static int32_t HallSouthPolarityIrqFunc(uint16_t gpio, void *data)
{
    (void)gpio;

    struct HallDrvData *drvData = (struct HallDrvData *)data;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!HdfAddWork(&drvData->hallWorkQueue, &drvData->hallWork)) {
        HDF_LOGE("%s: Hall add work queue failed", __func__);
    }

    return HDF_SUCCESS;
}

static int32_t InitHallData(struct HallDrvData *drvData)
{
    if (HdfWorkQueueInit(&drvData->hallWorkQueue, HDF_HALL_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: Hall init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->hallWork, HallDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: Hall create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->enable = false;
    drvData->detectFlag = false;

    return HDF_SUCCESS;
}

static int32_t SetHallEnable(void)
{
    int32_t ret;
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: Hall sensor is enabled", __func__);
        return HDF_SUCCESS;
    }

    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        ret = GpioSetIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], OSAL_IRQF_TRIGGER_FALLING,
            HallNorthPolarityIrqFunc, drvData);
        if (ret != 0) {
            HDF_LOGE("Gpio set north irq failed: %d", ret);
        }
        ret = GpioEnableIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("Gpio enable north irq failed: %d", ret);
        }
    }

    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        ret = GpioSetIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], OSAL_IRQF_TRIGGER_FALLING,
            HallSouthPolarityIrqFunc, drvData);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio set south irq failed", __func__);
        }
        ret = GpioEnableIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio enable south irq failed", __func__);
        }
    }

    drvData->enable = true;
    return HDF_SUCCESS;
}

static int32_t SetHallDisable(void)
{
    int32_t ret;
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: Hall sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        ret = GpioUnSetIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio unset north irq failed", __func__);
        }

        ret = GpioDisableIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio disable north irq failed", __func__);
        }
    }

    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        ret = GpioUnSetIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio unset south irq failed", __func__);
        }

        ret = GpioDisableIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: Gpio disable south irq failed", __func__);
        }
    }
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetHallBatch(int64_t samplingInterval, int64_t interval)
{
    return HDF_SUCCESS;
}

static int32_t SetHallMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_ON_CHANGE) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetHallOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchHall(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t HallBindDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct HallDrvData *drvData = (struct HallDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: Malloc hall drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchHall;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_hallDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitHallOps(struct SensorCfgData *config, struct SensorDeviceInfo *deviceInfo)
{
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetHallEnable;
    deviceInfo->ops.Disable = SetHallDisable;
    deviceInfo->ops.SetBatch = SetHallBatch;
    deviceInfo->ops.SetMode = SetHallMode;
    deviceInfo->ops.SetOption = SetHallOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &config->sensorInfo, sizeof(config->sensorInfo)) != EOK) {
        HDF_LOGE("%s: Copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t ParserHallPinConfigData(const struct DeviceResourceNode *node, struct HallDrvData *drvData)
{
    int32_t ret;
    struct DeviceResourceIface *parser = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(node, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    parser = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    CHECK_NULL_PTR_RETURN_VALUE(parser, HDF_ERR_INVALID_PARAM);

    CHECK_NULL_PTR_RETURN_VALUE(parser->GetChildNode, HDF_ERR_INVALID_PARAM);
    
    const struct DeviceResourceNode *pinNode = parser->GetChildNode(node, "hallPinConfig");
    CHECK_NULL_PTR_RETURN_VALUE(pinNode, HDF_ERR_INVALID_PARAM);

    ret = parser->GetUint32(pinNode, "NorthPolarityGpio", (uint32_t *)&drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], 0);
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "NorthPolarityGpio");
    ret = parser->GetUint32(pinNode, "SouthPolarityGpio", (uint32_t *)&drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], 0);
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "SouthPolarityGpio");

    return HDF_SUCCESS;
}

static int32_t InitHallAfterDetected(const struct DeviceResourceNode *node, struct HallDrvData *drvData)
{
    struct SensorDeviceInfo deviceInfo;
    CHECK_NULL_PTR_RETURN_VALUE(node, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (InitHallOps(drvData->hallCfg, &deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: Init hall ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: Add hall device failed", __func__);
        return HDF_FAILURE;
    }

    if (ParserHallPinConfigData(node, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: get hall pin config failed", __func__);
        (void)DeleteSensorDevice(&drvData->hallCfg->sensorInfo);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t SetHallGpioPin(struct HallDrvData *drvData)
{
    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        int ret = GpioSetDir(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], GPIO_DIR_IN);
        if (ret != 0) {
            HDF_LOGE("%s:%d set north gpio dir failed", __func__, __LINE__);
            return HDF_FAILURE;
        }
    }

    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        int ret = GpioSetDir(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], GPIO_DIR_IN);
        if (ret != 0) {
            HDF_LOGE("%s:%d south south gpio dir failed", __func__, __LINE__);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

struct SensorCfgData *HallCreateCfgData(const struct DeviceResourceNode *node)
{
    struct HallDrvData *drvData = HallGetDrvData();

    if (drvData == NULL || node == NULL) {
        HDF_LOGE("%s: Hall node pointer NULL", __func__);
        return NULL;
    }

    if (drvData->detectFlag) {
        HDF_LOGE("%s: Hall sensor have detected", __func__);
        return NULL;
    }

    if (drvData->hallCfg == NULL) {
        HDF_LOGE("%s: Hall hallCfg pointer NULL", __func__);
        return NULL;
    }

    if (GetSensorBaseConfigData(node, drvData->hallCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: Get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    if (SetHallGpioPin(drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: set hall gpio pin failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    if (DetectSensorDevice(drvData->hallCfg) != HDF_SUCCESS) {
        HDF_LOGI("%s: Hall sensor detect device no exist", __func__);
        drvData->detectFlag = false;
        goto BASE_CONFIG_EXIT;
    }

    drvData->detectFlag = true;
    if (InitHallAfterDetected(node, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: Hall sensor detect device no exist", __func__);
        goto BASE_CONFIG_EXIT;
    }

    return drvData->hallCfg;

BASE_CONFIG_EXIT:
    drvData->hallCfg->root = NULL;
    (void)memset_s(&drvData->hallCfg->sensorInfo, sizeof(struct SensorBasicInfo), 0, sizeof(struct SensorBasicInfo));
    (void)memset_s(&drvData->hallCfg->sensorAttr, sizeof(struct SensorAttr), 0, sizeof(struct SensorAttr));
    return NULL;
}

void HallReleaseCfgData(struct SensorCfgData *hallCfg)
{
    CHECK_NULL_PTR_RETURN(hallCfg);

    (void)DeleteSensorDevice(&hallCfg->sensorInfo);

    hallCfg->root = NULL;
    (void)memset_s(&hallCfg->sensorInfo, sizeof(struct SensorBasicInfo), 0, sizeof(struct SensorBasicInfo));
    (void)memset_s(&hallCfg->sensorAttr, sizeof(struct SensorAttr), 0, sizeof(struct SensorAttr));
}

int32_t HallInitDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct HallDrvData *drvData = (struct HallDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (InitHallData(drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: Init hall config failed", __func__);
        return HDF_FAILURE;
    }

    drvData->hallCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->hallCfg));
    if (drvData->hallCfg == NULL) {
        HDF_LOGE("%s: Malloc hall config data failed", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: Init hall driver success", __func__);
    return HDF_SUCCESS;
}

void HallReleaseDriver(struct HdfDeviceObject *device)
{
    struct HallDrvData *drvData = NULL;
    CHECK_NULL_PTR_RETURN(device);

    drvData = (struct HallDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    if (drvData->detectFlag) {
        HallReleaseCfgData(drvData->hallCfg);
    }

    OsalMemFree(drvData->hallCfg);
    drvData->hallCfg = NULL;

    HdfWorkDestroy(&drvData->hallWork);
    HdfWorkQueueDestroy(&drvData->hallWorkQueue);
    OsalMemFree(drvData);
}

struct HdfDriverEntry g_sensorHallDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_HALL",
    .Bind = HallBindDriver,
    .Init = HallInitDriver,
    .Release = HallReleaseDriver,
};

HDF_INIT(g_sensorHallDevEntry);