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

#define private public
#include <cstdio>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"
#include "distributed_permission_manager_service.h"
#include "on_using_permission_reminder_stub.h"
#include "on_using_permission_reminder_proxy.h"
#include "permission_reminder_info.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Security::Permission;
using namespace OHOS;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";

static const int32_t uid_normal_1_ = 12612345;
static const int32_t uid_normal_2_ = 12512345;
static const int32_t uid_normal_3_ = 12600000;
static const int32_t uid_normal_4_ = 12500000;

} // namespace OHOS

class PermissionRemindTest : public testing::Test {
public:
    std::shared_ptr<DistributedPermissionManagerService> dpms_;

public:
    std::string permName = Constant::CAMERA;
    int32_t pid = 101;
    int32_t uid = uid_normal_1_;
    std::string deviceId = "ohos.deviceId.test";
    std::string deviceName = "ohos.deviceLabel.test";

    static void SetUpTestCase()
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

    static void TearDownTestCase()
    {
        cout << "TearDownTestCase()" << endl;
    }
    void SetUp()
    {
        dpms_ = OHOS::DelayedSingleton<DistributedPermissionManagerService>::GetInstance();
        dpms_->OnStart();
        DeviceInfoRepository::GetInstance().SaveDeviceInfo(deviceId, deviceId, deviceId, deviceName, "");

        BundlePermissionsDto bundles;
        bundles.name = "bundles.name";
        bundles.bundleLabel = "bundles.bundleLabel";
        SignDto signDto;
        signDto.sha256 = "bundles.sign";
        bundles.sign.push_back(signDto);

        PermissionDto permission;
        permission.name = permName;
        permission.grantMode = 0;
        permission.status = Constant::PERMISSION_GRANTED_STATUS;
        bundles.permissions.push_back(permission);

        UidBundleBo uidBundleBo;
        uidBundleBo.DEFAULT_SIZE = 10; // default 10
        uidBundleBo.MIN_UID_PACKAGES_NUM = 1; // default 1
        uidBundleBo.uid = uid_normal_1_;
        uidBundleBo.appAttribute = 0;
        uidBundleBo.bundles.push_back(bundles);
        uidBundleBo.uidState = 1;

        std::set<std::string> remoteSensitivePermission;
        remoteSensitivePermission.insert(permName);

        SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
            std::pair<int32_t, UidBundleBo>(uid, uidBundleBo));
    }
    void TearDown()
    {
        DeviceInfoRepository::GetInstance().Clear();
        SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.clear();
        dpms_->OnStop();
    }
};

class TestCallback : public OnUsingPermissionReminderStub {
public:
    TestCallback() = default;
    virtual ~TestCallback() = default;

    std::string permName = Constant::CAMERA;

    void StartUsingPermission(const PermissionReminderInfo& permReminderInfo)
    {
        EXPECT_EQ(permName, OHOS::Str16ToStr8(permReminderInfo.permName));
    }

    void StopUsingPermission(const PermissionReminderInfo& permReminderInfo)
    {
        EXPECT_EQ(permName, OHOS::Str16ToStr8(permReminderInfo.permName));
    }
};

HWTEST_F(PermissionRemindTest, PermissionRemindTest01, Function | MediumTest | Level1)
{
    OHOS::sptr<TestCallback> callback;
    int ret = -1;
    ret = dpms_->RegisterUsingPermissionReminder(callback);
    EXPECT_EQ(-2, ret);

    ret = dpms_->UnregisterUsingPermissionReminder(callback);
    EXPECT_EQ(-2, ret);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest02, Function | MediumTest | Level1)
{
    OHOS::sptr<TestCallback> callback(new TestCallback());
    int ret = -1;

    ret = dpms_->RegisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);

    ret = dpms_->RegisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);

    ret = dpms_->UnregisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);

    ret = dpms_->UnregisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest03, Function | MediumTest | Level1)
{
    OHOS::sptr<TestCallback> callback(new TestCallback());
    int ret = -1;

    ret = dpms_->RegisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);

    // Case 01
    permName = Constant::CAMERA;
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";

    dpms_->StartUsingPermission(permName, pid, uid, deviceId);

    dpms_->StopUsingPermission(permName, pid, uid, deviceId);

    // Case 02
    permName = "";
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";
    dpms_->StartUsingPermission(permName, pid, uid, deviceId);
    dpms_->StopUsingPermission(permName, pid, uid, deviceId);

    ret = dpms_->UnregisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest04, Function | MediumTest | Level1)
{
    OHOS::sptr<TestCallback> callback(new TestCallback());
    int ret = -1;

    ret = dpms_->RegisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);

    // Case 01
    permName = Constant::CAMERA;
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";
    ret = dpms_->CheckPermissionAndStartUsing(permName, pid, uid, deviceId);
    EXPECT_EQ(0, ret);

    // Case 02
    permName = "ohos.permission.NotSensitivePermission";
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";
    ret = dpms_->CheckPermissionAndStartUsing(permName, pid, uid, deviceId);
    EXPECT_EQ(-1, ret);

    // Case 03
    permName = Constant::CAMERA;
    pid = 101;
    uid = uid_normal_2_;
    deviceId = "ohos.deviceId.test1";
    ret = dpms_->CheckPermissionAndStartUsing(permName, pid, uid, deviceId);
    EXPECT_EQ(-1, ret);

    // Case 04
    permName = Constant::CAMERA;
    pid = 101;
    uid = uid_normal_3_;
    deviceId = "ohos.deviceId.test";
    ret = dpms_->CheckPermissionAndStartUsing(permName, pid, uid, deviceId);
    EXPECT_EQ(0, ret);

    // Case 05
    permName = "";
    pid = 101;
    uid = uid_normal_4_;
    deviceId = "ohos.deviceId.test";
    ret = dpms_->CheckPermissionAndStartUsing(permName, pid, uid, deviceId);
    EXPECT_EQ(-1, ret);

    ret = dpms_->UnregisterUsingPermissionReminder(callback);
    EXPECT_EQ(0, ret);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest05, Function | MediumTest | Level1)
{
    auto callback = OHOS::sptr<TestCallback>(new TestCallback);

    PermissionReminderInfo info;
    info.deviceId = OHOS::Str8ToStr16("A");
    info.deviceLabel = OHOS::Str8ToStr16("B");
    info.bundleName = OHOS::Str8ToStr16("C");
    info.bundleLabel = OHOS::Str8ToStr16("D");
    info.permName = OHOS::Str8ToStr16(permName);

    OHOS::MessageParcel data;
    data.WriteParcelable(&info);
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    uint32_t ret = -1;

    ret = static_cast<int32_t>(OnUsingPermissionReminder::Message::START_USING_PERMISSION);
    callback->OnRemoteRequest(ret, data, reply, option);

    ret = static_cast<int32_t>(OnUsingPermissionReminder::Message::STOP_USING_PERMISSION);
    callback->OnRemoteRequest(ret, data, reply, option);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest06, Function | MediumTest | Level1)
{
    PermissionReminderInfo info;
    info.deviceId = OHOS::Str8ToStr16("A");
    info.deviceLabel = OHOS::Str8ToStr16("B");
    info.bundleName = OHOS::Str8ToStr16("C");
    info.bundleLabel = OHOS::Str8ToStr16("D");
    info.permName = OHOS::Str8ToStr16(permName);

    OHOS::MessageParcel data;
    EXPECT_TRUE(info.Marshalling(data));

    OHOS::sptr<PermissionReminderInfo> infoSptr = PermissionReminderInfo::Unmarshalling(data);
    EXPECT_NE(infoSptr, nullptr);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest07, Function | MediumTest | Level1)
{
    const OHOS::sptr<OHOS::IRemoteObject> object;
    OnUsingPermissionReminderProxy proxyObj(object);

    PermissionReminderInfo info;
    info.deviceId = OHOS::Str8ToStr16("A");
    info.deviceLabel = OHOS::Str8ToStr16("B");
    info.bundleName = OHOS::Str8ToStr16("C");
    info.bundleLabel = OHOS::Str8ToStr16("D");
    info.permName = OHOS::Str8ToStr16(permName);

    proxyObj.StartUsingPermission(info);
    proxyObj.StopUsingPermission(info);
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest08, Function | MediumTest | Level1)
{
    PermissionReminderInfo info;
    ASSERT_FALSE(info.ToString().empty());
}

HWTEST_F(PermissionRemindTest, PermissionRemindTest09, Function | MediumTest | Level1)
{
    permName = Constant::CAMERA;
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";

    dpms_->StartUsingPermission(permName, pid, uid, deviceId);

    permName = Constant::LOCATION;
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";

    dpms_->StartUsingPermission(permName, pid, uid, deviceId);

    permName = Constant::LOCATION_IN_BACKGROUND;
    pid = 101;
    uid = uid_normal_1_;
    deviceId = "ohos.deviceId.test";

    dpms_->StopUsingPermission(permName, pid, uid, deviceId);
}