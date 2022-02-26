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
#include "sensor_service.h"

#include <ohos_init.h>
#include "feature.h"
#include "iproxy_server.h"
#include "samgr_lite.h"
#include "sensor_service_impl.h"
#include "service.h"

int32_t GetAllSensorsInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_GetAllSensors);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else{
        SensorInfo *sensorInfo = NULL;
        int32_t count = 0;
        int32_t ret = defaultApi->GetAllSensors(&sensorInfo, &count);
        if (ret != SENSOR_OK) {
            HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, ret: %d", SENSOR_SERVICE, __func__, ret);
            IpcIoPushInt32(reply, ret);
            return ret;
        }
        BuffPtr dataBuff = {
            .buffSz = (uint32_t)(count * sizeof(SensorInfo)),
            .buff = sensorInfo
        };
        IpcIoPushInt32(reply, SENSOR_OK);
        IpcIoPushInt32(reply, count);
        IpcIoPushDataBuff(reply, &dataBuff);
        return SENSOR_OK;
    }
}

int32_t ActivateSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_ActivateSensor);
    int32_t sensorId = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->ActivateSensor(sensorId, &sensorUser);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t DeactivateSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_DeactivateSensor);
    int32_t sensorId = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->DeactivateSensor(sensorId, &sensorUser);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t SetBatchInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_SetBatchs);
    int32_t sensorId = IpcIoPopInt32(req);
    int64_t updateInterval = IpcIoPopInt64(req);
    int64_t maxDelay = IpcIoPopInt64(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->SetBatch(sensorId, &sensorUser, updateInterval, maxDelay);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t SubscribeSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_SubscribeSensor);
    int32_t sensorId = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->SubscribeSensor(sensorId, &sensorUser);
        SetSvcIdentity(req, reply);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t UnsubscribeSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_UnsubscribeSensor);
    int32_t sensorId = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {    
        SensorUser sensorUser;
        int32_t ret = defaultApi->UnsubscribeSensor(sensorId, &sensorUser);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t SetModeInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_SetMode);
    int32_t sensorId = IpcIoPopInt32(req);
    int32_t mode = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->SetMode(sensorId, &sensorUser, mode);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

int32_t SetOptionInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    IpcIoPushInt32(reply, SENSOR_SERVICE_ID_SetOption);
    int32_t sensorId = IpcIoPopInt32(req);
    int32_t option = IpcIoPopInt32(req);
    if (defaultApi == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s failed, defaultApi is null", SENSOR_SERVICE, __func__);
        IpcIoPushInt32(reply, SENSOR_ERROR_UNKNOWN);
        return SENSOR_ERROR_UNKNOWN;
    } else {
        SensorUser sensorUser;
        int32_t ret = defaultApi->SetOption(sensorId, &sensorUser, option);
        IpcIoPushInt32(reply, ret);
        return ret;
    }
}

static InvokeFunc g_invokeFuncList[] = {
    GetAllSensorsInvoke,
    ActivateSensorInvoke,
    DeactivateSensorInvoke,
    SetBatchInvoke,
    SubscribeSensorInvoke,
    UnsubscribeSensorInvoke,
    SetModeInvoke,
    SetOptionInvoke,
};

int32_t Invoke(IServerProxy *iProxy, int funcId, void *origin, IpcIo *req, IpcIo *reply)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin", SENSOR_SERVICE, __func__);
    if ((iProxy == NULL) || (req == NULL) || (reply == NULL)) {
        HILOG_ERROR(HILOG_MODULE_APP, "[SERVICE:%s]: %s iProxy or req or reply is NULL",
            SENSOR_SERVICE, __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *defaultApi = (SensorFeatureApi *)iProxy;
    if ((funcId >= 0) && (funcId <= SENSOR_SERVICE_ID_UnsubscribeSensor)) {
        return g_invokeFuncList[funcId](defaultApi, req, reply);
    }
    return SENSOR_ERROR_INVALID_PARAM;
}

static SensorService g_sensorService = {
    .GetName = SENSOR_GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
    SERVER_IPROXY_IMPL_BEGIN,
    .Invoke = Invoke,
    .GetAllSensors = GetAllSensorsImpl,
    .ActivateSensor = ActivateSensorImpl,
    .DeactivateSensor = DeactivateSensorImpl,
    .SetBatch = SetBatchImpl,
    .SubscribeSensor = SubscribeSensorImpl,
    .UnsubscribeSensor = UnsubscribeSensorImpl,
    .SetMode = SetModeImpl,
    .SetOption = SetOptionImpl,
    IPROXY_END,
};

static void Init(void)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "[SERVICE:%s]: %s begin",
        SENSOR_SERVICE, __func__);
    SAMGR_GetInstance()->RegisterService((Service *)&g_sensorService);
    SAMGR_GetInstance()->RegisterDefaultFeatureApi(SENSOR_SERVICE, GET_IUNKNOWN(g_sensorService));
}
SYSEX_SERVICE_INIT(Init);