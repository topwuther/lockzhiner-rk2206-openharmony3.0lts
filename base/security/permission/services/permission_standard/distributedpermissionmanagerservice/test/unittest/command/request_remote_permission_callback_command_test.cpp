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
#include "base_remote_command.h"

#include "gtest/gtest.h"
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"

#define private public
#include "distributed_uid_allocator.h"
#include "request_remote_permission_callback_command.h"
#include "subject_device_permission_manager.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

class RequestRemotePermissionCallbackCommandTest : public testing::Test {
public:
    RequestRemotePermissionCallbackCommandTest();
    ~RequestRemotePermissionCallbackCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<RequestRemotePermissionCallbackCommand> GetClass() const;

private:
};

RequestRemotePermissionCallbackCommandTest::RequestRemotePermissionCallbackCommandTest()
{}
RequestRemotePermissionCallbackCommandTest::~RequestRemotePermissionCallbackCommandTest()
{}
void RequestRemotePermissionCallbackCommandTest::SetUpTestCase()
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
void RequestRemotePermissionCallbackCommandTest::TearDownTestCase()
{}
void RequestRemotePermissionCallbackCommandTest::SetUp()
{}
void RequestRemotePermissionCallbackCommandTest::TearDown()
{
    DistributedUidAllocator::GetInstance().Clear();
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0001,
    Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::string bundleName = "bundleName";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(srcDeviceId, dstDeviceId, requestId, uid, bundleName);

    EXPECT_EQ(class_->remoteProtocol_.commandName, "RequestRemotePermissionCallbackCommand");
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "RequestRemotePermissionCallbackCommand-1");
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, srcDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, dstDeviceId);
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0002,
    Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::string bundleName = "bundleName";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(dstDeviceId, dstDeviceId, requestId, uid, bundleName);

    std::hash<std::string> hashStr;
    std::string key = std::to_string(hashStr(dstDeviceId)) + "_" + std::to_string(uid);

    DistributedUidEntity entity;
    entity.deviceId = dstDeviceId;
    entity.uid = uid;
    entity.distributedUid = DistributedUidAllocator::GetInstance().FindCurrentIdleDuidLocked();

    DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
        std::pair<std::string, DistributedUidEntity>(key, entity));

    UidBundleBo info;
    std::set<std::string> remoteSensitivePermission;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        remoteSensitivePermission.insert("permission.permissionName" + std::to_string(i));
    }
    info.remoteSensitivePermission = remoteSensitivePermission;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int32_t, UidBundleBo>(entity.distributedUid, info));

    class_->Prepare();
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0003,
    Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::string bundleName = "bundleName";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(srcDeviceId, dstDeviceId, requestId, uid, bundleName);

    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0004,
    Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::string bundleName = "bundleName";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(srcDeviceId, dstDeviceId, requestId, uid, bundleName);

    class_->Finish();

    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0005,
    Function | MediumTest | Level1)
{
    int32_t uid = 1;
    std::string srcDeviceId = "srcDeviceId";
    std::string dstDeviceId = "dstDeviceId";
    std::string requestId = "requestId";
    std::string bundleName = "bundleName";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(srcDeviceId, dstDeviceId, requestId, uid, bundleName);

    std::hash<std::string> hashStr;
    std::string key = std::to_string(hashStr(dstDeviceId)) + "_" + std::to_string(uid);

    DistributedUidEntity entity;
    entity.deviceId = dstDeviceId;
    entity.uid = uid;
    entity.distributedUid = DistributedUidAllocator::GetInstance().FindCurrentIdleDuidLocked();

    DistributedUidAllocator::GetInstance().distributedUidMapByKey_.insert(
        std::pair<std::string, DistributedUidEntity>(key, entity));

    UidBundleBo info;
    std::set<std::string> remoteSensitivePermission;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        remoteSensitivePermission.insert("permission.permissionName" + std::to_string(i));
    }
    info.remoteSensitivePermission = remoteSensitivePermission;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int32_t, UidBundleBo>(entity.distributedUid, info));

    class_->Prepare();
    std::string result = class_->ToJsonPayload();
    std::string json =
        "{\"bundleName\":\"bundleName\",\"commandName\":\"RequestRemotePermissionCallbackCommand\",\"dstDeviceId\":"
        "\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":\"success\",\"objectGrantedResult\":[\"permission."
        "permissionName1\",\"permission.permissionName2\",\"permission.permissionName3\",\"permission."
        "permissionName4\",\"permission.permissionName5\"],\"requestId\":\"requestId\",\"requestVersion\":1,"
        "\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":\"srcDeviceId\",\"srcDeviceLevel\":\"\","
        "\"statusCode\":0,\"uid\":1,\"uniqueId\":\"RequestRemotePermissionCallbackCommand-1\"}";
    EXPECT_EQ(result, json);
}

HWTEST_F(RequestRemotePermissionCallbackCommandTest, RequestRemotePermissionCallbackCommandTest_0006,
    Function | MediumTest | Level1)
{
    std::string json =
        "{\"bundleName\":\"bundleName\",\"commandName\":\"RequestRemotePermissionCallbackCommand\",\"dstDeviceId\":"
        "\"dstDeviceId\",\"dstDeviceLevel\":\"\",\"message\":\"success\",\"objectGrantedResult\":[\"permission."
        "permissionName1\",\"permission.permissionName2\",\"permission.permissionName3\",\"permission."
        "permissionName4\",\"permission.permissionName5\"],\"requestId\":\"requestId\",\"requestVersion\":1,"
        "\"responseDeviceId\":\"\",\"responseVersion\":1,\"srcDeviceId\":\"srcDeviceId\",\"srcDeviceLevel\":\"\","
        "\"statusCode\":0,\"uid\":1,\"uniqueId\":\"RequestRemotePermissionCallbackCommand-1\"}";

    std::shared_ptr<RequestRemotePermissionCallbackCommand> class_ =
        std::make_shared<RequestRemotePermissionCallbackCommand>(json);
    class_->Execute();

    EXPECT_EQ(class_->remoteProtocol_.commandName, "RequestRemotePermissionCallbackCommand");
    EXPECT_EQ(class_->remoteProtocol_.dstDeviceId, "dstDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.message, "success");
    EXPECT_EQ(class_->remoteProtocol_.requestVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.responseDeviceId, "1004");
    EXPECT_EQ(class_->remoteProtocol_.responseVersion, 1);
    EXPECT_EQ(class_->remoteProtocol_.srcDeviceId, "srcDeviceId");
    EXPECT_EQ(class_->remoteProtocol_.statusCode, 0);
    EXPECT_EQ(class_->remoteProtocol_.uniqueId, "RequestRemotePermissionCallbackCommand-1");
}