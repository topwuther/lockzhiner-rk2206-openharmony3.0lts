/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "sensor_gyro_driver.h"
#include <securec.h>
#include "gyro_bmi160.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_gyro_driver_c

#define HDF_GYRO_WORK_QUEUE_NAME    "hdf_gyro_work_queue"

static struct GyroDetectIfList g_gyroDetectIfList[] = {
    {GYRO_CHIP_NAME_BMI160, DetectGyroBim160Chip},
};

static struct GyroDrvData *g_gyroDrvData = NULL;

static struct GyroDrvData *GyroGetDrvData(void)
{
    return g_gyroDrvData;
}

static struct SensorRegCfgGroupNode *g_regCfgGroup[SENSOR_GROUP_MAX] = { NULL };

int32_t RegisterGyroChipOps(const struct GyroOpsCall *ops)
{
    struct GyroDrvData *drvData = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);

    drvData = GyroGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    drvData->ops.Init = ops->Init;
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void GyroDataWorkEntry(void *arg)
{
    int32_t ret;
    struct GyroDrvData *drvData = (struct GyroDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);
    CHECK_NULL_PTR_RETURN(drvData->ops.ReadData);

    ret = drvData->ops.ReadData(drvData->gyroCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro read data failed", __func__);
        return;
    }
}

static void GyroTimerEntry(uintptr_t arg)
{
    int64_t interval;
    int32_t ret;
    struct GyroDrvData *drvData = (struct GyroDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (!HdfAddWork(&drvData->gyroWorkQueue, &drvData->gyroWork)) {
        HDF_LOGE("%s: gyro add work queue failed", __func__);
    }

    interval = OsalDivS64(drvData->interval, (SENSOR_CONVERT_UNIT * SENSOR_CONVERT_UNIT));
    interval = (interval < SENSOR_TIMER_MIN_TIME) ? SENSOR_TIMER_MIN_TIME : interval;
    ret = OsalTimerSetTimeout(&drvData->gyroTimer, interval);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro modify time failed", __func__);
    }
}

static int32_t InitGyroData(void)
{
    struct GyroDrvData *drvData = GyroGetDrvData();
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->initStatus) {
        return HDF_SUCCESS;
    }

    if (HdfWorkQueueInit(&drvData->gyroWorkQueue, HDF_GYRO_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->gyroWork, GyroDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro create thread failed", __func__);
        return HDF_FAILURE;
    }

    CHECK_NULL_PTR_RETURN_VALUE(drvData->ops.Init, HDF_ERR_INVALID_PARAM);

    ret = drvData->ops.Init(drvData->gyroCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->initStatus = true;
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetGyroEnable(void)
{
    int32_t ret;
    struct GyroDrvData *drvData = GyroGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->gyroCfg, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: gyro sensor is enabled", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->gyroCfg->busCfg, drvData->gyroCfg->regCfgGroup[SENSOR_ENABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro sensor enable config failed", __func__);
        return ret;
    }

    ret = OsalTimerCreate(&drvData->gyroTimer, SENSOR_TIMER_MIN_TIME, GyroTimerEntry, (uintptr_t)drvData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro create timer failed[%d]", __func__, ret);
        return ret;
    }

    ret = OsalTimerStartLoop(&drvData->gyroTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro start timer failed[%d]", __func__, ret);
        return ret;
    }
    drvData->enable = true;

    return HDF_SUCCESS;
}

static int32_t SetGyroDisable(void)
{
    int32_t ret;
    struct GyroDrvData *drvData = GyroGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->gyroCfg, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: gyro sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->gyroCfg->busCfg, drvData->gyroCfg->regCfgGroup[SENSOR_DISABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro sensor disable config failed", __func__);
        return ret;
    }

    ret = OsalTimerDelete(&drvData->gyroTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro delete timer failed", __func__);
        return ret;
    }
    drvData->enable = false;
    return HDF_SUCCESS;
}

static int32_t SetGyroBatch(int64_t samplingInterval, int64_t interval)
{
    (void)interval;

    struct GyroDrvData *drvData = NULL;

    drvData = GyroGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    drvData->interval = samplingInterval;

    return HDF_SUCCESS;
}

static int32_t SetGyroMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_REALTIME) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetGyroOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchGyro(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t BindGyroDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct GyroDrvData *drvData = (struct GyroDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: malloc gyro drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchGyro;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_gyroDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitGyroOps(struct SensorDeviceInfo *deviceInfo)
{
    struct GyroDrvData *drvData = GyroGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetGyroEnable;
    deviceInfo->ops.Disable = SetGyroDisable;
    deviceInfo->ops.SetBatch = SetGyroBatch;
    deviceInfo->ops.SetMode = SetGyroMode;
    deviceInfo->ops.SetOption = SetGyroOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &drvData->gyroCfg->sensorInfo, sizeof(drvData->gyroCfg->sensorInfo)) != EOK) {
        HDF_LOGE("%s: copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitGyroAfterConfig(void)
{
    struct SensorDeviceInfo deviceInfo;

    if (InitGyroData() != HDF_SUCCESS) {
        HDF_LOGE("%s: init gyro config failed", __func__);
        return HDF_FAILURE;
    }

    if (InitGyroOps(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: init gyro ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: add gyro device failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DetectGyroChip(void)
{
    int32_t num;
    int32_t ret;
    int32_t loop;
    struct GyroDrvData *drvData = GyroGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->gyroCfg, HDF_ERR_INVALID_PARAM);

    num = sizeof(g_gyroDetectIfList) / sizeof(g_gyroDetectIfList[0]);
    for (loop = 0; loop < num; ++loop) {
        if (g_gyroDetectIfList[loop].DetectChip != NULL) {
            ret = g_gyroDetectIfList[loop].DetectChip(drvData->gyroCfg);
            if (ret == HDF_SUCCESS) {
                drvData->detectFlag = true;
                return HDF_SUCCESS;
            }
        }
    }

    HDF_LOGE("%s: detect gyro device failed", __func__);
    drvData->detectFlag = false;
    return HDF_FAILURE;
}

int32_t InitGyroDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct GyroDrvData *drvData = (struct GyroDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->detectFlag) {
        HDF_LOGE("%s: gyro sensor have detected", __func__);
        return HDF_SUCCESS;
    }

    drvData->gyroCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->gyroCfg));
    if (drvData->gyroCfg == NULL) {
        HDF_LOGE("%s: malloc sensor config data failed", __func__);
        return HDF_FAILURE;
    }

    drvData->gyroCfg->regCfgGroup = &g_regCfgGroup[0];

    if (GetSensorBaseConfigData(device->property, drvData->gyroCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    // if return failure, hdf framework go to next detect sensor
    if (DetectGyroChip() != HDF_SUCCESS) {
        HDF_LOGE("%s: gyro sensor detect device no exist", __func__);
        goto DETECT_CHIP_EXIT;
    }
    drvData->detectFlag = true;

    if (ParseSensorRegConfig(drvData->gyroCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: detect sensor device failed", __func__);
        goto REG_CONFIG_EXIT;
    }

    if (InitGyroAfterConfig() != HDF_SUCCESS) {
        HDF_LOGE("%s: init gyro after config failed", __func__);
        goto INIT_EXIT;
    }

    HDF_LOGI("%s: init gyro driver success", __func__);
    return HDF_SUCCESS;

INIT_EXIT:
    (void)DeleteSensorDevice(&drvData->gyroCfg->sensorInfo);
REG_CONFIG_EXIT:
    ReleaseSensorAllRegConfig(drvData->gyroCfg);
    (void)ReleaseSensorBusHandle(&drvData->gyroCfg->busCfg);
DETECT_CHIP_EXIT:
    drvData->detectFlag = false;
BASE_CONFIG_EXIT:
    drvData->gyroCfg->root = NULL;
    drvData->gyroCfg->regCfgGroup = NULL;
    OsalMemFree(drvData->gyroCfg);
    drvData->gyroCfg = NULL;
    return HDF_FAILURE;
}

void ReleaseGyroDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);

    struct GyroDrvData *drvData = (struct GyroDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    (void)DeleteSensorDevice(&drvData->gyroCfg->sensorInfo);
    drvData->detectFlag = false;

    if (drvData->gyroCfg != NULL) {
        drvData->gyroCfg->root = NULL;
        drvData->gyroCfg->regCfgGroup = NULL;
        ReleaseSensorAllRegConfig(drvData->gyroCfg);
        (void)ReleaseSensorBusHandle(&drvData->gyroCfg->busCfg);
        OsalMemFree(drvData->gyroCfg);
        drvData->gyroCfg = NULL;
    }

    drvData->initStatus = false;
}

struct HdfDriverEntry g_sensorGyroDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_GYRO",
    .Bind = BindGyroDriver,
    .Init = InitGyroDriver,
    .Release = ReleaseGyroDriver,
};

HDF_INIT(g_sensorGyroDevEntry);
