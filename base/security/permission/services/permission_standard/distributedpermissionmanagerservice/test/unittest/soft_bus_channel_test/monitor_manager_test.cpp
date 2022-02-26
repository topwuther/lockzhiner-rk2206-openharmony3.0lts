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
#include <atomic>

#define private public
#include "monitor_manager.h"

#include "soft_bus_channel.h"
#include "distributed_permission_manager_service.h"
#include "permission_log.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "MonitorManagerTest"};
}
namespace {
int listenerSleepMs = 500;
std::atomic<int> count__(0);
void OnPermissionChanged__(int uid, const std::string &packageName)
{
    PERMISSION_LOG_DEBUG(LABEL, "OnPermissionChanged uid: %{public}d, package: %{public}s", uid, packageName.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(listenerSleepMs));

    count__++;
    PERMISSION_LOG_DEBUG(LABEL,
        "OnPermissionChanged uid: %{public}d, "
        "package: %{public}s, count: %{public}d",
        uid,
        packageName.c_str(),
        count__.load());
};
}  // namespace

class MonitorManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        // init service
        // PermissionBmsManager will register a permission changed listener on MonitorManager.
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStart();

        // simulator register
        ::RegNodeDeviceStateCb("", nullptr);

        // wait service to init;
        int sleep1000 = 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep1000));
        ASSERT_EQ(SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_, true);

        MonitorManager *instance = &MonitorManager::GetInstance();
        // remove permission_bms_manager listener for test
        instance->RemoveOnPermissionChangedListener("PERMISSION_BMS_MANAGER");
    }
    static void TearDownTestCase(void)
    {
        MonitorManager *instance = &MonitorManager::GetInstance();
        instance->RemoveOnPermissionChangedListener("test.package");

        // simulator unregister
        ::UnregNodeDeviceStateCb(nullptr);

        int sessionLimit = 20;
        for (int i = 0; i < sessionLimit + 1; i++) {
            // 1: mock session id
            ::CloseSession(1);
        }

        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: MonitorManager
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(MonitorManagerTest, MonitorManager_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "MonitorManager_Construct_001");
    MonitorManager *instance = &MonitorManager::GetInstance();
    EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)(1));
    EXPECT_EQ(instance->deviceOnlineListeners_.size(), (const unsigned int)0);
    EXPECT_EQ(instance->deviceOfflineListeners_.size(), (const unsigned int)0);
}

/*
 * Feature: DPMS
 * Function: MonitorManager
 * SubFunction: AddOnPermissionChangedListener
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify add permission changed listener
 */
HWTEST_F(MonitorManagerTest, MonitorManager_AddOnPermissionChangedListener_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "MonitorManager_AddOnPermissionChangedListener_001");

    {  // factory create
        MonitorManager *instance = &MonitorManager::GetInstance();
        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)1);

        std::string moduleName = "test.package";
        auto permissionListener =
            std::make_shared<std::function<void(int uid, const std::string &packageName)>>(OnPermissionChanged__);
        PERMISSION_LOG_DEBUG(LABEL,
            "AddOnPermissionChangedListener begin, function: %{public}lX, original function: %{public}lX",
            (unsigned long)(permissionListener.get()),
            (unsigned long)OnPermissionChanged__);
        instance->AddOnPermissionChangedListener(moduleName, permissionListener);
        PERMISSION_LOG_DEBUG(LABEL, "AddOnPermissionChangedListener end");

        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)2);
    }
}

/*
 * Feature: DPMS
 * Function: MonitorManager
 * SubFunction: AddOnPermissionChangedListener
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify add and remove permission changed listener
 */
HWTEST_F(MonitorManagerTest, MonitorManager_AddOnPermissionChangedListener_002, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "MonitorManager_AddOnPermissionChangedListener_002");

    {  // factory create
        MonitorManager *instance = &MonitorManager::GetInstance();
        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)2);

        std::string moduleName = "test.package2";
        auto permissionListener = [](int uid, const std::string &packageName) {
            PERMISSION_LOG_INFO(LABEL,
                "MonitorManager_AddOnPermissionChangedListener_002: begin, uid: %{public}d, pakcageName: %{public}s",
                uid,
                packageName.c_str());
            int sleep1500 = 1500;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep1500));
            PERMISSION_LOG_INFO(LABEL, "MonitorManager_AddOnPermissionChangedListener_002: end");
        };

        auto ptrPermissionListener =
            std::make_shared<std::function<void(int uid, const std::string &packageName)>>(permissionListener);
        instance->AddOnPermissionChangedListener(moduleName, ptrPermissionListener);

        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)3);

        // remove listener
        instance->RemoveOnPermissionChangedListener(moduleName);
        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)2);

        for (auto it = instance->permissionChangedListeners_.begin(); it != instance->permissionChangedListeners_.end();
             it++) {
            std::string name = it->first;
            auto li = it->second;
            PERMISSION_LOG_DEBUG(
                LABEL, "listener name: %{public}s, ref count: %{public}ld", name.c_str(), li.use_count());
        }
    }
}

/*
 * Feature: DPMS
 * Function: MonitorManager
 * SubFunction: OnPermissionChange
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify handle permission change event
 */
HWTEST_F(MonitorManagerTest, MonitorManager_OnPermissionChange_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "MonitorManager_OnPermissionChange_001");

    {
        // factory create
        MonitorManager *instance = &MonitorManager::GetInstance();
        EXPECT_EQ(instance->permissionChangedListeners_.size(), (const unsigned int)2);

        std::string moduleName = "test.package";
        int uid = 10009;
        EXPECT_EQ(count__.load(), 0);
        // response after 500ms

        PERMISSION_LOG_DEBUG(LABEL, "local calling, uid: %{public}d, module name: %{public}s", uid, moduleName.c_str());
        OnPermissionChanged__(uid, moduleName);
        EXPECT_EQ(count__.load(), 1);

        PERMISSION_LOG_DEBUG(
            LABEL, "event runner calling, uid: %{public}d, module name: %{public}s", uid, moduleName.c_str());
        instance->OnPermissionChange(uid, moduleName);
        // 300ms later, no response
        std::this_thread::sleep_for(std::chrono::milliseconds(listenerSleepMs / 2));
        EXPECT_EQ(count__.load(), 1);

        // 600ms later, got response
        int sleep1000 = 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(listenerSleepMs / 2 + sleep1000));
        PERMISSION_LOG_DEBUG(LABEL, "count is %{public}d", count__.load());
        EXPECT_EQ(count__.load(), 2);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS