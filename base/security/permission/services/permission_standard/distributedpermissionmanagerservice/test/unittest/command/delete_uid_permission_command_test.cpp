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
#include <thread>
#include <functional>

#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <iostream>
#include "gtest/gtest.h"
#include "base_remote_command.h"
#define private public
#include "distributed_uid_allocator.h"
#include "delete_uid_permission_command.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class DeleteUidPermissionCommandTest : public testing::Test {
public:
    DeleteUidPermissionCommandTest();
    ~DeleteUidPermissionCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<DeleteUidPermissionCommand> GetClass() const;

private:
};

DeleteUidPermissionCommandTest::DeleteUidPermissionCommandTest()
{}
DeleteUidPermissionCommandTest::~DeleteUidPermissionCommandTest()
{}
void DeleteUidPermissionCommandTest::SetUpTestCase()
{}
void DeleteUidPermissionCommandTest::TearDownTestCase()
{}
void DeleteUidPermissionCommandTest::SetUp()
{}
void DeleteUidPermissionCommandTest::TearDown()
{}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0001, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "DeleteUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "DeleteUidPermissionCommand-1");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0002, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0003, Function | MediumTest | Level1)
{
    int32_t uid = 126100001;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    std::hash<std::string> hashStr;
    std::string key = std::to_string(hashStr(srcDeviceId)) + "_" + std::to_string(uid);

    DistributedUidEntity entity;
    entity.deviceId = srcDeviceId;
    entity.uid = 126100001;
    entity.distributedUid = DistributedUidAllocator::GetInstance().FindCurrentIdleDuidLocked();

    DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
        std::pair<std::string, DistributedUidEntity>(key, entity));

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0004, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "01234567890123456789012345678901234567890123456789012345678912345";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "execute command failed");
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0005, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0006, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<DeleteUidPermissionCommand> class_ =
        std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    std::string result = class_->ToJsonPayload();
    std::string json =
        "{\"commandName\":\"DeleteUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\","
        "\"message\":\"success\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uniqueId\":\"DeleteUidPermissionCommand-"
        "1\"}";

    EXPECT_EQ(result, json);
}

HWTEST_F(DeleteUidPermissionCommandTest, DeleteUidPermissionCommandTest_0007, Function | MediumTest | Level1)
{
    std::string json =
        "{\"commandName\":\"DeleteUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"message\":\"\","
        "\"requestVersion\":0,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":\"srcDeviceId\","
        "\"statusCode\":0,\"uid\":1,\"uniqueId\":\"DeleteUidPermissionCommand-1\"}";

    std::shared_ptr<BaseRemoteCommand> class_ = std::make_shared<DeleteUidPermissionCommand>(json);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "DeleteUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 0);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "DeleteUidPermissionCommand-1");
}