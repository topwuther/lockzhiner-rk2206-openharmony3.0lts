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

#include "bluetooth_gatt_service.h"
#include "uuid.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
class GattServiceTest : public testing::Test {
public:
    GattServiceTest()
    {}
    ~GattServiceTest()
    {}

    int tempData_ = 0;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattServiceTest::SetUpTestCase(void)
{}
void GattServiceTest::TearDownTestCase(void)
{}
void GattServiceTest::SetUp()
{
    tempData_ = 0;
}

void GattServiceTest::TearDown()
{}

HWTEST_F(GattServiceTest, GattService_ModuleTest_IsPrimary, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    EXPECT_EQ(true, GattService(uuid_, serviceType_).IsPrimary());
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::IsPrimary ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_GetHandle_, TestSize.Level1)
{
    UUID uuid_;
    uuid_ = UUID::FromString("00001820-0000-1000-8000-00805F9B34FB");
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    uint16_t result = GattService(uuid_, handle_, endHandle_, serviceType_).GetHandle();
    EXPECT_GE(0, result);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::GetHandle ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_GetUuid, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    const UUID result = GattService(uuid_, handle_, endHandle_, serviceType_).GetUuid();
    bool cmp;
    if (uuid_.ToString() == result.ToString()) {
        cmp = true;
    } else {
        cmp = false;
    }
    EXPECT_EQ(true, cmp);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::GetUuid ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_AddService, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    std::vector<std::reference_wrapper<GattService>> includedservices;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService addservice = GattService(uuid_test, serviceType_);
    GattService service = GattService(uuid_, handle_, endHandle_, serviceType_);
    service.AddService(addservice);
    includedservices = service.GetIncludedServices();
    bool result = false;
    if (includedservices.size() == 0) {
        result = true;
    }
    EXPECT_EQ(false, result);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::AddService ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_GetIncludedServices, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    std::vector<std::reference_wrapper<GattService>> includedservices;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattService addservice = GattService(uuid_test, serviceType_);
    GattService service = GattService(uuid_, handle_, endHandle_, serviceType_);
    service.AddService(addservice);
    includedservices = service.GetIncludedServices();
    bool result = false;
    if (includedservices.size() == 0) {
        result = true;
    }
    EXPECT_EQ(true, result);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::GetIncludedServices ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_AddCharacteristic, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    int permissions = 27;
    int properties = 26;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);

    GattService addcharacter = GattService(uuid_, handle_, endHandle_, serviceType_);
    addcharacter.AddCharacteristic(characteristic);
    auto test = addcharacter.GetCharacteristic(uuid_test);
    bool result = false;
    if (test == nullptr) {
        result = true;
    }
    EXPECT_EQ(false, result);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::AddCharacteristic ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_GetCharacteristic, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    int permissions = 27;
    int properties = 26;
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);
    GattService getcharacter = GattService(uuid_, handle_, endHandle_, serviceType_);
    getcharacter.AddCharacteristic(characteristic);
    auto test = getcharacter.GetCharacteristic(uuid_test);
    bool result = false;
    if (test == nullptr) {
        result = true;
    }
    EXPECT_EQ(false, result);

    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::GetCharacteristic ends";
}

HWTEST_F(GattServiceTest, GattService_ModuleTest_GetCharacteristics, TestSize.Level1)
{
    UUID uuid_ = UUID::RandomUUID();
    GattServiceType serviceType_ = GattServiceType::PRIMARY;
    uint16_t handle_ = 0b0000000000010000;
    uint16_t endHandle_ = 0b0000000000010001;
    UUID uuid_test;
    uuid_test = UUID::FromString("00001810-0000-1000-8000-00805F9B34FB");
    int permissions = 27;
    int properties = 26;
    GattCharacteristic characteristic = GattCharacteristic(uuid_test, permissions, properties);

    GattService getcharacters = GattService(uuid_, handle_, endHandle_, serviceType_);
    getcharacters.AddCharacteristic(characteristic);

    std::vector<GattCharacteristic> res;
    res = getcharacters.GetCharacteristics();
    bool result = false;
    if (res.size() == 0) {
        result = true;
    }
    EXPECT_EQ(false, result);
    GTEST_LOG_(INFO) << "GattService::GattService ends";
    GTEST_LOG_(INFO) << "GattService::GetCharacteristics ends";
}
}  // namespace Bluetooth
}  // namespace OHOS
