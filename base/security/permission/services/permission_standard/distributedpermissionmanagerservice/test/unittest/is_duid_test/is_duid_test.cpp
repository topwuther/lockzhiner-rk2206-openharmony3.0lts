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
#include "distributed_permission_kit.h"
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
class IsDuidTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/**
 * @tc.number: IsDuid_0001
 * @tc.name: IsDuid
 * @tc.desc: IsDuid is valid.
 */
HWTEST_F(IsDuidTest, IsDuid_0001, Function | MediumTest | Level1)
{
    bool flag = DistributedPermissionKit::IsDuid(12610001);
    GTEST_LOG_(INFO) << flag;
    EXPECT_TRUE(flag);
}

/**
 * @tc.number: IsDuid_0002
 * @tc.name: IsDuid
 * @tc.desc: IsDuid is invalid.
 */
HWTEST_F(IsDuidTest, IsDuid_0002, Function | MediumTest | Level1)
{
    bool flag = DistributedPermissionKit::IsDuid(1);
    GTEST_LOG_(INFO) << flag;
    EXPECT_FALSE(flag);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
