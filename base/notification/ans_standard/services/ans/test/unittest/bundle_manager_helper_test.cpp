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

#include <functional>
#include <gtest/gtest.h>

#include "bundle_manager_helper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

class BundleManagerHelperTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.number    : BundleManagerHelperTest_00100
 * @tc.name      : AMS_ANS_GetBundleNameByUid_0100
 * @tc.desc      : Test GetBundleNameByUid function
 */
HWTEST_F(BundleManagerHelperTest, BundleManagerHelperTest_00100, Function | SmallTest | Level1)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    std::shared_ptr<BundleManagerHelper> bundleManager = BundleManagerHelper::GetInstance();
    EXPECT_EQ(bundleManager->GetBundleNameByUid(callingUid), "bundleName");
}

/**
 * @tc.number    : BundleManagerHelperTest_00200
 * @tc.name      : AMS_ANS_IsSystemApp_0100
 * @tc.desc      : Test IsSystemApp function
 */
HWTEST_F(BundleManagerHelperTest, BundleManagerHelperTest_00200, Function | SmallTest | Level1)
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    std::shared_ptr<BundleManagerHelper> bundleManager = BundleManagerHelper::GetInstance();
    EXPECT_TRUE(bundleManager->IsSystemApp(callingUid));
}
}  // namespace Notification
}  // namespace OHOS
