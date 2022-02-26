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
#include "client_socket.h"
#undef private
#undef protected

#include "securec.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

class ClientSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ClientSocketTest::SetUpTestCase()
{}

void ClientSocketTest::TearDownTestCase()
{}

void ClientSocketTest::SetUp()
{}

void ClientSocketTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: CreateClient & ConnectSocket
 * FunctionPoints: create client socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify although the client socket created success but don't create the server socket, the connect
 * socket still fail.
 */
HWTEST(ClientSocketTest, Client_Socket_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_001 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");

    EXPECT_EQ(-1, clientSocket->GetSocketFd());
    EXPECT_EQ(0, clientSocket->CreateClient());
    int32_t socketFd = clientSocket->GetSocketFd();
    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_EQ(socketFd, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->ConnectSocket());

    GTEST_LOG_(INFO) << "Client_Socket_001 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: ConnectSocket
 * FunctionPoints: connect socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify connect socket fail when don't create client socket.
 */
HWTEST(ClientSocketTest, Client_Socket_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_002 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");

    EXPECT_EQ(-1, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->ConnectSocket());

    GTEST_LOG_(INFO) << "Client_Socket_002 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: write message
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify write message fail when don't create client socket.
 */
HWTEST(ClientSocketTest, Client_Socket_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_003 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    std::string buff = "hiworld";

    EXPECT_EQ(-1, clientSocket->WriteSocketMessage(buff.c_str(), buff.length()));

    GTEST_LOG_(INFO) << "Client_Socket_003 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the invalid buffer pointer.
 */
HWTEST(ClientSocketTest, Client_Socket_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_004 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    std::unique_ptr<uint8_t[]> buff = nullptr;
    uint32_t len = 10;

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->WriteSocketMessage(buff.get(), len));

    GTEST_LOG_(INFO) << "Client_Socket_004 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the buffer length is 0.
 */
HWTEST(ClientSocketTest, Client_Socket_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_005 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    std::string buff = "hiworld";
    uint32_t len = 0;

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->WriteSocketMessage(buff.c_str(), len));

    GTEST_LOG_(INFO) << "Client_Socket_005 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: WriteSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the buffer length < 0.
 */
HWTEST(ClientSocketTest, Client_Socket_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_006 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    std::string buff = "hiworld";
    uint32_t len = -1;

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->WriteSocketMessage(buff.c_str(), len));

    GTEST_LOG_(INFO) << "Client_Socket_006 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: read message
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify read message fail when don't create client socket.
 */
HWTEST(ClientSocketTest, Client_Socket_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_007 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    int32_t len = 10;
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(len);

    EXPECT_EQ(-1, clientSocket->ReadSocketMessage(buff.get(), len));

    GTEST_LOG_(INFO) << "Client_Socket_007 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the invalid buffer pointer.
 */
HWTEST(ClientSocketTest, Client_Socket_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_008 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    int32_t len = 10;
    std::unique_ptr<uint8_t[]> buff = nullptr;

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->ReadSocketMessage(buff.get(), len));

    GTEST_LOG_(INFO) << "Client_Socket_008 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the buffer length is 0.
 */
HWTEST(ClientSocketTest, Client_Socket_009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_009 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    int32_t len = 0;
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(10);

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->ReadSocketMessage(buff.get(), len));

    GTEST_LOG_(INFO) << "Client_Socket_009 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: ReadSocketMessage
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function WriteSocketMessage can check the buffer length < 0.
 */
HWTEST(ClientSocketTest, Client_Socket_010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_010 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");
    int32_t len = -1;
    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(10);

    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    EXPECT_EQ(-1, clientSocket->ReadSocketMessage(buff.get(), len));

    GTEST_LOG_(INFO) << "Client_Socket_010 end";
}

/*
 * Feature: AppSpawn
 * Function: ClientSocket
 * SubFunction: GetSocketFd
 * FunctionPoints: close the socket
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function CloseClient can close the socket which socket fd has created.
 */
HWTEST(ClientSocketTest, Client_Socket_011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Client_Socket_011 start";

    std::unique_ptr<ClientSocket> clientSocket = std::make_unique<ClientSocket>("ClientSocketTest");

    EXPECT_EQ(-1, clientSocket->GetSocketFd());
    EXPECT_EQ(0, clientSocket->CreateClient());
    EXPECT_LE(0, clientSocket->GetSocketFd());
    clientSocket->CloseClient();
    EXPECT_EQ(-1, clientSocket->GetSocketFd());

    GTEST_LOG_(INFO) << "Client_Socket_011 end";
}
