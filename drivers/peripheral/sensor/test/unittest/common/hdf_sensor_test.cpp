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

#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <gtest/gtest.h>
#include <securec.h>
#include "hdf_base.h"
#include "osal_time.h"
#include "sensor_if.h"
#include "sensor_type.h"

using namespace testing::ext;

namespace {
    struct SensorValueRange {
        float highThreshold;
        float lowThreshold;
    };

    struct SensorDevelopmentList {
        int32_t sensorTypeId;
        char sensorName[SENSOR_NAME_MAX_LEN];
        char vendorName[SENSOR_NAME_MAX_LEN];
        int32_t dataForm;    // 0: fixed, 1: range
        int32_t dataDimension;
        struct SensorValueRange *valueRange;
    };

    static struct SensorValueRange g_testRange[] = {{1e5, 0}};
    static struct SensorValueRange g_accelRange[] = {{11, -11}, {11, -11}, {11, -11}};
    static struct SensorValueRange g_proximityRange[] = {{5, 0}};
    static struct SensorValueRange g_hallRange[] = {{1, 0}};
    static struct SensorValueRange g_barometerRange[] = {{1100, 300}, {85, -40}};

    static struct SensorDevelopmentList g_sensorList[] = {
        {SENSOR_TYPE_NONE, "sensor_test", "default", 1, 1, g_testRange},
        {SENSOR_TYPE_ACCELEROMETER, "accelerometer", "borsh_bmi160", 1, 3, g_accelRange},
        {SENSOR_TYPE_PROXIMITY, "proximitymeter", "stk3338", 0, 1, g_proximityRange},
        {SENSOR_TYPE_HALL, "hallrometer", "akm_ak8789", 0, 1, g_hallRange},
        {SENSOR_TYPE_BAROMETER, "barometer", "borsh_bmp180", 1, 2, g_barometerRange},
    };

    static int g_listNum = sizeof(g_sensorList) / sizeof(g_sensorList[0]);
    static int32_t g_sensorDataFlag = 1;
    const int32_t SENSOR_ID = 0;
    const int32_t SENSOR_INTERVAL = 200000000;
    const int32_t SENSOR_POLL_TIME = 1;
    const int32_t SENSOR_WAIT_TIME = 400;
    const struct SensorInterface *g_sensorDev = nullptr;

    void SensorDataVerification(const float &data, const struct SensorDevelopmentList &sensorNode)
    {
        for (int32_t j = 0; j < sensorNode.dataDimension; ++j) {
            printf("sensor id :[%d], data[%d]: %f\n\r", sensorNode.sensorTypeId, j + 1, *(&data + j));
            if (sensorNode.dataForm == 0) {
                if (*(&data + j) == sensorNode.valueRange[j].highThreshold ||
                    *(&data + j) == sensorNode.valueRange[j].lowThreshold) {
                    g_sensorDataFlag &= 1;
                } else {
                    g_sensorDataFlag = 0;
                    printf("%s: %s Not expected\n\r", __func__, sensorNode.sensorName);
                }
            }

            if (sensorNode.dataForm == 1) {
                if (*(&data + j) > sensorNode.valueRange[j].lowThreshold &&
                    *(&data + j) < sensorNode.valueRange[j].highThreshold) {
                    g_sensorDataFlag &= 1;
                    printf("g_sensorDataFlag = 1;");
                } else {
                    g_sensorDataFlag = 0;
                    printf("%s: %s Not expected\n\r", __func__, sensorNode.sensorName);
                }
            }
        }
    }

    int SensorTestDataCallback(const struct SensorEvents *event)
    {
        if (event == nullptr || event->data == nullptr) {
            return -1;
        }

        float *data = (float*)event->data;

        for (int i = 0; i < g_listNum; ++i) {
            if (event->sensorId == g_sensorList[i].sensorTypeId) {
                SensorDataVerification(*data, g_sensorList[i]);
            }
        }

        return 0;
    }
}

class HdfSensorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfSensorTest::SetUpTestCase()
{
    g_sensorDev = NewSensorInterfaceInstance();
    if (g_sensorDev == nullptr) {
        printf("test sensorHdi get Module instance failed\n\r");
    }
}

void HdfSensorTest::TearDownTestCase()
{
    if (g_sensorDev != nullptr) {
        FreeSensorInterfaceInstance();
        g_sensorDev = nullptr;
    }
}

void HdfSensorTest::SetUp()
{
}

void HdfSensorTest::TearDown()
{
}

/**
  * @tc.name: GetSensorInstance001
  * @tc.desc: Create a sensor instance and check whether the instance is empty.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869N, AR000F8QNL
  */
HWTEST_F(HdfSensorTest, GetSensorInstance001, TestSize.Level1)
{
    ASSERT_NE(nullptr, g_sensorDev);
    const struct SensorInterface *sensorDev = NewSensorInterfaceInstance();
    EXPECT_EQ(sensorDev, g_sensorDev);
}

/**
  * @tc.name: RemoveSensorInstance001
  * @tc.desc: The sensor instance is successfully removed.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869O, AR000F8QNL
  */
HWTEST_F(HdfSensorTest, RemoveSensorInstance001, TestSize.Level1)
{
    int32_t ret = FreeSensorInterfaceInstance();
    ASSERT_EQ(0, ret);
    ret = FreeSensorInterfaceInstance();
    EXPECT_EQ(0, ret);
    g_sensorDev = NewSensorInterfaceInstance();
    if (g_sensorDev == nullptr) {
        printf("test sensorHdi get Module instance failed\n\r");
        ASSERT_EQ(0, ret);
    }
}

/**
  * @tc.name: RegisterDataCb001
  * @tc.desc: Returns 0 if the callback is successfully registered; returns a negative value otherwise.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869P, AR000F8QNL
  */
HWTEST_F(HdfSensorTest, RegisterSensorDataCb001, TestSize.Level1)
{
    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/**
  * @tc.name: RegisterDataCb002
  * @tc.desc: Returns 0 if the callback is successfully registered; returns a negative value otherwise.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869P
  */
HWTEST_F(HdfSensorTest, RegisterSensorDataCb002, TestSize.Level1)
{
    int32_t ret = g_sensorDev->Register(nullptr);
    EXPECT_EQ(SENSOR_NULL_PTR, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/**
  * @tc.name: GetSensorList001
  * @tc.desc: Obtains information about all sensors in the system. Validity check of input parameters.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869Q
  */
HWTEST_F(HdfSensorTest, GetSensorList001, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    struct SensorInformation *info = nullptr;
    int32_t count = 0;
    int j;
    int32_t ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);

    if (sensorInfo == nullptr) {
        EXPECT_NE(nullptr, sensorInfo);
        return;
    }
    printf("get sensor list num[%d]\n\r", count);
    info = sensorInfo;

    for (int i = 0; i < count; ++i) {
        printf("get sensoriId[%d], info name[%s], power[%f]\n\r", info->sensorId, info->sensorName, info->power);
        for (j = 0; j < g_listNum; ++j) {
            if (info->sensorId == g_sensorList[j].sensorTypeId) {
                EXPECT_STREQ(g_sensorList[j].sensorName, info->sensorName);
                EXPECT_STREQ(g_sensorList[j].vendorName, info->vendorName);
                break;
            }
        }

        if (j == g_listNum) {
            EXPECT_NE(g_listNum, j);
            printf("%s: The sensor ID[%d] does not match. Please check the use case or the reported sensor ID",
            __func__, info->sensorId);
        }
        info++;
    }
}

/**
  * @tc.name: GetSensorList002
  * @tc.desc: Obtains information about all sensors in the system. The operations include obtaining sensor information,
  * subscribing to or unsubscribing from sensor data, enabling or disabling a sensor,
  * setting the sensor data reporting mode, and setting sensor options such as the accuracy and measurement range.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869Q
  */
HWTEST_F(HdfSensorTest, GetSensorList002, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->GetAllSensors(nullptr, &count);
    EXPECT_EQ(SENSOR_NULL_PTR, ret);
    ret = g_sensorDev->GetAllSensors(&sensorInfo, nullptr);
    EXPECT_EQ(SENSOR_NULL_PTR, ret);
    ret = g_sensorDev->GetAllSensors(nullptr, nullptr);
    EXPECT_EQ(SENSOR_NULL_PTR, ret);
}

/**
  * @tc.name: EnableSensor001
  * @tc.desc: Enables the sensor unavailable in the sensor list based on the specified sensor ID.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869R, AR000F8QNL
  */
HWTEST_F(HdfSensorTest, EnableSensor001, TestSize.Level1)
{
    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);

    struct SensorInformation *sensorInfo = nullptr;
    struct SensorInformation *info = nullptr;
    int32_t count = 0;

    ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);

    if (sensorInfo == nullptr) {
        EXPECT_NE(nullptr, sensorInfo);
        return;
    }

    info = sensorInfo;
    for (int i = 0; i < count; i++) {
        ret = g_sensorDev->SetBatch(SENSOR_ID, SENSOR_INTERVAL, SENSOR_POLL_TIME);
        EXPECT_EQ(0, ret);
        ret = g_sensorDev->Enable(info->sensorId);
        EXPECT_EQ(0, ret);
        OsalSleep(SENSOR_POLL_TIME);
        ret = g_sensorDev->Disable(info->sensorId);
        EXPECT_EQ(0, ret);
        info++;
    }
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/**
  * @tc.name: EnableSensor002
  * @tc.desc: Enables the sensor available in the sensor list based on the specified sensor ID.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869R
  */
HWTEST_F(HdfSensorTest, EnableSensor002, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;
    int32_t ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Enable(-1);
    EXPECT_EQ(-2, ret);
    ret = g_sensorDev->Disable(-1);
    EXPECT_EQ(-2, ret);
}

/**
  * @tc.name: SetSensorBatch001
  * @tc.desc: Sets the sampling time and data report interval for sensors in batches.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869T
  */
HWTEST_F(HdfSensorTest, SetSensorBatch001, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetBatch(SENSOR_ID, SENSOR_INTERVAL, SENSOR_POLL_TIME);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Enable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    OsalMSleep(SENSOR_WAIT_TIME);
    ret = g_sensorDev->Disable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/** @tc.name: SetSensorBatch002
    @tc.desc: Sets the sampling time and data report interval for sensors in batches.
    @tc.type: FUNC
    @tc.requrire: SR000F869M, AR000F869U
    */
HWTEST_F(HdfSensorTest, SetSensorBatch002, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetBatch(-1, 0, 0);
    EXPECT_EQ(-2, ret);
}

/**
  * @tc.name: SetSensorMode001
  * @tc.desc: Sets the data reporting mode for the specified sensor.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869U, AR000F8QNL
  */
HWTEST_F(HdfSensorTest, SetSensorMode001, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetBatch(SENSOR_ID, SENSOR_INTERVAL, SENSOR_POLL_TIME);
    EXPECT_EQ(0, ret);
    if (SENSOR_ID == SENSOR_TYPE_HALL) {
        ret = g_sensorDev->SetMode(SENSOR_ID, SENSOR_MODE_ON_CHANGE);
        EXPECT_EQ(0, ret);
    } else {
        ret = g_sensorDev->SetMode(SENSOR_ID, SENSOR_MODE_REALTIME);
        EXPECT_EQ(0, ret);
    }
    ret = g_sensorDev->Enable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    OsalMSleep(SENSOR_WAIT_TIME);
    ret = g_sensorDev->Disable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/**
  * @tc.name: SetSensorMode002
  * @tc.desc: Sets the data reporting mode for the specified sensor.The current real-time polling mode is valid.
  * Other values are invalid.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869U
  */
HWTEST_F(HdfSensorTest, SetSensorMode002, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetBatch(SENSOR_ID, SENSOR_INTERVAL, SENSOR_POLL_TIME);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetMode(SENSOR_ID, SENSOR_MODE_DEFAULT);
    EXPECT_EQ(-1, ret);
    ret = g_sensorDev->Enable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    OsalMSleep(SENSOR_WAIT_TIME);
    ret = g_sensorDev->Disable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}

/**
  * @tc.name: SetSensorOption001
  * @tc.desc: Sets options for the specified sensor, including its measurement range and accuracy.
  * @tc.type: FUNC
  * @tc.require: SR000F869M, AR000F869U
  */
HWTEST_F(HdfSensorTest, SetSensorOption001, TestSize.Level1)
{
    struct SensorInformation *sensorInfo = nullptr;
    int32_t count = 0;

    int32_t ret = g_sensorDev->Register(SensorTestDataCallback);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->GetAllSensors(&sensorInfo, &count);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetBatch(SENSOR_ID, SENSOR_INTERVAL, SENSOR_POLL_TIME);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->SetOption(SENSOR_ID, 0);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Enable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    OsalMSleep(SENSOR_WAIT_TIME);
    ret = g_sensorDev->Disable(SENSOR_ID);
    EXPECT_EQ(0, ret);
    ret = g_sensorDev->Unregister();
    EXPECT_EQ(0, ret);
}
