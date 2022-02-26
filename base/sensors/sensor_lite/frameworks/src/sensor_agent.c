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

void *g_proxy;

int32_t GetAllSensors(SensorInfo **sensorInfo, int32_t *count)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return GetAllSensorsByProxy(g_proxy, sensorInfo, count);
}

int32_t ActivateSensor(int32_t sensorTypeId, SensorUser *user)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return ActivateSensorByProxy(g_proxy, sensorTypeId, user);
}

int32_t DeactivateSensor(int32_t sensorTypeId, SensorUser *user)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return DeactivateSensorByProxy(g_proxy, sensorTypeId, user);
}

int32_t SetBatch(int32_t sensorTypeId, SensorUser *user, int64_t samplingInterval, int64_t reportInterval)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return SetBatchByProxy(g_proxy, sensorTypeId, user, samplingInterval, reportInterval);
}

int32_t SubscribeSensor(int32_t sensorTypeId, SensorUser *user)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return SubscribeSensorByProxy(g_proxy, sensorTypeId, user);
}

int32_t UnsubscribeSensor(int32_t sensorTypeId, SensorUser *user)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return UnsubscribeSensorByProxy(g_proxy, sensorTypeId, user);
}

int32_t SetMode(int32_t sensorTypeId, SensorUser *user, int32_t mode)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return SetModeByProxy(g_proxy, sensorTypeId, user, mode);
}

int32_t SetOption(int32_t sensorTypeId, SensorUser *user, int32_t option)
{
    if (g_proxy == NULL) {
        g_proxy = GetServiceProxy();
    }
    return SetOptionByProxy(g_proxy, sensorTypeId, user, option);
}