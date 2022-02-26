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

#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"

#define private public
#include "distributed_uid_allocator.h"
#include "request_remote_permission_command.h"
#include "subject_device_permission_manager.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class RequestRemotePermissionCommandTest : public testing::Test {
public:
    RequestRemotePermissionCommandTest();
    ~RequestRemotePermissionCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<RequestRemotePermissionCommand> GetClass() const;

private:
};

RequestRemotePermissionCommandTest::RequestRemotePermissionCommandTest()
{}
RequestRemotePermissionCommandTest::~RequestRemotePermissionCommandTest()
{}
void RequestRemotePermissionCommandTest::SetUpTestCase()
{
    OHOS::sptr<OHOS::IRemoteObject> bundleObject = NULL;
    OHOS::sptr<OHOS::IRemoteObject> permissionObject = new PermissionManagerService();
    auto sysMgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->AddSystemAbility(Constant::ServiceId::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
    sysMgr->AddSystemAbility(Constant::ServiceId::SUBSYS_SECURITY_PERMISSION_SYS_SERVICE_ID, permissionObject);
}
void RequestRemotePermissionCommandTest::TearDownTestCase()
{}
void RequestRemotePermissionCommandTest::SetUp()
{}
void RequestRemotePermissionCommandTest::TearDown()
{
    DistributedUidAllocator::GetInstance().Clear();
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0001, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);

    class_->SetRequestPermissionInfo(uid, permissions, bundleName, reason);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "RequestRemotePermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "RequestRemotePermissionCommand-requestId");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->requestId_, requestId);
    EXPECT_EQ(class_->uid_, uid);
    EXPECT_EQ(class_->bundleName_, bundleName);
    EXPECT_EQ(class_->reason_, reason);
    EXPECT_EQ(class_->permissions_, permissions);
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0002, Function | MediumTest | Level1)
{
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);

    class_->Prepare();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0003, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::hash<std::string> hashStr;
    std::string key = std::to_string(hashStr(srcDeviceId)) + "_" + std::to_string(uid);

    DistributedUidEntity entity;
    entity.deviceId = srcDeviceId;
    entity.uid = uid;
    entity.distributedUid = DistributedUidAllocator::GetInstance().FindCurrentIdleDuidLocked();

    DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
        std::pair<std::string, DistributedUidEntity>(key, entity));

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);
    class_->SetRequestPermissionInfo(uid, permissions, bundleName, reason);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0004, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);
    class_->SetRequestPermissionInfo(uid, permissions, bundleName, reason);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0005, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);
    class_->SetRequestPermissionInfo(uid, permissions, bundleName, reason);

    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0006, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::vector<std::string> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        permissions.push_back("permission" + std::to_string(i));
    }
    std::string bundleName = "bundleName";
    std::string reason = "reason";

    std::shared_ptr<RequestRemotePermissionCommand> class_ =
        std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);
    class_->SetRequestPermissionInfo(uid, permissions, bundleName, reason);

    class_->Prepare();
    std::string result = class_->ToJsonPayload();
    std::string json =
        "{\"bundleName\":\"bundleName\",\"commandName\":\"RequestRemotePermissionCommand\",\"dstDeviceId\":"
        "\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":\"success\",\"permissions\":[\"permission1\","
        "\"permission2\",\"permission3\",\"permission4\",\"permission5\"],\"reason\":\"reason\",\"requestId\":"
        "\"requestId\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uniqueId\":"
        "\"RequestRemotePermissionCommand-requestId\"}";
    EXPECT_EQ(result, json);
}

HWTEST_F(RequestRemotePermissionCommandTest, RequestRemotePermissionCommandTest_0007, Function | MediumTest | Level1)
{
    std::string json =
        "{\"bundleName\":\"bundleName\",\"commandName\":\"RequestRemotePermissionCommand\",\"dstDeviceId\":"
        "\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":\"success\",\"permissions\":[\"permission1\","
        "\"permission2\",\"permission3\",\"permission4\",\"permission5\"],\"reason\":\"reason\",\"requestId\":"
        "\"requestId\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uniqueId\":"
        "\"RequestRemotePermissionCommand-requestId\"}";

    std::shared_ptr<BaseRemoteCommand> class_ = std::make_shared<RequestRemotePermissionCommand>(json);
    class_->Execute();
    EXPECT_EQ(class_->remoteProtocol_.commandName, "RequestRemotePermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "1004");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "RequestRemotePermissionCommand-requestId");
}