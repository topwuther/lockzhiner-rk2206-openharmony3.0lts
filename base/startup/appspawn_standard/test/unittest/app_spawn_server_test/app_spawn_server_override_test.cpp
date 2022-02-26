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
#include <thread>
#include <unistd.h>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "appspawn_server.h"
#undef private
#undef protected

#include <dirent.h>
#include <dlfcn.h>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

static constexpr int TEST_WAIT_TIME = 50 * 1000;  // 50 ms

class AppSpawnServerOverrideTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
protected:
    std::unique_ptr<AppSpawnServer> appSpawnServer_ = nullptr;
};

void AppSpawnServerOverrideTest::SetUpTestCase()
{}

void AppSpawnServerOverrideTest::TearDownTestCase()
{}

void AppSpawnServerOverrideTest::SetUp()
{
    if (appSpawnServer_ == nullptr) {
        appSpawnServer_ = std::make_unique<AppSpawnServer>("AppSpawnServerOverrideTest");
    }
}

void AppSpawnServerOverrideTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: ServerMain
 * FunctionPoints: fork app process
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ServerMain fork app process success.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_001 start";

    char argv[20] = "LongNameTest";
    auto func = [&]() {
        // wait ServerMain unit test case
        usleep(TEST_WAIT_TIME);
        appSpawnServer_->SetRunning(false);
    };
    std::thread(func).detach();
    EXPECT_EQ(false, appSpawnServer_->ServerMain(argv, sizeof(argv)));

    // wait release
    usleep(TEST_WAIT_TIME);

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_001 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and longProcName param is nullptr.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_002 start";

    char *longProcName = nullptr;
    int64_t longProcNameLen = sizeof(longProcName);
    char processName[16] = "LongNameTest";
    int32_t len = sizeof(processName);

    EXPECT_EQ(-EINVAL, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_002 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and processName param is nullptr.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_003 start";

    char longProcName[20] = "longProcName";
    int64_t longProcNameLen = sizeof(longProcName);
    char *processName = nullptr;
    int32_t len = sizeof(processName);

    EXPECT_EQ(-EINVAL, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_003 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and len param is 0.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_004 start";

    char longProcName[20] = "longProcName";
    int64_t longProcNameLen = sizeof(longProcName);
    char processName[16] = "processName";
    int32_t len = 0;

    EXPECT_EQ(-EINVAL, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_004 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and len param is -1.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_005 start";

    char longProcName[20] = "longProcName";
    int64_t longProcNameLen = sizeof(longProcName);
    char processName[16] = "processName";
    int32_t len = -1;

    EXPECT_EQ(-EINVAL, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_005 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and processName length < 16.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_006 start";

    char longProcName[20] = "longProcName";
    int64_t longProcNameLen = sizeof(longProcName);
    char processName[16] = "processName";
    int32_t len = sizeof(processName);

    EXPECT_EQ(0, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_006 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: SetProcessName
 * FunctionPoints: set process name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function SetProcessName and processName length > 16.
 */
HWTEST_F(AppSpawnServerOverrideTest, App_Spawn_Server_Override_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_007 start";

    char longProcName[20] = "longProcName";
    int64_t longProcNameLen = strlen(longProcName);
    char processName[32] = "processName0123456789";
    int32_t len = sizeof(processName);

    EXPECT_EQ(0, appSpawnServer_->SetProcessName(longProcName, longProcNameLen, processName, len));

    GTEST_LOG_(INFO) << "App_Spawn_Server_Override_007 end";
}