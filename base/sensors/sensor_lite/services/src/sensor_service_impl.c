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

#include "sensor_service_impl.h"

#include <securec.h>
#include <stdlib.h>
#include <string.h>

#include "liteipc_adapter.h"
#include "sensor_service.h"

static struct SensorInfos *g_sensorLists;
static int32_t g_sensorListsLength;
static struct SensorInterface *g_sensorDevice;
static SvcIdentity g_svcIdentity = {
    .handle = 0,
    .token = 0,
};

int32_t InitSensorList()
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    int32_t ret = g_sensorDevice->GetAllSensors(&g_sensorLists, &g_sensorListsLength);
    if ((ret != 0) || (g_sensorLists == NULL)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s getAllSensors failed, ret: %d",
            SENSOR_SERVICE, __func__, ret);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    return SENSOR_OK;
}

const char *SENSOR_GetName(Service *service)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    return SENSOR_SERVICE;
}

static int SensorDataCallback(const SensorEvent *event)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    if ((event == NULL) || (event->dataLen == 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s event is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_ID;
    }
    IpcIo io;
    uint8_t data[IPC_IO_DATA_MAX];
    IpcIoInit(&io, data, IPC_IO_DATA_MAX, IPC_MAX_OBJECTS);
    BuffPtr eventBuff = {
        .buffSz = (uint32_t)(sizeof(SensorEvent)),
        .buff = event
    };
    BuffPtr sensorDataBuff = {
        .buffSz = (uint32_t)(event->dataLen),
        .buff = event->data
    };
    IpcIoPushDataBuff(&io, &eventBuff);
    IpcIoPushDataBuff(&io, &sensorDataBuff);
    if (!IpcIoAvailable(&io)) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s TransmitServiceId ipc failed", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    IpcContext context;
    SendRequest(&context, g_svcIdentity, 0, &io, NULL, LITEIPC_FLAG_ONEWAY, NULL);
    return SENSOR_OK;
}

void SetSvcIdentity(const IpcIo *req, const IpcIo *reply)
{
    SvcIdentity *sid = IpcIoPopSvc(req);
    if (sid == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s sid is NULL", __func__);
        return;
    }
    g_svcIdentity.handle = sid->handle;
    g_svcIdentity.token = sid->token;
}

BOOL Initialize(Service *service, Identity identity)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    g_sensorDevice = NewSensorInterfaceInstance();
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
    }
    return TRUE;
}

BOOL MessageHandle(Service *service, Request *msg)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    return TRUE;
}

TaskConfig GetTaskConfig(Service *service)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, TASK_CONFIG_STACK_SIZE, TASK_CONFIG_QUEUE_SIZE, SHARED_TASK};
    return config;
}

int32_t GetAllSensorsImpl(SensorInfo **sensorInfo, int32_t *count)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    if ((sensorInfo == NULL) || (count == NULL)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorInfo or count is null",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    if ((g_sensorLists == NULL) || (g_sensorListsLength <= 0)) {
        if (InitSensorList() != 0) {
            HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s initSensorList failed!",
                SENSOR_SERVICE, __func__);
            return SENSOR_ERROR_INVALID_PARAM;
        }
    }
    *sensorInfo = (SensorInfo *)g_sensorLists;
    *count = g_sensorListsLength;
    return SENSOR_OK;
}

int32_t ActivateSensorImpl(int32_t sensorId, const SensorUser *user)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    if (user == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s user is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    int32_t ret = g_sensorDevice->Enable(sensorId);
    if (ret != 0) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s ActivateSensor sensor failed, ret: %d",
            SENSOR_SERVICE, __func__, ret);
        return ret;
    }
    return SENSOR_OK;
}

int32_t DeactivateSensorImpl(int32_t sensorId, const SensorUser *user)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    if (user == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s user is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    int32_t ret = g_sensorDevice->Disable(sensorId);
    if (ret != 0) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s DeactivateSensor sensor failed, ret: %d",
            SENSOR_SERVICE, __func__, ret);
        return ret;
    }
    return SENSOR_OK;
}

int32_t SetBatchImpl(int32_t sensorId, const SensorUser *user, int64_t samplingInterval, int64_t reportInterval)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    if ((samplingInterval < 0) || (reportInterval < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP,
            "[SERVICE:%s]: %s samplingInterval: %lld or reportInterval: %lld is invalid",
            SENSOR_SERVICE, __func__, samplingInterval, reportInterval);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    return SENSOR_OK;
}

int32_t SubscribeSensorImpl(int32_t sensorId, const SensorUser *user)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    if (user == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s user is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    int32_t ret = g_sensorDevice->Register(SensorDataCallback);
    if (ret != 0) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s register sensor user failed, ret: %d",
            SENSOR_SERVICE, __func__, ret);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    return SENSOR_OK;
}

int32_t UnsubscribeSensorImpl(int32_t sensorId, const SensorUser *user)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    if (user == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s user is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    if (g_sensorDevice == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s g_sensorDevice is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    int32_t ret = g_sensorDevice->Unregister();
    if (ret != 0) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s unregister sensor failed, ret: %d",
            SENSOR_SERVICE, __func__, ret);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    return SENSOR_OK;
}

int32_t SetModeImpl(int32_t sensorId, const SensorUser *user, int32_t mode)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    return SENSOR_OK;
}

int32_t SetOptionImpl(int32_t sensorId, const SensorUser *user, int32_t option)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    if ((sensorId >= SENSOR_TYPE_ID_MAX) || (sensorId < 0)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s sensorId: %d is invalid",
            SENSOR_SERVICE, __func__, sensorId);
        return SENSOR_ERROR_INVALID_ID;
    }
    return SENSOR_OK;
}
