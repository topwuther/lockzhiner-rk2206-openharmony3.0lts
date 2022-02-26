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
#define private public
#include "distributed_permission_manager_service.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
namespace Security {
namespace Permission {
class QueryDuidTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {
        DistributedUidAllocator::GetInstance().Clear();
        DeviceInfoRepository::GetInstance().Clear();
    }
    std::shared_ptr<DistributedPermissionManagerService> GetInstance();

    int32_t validRuid_ = 1024;
    int32_t resultCode_ = -2;
};
std::shared_ptr<DistributedPermissionManagerService> QueryDuidTest::GetInstance()
{
    return std::make_shared<DistributedPermissionManagerService>();
}
/**
 * @tc.number: QueryDuid_0001
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is invalid.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0001, Function | MediumTest | Level1)
{
    int32_t duid = GetInstance()->QueryDuid("", validRuid_);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, resultCode_);
}

/**
 * @tc.number: QueryDuid_0002
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is not existing in deviceIdMap.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0002, Function | MediumTest | Level1)
{
    int32_t duid = GetInstance()->QueryDuid("notExistDeviceId", validRuid_);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, resultCode_);
}

/**
 * @tc.number: QueryDuid_0003
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is existing in deviceIdMap,
 *           ruid is invalid.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0003, Function | MediumTest | Level1)
{
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        "networkId", "universallyUniqueId", "uniqueDisabilityId", "deviceName", "deviceType");
    int32_t duid = GetInstance()->QueryDuid("networkId", -1);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, resultCode_);
}

/**
 * @tc.number: QueryDuid_0004
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is existing in deviceIdMap,
 *           uniqueDisabilityId is empty,
 *           get result from SoftBusManager,
 *           duid is not existing.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0004, Function | MediumTest | Level1)
{
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        "networkId", "universallyUniqueId", "", "deviceName", "deviceType");
    int32_t duid = GetInstance()->QueryDuid("networkId", validRuid_);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, resultCode_);
}

/**
 * @tc.number: QueryDuid_0005
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is existing in deviceIdMap,
 *           duid is not existing.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0005, Function | MediumTest | Level1)
{
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        "networkId", "universallyUniqueId", "uniqueDisabilityId", "deviceName", "deviceType");
    int32_t duid = GetInstance()->QueryDuid("networkId", validRuid_);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, resultCode_);
}

/**
 * @tc.number: QueryDuid_0006
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is existing in deviceIdMap,
 *           duid is existing.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0006, Function | MediumTest | Level1)
{
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        "networkId", "universallyUniqueId", "uniqueDisabilityId", "deviceName", "deviceType");
    std::string key = DistributedUidAllocator::GetInstance().Hash("uniqueDisabilityId", validRuid_);
    DistributedUidEntity distributedUidEntity;
    distributedUidEntity.distributedUid = 12610001;
    DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
        std::pair<std::string, DistributedUidEntity>(key, distributedUidEntity));
    int32_t duid = GetInstance()->QueryDuid("uniqueDisabilityId", validRuid_);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, 12610001);
}

/**
 * @tc.number: QueryDuid_0007
 * @tc.name: QueryDuid
 * @tc.desc: deviceId is existing in deviceIdMap,
 *           ruid is special.
 */
HWTEST_F(QueryDuidTest, QueryDuid_0007, Function | MediumTest | Level1)
{
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        "networkId", "universallyUniqueId", "uniqueDisabilityId", "deviceName", "deviceType");
    int32_t duid = GetInstance()->QueryDuid("uniqueDisabilityId", 1000);
    GTEST_LOG_(INFO) << duid;
    EXPECT_EQ(duid, 12601000);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
