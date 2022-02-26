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
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"
#include "base_remote_command.h"
#include "sync_uid_permission_command.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class SyncUidPermissionCommandTest : public testing::Test {
public:
    SyncUidPermissionCommandTest();
    ~SyncUidPermissionCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
};

SyncUidPermissionCommandTest::SyncUidPermissionCommandTest()
{}
SyncUidPermissionCommandTest::~SyncUidPermissionCommandTest()
{}
void SyncUidPermissionCommandTest::SetUpTestCase()
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
void SyncUidPermissionCommandTest::TearDownTestCase()
{}
void SyncUidPermissionCommandTest::SetUp()
{}
void SyncUidPermissionCommandTest::TearDown()
{}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0001, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidPermissionCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "SyncUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "SyncUidPermissionCommand-1");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0002, Function | MediumTest | Level2)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidPermissionCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0003, Function | MediumTest | Level1)
{
    int32_t uid = 1024;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "permissions exceed MAX_UID_PERMISSIONS_COUNT");
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0004, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0005, Function | MediumTest | Level1)
{
    int32_t uid = 2;
    std::string srcDeviceId = "012345678900123456789001234567890012345678900123456789001234567890012345";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "execute command failed");
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0006, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<BaseRemoteCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0007, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<SyncUidPermissionCommand> class_ =
        std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");

    std::string result = class_->ToJsonPayload();
    std::string json =
        "{\"commandName\":\"SyncUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\","
        "\"message\":\"success\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uidPermission\":{\"DEFAULT_SIZE\":10,"
        "\"MIN_UID_PACKAGES_NUM\":1,\"appAttribute\":2,\"bundles\":[{\"appId\":\"bundleInfo.appId\",\"bundleLabel\":"
        "\"bundleInfo.label\",\"name\":\"bundleInfo.name\",\"permissions\":[{\"defInfo\":0,\"grantMode\":0,\"level\":"
        "0,\"name\":\"reqPermissions1\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions2\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions3\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions4\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions5\",\"status\":1,\"type\":\"app\"}],\"sign\":[{\"sha256\":\"applicationInfo."
        "signatureKey\"}]}],\"remoteSensitivePermission\":null,\"uid\":1,\"uidState\":0},\"uniqueId\":"
        "\"SyncUidPermissionCommand-1\"}";

    EXPECT_EQ(result, json);
}

HWTEST_F(SyncUidPermissionCommandTest, SyncUidPermissionCommandTest_0008, Function | MediumTest | Level1)
{
    std::string json =
        "{\"commandName\":\"SyncUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\","
        "\"message\":\"success\",\"requestVersion\":1,\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":"
        "\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":1,\"uidPermission\":{\"DEFAULT_SIZE\":10,"
        "\"MIN_UID_PACKAGES_NUM\":1,\"appAttribute\":2,\"bundles\":[{\"appId\":\"bundleInfo.appId\",\"bundleLabel\":"
        "\"bundleInfo.label\",\"name\":\"bundleInfo.name\",\"permissions\":[{\"defInfo\":0,\"grantMode\":0,\"level\":"
        "0,\"name\":\"reqPermissions1\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions2\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions3\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions4\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,\"grantMode\":0,\"level\":0,"
        "\"name\":\"reqPermissions5\",\"status\":1,\"type\":\"app\"}],\"sign\":[{\"sha256\":\"applicationInfo."
        "signatureKey\"}]}],\"remoteSensitivePermission\":null,\"uid\":1,\"uidState\":0},\"uniqueId\":"
        "\"SyncUidPermissionCommand-1\"}";

    std::shared_ptr<SyncUidPermissionCommand> class_ = std::make_shared<SyncUidPermissionCommand>(json);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.commandName, "SyncUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "1004");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "SyncUidPermissionCommand-1");
}