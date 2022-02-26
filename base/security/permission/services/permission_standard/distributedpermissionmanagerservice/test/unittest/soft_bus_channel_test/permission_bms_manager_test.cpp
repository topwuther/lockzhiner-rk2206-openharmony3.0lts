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

#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "permission_manager_client.h"

#define private public
#include "permission_bms_manager.h"

// #include "soft_bus_channel.h"
#include "distributed_permission_manager_service.h"
#include "permission_log.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionBmsManagerTest"};
}  // namespace

class PermissionBmsManagerTest : public testing::Test {
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
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_Construct_001");
    {
        PermissionBmsManager xinstance;
        PermissionBmsManager *instance = &xinstance;
        EXPECT_EQ(instance->iBundleManager_, nullptr);
        EXPECT_EQ(instance->iPermissionManager_, nullptr);

        EXPECT_EQ(instance->permissionFetcher_, nullptr);
        EXPECT_EQ(instance->appInfoFetcher_, nullptr);
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: Init
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify init:
 * 1) app fetcher and permission fetch,
 * 2) permission listener(PermissionChangeReceiver),
 * 3) monitor manager(AddOnPermissionChangedListener)
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_Init_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_Init_001");
    {
        PermissionBmsManager xinstance;
        PermissionBmsManager *instance = &xinstance;
        EXPECT_EQ(instance->permissionFetcher_, nullptr);

        MonitorManager *mm = &MonitorManager::GetInstance();
        mm->RemoveOnPermissionChangedListener("PERMISSION_BMS_MANAGER");
        mm->RemoveOnPermissionChangedListener("ObjectDevicePermissionManager");
        {
            auto map = mm->permissionChangedListeners_;
            for (auto it = map.begin(); it != map.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it1: key: %{public}s ", it->first.c_str());
            }
        }
        EXPECT_EQ(mm->permissionChangedListeners_.empty(), true);

        PERMISSION_LOG_INFO(LABEL,
            "instance->iBundleManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iBundleManager_,
            instance->iBundleManager_.GetRefPtr());
        PERMISSION_LOG_INFO(LABEL,
            "instance->iPermissionManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iPermissionManager_,
            instance->iPermissionManager_.GetRefPtr());

        int code = instance->Init();
        EXPECT_TRUE(code == Constant::SUCCESS);

        // verify: add listener to monitor manager
        EXPECT_EQ(mm->permissionChangedListeners_.empty(), false);

        // normal: everything is ok
        PERMISSION_LOG_INFO(LABEL,
            "instance->iBundleManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iBundleManager_,
            instance->iBundleManager_.GetRefPtr());
        PERMISSION_LOG_INFO(LABEL,
            "instance->iPermissionManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iPermissionManager_,
            instance->iPermissionManager_.GetRefPtr());

        EXPECT_NE(instance->iPermissionManager_.GetRefPtr(), (void *)NULL);

        EXPECT_NE(instance->permissionFetcher_, nullptr);
        EXPECT_NE(instance->appInfoFetcher_, nullptr);
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: GetPermissions
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify get permissions by permission fetcher
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_GetPermissions_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetPermissions_001");
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetPermissions_001-1");
        // simulate not init
        PermissionBmsManager x;
        PermissionBmsManager *instance = &x;

        UidBundleBo info;
        int code = instance->GetPermissions(1, info);
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetPermissions_001-2");
        // check parameter: uid
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        // init
        instance->Init();

        PERMISSION_LOG_INFO(LABEL,
            "instance->iBundleManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iBundleManager_,
            instance->iBundleManager_.GetRefPtr());
        PERMISSION_LOG_INFO(LABEL,
            "instance->iPermissionManager_: %{public}p, GetRefPtr(): %{public}p",
            (void *)instance->iPermissionManager_,
            instance->iPermissionManager_.GetRefPtr());

        UidBundleBo info;
        int code = instance->GetPermissions(-1, info);
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetPermissions_001-3");
        // check parameter: uid
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        UidBundleBo info;
        int uid = 2101;  // ohos.samples.clock
        int code = instance->GetPermissions(uid, info);
        EXPECT_TRUE(code == Constant::SUCCESS);

        PERMISSION_LOG_INFO(LABEL, "uid         : %{public}d", info.uid);
        PERMISSION_LOG_INFO(LABEL, "uidState    : %{public}d", info.uidState);
        PERMISSION_LOG_INFO(LABEL, "appAttribute: %{public}d", info.appAttribute);
        PERMISSION_LOG_INFO(LABEL, "sensitives  : %{public}d", (int)info.remoteSensitivePermission.size());

        auto bundle = info.bundles;
        // PERMISSION_LOG_INFO(LABEL, "bundle.sign       : %{public}d", bundle.at(0).sign.size());
        PERMISSION_LOG_INFO(LABEL, "bundle.name       : %{public}s", bundle.at(0).name.c_str());
        PERMISSION_LOG_INFO(LABEL, "bundle.label      : %{public}s", bundle.at(0).bundleLabel.c_str());
        PERMISSION_LOG_INFO(LABEL, "bundle.permissions: %{public}d", (int)bundle.at(0).permissions.size());
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: GetRegrantedPermissions
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify get regranted permission
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_GetRegrantedPermissions_001, TestSize.Level1)
{
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetRegrantedPermissions_001-1");
        // simulate not init
        PermissionBmsManager x;
        PermissionBmsManager *instance = &x;

        UidBundleBo infop;
        UidBundleBo infor;
        int code = instance->GetRegrantedPermissions(infop, infor);
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetRegrantedPermissions_001-2");
        // check: has no permissions
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        UidBundleBo infop;
        UidBundleBo infor;
        int code = instance->GetRegrantedPermissions(infop, infor);
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetRegrantedPermissions_001-3");
        // check: invalid appAttribute
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        UidBundleBo infop;
        PermissionDto pd;
        BundlePermissionsDto bundle;
        infop.bundles.push_back(bundle);
        infop.bundles.at(0).permissions.push_back(pd);
        UidBundleBo infor;
        int code = instance->GetRegrantedPermissions(infop, infor);
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_GetRegrantedPermissions_001-4");
        // normal
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        UidBundleBo infop;
        PermissionDto pd;
        BundlePermissionsDto bundle;
        infop.bundles.push_back(bundle);
        infop.bundles.at(0).permissions.push_back(pd);

        infop.appAttribute = Constant::INITIAL_APP_ATTRIBUTE + 1;
        infop.DEFAULT_SIZE = 1;
        infop.uid = 9;
        UidBundleBo infor;
        int code = instance->GetRegrantedPermissions(infop, infor);
        EXPECT_TRUE(code == Constant::SUCCESS);
        EXPECT_EQ(infop.DEFAULT_SIZE, 1);
        EXPECT_EQ(infor.DEFAULT_SIZE, 10);
        EXPECT_EQ(infor.uid, infop.uid);
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: IsSystemSignatureUid
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify when Uid is <10000, or a platform signature uid will return true
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_IsSystemSignatureUid_001, TestSize.Level1)
{
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_IsSystemSignatureUid_001-1");
        // uid < 0
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        bool code = instance->IsSystemSignatureUid(-1);
        EXPECT_TRUE(code == false);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_IsSystemSignatureUid_001-2");
        // uid: mod 100000 < 10000
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        int code = instance->IsSystemSignatureUid(0);
        EXPECT_TRUE(code == true);

        code = instance->IsSystemSignatureUid(1);
        EXPECT_TRUE(code == true);
        code = instance->IsSystemSignatureUid(1000);
        EXPECT_TRUE(code == true);
        code = instance->IsSystemSignatureUid(9999);
        EXPECT_TRUE(code == true);
        code = instance->IsSystemSignatureUid(109999);
        EXPECT_TRUE(code == true);
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: InitSystemDefinedPermissions
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify Initialize system defined permissions set, maybe not used in l2 code
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_InitSystemDefinedPermissions_001, TestSize.Level1)
{
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_InitSystemDefinedPermissions_001-1");
        // simulate not init
        PermissionBmsManager x;
        PermissionBmsManager *instance = &x;

        int code = instance->InitSystemDefinedPermissions();
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_InitSystemDefinedPermissions_001-2");

        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        int code = instance->InitSystemDefinedPermissions();
        EXPECT_TRUE(code == Constant::SUCCESS);
    }
}

/*
 * Feature: DPMS
 * Function: PermissionBmsManager
 * SubFunction: ReGrantDuidPermissions
 * FunctionPoints:
 * EnvConditions: NA
 * CaseDescription: verify Regrant cached permissions for subject device when the permission define may change.
 */
HWTEST_F(PermissionBmsManagerTest, PermissionBmsManager_ReGrantDuidPermissions_001, TestSize.Level1)
{
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_ReGrantDuidPermissions_001-1");
        // no remoteSensitivePermission
        PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();
        UidBundleBo info;
        BundlePermissionsDto bundle;
        info.bundles.push_back(bundle);
        instance->ReGrantDuidPermissions(info);
        EXPECT_TRUE(info.bundles.at(0).permissions.empty());
    }
    {
        PERMISSION_LOG_INFO(LABEL, "PermissionBmsManager_ReGrantDuidPermissions_001-2");
        // PermissionBmsManager *instance = &PermissionBmsManager::GetInstance();

        PermissionBmsManager xinstance;
        PermissionBmsManager *instance = &xinstance;

        instance->Init();
        {
            UidBundleBo info;
            PermissionDto pd;
            BundlePermissionsDto bundle;
            info.bundles.push_back(bundle);
            pd.name = "ohos.permission.READ_CONTACTS";  // external_deps_mock.cpp
            pd.grantMode = 0;                           // USER_GRANT
            pd.status = 2;                              // 0b10
            info.bundles.at(0).permissions.push_back(pd);
            info.remoteSensitivePermission.insert("CAMERA");

            // 1)set conditions:
            // (permission.status & FLAG_PERMISSION_STATUS_REMOTE_GRANTED) != 0
            // FLAG_PERMISSION_STATUS_REMOTE_GRANTED(8): 0b1000
            // 0b10 & 0b1000 -> 0 -> reset
            PERMISSION_LOG_INFO(LABEL, "status1 set condition: %{public}d", (pd.status & 8) != 0);

            // 2)set logic
            // (permission.status | FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED) &
            //             (bit-not(~) FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED)
            // FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED: 0b01
            // FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED: 0b10
            // ~FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED: 0xFFFF_FFFD
            // --> (0b10 | 0b1) & 0xFFFF_FFFD --> 0b11 & 0b1101 --> 0b1
            // set 0b1, reset 0b10 -> set 0b*01
            PERMISSION_LOG_INFO(LABEL, "status1 set willbe: %{public}d", (pd.status | 1) & (~2));

            // 3)reset logic:
            // (permission.status | FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED) &
            //                     (bit-not(~) FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED)
            // (0b10 | 0b10) & ~0b01 -> 0b10 & ~0b01 -> 0bxxxx10
            PERMISSION_LOG_INFO(LABEL, "status1 reset willbe: %{public}d", (pd.status | 2) & (~1));

            // will reset data
            instance->ReGrantDuidPermissions(info);
            PERMISSION_LOG_INFO(LABEL, "status1: %{public}d", info.bundles.at(0).permissions.at(0).status);
            EXPECT_TRUE(info.bundles.at(0).permissions.at(0).status == 2);
        }
        {
            UidBundleBo info;
            PermissionDto pd;
            BundlePermissionsDto bundle;
            info.bundles.push_back(bundle);
            pd.name = "ohos.permission.READ_CONTACTS";  // external_deps_mock.cpp
            pd.grantMode = 0;                           // USER_GRANT
            pd.status = 8;                              // 0b100
            info.bundles.at(0).permissions.push_back(pd);
            info.remoteSensitivePermission.insert("CAMERA");
            // will set data
            PERMISSION_LOG_INFO(LABEL, "status2 set condition: %{public}d", (pd.status & 8) != 0);
            PERMISSION_LOG_INFO(LABEL, "status2 set willbe: %{public}d", (pd.status | 1) & (~2));
            PERMISSION_LOG_INFO(LABEL, "status2 reset willbe: %{public}d", (pd.status | 2) & (~1));

            instance->ReGrantDuidPermissions(info);
            PERMISSION_LOG_INFO(LABEL, "status2: %{public}d", info.bundles.at(0).permissions.at(0).status);
            EXPECT_TRUE(info.bundles.at(0).permissions.at(0).status == 9);
        }
        {
            UidBundleBo info;
            PermissionDto pd;
            BundlePermissionsDto bundle;
            info.bundles.push_back(bundle);
            pd.name = "ohos.permission.READ_CONTACTS";  // external_deps_mock.cpp
            pd.grantMode = 1;                           // SYSTEM_GRANT
            pd.status = 2;
            info.bundles.at(0).permissions.push_back(pd);
            info.remoteSensitivePermission.insert("CAMERA");
            // will set data
            PERMISSION_LOG_INFO(LABEL, "status3-0 set willbe: %{public}d", (pd.status | 1) & (~2));
            instance->ReGrantDuidPermissions(info);
            PERMISSION_LOG_INFO(LABEL, "status3-0: %{public}d", info.bundles.at(0).permissions.at(0).status);
            EXPECT_TRUE(info.bundles.at(0).permissions.at(0).status == 1);
        }
        {
            UidBundleBo info;
            PermissionDto pd;
            BundlePermissionsDto bundle;
            info.bundles.push_back(bundle);
            pd.name = "ohos.permission.READ_CONTACTS";  // external_deps_mock.cpp
            pd.grantMode = 1;                           // SYSTEM_GRANT
            pd.status = 1024;                           // 0b10_0xff
            info.bundles.at(0).permissions.push_back(pd);
            info.remoteSensitivePermission.insert("CAMERA");
            // will set data
            PERMISSION_LOG_INFO(LABEL, "status3 set willbe: %{public}d", (pd.status | 1) & (~2));
            instance->ReGrantDuidPermissions(info);
            PERMISSION_LOG_INFO(LABEL, "status3: %{public}d", info.bundles.at(0).permissions.at(0).status);
            EXPECT_TRUE(info.bundles.at(0).permissions.at(0).status == 1025);
        }
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS