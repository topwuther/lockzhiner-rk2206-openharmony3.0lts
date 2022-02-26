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

#include "sensor_agent_proxy.h"
#include "sensor_service.h"

void *GetServiceProxy()
{
    IUnknown *iUnknown = SAMGR_GetInstance()->GetDefaultFeatureApi(SENSOR_SERVICE);
    if (iUnknown == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s get sensor service failed", __func__);
        return NULL;
    }
    SensorFeatureApi *sensor = NULL;
    int32_t result = iUnknown->QueryInterface(iUnknown, DEFAULT_VERSION, (void **)&sensor);
    if ((result != 0) || (sensor == NULL)) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s query interface failed", __func__);
        return NULL;
    }
    return sensor;
}

int32_t GetAllSensorsByProxy(const void *proxy, SensorInfo **sensorInfo, int32_t *count)
{
    if ((proxy == NULL) || (count == NULL)) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy or count is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->GetAllSensors(sensorInfo, count);
}

int32_t ActivateSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *sensorUser)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->ActivateSensor(sensorId);
}

int32_t DeactivateSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *sensorUser)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->DeactivateSensor(sensorId);
}

int32_t SetBatchByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int64_t samplingInterval,
    int64_t reportInterval)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->SetBatch(sensorId, user, samplingInterval, reportInterval);
}

int32_t SubscribeSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *sensorUser)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->SubscribeSensor(sensorId, sensorUser);
}

int32_t UnsubscribeSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *sensorUser)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->UnsubscribeSensor(sensorId, sensorUser);
}

int32_t SetModeByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int32_t mode)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->SetMode(sensorId, user, mode);
}

int32_t SetOptionByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int32_t option)
{
    if (proxy == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "%s proxy is NULL", __func__);
        return SENSOR_ERROR_INVALID_PARAM;
    }
    SensorFeatureApi *sensor = (SensorFeatureApi *)proxy;
    return sensor->SetOption(sensorId, user, option);
}
