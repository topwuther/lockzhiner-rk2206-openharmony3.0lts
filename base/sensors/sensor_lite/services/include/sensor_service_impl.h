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

#ifndef SENSOR_SERVICE_INTERFACE_H
#define SENSOR_SERVICE_INTERFACE_H

#include "iproxy_server.h"
#include "liteipc_adapter.h"
#include "sensor_if.h"
#include "sensor_service.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define TASK_CONFIG_STACK_SIZE 0x800
#define TASK_CONFIG_QUEUE_SIZE 20

const char *SENSOR_GetName(Service *service);
BOOL Initialize(Service *service, Identity identity);
BOOL MessageHandle(Service *service, Request *msg);
TaskConfig GetTaskConfig(Service *service);
int32_t GetAllSensorsImpl(SensorInfo **sensorInfo, int32_t *count);
int32_t ActivateSensorImpl(int32_t sensorId, const SensorUser *user);
int32_t DeactivateSensorImpl(int32_t sensorId, const SensorUser *user);
int32_t SetBatchImpl(int32_t sensorId, const SensorUser *user, int64_t samplingInterval, int64_t reportInterval);
int32_t SubscribeSensorImpl(int32_t sensorId, const SensorUser *user);
int32_t UnsubscribeSensorImpl(int32_t sensorId, const SensorUser *user);
int32_t SetModeImpl(int32_t sensorId, const SensorUser *user, int32_t mode);
int32_t SetOptionImpl(int32_t sensorId, const SensorUser *user, int32_t option);

typedef int32 (*InvokeFunc)(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t GetAllSensorsInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t ActivateSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t DeactivateSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t SetBatchInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t SubscribeSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t UnsubscribeSensorInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t SetModeInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
int32_t SetOptionInvoke(SensorFeatureApi *defaultApi, IpcIo *req, IpcIo *reply);
void SetSvcIdentity(const IpcIo *req, const IpcIo *reply);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif