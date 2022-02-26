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

#include <ctime>

#include "gtest/gtest.h"
#include "hilog/log.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#undef LOG_TAG
#define LOG_TAG "SENSOR_LITE"

using namespace std;
using namespace testing::ext;

#define NULLPTR ((void *)0)

namespace OHOS {
const int MS_PER_SECOND = 1000;
const int NS_PER_MS = 1000000;
const int SENSOR_PERFORMANCE_TIME = 100;
static int32_t g_dataCount = 0;

class SensorLiteTestPerformance : public testing::Test {
protected:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {}
};

void SensorDataCallbackImpl(SensorEvent *event)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "SensorDataCallbackImpl called");
    if (event == NULLPTR) {
        HILOG_ERROR(HILOG_MODULE_APP, "SensorDataCallbackImpl event is NULL");
        return;
    }
    float *sensorData = (float *)event->data;
    for (int32_t i = 0; i < (int32_t)(event->dataLen / sizeof(uint8_t *)); i++) {
        HILOG_DEBUG(HILOG_MODULE_APP, "SensorDataCallbackImpl data: %f", *(sensorData + i));
    }
    g_dataCount++;
}

uint64_t GetCurrentMillSecTime()
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)((ts.tv_sec * MS_PER_SECOND) + (ts.tv_nsec / NS_PER_MS));
}

/**
 * @tc.name: GetAllSensorsPerformance001
 * @tc.desc: Get sensor list performance test.
 * @tc.type: PERF
 * @tc.require: SR000F46AC
 */
HWTEST_F(SensorLiteTestPerformance, GetAllSensorsPerformance001, TestSize.Level2)
{
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;
    uint64_t startTime = GetCurrentMillSecTime();
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    uint64_t endTime = GetCurrentMillSecTime();
    uint64_t duration = endTime - startTime;
    EXPECT_TRUE(((duration >= 0) && (duration <= SENSOR_PERFORMANCE_TIME)));
    HILOG_DEBUG(HILOG_MODULE_APP, "GetAllSensorsPerformance001 starttime: %lld, endTime: %lld, duration:%lld",
        startTime, endTime, duration);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;
    for (int i = 0; i < count; i++) {
        HILOG_INFO(HILOG_MODULE_APP, "%s %s %s %s %d %d %f %f %f!",
            (sensorInfo + i)->sensorName, (sensorInfo + i)->vendorName,
            (sensorInfo + i)->firmwareVersion, (sensorInfo + i)->hardwareVersion,
            (sensorInfo + i)->sensorTypeId, (sensorInfo + i)->sensorId,
            (sensorInfo + i)->maxRange, (sensorInfo + i)->precision,
            (sensorInfo + i)->power);
    }
};

/**
 * @tc.name: SubscribeSensorPerformance001
 * @tc.desc: Subscribe sensor performance test.
 * @tc.type: PERF
 * @tc.require: SR000F46AE
 */
HWTEST_F(SensorLiteTestPerformance, SubscribeSensorPerformance001, TestSize.Level2)
{
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;

    uint64_t startTime = GetCurrentMillSecTime();
    ret = SubscribeSensor(0, &sensorUser);
    uint64_t endTime = GetCurrentMillSecTime();
    uint64_t duration = endTime - startTime;
    EXPECT_TRUE(((duration >= 0) && (duration <= SENSOR_PERFORMANCE_TIME)));
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    HILOG_DEBUG(HILOG_MODULE_APP, "SubscribeSensorPerformance001 starttime: %lld, endTime: %lld, duration:%lld",
        startTime, endTime, duration);
    sleep(1);
    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: UnsubscribeSensorPerformance001
 * @tc.desc: Unsubscribe sensor performance test.
 * @tc.type: PERF
 * @tc.require: AR000F46AM
 */
HWTEST_F(SensorLiteTestPerformance, UnsubscribeSensorPerformance001, TestSize.Level2)
{
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensorPerformance001 ret = " << ret << endl;

    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    sleep(1);

    uint64_t startTime = GetCurrentMillSecTime();
    ret = UnsubscribeSensor(0, &sensorUser);
    uint64_t endTime = GetCurrentMillSecTime();
    uint64_t duration = endTime - startTime;
    EXPECT_TRUE(((duration >= 0) && (duration <= SENSOR_PERFORMANCE_TIME)));
    HILOG_DEBUG(HILOG_MODULE_APP, "UnsubscribeSensorPerformance001 starttime: %lld, endTime: %lld, duration:%lld",
        startTime, endTime, duration);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: ActivateSensorPerformance001
 * @tc.desc: Get sensor list performance test.
 * @tc.type: PERF
 * @tc.require: SR000F46AG
 */
HWTEST_F(SensorLiteTestPerformance, ActivateSensorPerformance001, TestSize.Level2)
{
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;

    uint64_t startTime = GetCurrentMillSecTime();
    ret = ActivateSensor(0, &sensorUser);
    uint64_t endTime = GetCurrentMillSecTime();
    uint64_t duration = endTime - startTime;
    EXPECT_TRUE(((duration >= 0) && (duration <= SENSOR_PERFORMANCE_TIME)));
    HILOG_DEBUG(HILOG_MODULE_APP, "ActivateSensorPerformance001 starttime: %lld, endTime: %lld, duration:%lld",
        startTime, endTime, duration);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);
};

/**
 * @tc.name: DeactivateSensorPerformance001
 * @tc.desc: Get sensor list performance test.
 * @tc.type: PERF
 * @tc.require: AR000F46AJ
 */
HWTEST_F(SensorLiteTestPerformance, DeactivateSensorPerformance001, TestSize.Level2)
{
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    uint64_t startTime = GetCurrentMillSecTime();
    ret = DeactivateSensor(0, &sensorUser);
    uint64_t endTime = GetCurrentMillSecTime();
    uint64_t duration = endTime - startTime;
    EXPECT_TRUE(((duration >= 0) && (duration <= SENSOR_PERFORMANCE_TIME)));
    HILOG_DEBUG(HILOG_MODULE_APP, "DeactivateSensorPerformance001 starttime: %lld, endTime: %lld, duration:%lld",
        startTime, endTime, duration);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);
};


/**
 * @tc.name: DataReportPerformanceTest001
 * @tc.desc: Sensor data report.
 * @tc.type: PERF
 * @tc.require: SR000F46AN
 */
HWTEST_F(SensorLiteTestPerformance, DataReportPerformanceTest001, TestSize.Level2)
{
    HILOG_INFO(HILOG_MODULE_APP, "%s called", __func__);
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;
    sleep(1);

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    g_dataCount = 0;
    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    sleep(5);
    HILOG_INFO(HILOG_MODULE_APP, "%s g_dataCount: %d", __func__, g_dataCount);
    EXPECT_LT(g_dataCount, 50);
    EXPECT_GT(g_dataCount, 0);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};
}
