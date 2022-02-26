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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_host.h"
#include "uuid.h"
#include "vector"

using namespace testing;
using namespace testing::ext;
using namespace bluetooth;

namespace OHOS {
namespace Bluetooth {
class GattClientCallbackTest : public GattClientCallback {
public:
    GattClientCallbackTest()
    {}
    ~GattClientCallbackTest()
    {}

    void OnConnectionStateChanged(int connectionState, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnConnectionStateChanged called";
    }

    void OnCharacteristicChanged(const GattCharacteristic &characteristic)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnCharacteristicChanged called";
    }

    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnCharacteristicReadResult called";
    }

    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnCharacteristicWriteResult called";
    }

    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnDescriptorReadResult called";
    }

    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnDescriptorWriteResult called";
    }

    void OnMtuUpdate(int mtu, int ret)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnMtuUpdate called";
    }

    void OnServicesDiscovered(int status)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnServicesDiscovered called";
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnConnectionParameterChanged called";
    }

    void OnSetNotifyCharacteristic(int status)
    {
        GTEST_LOG_(INFO) << "GattClientCallbackTest::OnSetNotifyCharacteristic called";
    }
};

class GattClientTest : public testing::Test {
public:
    GattClientTest()
    {}
    ~GattClientTest()
    {}

    int tempData_ = 0;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GattClientTest::SetUpTestCase(void)
{}
void GattClientTest::TearDownTestCase(void)
{}
void GattClientTest::SetUp()
{
    tempData_ = 0;
}

void GattClientTest::TearDown()
{}

HWTEST_F(GattClientTest, GattClient_ModuleTest_GattClient, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GattClient start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GattClient end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_Connect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_Connect start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    int result = client.Connect(callback_, isAutoConnect, transport);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_Connect end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_Disconnect, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_Disconnect start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    int result = client.Disconnect();
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_Disconnect end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_DiscoverServices, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_DiscoverServices start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    int result = client.DiscoverServices();
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_DiscoverServices end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_GetService_1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GetService_1 start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID id = UUID::RandomUUID();
    client.GetService(id);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GetService_1 end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_GetService_2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GetService_2 start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    client.GetService();
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_GetService_2 end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_ReadCharacteristic, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_ReadCharacteristic start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client.ReadCharacteristic(characteristic);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_ReadCharacteristic end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_ReadDescriptor, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_ReadDescriptor start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client.ReadDescriptor(descriptor);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_ReadDescriptor end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_RequestBleMtuSize, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_RequestBleMtuSize start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    int result = client.Connect(callback_, isAutoConnect, transport);
    int mtu = 17;
    result = client.RequestBleMtuSize(mtu);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_RequestBleMtuSize end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_SetNotifyCharacteristic, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_SetNotifyCharacteristic start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    bool enable = true;
    int result = client.SetNotifyCharacteristic(characteristic, enable);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_SetNotifyCharacteristic end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_WriteCharacteristic, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_WriteCharacteristic start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    int properties = 37;
    GattCharacteristic characteristic = GattCharacteristic(uuid_, permissions, properties);
    int result = client.WriteCharacteristic(characteristic);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_WriteCharacteristic end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_WriteDescriptor, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_WriteDescriptor start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    UUID uuid_ = UUID::RandomUUID();
    int permissions = 17;
    GattDescriptor descriptor = GattDescriptor(uuid_, permissions);
    int result = client.WriteDescriptor(descriptor);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_WriteDescriptor end";
}

HWTEST_F(GattClientTest, GattClient_ModuleTest_RequestConnectionPriority, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_RequestConnectionPriority start";
    BluetoothRemoteDevice device;
    GattClient client(device);
    GattClientCallbackTest callback_;
    bool isAutoConnect = true;
    int transport = 12;
    client.Connect(callback_, isAutoConnect, transport);
    int connPriority = 2;
    int result = client.RequestConnectionPriority(connPriority);
    EXPECT_EQ(result, (int)bluetooth::GattStatus::GATT_SUCCESS);
    GTEST_LOG_(INFO) << "GattClient_ModuleTest_RequestConnectionPriority end";
}
}  // namespace Bluetooth
}  // namespace OHOS
