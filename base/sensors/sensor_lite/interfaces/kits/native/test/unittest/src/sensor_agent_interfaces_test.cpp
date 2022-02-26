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

#include "gtest/gtest.h"
#include "hilog/log.h"
#include "sensor_agent.h"
#include "sensor_agent_type.h"

#undef LOG_TAG
#define LOG_TAG "SENSOR_LITE"
#undef SENSOR_ERROR_INVALID_PARAM
#define SENSOR_ERROR_INVALID_PARAM (-3)

using namespace std;
using namespace testing::ext;

#define NULLPTR ((void *)0)

namespace OHOS {
class SensorAgentInterfacesTest : public testing::Test {
protected:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

void SensorDataCallbackImpl(SensorEvent *event)
{
    HILOG_DEBUG(HILOG_MODULE_APP, "SensorDataCallbackImpl called");
    if (event == NULLPTR) {
        return;
    }
    float *sensorData = (float *)event->data;
    for (int32_t i = 0; i < (int32_t)(event->dataLen / sizeof(uint8_t *)); i++) {
        HILOG_DEBUG(HILOG_MODULE_APP, "SensorDataCallbackImpl data: %f", *(sensorData + i));
    }
}

/**
 * @tc.name: GetAllSensorsInterfaceTest001
 * @tc.desc: Test the interface for obtaining the sensor list in normal scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000F46AD
 */
HWTEST_F(SensorAgentInterfacesTest, GetAllSensorsInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "GetAllSensorsInterfaceTest001 called");
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    HILOG_INFO(HILOG_MODULE_APP, "GetAllSensorsInterfaceTest001 sensor lists count: %d", count);
    for (int i = 0; i < count; i++) {
        HILOG_INFO(HILOG_MODULE_APP, "%s %s %s %s %d %d %f %f %f!",
            (sensorInfo + i)->sensorName, (sensorInfo + i)->vendorName,
            (sensorInfo + i)->firmwareVersion, (sensorInfo + i)->hardwareVersion,
            (sensorInfo + i)->sensorTypeId, (sensorInfo + i)->sensorId,
            (sensorInfo + i)->maxRange, (sensorInfo + i)->precision,
            (sensorInfo + i)->power);
    }
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
};

/**
 * @tc.name: GetAllSensorsInterfaceTest002
 * @tc.desc: Test the interface for obtaining the sensor list in abnormal scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000F46AH
 */
HWTEST_F(SensorAgentInterfacesTest, GetAllSensorsInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "GetAllSensorsInterfaceTest002 called");
    SensorInfo **sensorInfo = (SensorInfo **)NULLPTR;
    int32_t ret = GetAllSensors(sensorInfo, (int32_t *)NULLPTR);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "GetAllSensors ret = " << ret << endl;
};

/**
 * @tc.name: GetAllSensorsInterfaceTest003
 * @tc.desc: Test the interface for obtaining the sensor list in abnormal scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000F46AH
 */
HWTEST_F(SensorAgentInterfacesTest, GetAllSensorsInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "GetAllSensorsInterfaceTest003 called");
    int32_t count = 0;
    SensorInfo **sensorInfo = (SensorInfo **)NULLPTR;
    int32_t ret = GetAllSensors(sensorInfo, &count);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "GetAllSensors ret = " << ret << endl;
};

/**
 * @tc.name: GetAllSensorsInterfaceTest004
 * @tc.desc: Test the interface for obtaining the sensor list in abnormal scenarios.
 * @tc.type: FUNC
 * @tc.require: AR000F46AH
 */
HWTEST_F(SensorAgentInterfacesTest, GetAllSensorsInterfaceTest004, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "GetAllSensorsInterfaceTest004 called");
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t ret = GetAllSensors(&sensorInfo, (int32_t *)NULLPTR);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "GetAllSensors ret = " << ret << endl;
};

/**
 * @tc.name: ActivateSensorInterfaceTest001
 * @tc.desc: Test the enable sensor interface, the input parameter sensorid is set to -1.
 * @tc.type: FUNC
 * @tc.require: AR000F46AI
 */
HWTEST_F(SensorAgentInterfacesTest, ActivateSensorInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "ActivateSensorInterfaceTest001 called");
    SensorUser sensorUser;
    int32_t ret = ActivateSensor(-1, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "ActivateSensor ret = " << ret << endl;
};

/**
 * @tc.name: ActivateSensorInterfaceTest002
 * @tc.desc: Test the enable sensor interface, the input parameter sensorUser is set to NULL.
 * @tc.type: FUNC
 * @tc.require: AR000F46AI
 */
HWTEST_F(SensorAgentInterfacesTest, ActivateSensorInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "ActivateSensorInterfaceTest002 called");
    SensorUser *sensorUser = (SensorUser *)NULLPTR;
    int32_t ret = ActivateSensor(0, sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "ActivateSensor ret = " << ret << endl;
};

/**
 * @tc.name: ActivateSensorInterfaceTest003
 * @tc.desc: Test the enable sensor interface, the process is to obtain the sensor list, register, enable, disable,
 * and deregister the sensor. The enabled sensor ID is 1.
 * @tc.type: FUNC
 * @tc.require: AR000F46AI
 */
HWTEST_F(SensorAgentInterfacesTest, ActivateSensorInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "ActivateSensorInterfaceTest003 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
    sleep(1);

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(3);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);
};

/**
 * @tc.name: DeactivateSensorInterfaceTest001
 * @tc.desc: Disable sensor, the input parameter sensorUser is set to NULL.
 * @tc.type: FUNC
 * @tc.require: AR000F46AJ
 */
HWTEST_F(SensorAgentInterfacesTest, DeactivateSensorInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "DeactivateSensorInterfaceTest001 called");
    SensorUser *sensorUser = (SensorUser *)NULLPTR;
    int32_t ret = DeactivateSensor(0, sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "DeactivateSensor ret = " << ret << endl;
};

/**
 * @tc.name: DeactivateSensorInterfaceTest002
 * @tc.desc: Disable sensor, the input parameter sensor id is -1.
 * @tc.type: FUNC
 * @tc.require: AR000F46AJ
 */
HWTEST_F(SensorAgentInterfacesTest, DeactivateSensorInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "DeactivateSensorInterfaceTest002 called");
    SensorUser sensorUser;
    int32_t ret = DeactivateSensor(-1, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "DeactivateSensor ret = " << ret << endl;
};

/**
 * @tc.name: SetBatchInterfaceTest001
 * @tc.desc: Set sensor param, sensorUser is NULL.
 * @tc.type: FUNC
 * @tc.require: AR000F46AD
 */
HWTEST_F(SensorAgentInterfacesTest, SetBatchInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SetBatchInterfaceTest001 called");
    SensorUser *sensorUser = (SensorUser *)nullptr;
    int32_t ret = SetBatch(0, sensorUser, 200, 3000);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SetBatch ret = " << ret << endl;
};

/**
 * @tc.name: SetBatchInterfaceTest002
 * @tc.desc: Set sensor param.
 * @tc.type: FUNC
 * @tc.require: AR000F46AD
 */
HWTEST_F(SensorAgentInterfacesTest, SetBatchInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SetBatchInterfaceTest002 called");
    SensorUser sensorUser;
    int32_t ret = SetBatch(0, &sensorUser, -1, 3000);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SetBatch ret = " << ret << endl;
};

/**
 * @tc.name: SetBatchInterfaceTest003
 * @tc.desc: Set sensor param.
 * @tc.type: FUNC
 * @tc.require: AR000F46AF
 */
HWTEST_F(SensorAgentInterfacesTest, SetBatchInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SetBatchInterfaceTest003 called");
    SensorUser sensorUser;
    int32_t ret = SetBatch(0, &sensorUser, 200, 3000);
    EXPECT_EQ(ret, 0) << "SetBatch ret = " << ret << endl;
};

/**
 * @tc.name: SetBatchInterfaceTest004
 * @tc.desc: Set sensor param.
 * @tc.type: FUNC
 * @tc.require: AR000F46AF
 */
HWTEST_F(SensorAgentInterfacesTest, SetBatchInterfaceTest004, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SetBatchInterfaceTest004 called");
    SensorUser sensorUser;
    int32_t ret = SetBatch(0, &sensorUser, 200, -1);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SetBatch ret = " << ret << endl;
};

/**
 * @tc.name: SetBatchInterfaceTest005
 * @tc.desc: Set sensor param.
 * @tc.type: FUNC
 * @tc.require: AR000F46AF
 */
HWTEST_F(SensorAgentInterfacesTest, SetBatchInterfaceTest005, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SetBatchInterfaceTest005 called");
    SensorUser sensorUser;
    int32_t ret = SetBatch(-1, &sensorUser, 200, 3000);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SetBatch ret = " << ret << endl;
};

/**
 * @tc.name: SubscribeSensorInterfaceTest001
 * @tc.desc: SubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: AR000F46AL
 */
HWTEST_F(SensorAgentInterfacesTest, SubscribeSensorInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SubscribeSensorInterfaceTest001 called");
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
    sleep(1);

    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;
    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: SubscribeSensorInterfaceTest002
 * @tc.desc: SubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: AR000F46AL
 */
HWTEST_F(SensorAgentInterfacesTest, SubscribeSensorInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SubscribeSensorInterfaceTest002 called");
    SensorUser *sensorUser = (SensorUser *)NULLPTR;
    int32_t ret = SubscribeSensor(0, sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: SubscribeSensorInterfaceTest003
 * @tc.desc: SubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: AR000F46AL
 */
HWTEST_F(SensorAgentInterfacesTest, SubscribeSensorInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "SubscribeSensorInterfaceTest003 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;
    int32_t ret = SubscribeSensor(-1, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "SubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: UnSubscribeSensorInterfaceTest001
 * @tc.desc: UnSubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: AR000F46AM
 */
HWTEST_F(SensorAgentInterfacesTest, UnSubscribeSensorInterfaceTest001, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "UnSubscribeSensorInterfaceTest001 called");
    SensorUser *sensorUser = (SensorUser *)NULLPTR;
    int32_t ret = UnsubscribeSensor(0, sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: UnSubscribeSensorInterfaceTest002
 * @tc.desc: UnSubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: AR000F46AM
 */
HWTEST_F(SensorAgentInterfacesTest, UnSubscribeSensorInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "UnSubscribeSensorInterfaceTest002 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;
    int32_t ret = UnsubscribeSensor(-1, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: UnSubscribeSensorInterfaceTest003
 * @tc.desc: UnSubscribeSensor sensor.
 * @tc.type: FUNC
 * @tc.require: SR000F46AK
 */
HWTEST_F(SensorAgentInterfacesTest, UnSubscribeSensorInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "DataReportInterfaceTest001 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;
    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensorsTest001 ret = " << ret << endl;
    sleep(1);

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: DataReportInterfaceTest001
 * @tc.desc: Sensor data report.
 * @tc.type: FUNC
 * @tc.require: AR000F46AO
 */
HWTEST_F(SensorAgentInterfacesTest, DataReportInterfaceTest001, TestSize.Level0)
{
    HILOG_INFO(HILOG_MODULE_APP, "DataReportInterfaceTest001 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
    sleep(1);

    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    sleep(1);

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: DataReportInterfaceTest002
 * @tc.desc: Sensor data report.
 * @tc.type: FUNC
 * @tc.require: AR000F46AO
 */
HWTEST_F(SensorAgentInterfacesTest, DataReportInterfaceTest002, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "DataReportInterfaceTest002 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
    sleep(1);

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    sleep(1);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;
};

/**
 * @tc.name: DataReportInterfaceTest003
 * @tc.desc: Sensor data report.
 * @tc.type: FUNC
 * @tc.require: AR000F46AO
 */
HWTEST_F(SensorAgentInterfacesTest, DataReportInterfaceTest003, TestSize.Level1)
{
    HILOG_INFO(HILOG_MODULE_APP, "DataReportInterfaceTest003 called");
    SensorUser sensorUser;
    sensorUser.callback = SensorDataCallbackImpl;

    SensorInfo *sensorInfo = (SensorInfo *)NULLPTR;
    int32_t count = 0;

    int32_t ret = GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(ret, 0) << "GetAllSensors ret = " << ret << endl;
    sleep(1);

    ret = SubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "SubscribeSensor ret = " << ret << endl;
    sleep(1);

    ret = ActivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "ActivateSensor ret = " << ret << endl;
    sleep(1);

    ret = DeactivateSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "DeactivateSensor ret = " << ret << endl;
    sleep(1);

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, 0) << "UnsubscribeSensor ret = " << ret << endl;

    ret = UnsubscribeSensor(0, &sensorUser);
    EXPECT_EQ(ret, SENSOR_ERROR_INVALID_PARAM) << "UnsubscribeSensor ret = " << ret << endl;
};
}