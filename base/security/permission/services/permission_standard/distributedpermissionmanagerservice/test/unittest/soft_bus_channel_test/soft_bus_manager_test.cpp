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

#define private public
#include "soft_bus_manager.h"

#undef private
#include "device_info_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusManagerTest"};
}
namespace {
static const std::string DEVICE_("test-device-id-001");
static const std::string NETWORK_ID_(DEVICE_ + ":network-id-001");
static const std::string UUID_(NETWORK_ID_ + ":uuid-001");
static const std::string UDID_(NETWORK_ID_ + ":udid-001");
static const int RETRY_TIMES = 30;
}  // namespace

class SoftBusManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_Construct_001");
    SoftBusManager instance;
    EXPECT_EQ(instance.isSoftBusServiceBindSuccess_, false);
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: Initialize
 * FunctionPoints: Initialize
 * EnvConditions: NA
 * CaseDescription: verify init and destroy
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_Initialize, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_Initialize");

    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_Initialize: create new instance");
    {  // factory create
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        // ensure destroied for the other test maybe construct SoftBusManager
        instance->Destroy();

        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        EXPECT_EQ(instance->inited_.load(), false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);
        EXPECT_EQ(instance->inited_.load(), true);
        {
            // will be stored
            DeviceIdType nodeIdType = DeviceIdType::NETWORK_ID;
            std::string nodeId = NETWORK_ID_;
            bool exists = DeviceInfoManager::GetInstance().ExistDeviceInfo(nodeId, nodeIdType);
            EXPECT_EQ(exists, true);

            DeviceInfo info;
            PERMISSION_LOG_DEBUG(LABEL, "GetDeviceInfo");
            bool found = DeviceInfoManager::GetInstance().GetDeviceInfo(nodeId, nodeIdType, info);
            PERMISSION_LOG_DEBUG(LABEL,
                "GetDeviceInfo, found: %{public}d, networkId: %{public}s",
                found,
                info.deviceId.networkId.c_str());
        }
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_Initialize: get instance");
    {
        // factory get
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        // should be already inited
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);
        EXPECT_EQ(instance->inited_.load(), true);
    }

    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_Initialize: recreate new instance");
    {
        // factory get
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        instance->Destroy();
        // should be not inited yet
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        EXPECT_EQ(instance->inited_.load(), false);

        // init
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);
        EXPECT_EQ(instance->inited_.load(), true);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: FulfillLocalDeviceInfo
 * FunctionPoints:
 * EnvConditions: mock softbus interface: GetLocalNodeDeviceInfo
 * CaseDescription: Verify fulfill local device info
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_FulfillLocalDeviceInfo_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_FulfillLocalDeviceInfo_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_FulfillLocalDeviceInfo_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        int code = instance->FulfillLocalDeviceInfo();
        // link error: OHOS::Security::Permission:Constant::FAILURE not found
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_FulfillLocalDeviceInfo_001-1: code is %{public}d", code);
        EXPECT_EQ(code, -1);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_FulfillLocalDeviceInfo_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();

        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        // will work
        int code = instance->FulfillLocalDeviceInfo();
        EXPECT_EQ(code, 0);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_FulfillLocalDeviceInfo_001-end");
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: GetUuidByNodeId
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify get uuid by node id
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_GetUuidByNodeId_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUuidByNodeId_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUuidByNodeId_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        std::string uuid = instance->GetUuidByNodeId(NETWORK_ID_);
        EXPECT_EQ(uuid, "");
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUuidByNodeId_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        // will work
        std::string uuid = instance->GetUuidByNodeId(NETWORK_ID_);
        EXPECT_EQ(uuid, UUID_);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUuidByNodeId_001-end");
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: GetUdidByNodeId
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify get udid by node id from softbus
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_GetUdidByNodeId_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUdidByNodeId_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUdidByNodeId_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        std::string udid = instance->GetUdidByNodeId(NETWORK_ID_);
        EXPECT_EQ(udid, "");
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUdidByNodeId_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        // will work
        std::string udid = instance->GetUdidByNodeId(NETWORK_ID_);
        EXPECT_EQ(udid, UDID_);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUdidByNodeId_001-end");
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: GetUniversallyUniqueIdByNodeId
 * FunctionPoints: get uuid and cache it if local exists
 * EnvConditions:
 * CaseDescription: verify get uuid by node id from softbus and update cache.
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_GetUniversallyUniqueIdByNodeId_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniversallyUniqueIdByNodeId_001");

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniversallyUniqueIdByNodeId_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        std::string uuid = instance->GetUniversallyUniqueIdByNodeId(NETWORK_ID_);
        EXPECT_EQ(uuid, "");
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniversallyUniqueIdByNodeId_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        // will work
        std::string uuid = instance->GetUniversallyUniqueIdByNodeId(NETWORK_ID_);
        EXPECT_EQ(uuid, UUID_);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniversallyUniqueIdByNodeId_001-end");
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: GetUniqueDisabilityIdByNodeId
 * FunctionPoints: get uuid and cache it if local exists
 * EnvConditions:
 * CaseDescription: verify get uuid by node id from softbus and update cache.
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_GetUniqueDisabilityIdByNodeId_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniqueDisabilityIdByNodeId_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniqueDisabilityIdByNodeId_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        std::string udid = instance->GetUniqueDisabilityIdByNodeId(NETWORK_ID_);
        EXPECT_EQ(udid, "");
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniqueDisabilityIdByNodeId_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        // will work
        std::string udid = instance->GetUniqueDisabilityIdByNodeId(NETWORK_ID_);
        EXPECT_EQ(udid, UDID_);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_GetUniqueDisabilityIdByNodeId_001-end");
}

/*
 * Feature: DPMS
 * Function: SoftBusManager
 * SubFunction: OpenSession
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify open and close session
 */
HWTEST_F(SoftBusManagerTest, SoftBusManager_OpenSession_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-1");
        // before reg,
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);

        // will not work
        int code = instance->OpenSession(UDID_);
        EXPECT_EQ(code, -1);

        instance->CloseSession(code);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-2");
        // reg
        SoftBusManager *instance = &SoftBusManager::GetInstance();
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, false);
        // simulator a register operator. should be register by device manager.
        ::RegNodeDeviceStateCb("", nullptr);
        instance->Initialize();
        int count = 1;
        while (instance->isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // should be inited.
        EXPECT_EQ(instance->isSoftBusServiceBindSuccess_, true);

        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-2: open1");
        // will work
        int code = instance->OpenSession(UDID_);
        EXPECT_GT(code, -1);

        // failure test: mock: index > 20 will be error
        int sessionLimit = 20;
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-2: open-many");
        for (int i = 0; i < sessionLimit; i++) {
            instance->OpenSession(UDID_);
        }
        code = instance->OpenSession(UDID_);
        EXPECT_EQ(code, -1);

        // index --
        for (int i = 0; i < sessionLimit + 1; i++) {
            // 1: mock session id
            instance->CloseSession(1);
        }

        PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-2: reopen-1");
        // will work
        code = instance->OpenSession(UDID_);
        EXPECT_GT(code, -1);
        // simulator a unregister operator.
        UnregNodeDeviceStateCb(nullptr);
    }

    // clear
    SoftBusManager *instance = &SoftBusManager::GetInstance();
    instance->Destroy();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusManager_OpenSession_001-end");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS