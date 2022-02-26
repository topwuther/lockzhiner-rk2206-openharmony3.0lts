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

#ifndef SENSOR_CLIENT_ADAPTER_DEF_H
#define SENSOR_CLIENT_ADAPTER_DEF_H

#include <pthread.h>

#include <iproxy_client.h>
#include <iunknown.h>
#include "liteipc_adapter.h"
#include <log.h>
#include "samgr_lite.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"
#include "sensor_service.h"
#include "serializer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SENSOR_SERVICE "sensor_service"
#define SENSOR_FEATURE "sensor_feature"
#define MAX_DATA_LEN 0x100
#define IPC_MAX_OBJECTS 3

void *GetServiceProxy();
int32_t GetAllSensorsByProxy(const void *proxy, SensorInfo **sensorInfo, int32_t *count);
int32_t ActivateSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *user);
int32_t DeactivateSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *user);
int32_t SetBatchByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int64_t samplingInterval,
    int64_t reportInterval);
int32_t SubscribeSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *user);
int32_t UnsubscribeSensorByProxy(const void *proxy, int32_t sensorId, const SensorUser *user);
int32_t SetModeByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int32_t mode);
int32_t SetOptionByProxy(const void *proxy, int32_t sensorId, const SensorUser *user, int32_t option);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif