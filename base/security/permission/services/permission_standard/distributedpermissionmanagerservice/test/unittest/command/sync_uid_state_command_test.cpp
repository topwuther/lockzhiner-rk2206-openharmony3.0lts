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
#include "distributed_uid_allocator.h"
#include "sync_uid_state_command.h"
#define private public

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class SyncUidStateCommandTest : public testing::Test {
public:
    SyncUidStateCommandTest();
    ~SyncUidStateCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<SyncUidStateCommand> GetClass() const;

private:
};

SyncUidStateCommandTest::SyncUidStateCommandTest()
{}
SyncUidStateCommandTest::~SyncUidStateCommandTest()
{}
void SyncUidStateCommandTest::SetUpTestCase()
{}
void SyncUidStateCommandTest::TearDownTestCase()
{}
void SyncUidStateCommandTest::SetUp()
{}
void SyncUidStateCommandTest::TearDown()
{}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0001, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidStateCommand> class_ = std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "SyncUidStateCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "SyncUidStateCommand-1");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0002, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidStateCommand> class_ = std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0003, Function | MediumTest | Level1)
{
    int32_t uid = 126100001;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidStateCommand> class_ = std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0004, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidStateCommand> class_ = std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0005, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidStateCommand> class_ = std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    std::string result = class_->ToJsonPayload();
    std::string json =
        "{\"commandName\":\"SyncUidStateCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":"
        "\"success\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uniqueId\":\"SyncUidStateCommand-1\"}";
    EXPECT_EQ(result, json);
}

HWTEST_F(SyncUidStateCommandTest, SyncUidStateCommandTest_0006, Function | MediumTest | Level1)
{
    std::string json =
        "{\"commandName\":\"SyncUidStateCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":"
        "\"success\",\"requestVersion\":1,\"responseDeviceId\":\"999\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uniqueId\":\"SyncUidStateCommand-1\"}";

    std::shared_ptr<BaseRemoteCommand> class_ = std::make_shared<SyncUidStateCommand>(json);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "SyncUidStateCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "999");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "SyncUidStateCommand-1");
}