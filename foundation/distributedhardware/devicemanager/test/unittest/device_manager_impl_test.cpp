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

#include "device_manager_impl_test.h"
#include "device_manager_errno.h"
#include "dm_device_info.h"

#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerImplTest::SetUp()
{
}

void DeviceManagerImplTest::TearDown()
{
}

void DeviceManagerImplTest::SetUpTestCase()
{
}

void DeviceManagerImplTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(DeviceManagerImplTest, InitDeviceManager, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, nullptr);
    ASSERT_EQ(ret, DEVICEMANAGER_INVALID_VALUE);
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    DmDeviceInfo dmDeviceInfo;
    DmAppImageInfo dmAppImageInfo;
    std::string extra= "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    int32_t ret= DeviceManager::GetInstance().AuthenticateDevice(packName, dmDeviceInfo,
        dmAppImageInfo, extra, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_INVALID_VALUE);
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo dmDeviceInfo;
    DmAppImageInfo dmAppImageInfo;
    std::string extra = "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_FAILED));
    int32_t ret= DeviceManager::GetInstance().AuthenticateDevice(packName, dmDeviceInfo,
        dmAppImageInfo, extra, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_IPC_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo dmDeviceInfo;
    DmAppImageInfo dmAppImageInfo;
    std::string extra = "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_OK));
    int32_t ret= DeviceManager::GetInstance().AuthenticateDevice(packName, dmDeviceInfo,
        dmAppImageInfo, extra, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, CheckAuthentication1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::string authPara = "";
    std::shared_ptr<CheckAuthCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().CheckAuthentication(packName, authPara, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_INVALID_VALUE);
}

HWTEST_F(DeviceManagerImplTest, CheckAuthentication2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string authPara = "";
    std::shared_ptr<CheckAuthCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_FAILED));
    int32_t ret= DeviceManager::GetInstance().CheckAuthentication(packName, authPara, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_IPC_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, CheckAuthentication3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string authPara = "";
    std::shared_ptr<CheckAuthCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_OK));
    int32_t ret= DeviceManager::GetInstance().CheckAuthentication(packName, authPara, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    DmSubscribeInfo subscribeInfo;
    std::shared_ptr<DiscoverCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, callback);
    ASSERT_EQ(ret, DEVICEMANAGER_INVALID_VALUE);
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    test_callback_ = std::make_shared<DeviceDiscoverCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_OK));
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, test_callback_);
    ASSERT_EQ(ret, DEVICEMANAGER_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    test_callback_ = std::make_shared<DeviceDiscoverCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(DEVICEMANAGER_FAILED));
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, test_callback_);
    ASSERT_EQ(ret,  DEVICEMANAGER_IPC_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}
} // namespace

void DeviceDiscoverCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    (void)subscribeId;
}

void DeviceDiscoverCallback::OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason)
{
    (void)subscribeId;
    (void)failedReason;
}

void DeviceDiscoverCallback::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    (void)subscribeId;
}
} // namespace Vsync
} // namespace OHOS
