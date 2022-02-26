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
#include <cstring>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "appspawn_socket.h"
#undef private
#undef protected

#include "securec.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

class AppSpawnSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AppSpawnSocketTest::SetUpTestCase()
{}

void AppSpawnSocketTest::TearDownTestCase()
{}

void AppSpawnSocketTest::SetUp()
{}

void AppSpawnSocketTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: CreateSocket & CloseSocket
 * FunctionPoints: create socket and close socket.
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify if CreateSocket success then can close the socket.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_001 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);

    EXPECT_EQ(-1, appSpawnSocket->GetSocketFd());
    auto socketFd = appSpawnSocket->CreateSocket();
    EXPECT_LE(0, socketFd);
    appSpawnSocket->CloseSocket(socketFd);
    EXPECT_EQ(-1, socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_001 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: CloseSocket
 * FunctionPoints: close the invalid socket fd.
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function CloseSocket don't close the socket which socket fd is invalid.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_002 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);

    int32_t socketFd = -2;
    appSpawnSocket->CloseSocket(socketFd);

    EXPECT_EQ(-2, socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_002 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: PackSocketAddr
 * FunctionPoints: check the invalid socket name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function PackSocketAddr can check the empty socket name.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_003 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("");
    EXPECT_TRUE(appSpawnSocket);

    EXPECT_EQ(-1, appSpawnSocket->PackSocketAddr());

    GTEST_LOG_(INFO) << "App_Spawn_Socket_003 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: PackSocketAddr
 * FunctionPoints: check the invalid socket name
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function PackSocketAddr can check too long socket name.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_004 start";

    std::string invalidSocketName =
        "InvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalid"
        "InvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalid";
    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>(invalidSocketName.c_str());
    EXPECT_TRUE(appSpawnSocket);

    EXPECT_EQ(-1, appSpawnSocket->PackSocketAddr());

    GTEST_LOG_(INFO) << "App_Spawn_Socket_004 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: PackSocketAddr
 * FunctionPoints: pack socket address
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function PackSocketAddr can pack the socket address with valid socket name.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_005 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);

    EXPECT_EQ(0, appSpawnSocket->PackSocketAddr());

    GTEST_LOG_(INFO) << "App_Spawn_Socket_005 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the invalid socket fd.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_006 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);

    int32_t socketFd = -1;
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(10);
    EXPECT_TRUE(buff);
    int32_t len = 10;

    EXPECT_EQ(-1, appSpawnSocket->ReadSocketMessage(socketFd, buff.get(), len));

    GTEST_LOG_(INFO) << "App_Spawn_Socket_006 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the invalid buffer pointer.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_007 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::unique_ptr<uint8_t[]> buff = nullptr;
    int32_t len = 10;

    EXPECT_EQ(-1, appSpawnSocket->ReadSocketMessage(socketFd, buff.get(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_007 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the buffer length is 0.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_008 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(10);
    EXPECT_TRUE(buff);
    int32_t len = 0;

    EXPECT_EQ(-1, appSpawnSocket->ReadSocketMessage(socketFd, buff.get(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_008 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the buffer length < 0.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_009 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(10);
    EXPECT_TRUE(buff);
    int32_t len = -1;

    EXPECT_EQ(-1, appSpawnSocket->ReadSocketMessage(socketFd, buff.get(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_009 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: normal read data
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can read the normal message.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_010 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    std::string content = "hiworld";
    int32_t len = content.length();
    EXPECT_TRUE(len);
    std::unique_ptr<int8_t[]> buff = std::make_unique<int8_t[]>(len);
    EXPECT_TRUE(buff);
    int32_t fd[2] = {0, 0};

    if (pipe(fd) == -1) {
        GTEST_LOG_(WARNING) << "create pipe fail";
        return;
    }
    write(fd[1], content.c_str(), len);

    EXPECT_EQ(len, appSpawnSocket->ReadSocketMessage(fd[0], buff.get(), len));
    EXPECT_EQ(0, strncmp(content.c_str(), (const char *)(buff.get()), len));

    // close pipe
    close(fd[0]);
    close(fd[1]);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_010 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the invalid socket fd.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_011 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    int32_t socketFd = -1;
    std::string buff = "hiworld";

    EXPECT_EQ(-1, appSpawnSocket->WriteSocketMessage(socketFd, buff.c_str(), buff.length()));

    GTEST_LOG_(INFO) << "App_Spawn_Socket_011 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the invalid buffer pointer.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_012 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::unique_ptr<uint8_t[]> buff = nullptr;
    int32_t len = 10;

    EXPECT_EQ(-1, appSpawnSocket->WriteSocketMessage(socketFd, buff.get(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_012 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the buffer length is 0.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_013, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_013 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::string buff = "hiworld";
    int32_t len = 0;

    EXPECT_EQ(-1, appSpawnSocket->WriteSocketMessage(socketFd, buff.c_str(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_013 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function ReadSocketMessage can check the buffer length < 0.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_014, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_014 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    auto socketFd = appSpawnSocket->CreateSocket();
    std::string buff = "hiworld";
    int32_t len = -1;

    EXPECT_EQ(-1, appSpawnSocket->WriteSocketMessage(socketFd, buff.c_str(), len));
    appSpawnSocket->CloseSocket(socketFd);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_014 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: normal write data
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can write the normal message.
 */
HWTEST(AppSpawnSocketTest, App_Spawn_Socket_015, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Socket_015 start";

    std::unique_ptr<AppSpawnSocket> appSpawnSocket = std::make_unique<AppSpawnSocket>("AppSpawnSocketTest");
    EXPECT_TRUE(appSpawnSocket);
    std::string content = "hiworld";
    int32_t len = content.length();
    EXPECT_TRUE(len);
    std::unique_ptr<int8_t[]> buff = std::make_unique<int8_t[]>(len);
    EXPECT_TRUE(buff);
    int32_t fd[2] = {0, 0};

    if (pipe(fd) == -1) {
        GTEST_LOG_(WARNING) << "create pipe fail";
        return;
    }

    EXPECT_EQ(len, appSpawnSocket->WriteSocketMessage(fd[1], content.c_str(), len));
    read(fd[0], buff.get(), len);
    EXPECT_EQ(0, strncmp(content.c_str(), (const char *)(buff.get()), len));

    // close pipe
    close(fd[0]);
    close(fd[1]);

    GTEST_LOG_(INFO) << "App_Spawn_Socket_015 end";
}