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

#include "mock_server_socket.h"

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "appspawn_server.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

static constexpr int TEST_WAIT_TIME = 50 * 1000;  // 50 ms

class AppSpawnServerMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
protected:
    std::unique_ptr<AppSpawnServer> appSpawnServer_ = nullptr;
    std::shared_ptr<MockServerSocket> mockServerSocket_ = nullptr;
};

void AppSpawnServerMockTest::SetUpTestCase()
{}

void AppSpawnServerMockTest::TearDownTestCase()
{}

void AppSpawnServerMockTest::SetUp()
{
    if (mockServerSocket_ == nullptr) {
        mockServerSocket_ = std::make_shared<MockServerSocket>("MockServerSocket");
    }

    if (appSpawnServer_ == nullptr) {
        appSpawnServer_ = std::make_unique<AppSpawnServer>("AppSpawnServerMockTest");
        appSpawnServer_->SetServerSocket(mockServerSocket_);
    }
}

void AppSpawnServerMockTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: ServerMain
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify if the socket name is empty, the function ServerMain start fail.
 */
HWTEST_F(AppSpawnServerMockTest, App_Spawn_Server_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_001 start";

    std::unique_ptr<AppSpawnServer> appSpawnServer = std::make_unique<AppSpawnServer>("");
    char argv[20] = "LongNameTest";
    EXPECT_EQ(false, appSpawnServer->ServerMain(argv, sizeof(argv)));

    GTEST_LOG_(INFO) << "App_Spawn_Server_001 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: ServerMain
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ServerMain can check the gidcount > max.
 */
HWTEST_F(AppSpawnServerMockTest, App_Spawn_Server_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_002 start";

    char argv[20] = "LongNameTest";

    testing::Mock::AllowLeak(mockServerSocket_.get());
    EXPECT_CALL(*mockServerSocket_, RegisterServerSocket()).WillOnce(Return(0));
    EXPECT_CALL(*mockServerSocket_, WaitForConnection()).WillRepeatedly(Return(1));
    EXPECT_CALL(*mockServerSocket_, SaveConnection(_)).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, ReadSocketMessage(_, _, _))
        .WillRepeatedly(Invoke(mockServerSocket_.get(), &MockServerSocket::ReadImplGidCountMax));
    EXPECT_CALL(*mockServerSocket_, CloseConnection(_)).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, CloseServerMonitor()).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, WriteSocketMessage(_, _, _)).WillRepeatedly(Return(sizeof(pid_t)));

    auto func = [&]() {
        // wait ServerMain unit test case
        usleep(TEST_WAIT_TIME);
        appSpawnServer_->SetRunning(false);
    };

    std::thread(func).detach();
    EXPECT_EQ(false, appSpawnServer_->ServerMain(argv, sizeof(argv)));

    // wait release
    usleep(TEST_WAIT_TIME);

    GTEST_LOG_(INFO) << "App_Spawn_Server_002 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnServer
 * SubFunction: ServerMain
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ServerMain can check the process name is empty.
 */
HWTEST_F(AppSpawnServerMockTest, App_Spawn_Server_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Server_003 start";

    char argv[20] = "LongNameTest";

    testing::Mock::AllowLeak(mockServerSocket_.get());
    EXPECT_CALL(*mockServerSocket_, RegisterServerSocket()).WillOnce(Return(0));
    EXPECT_CALL(*mockServerSocket_, WaitForConnection()).WillRepeatedly(Return(1));
    EXPECT_CALL(*mockServerSocket_, SaveConnection(_)).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, ReadSocketMessage(_, _, _))
        .WillRepeatedly(Invoke(mockServerSocket_.get(), &MockServerSocket::ReadImplProcessName));
    EXPECT_CALL(*mockServerSocket_, CloseConnection(_)).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, CloseServerMonitor()).WillRepeatedly(Return());
    EXPECT_CALL(*mockServerSocket_, WriteSocketMessage(_, _, _)).WillRepeatedly(Return(sizeof(pid_t)));

    auto func = [=]() {
        // wait ServerMain unit test case
        usleep(TEST_WAIT_TIME);
        appSpawnServer_->SetRunning(false);
    };

    std::thread(func).detach();
    EXPECT_EQ(false, appSpawnServer_->ServerMain(argv, sizeof(argv)));

    // wait release
    usleep(TEST_WAIT_TIME);

    GTEST_LOG_(INFO) << "App_Spawn_Server_003 end";
}
