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

#include "distributed_sched_permission_test.h"

#include "distributed_sched_interface.h"
#include "distributed_sched_permission.h"
#include "dtbschedmgr_log.h"
#include "test_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
void DistributedSchedPermissionTest::SetUpTestCase()
{
    DTEST_LOG << "DistributedSchedPermissionTest::SetUpTestCase" << std::endl;
}

void DistributedSchedPermissionTest::TearDownTestCase()
{
    DTEST_LOG << "DistributedSchedPermissionTest::TearDownTestCase" << std::endl;
}

void DistributedSchedPermissionTest::TearDown()
{
    DTEST_LOG << "DistributedSchedPermissionTest::TearDown" << std::endl;
}

void DistributedSchedPermissionTest::SetUp()
{
    DTEST_LOG << "DistributedSchedPermissionTest::SetUp" << std::endl;
}

/**
 * @tc.name: CheckDPermission_001
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_001 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    std::string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, abilityInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_001 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_002
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_002 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = false;
    std::string localDeviceId = "255.255.255.255";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, abilityInfo, localDeviceId);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_002 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_003
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_003, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_003 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    std::string localDeviceId = "";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, abilityInfo, localDeviceId);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_003 end ret:" << ret << std::endl;
}

/**
 * @tc.name: CheckDPermission_004
 * @tc.desc: input invalid params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedPermissionTest, CheckDPermission_004, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_004 begin" << std::endl;
    AAFwk::Want want;
    CallerInfo callerInfo;
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = IDistributedSched::DIFF_ACCOUNT_TYPE;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = false;
    std::string localDeviceId = "";
    int32_t ret = DistributedSchedPermission::GetInstance().CheckDPermission(want,
        callerInfo, accountInfo, abilityInfo, localDeviceId);
    EXPECT_TRUE(ret == INVALID_PARAMETERS_ERR);
    DTEST_LOG << "DistributedSchedPermissionTest CheckDPermission_004 end ret:" << ret << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS