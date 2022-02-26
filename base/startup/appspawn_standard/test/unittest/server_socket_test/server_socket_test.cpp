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

#include <memory>
#include <gtest/gtest.h>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "server_socket.h"
#undef private
#undef protected

#include "securec.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

class ServerSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ServerSocketTest::SetUpTestCase()
{}

void ServerSocketTest::TearDownTestCase()
{}

void ServerSocketTest::SetUp()
{}

void ServerSocketTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: RegisterServerSocket
 * FunctionPoints: create server socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function RegisterServerSocket can called twice, but the socket fd is same.
 */
HWTEST(ServerSocketTest, Server_Socket_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_001 start";

    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>("ServerSocketTest");

    EXPECT_EQ(-1, serverSocket->GetSocketFd());
    EXPECT_EQ(0, serverSocket->RegisterServerSocket());
    int32_t socketFd = serverSocket->GetSocketFd();
    EXPECT_LE(0, socketFd);
    EXPECT_EQ(0, serverSocket->RegisterServerSocket());
    EXPECT_EQ(socketFd, serverSocket->GetSocketFd());

    GTEST_LOG_(INFO) << "Server_Socket_001 end";
}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: RegisterServerSocket
 * FunctionPoints: create server socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function RegisterServerSocket create server socket fail with the empty socket name.
 */
HWTEST(ServerSocketTest, Server_Socket_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_002 start";

    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>("");
    EXPECT_EQ(-1, serverSocket->RegisterServerSocket());

    GTEST_LOG_(INFO) << "Server_Socket_002 end";
}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: WaitForConnection
 * FunctionPoints: accept socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WaitForConnection accept fail when don't create server socket.
 */
HWTEST(ServerSocketTest, Server_Socket_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_003 start";

    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>("ServerSocketTest");
    EXPECT_EQ(-1, serverSocket->WaitForConnection(0));

    GTEST_LOG_(INFO) << "Server_Socket_003 end";
}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: SaveConnection & VerifyConnection & CloseConnection
 * FunctionPoints: connect fd
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the connect fd which be saved or not saved, then close the saved connect fd and verify them.
 */
HWTEST(ServerSocketTest, Server_Socket_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_004 start";

    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>("ServerSocketTest");
    int32_t connectFd1 = 111;
    int32_t connectFd2 = 222;
    int32_t connectFd3 = 333;

    serverSocket->SaveConnection(connectFd1);
    serverSocket->SaveConnection(connectFd2);
    EXPECT_EQ(0, serverSocket->VerifyConnection(connectFd1));
    EXPECT_EQ(0, serverSocket->VerifyConnection(connectFd2));
    EXPECT_EQ(-1, serverSocket->VerifyConnection(connectFd3));
    serverSocket->CloseConnection(connectFd2);
    EXPECT_EQ(-1, serverSocket->VerifyConnection(connectFd2));

    GTEST_LOG_(INFO) << "Server_Socket_004 end";
}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: CloseServerMonitor
 * FunctionPoints: close the server socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function CloseServerMonitor which can close the server socket which has created
 * successfully.
 */
HWTEST(ServerSocketTest, Server_Socket_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_005 start";

    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>("ServerSocketTest");

    EXPECT_EQ(0, serverSocket->RegisterServerSocket());
    EXPECT_LE(0, serverSocket->GetSocketFd());
    serverSocket->CloseServerMonitor();
    EXPECT_EQ(-1, serverSocket->GetSocketFd());

    GTEST_LOG_(INFO) << "Server_Socket_005 end";
}

/*
 * Feature: AppSpawn
 * Function: ServerSocket
 * SubFunction: RegisterServerSocket
 * FunctionPoints: bind socket fail
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function RegisterServerSocket BindSocket fail and close the socket fd.
 */
HWTEST(ServerSocketTest, Server_Socket_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Server_Socket_006 start";

    std::string invalidSocketName =
        "InvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalid"
        "InvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalidInvalid";
    std::unique_ptr<ServerSocket> serverSocket = std::make_unique<ServerSocket>(invalidSocketName.c_str());

    EXPECT_EQ(-1, serverSocket->RegisterServerSocket());
    EXPECT_EQ(-1, serverSocket->GetSocketFd());

    GTEST_LOG_(INFO) << "Server_Socket_006 end";
}
