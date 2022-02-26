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
#include <thread>

#define private public
#include "permission_change_receiver.h"

#include "common_event_support.h"
#include "distributed_permission_manager_service.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionChangeReceiverTest"};
}
namespace {
static const std::string DEVICE_("test-device-id-001");
static const std::string NETWORK_ID_(DEVICE_ + ":network-id-001");
static const std::string UUID_(NETWORK_ID_ + ":uuid-001");
static const std::string UDID_(NETWORK_ID_ + ":udid-001");
}  // namespace

class PermissionChangeReceiverTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {  // init service
        // PermissionBmsManager will register a permission changed listener on MonitorManager.
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStart();
        // simulator register
        ::RegNodeDeviceStateCb("", nullptr);
        // wait service to init;
        int sleep500 = 500;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep500));
        ASSERT_EQ(SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_, true);
    }
    static void TearDownTestCase(void)
    {
        // simulator unregister
        ::UnregNodeDeviceStateCb(nullptr);
        int sessionLimit = 20;
        for (int i = 0; i < sessionLimit + 1; i++) {
            // 1: mock session id
            ::CloseSession(1);
        }
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
        int sleep10 = 10;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep10));
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: PermissionChangeReceiver
 * SubFunction: PackageAdded
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify package added event
 */
HWTEST_F(PermissionChangeReceiverTest, PermissionChangeReceiver_PackageAdded_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_PackageAdded_001");
    PermissionChangeReceiver instance;

    // check calling logs
    // MonitorManager: callback invoked

    // check calling logs
    // add permission items
    // SubjectDevicePermissionManager: ReGrantDuidPermissions
    int duid = 1;
    UidBundleBo permission;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int, UidBundleBo>(duid, permission));

    // 2105: ohos.samples.clock
    int uid = 2105;
    instance.PackageAdded(uid);
}

/*
 * Feature: DPMS
 * Function: PermissionChangeReceiver
 * SubFunction: PackageReplaced
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify package replaced event
 */
HWTEST_F(PermissionChangeReceiverTest, PermissionChangeReceiver_PackageReplaced_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_PackageReplaced_001");
    PermissionChangeReceiver instance;

    // check calling logs

    int duid = 1;
    UidBundleBo permission;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int, UidBundleBo>(duid, permission));

    // 2105: ohos.samples.clock
    int uid = 2105;
    instance.PackageReplaced(uid);
}

/*
 * Feature: DPMS
 * Function: PermissionChangeReceiver
 * SubFunction: PackageRemoved
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify package removed event
 */
HWTEST_F(PermissionChangeReceiverTest, PermissionChangeReceiver_PackageRemoved_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_PackageRemoved_001");
    PermissionChangeReceiver instance;

    // check calling logs

    int duid = 1;
    UidBundleBo permission;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int, UidBundleBo>(duid, permission));

    // 2105: ohos.samples.clock
    int uid = 2105;
    instance.PackageRemoved(uid);
}

/*
 * Feature: DPMS
 * Function: PermissionChangeReceiver
 * SubFunction: UserRemoved
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify user removed event
 */
HWTEST_F(PermissionChangeReceiverTest, PermissionChangeReceiver_UserRemoved_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_UserRemoved_001");
    PermissionChangeReceiver instance;

    // check calling logs
    int userId = 10001;
    instance.UserRemoved(userId);
}

/*
 * Feature: DPMS
 * Function: PermissionChangeReceiver
 * SubFunction: OnReceive
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify user removed event
 */
HWTEST_F(PermissionChangeReceiverTest, PermissionChangeReceiver_OnReceive_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001");
    PermissionChangeReceiver instance;
    // check logs
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-package_added");
        EventFwk::CommonEventData data;
        data.want_.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "ohos.samples.clock";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-package_replaced");
        EventFwk::CommonEventData data;
        data.want_.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED);
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "ohos.samples.clock";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-package_removed");
        EventFwk::CommonEventData data;
        data.want_.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "ohos.samples.clock";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-user_removed");
        EventFwk::CommonEventData data;
        data.want_.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "ohos.samples.clock";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-unknown_event");
        EventFwk::CommonEventData data;
        data.want_.SetAction("unknown");
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "ohos.samples.clock";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "PermissionChangeReceiver_OnReceive_001-unknown_bundle");
        EventFwk::CommonEventData data;
        data.want_.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
        // uid: 2105
        auto element = data.want_.GetElement();
        element.bundleName_ = "unknown";
        data.want_.SetElement(element);
        instance.OnReceive(data);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS