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

#include "mock_server_socket.h"
#include "appspawn_msg_peer.h"
#include "server_socket.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

class AppSpawnMsgPeerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AppSpawnMsgPeerTest::SetUpTestCase()
{}

void AppSpawnMsgPeerTest::TearDownTestCase()
{}

void AppSpawnMsgPeerTest::SetUp()
{}

void AppSpawnMsgPeerTest::TearDown()
{}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: GetConnectFd & GetMsg
 * FunctionPoints: simple function branch coverage
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify if new AppSpawnMsgPeer with the invalid params and the function GetConnectFd and GetMsg will
 * return default value.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_001 start";

    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("ServerSocket");
    int32_t connectFd = -1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(serverSocket, connectFd);

    EXPECT_EQ(-1, appSpawnMsgPeer->GetConnectFd());
    EXPECT_EQ(nullptr, appSpawnMsgPeer->GetMsg());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_001 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: MsgPeer
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function MsgPeer can check the invalid connect fd.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_002 start";

    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("ServerSocket");
    int32_t connectFd = -1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(serverSocket, connectFd);

    EXPECT_EQ(-1, appSpawnMsgPeer->MsgPeer());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_002 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: MsgPeer
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function MsgPeer can check the invalid server socket.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_003 start";

    std::shared_ptr<ServerSocket> serverSocket = nullptr;
    int32_t connectFd = 1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(serverSocket, connectFd);

    EXPECT_EQ(-1, appSpawnMsgPeer->MsgPeer());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_003 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: Response
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function Response can check the invalid connect fd.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_004 start";

    std::shared_ptr<ServerSocket> serverSocket = std::make_shared<ServerSocket>("ServerSocket");
    int32_t connectFd = -1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(serverSocket, connectFd);

    EXPECT_EQ(-1, appSpawnMsgPeer->Response(1));

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_004 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: Response
 * FunctionPoints: check params
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function Response can check the invalid server socket.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_005 start";

    std::shared_ptr<ServerSocket> serverSocket = nullptr;
    int32_t connectFd = 1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(serverSocket, connectFd);

    EXPECT_EQ(-1, appSpawnMsgPeer->Response(1));

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_005 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: MsgPeer
 * FunctionPoints: read socket message
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function MsgPeer can check the the invalid message read.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_006 start";

    std::shared_ptr<MockServerSocket> mockServerSocket = std::make_shared<MockServerSocket>("MockServerSocket");
    int32_t connectFd = 1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(mockServerSocket, connectFd);

    EXPECT_CALL(*mockServerSocket, ReadSocketMessage(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*mockServerSocket, CloseConnection(_)).WillOnce(Return());
    EXPECT_EQ(-1, appSpawnMsgPeer->MsgPeer());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_006 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: MsgPeer
 * FunctionPoints: read socket message
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function MsgPeer can check the the too long message read.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_007 start";

    std::shared_ptr<MockServerSocket> mockServerSocket = std::make_shared<MockServerSocket>("MockServerSocket");
    int32_t connectFd = 1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(mockServerSocket, connectFd);

    EXPECT_CALL(*mockServerSocket, ReadSocketMessage(_, _, _)).WillOnce(Return(sizeof(ClientSocket::AppProperty) + 1));
    EXPECT_CALL(*mockServerSocket, CloseConnection(_)).WillOnce(Return());
    EXPECT_EQ(-1, appSpawnMsgPeer->MsgPeer());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_007 end";
}

/*
 * Feature: AppSpawn
 * Function: AppSpawnMsgPeer
 * SubFunction: MsgPeer
 * FunctionPoints: read socket message
 * EnvConditions: mobile that can run ohos test framework
 * CaseDescription: Verify the function MsgPeer read socket message.
 */
HWTEST(AppSpawnMsgPeerTest, App_Spawn_Msg_Peer_008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_008 start";

    std::shared_ptr<MockServerSocket> mockServerSocket = std::make_shared<MockServerSocket>("MockServerSocket");
    int32_t connectFd = 1;
    std::unique_ptr<AppSpawnMsgPeer> appSpawnMsgPeer = std::make_unique<AppSpawnMsgPeer>(mockServerSocket, connectFd);

    EXPECT_EQ(nullptr, appSpawnMsgPeer->GetMsg());
    EXPECT_CALL(*mockServerSocket, ReadSocketMessage(_, _, _))
        .WillOnce(Invoke(mockServerSocket.get(), &MockServerSocket::ReadImplValid));
    EXPECT_CALL(*mockServerSocket, CloseConnection(_)).WillOnce(Return());
    EXPECT_EQ(0, appSpawnMsgPeer->MsgPeer());
    EXPECT_NE(nullptr, appSpawnMsgPeer->GetMsg());

    GTEST_LOG_(INFO) << "App_Spawn_Msg_Peer_008 end";
}
