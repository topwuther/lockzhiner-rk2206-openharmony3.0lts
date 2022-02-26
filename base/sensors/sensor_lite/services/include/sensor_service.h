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

#ifndef SENSOR_SERVICE_DEF_H
#define SENSOR_SERVICE_DEF_H

#include <iproxy_server.h>

#include <pthread.h>

#include "feature.h"
#include <iunknown.h>
#include <log.h>
#include "samgr_lite.h"
#include "sensor_agent.h"
#include "sensor_agent_proxy.h"
#include "sensor_agent_type.h"
#include "service.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SENSOR_SERVICE "sensor_service"
#define SENSOR_FEATURE "sensor_feature"

#define SENSOR_SERVICE_ID_GetAllSensors 0
#define SENSOR_SERVICE_ID_ActivateSensor 1
#define SENSOR_SERVICE_ID_DeactivateSensor 2
#define SENSOR_SERVICE_ID_SetBatchs 3
#define SENSOR_SERVICE_ID_SubscribeSensor 4
#define SENSOR_SERVICE_ID_UnsubscribeSensor 5
#define SENSOR_SERVICE_ID_SetMode 6
#define SENSOR_SERVICE_ID_SetOption 7
#define SENSORMGR_LISTENER_NAME_LEN 8

typedef struct SensorFeatureApi {
    INHERIT_SERVER_IPROXY;
    int32_t (*GetAllSensors)(SensorInfo **sensorInfo, int32_t *count);
    int32_t (*ActivateSensor)(int32_t sensorId, const SensorUser *user);
    int32_t (*DeactivateSensor)(int32_t sensorId, const SensorUser *user);
    int32_t (*SetBatch)(int32_t sensorId, const SensorUser *user, int64_t samplingInterval, int64_t reportInterval);
    int32_t (*SubscribeSensor)(int32_t sensorId, const SensorUser *user);
    int32_t (*UnsubscribeSensor)(int32_t sensorId, const SensorUser *user);
    int32_t (*SetMode)(int32_t sensorId, const SensorUser *user, int32_t mode);
    int32_t (*SetOption)(int32_t sensorId, const SensorUser *user, int32_t option);
} SensorFeatureApi;

typedef struct SensorService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(SensorFeatureApi);
    Identity identity;
} SensorService;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif