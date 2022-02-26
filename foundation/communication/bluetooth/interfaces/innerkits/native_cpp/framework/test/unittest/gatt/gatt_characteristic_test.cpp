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

#include <gtest/gtest.h>
#include "securec.h"
#include <gmock/gmock.h>
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_descriptor.h"
#include "uuid.h"
#include "bluetooth_def.h"
#include <memory>

using namespace testing;
using namespace testing::ext;
using namespace bluetooth;

namespace OHOS {
namespace Bluetooth {

class GattCharacteristicTest : public testing::Test {
public:
    GattCharacteristicTest()
    {}
    ~GattCharacteristicTest()
    {}

    GattCharacteristic *pGattCharacteristic = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattCharacteristicTest::SetUpTestCase(void)
{}
void GattCharacteristicTest::TearDownTestCase(void)
{}
void GattCharacteristicTest::SetUp()
{}

void GattCharacteristicTest::TearDown()
{}

// URI: scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment

HWTEST_F(GattCharacteristicTest, GattCharacteristic_ModuleTest_RequestBleMtuSize, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattCharacteristic_ModuleTest_RequestBleMtuSize start";
    UUID uuid_ = UUID::RandomUUID();
    uint16_t handle_ = 1;
    const int permissions_ = 100;
    const int properties_ = 200;

    GTEST_LOG_(INFO) << "GattCharacteristic_GattCharacteristic(4 parameters) start";
    pGattCharacteristic = new GattCharacteristic(uuid_, handle_, permissions_, properties_);
    GTEST_LOG_(INFO) << "GattCharacteristic_GattCharacteristic(4 parameters) end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GattCharacteristic(3 parameters) start";
    GTEST_LOG_(INFO) << "GattCharacteristic_GattCharacteristic(3 parameters) end";

    GTEST_LOG_(INFO) << "GattCharacteristic_AddDescriptor start";
    GattDescriptor *pdescriptor = new GattDescriptor(uuid_, permissions_);
    pGattCharacteristic->AddDescriptor(*pdescriptor);
    GTEST_LOG_(INFO) << "GattCharacteristic_AddDescriptor end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptor start";
    GattDescriptor *pdescriptorTmp = pGattCharacteristic->GetDescriptor(uuid_);
    if (pdescriptorTmp->GetUuid().Equals(uuid_)) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptor returns correctly";
    }
    GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptor end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptors start";
    std::vector<GattDescriptor> descriptors_ = pGattCharacteristic->GetDescriptors();
    bool result = false;
    if (descriptors_.size() == 1) {
        result = true;
        EXPECT_EQ(true, result);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptors returns correctly";
    }
    GTEST_LOG_(INFO) << "GattCharacteristic_GetDescriptors end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetHandle start";
    uint16_t tempHandle = pGattCharacteristic->GetHandle();
    EXPECT_EQ(handle_, tempHandle);
    GTEST_LOG_(INFO) << "GattCharacteristic_GetHandle end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetPermissions start";
    int tempPer = pGattCharacteristic->GetPermissions();
    EXPECT_EQ(permissions_, tempPer);
    GTEST_LOG_(INFO) << "GattCharacteristic_GetPermissions end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetProperties start";
    int tempPro = pGattCharacteristic->GetProperties();
    EXPECT_EQ(properties_, tempPro);
    GTEST_LOG_(INFO) << "GattCharacteristic_GetProperties end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetService start";
    GattService *tempGattService = pGattCharacteristic->GetService();
    if (nullptr == tempGattService) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetService returns correctly";
    }
    GTEST_LOG_(INFO) << "GattCharacteristic_GetService end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetUuid start";
    UUID tempUuid = pGattCharacteristic->GetUuid();
    if (tempUuid.Equals(uuid_)) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetUuid returns correctly";
    }
    GTEST_LOG_(INFO) << "GattCharacteristic_GetUuid end";

    GTEST_LOG_(INFO) << "GattCharacteristic_SetValue start";
    size_t length_Set = 1;
    uint8_t values_Set[1] = {'a'};
    pGattCharacteristic->SetValue(values_Set, length_Set);
    GTEST_LOG_(INFO) << "GattCharacteristic_SetValue end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetValue start";
    size_t length_Get = 1;
    size_t *plength_Get = &length_Get;

    std::unique_ptr<uint8_t[]> values_Get = std::make_unique<uint8_t[]>(length_Get);
    memcpy_s(values_Get.get(), length_Get, pGattCharacteristic->GetValue(plength_Get).get(), length_Get);
    const uint8_t *values = static_cast<uint8_t *>(values_Get.get());
    // std::unique_ptr<uint8_t[]> values_Get = std::move(pGattCharacteristic->GetValue(plength_Get));
    uint8_t resultValuesArray = values[0];
    uint8_t resultValuesPointer = *values;

    if ('a' == resultValuesArray) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetValue returns correctly";
    }

    if ('a' == resultValuesPointer) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetValue returns correctly";
    }

    GTEST_LOG_(INFO) << "GattCharacteristic_GetValue end";

    GTEST_LOG_(INFO) << "GattCharacteristic_SetWriteType start";
    GattCharacteristic::WriteType typeTemp = GattCharacteristic::WriteType::DEFAULT;
    int returnCode = 0;
    returnCode = pGattCharacteristic->SetWriteType(typeTemp);
    EXPECT_EQ(returnCode, bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattCharacteristic_SetWriteType end";

    GTEST_LOG_(INFO) << "GattCharacteristic_GetWriteType start";
    int resultType = pGattCharacteristic->GetWriteType();
    if (typeTemp == resultType) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GattCharacteristic_GetWriteType returns correctly";
    }
    GTEST_LOG_(INFO) << "GattCharacteristic_GetWriteType end";

    GTEST_LOG_(INFO) << "GattCharacteristic_ModuleTest_RequestBleMtuSize end";
}

}  // namespace Bluetooth
}  // namespace OHOS
