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
#include "mock_bundle_mgr.h"
#include "constant_mock.h"
#include "mock_permission_mgr.h"
#include "base_remote_command.h"
#include "get_uid_permission_command.h"

#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class GetUidPermissionCommandTest : public testing::Test {
public:
    GetUidPermissionCommandTest();
    ~GetUidPermissionCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
};

GetUidPermissionCommandTest::GetUidPermissionCommandTest()
{}
GetUidPermissionCommandTest::~GetUidPermissionCommandTest()
{}
void GetUidPermissionCommandTest::SetUpTestCase()
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
void GetUidPermissionCommandTest::TearDownTestCase()
{}
void GetUidPermissionCommandTest::SetUp()
{}
void GetUidPermissionCommandTest::TearDown()
{}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0001, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "GetUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "GetUidPermissionCommand-1");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0002, Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Prepare();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0003, Function | MediumTest | Level1)
{
    int32_t uid = 12600000;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0004, Function | MediumTest | Level1)
{
    int32_t uid = 12600000;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0005, Function | MediumTest | Level1)
{
    int32_t uid = -1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "failed to get permissions");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0006, Function | MediumTest | Level1)
{
    int32_t uid = 1024;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "permissions exceed MAX_UID_PERMISSIONS_COUNT");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0008, Function | MediumTest | Level1)
{
    int32_t uid = 12610001;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();
    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0009, Function | MediumTest | Level1)
{
    int32_t uid = 12610001;
    std::string srcDeviceId = "12345678901234567890123456789012345678901234567890123456789012345";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();
    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "execute command failed");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0010, Function | MediumTest | Level1)
{
    int32_t uid = 12600000;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();
    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, -2);
    EXPECT_EQ(class_->remoteProtocol_.message, "execute command failed");
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0011, Function | MediumTest | Level1)
{
    int32_t uid = 12600000;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";

    std::shared_ptr<GetUidPermissionCommand> class_ =
        std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");

    std::string result = class_->ToJsonPayload();

    std::string json =
        "{\"commandName\":\"GetUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\","
        "\"message\":\"success\",\"requestVersion\":1,\"responseDeviceId\":\"1004\",\"responseVersion\":1,"
        "\"srcDeviceId\":\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":12600000,\"uidPermission\":{"
        "\"DEFAULT_SIZE\":10,\"MIN_UID_PACKAGES_NUM\":1,\"appAttribute\":2,\"bundles\":[{\"appId\":\"bundleInfo."
        "appId\",\"bundleLabel\":\"bundleInfo.label\",\"name\":\"bundleInfo.name\",\"permissions\":[{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions1\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions2\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions3\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions4\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions5\",\"status\":1,\"type\":\"app\"}],\"sign\":[{"
        "\"sha256\":\"applicationInfo.signatureKey\"}]}],\"remoteSensitivePermission\":null,\"uid\":12600000,"
        "\"uidState\":0},\"uniqueId\":\"GetUidPermissionCommand-12600000\"}";
    EXPECT_EQ(result, json);
}

HWTEST_F(GetUidPermissionCommandTest, GetUidPermissionCommandTest_0012, Function | MediumTest | Level1)
{
    std::string json =
        "{\"commandName\":\"GetUidPermissionCommand\",\"dstDeviceId\":\"dstDeviceId\",\"dstDeviceLevel\":\"\","
        "\"message\":\"success\",\"requestVersion\":1,\"responseDeviceId\":\"1004\",\"responseVersion\":1,"
        "\"srcDeviceId\":\"srcDeviceId\",\"srcDeviceLevel\":\"\",\"statusCode\":0,\"uid\":12600000,\"uidPermission\":{"
        "\"DEFAULT_SIZE\":10,\"MIN_UID_PACKAGES_NUM\":1,\"appAttribute\":2,\"bundles\":[{\"appId\":\"bundleInfo."
        "appId\",\"bundleLabel\":\"bundleInfo.label\",\"name\":\"bundleInfo.name\",\"permissions\":[{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions1\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions2\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions3\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions4\",\"status\":1,\"type\":\"app\"},{\"defInfo\":0,"
        "\"grantMode\":0,\"level\":0,\"name\":\"reqPermissions5\",\"status\":1,\"type\":\"app\"}],\"sign\":[{"
        "\"sha256\":\"applicationInfo.signatureKey\"}]}],\"remoteSensitivePermission\":null,\"uid\":12600000,"
        "\"uidState\":0},\"uniqueId\":\"GetUidPermissionCommand-12600000\"}";

    std::shared_ptr<GetUidPermissionCommand> class_ = std::make_shared<GetUidPermissionCommand>(json);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "GetUidPermissionCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "1004");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "GetUidPermissionCommand-12600000");
}