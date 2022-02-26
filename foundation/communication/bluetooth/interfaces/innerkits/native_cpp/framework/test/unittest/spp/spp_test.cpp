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
#include "bluetooth_socket.h"
#include "bluetooth_gatt_characteristic.h"
#include "uuid.h"
#include "bluetooth_remote_device.h"

using namespace testing::ext;

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
class SocketTest : public testing::Test {
public:
    SocketTest()
    {}
    ~SocketTest()
    {}

    SppClientSocket *pSppClientSocket = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SocketTest::SetUpTestCase(void)
{}
void SocketTest::TearDownTestCase(void)
{}
void SocketTest::SetUp()
{}

void SocketTest::TearDown()
{}

/**
 * @tc.number:Spp_UnitTest001
 * @tc.name: Spp_Client
 * @tc.desc: 
 */
HWTEST_F(SocketTest, Spp_UnitTest_Client, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_001 start";
    BluetoothRemoteDevice *pbluetoothRomote = new BluetoothRemoteDevice();
    UUID randomUuid_ = UUID::RandomUUID();
    pSppClientSocket = new SppClientSocket(*pbluetoothRomote, randomUuid_, TYPE_RFCOMM, false);
    UUID insecureUuid_ = UUID::FromString("00001101-0000-1000-8000-00805F9B34FB");

    BluetoothRemoteDevice device_ = *pbluetoothRomote;

    GTEST_LOG_(INFO) << "SppClientSocket::Connect starts";
    pSppClientSocket->Connect();
    GTEST_LOG_(INFO) << "SppClientSocket::Connect ends";

    GTEST_LOG_(INFO) << "SppClientSocket::Close starts";
    pSppClientSocket->Close();
    GTEST_LOG_(INFO) << "SppClientSocket::Close ends";
    int fd = 37;
    SppClientSocket *pfd_SppClientSocket = new SppClientSocket(fd, device_.GetDeviceAddr());

    char receive[512];
    int DATASIZE = 1024;
    size_t returnInput = 0;
    size_t returnOutput = 0;
    char multiChar[10] = {'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    GTEST_LOG_(INFO) << "SppClientSocket::GetInputStream starts";
    InputStream input = pfd_SppClientSocket->GetInputStream();
    returnInput = input.Read(receive, DATASIZE);
    GTEST_LOG_(INFO) << "SppClientSocket::GetInputStream ends";

    GTEST_LOG_(INFO) << "SppClientSocket::GetOutputStream starts";
    OutputStream output = pfd_SppClientSocket->GetOutputStream();
    returnOutput = output.Write(multiChar, 10);
    GTEST_LOG_(INFO) << "SppClientSocket::GetOutputStream ends";

    GTEST_LOG_(INFO) << "SppClientSocket::GetRemoteDevice starts";
    BluetoothRemoteDevice tempRemoteDevice = pfd_SppClientSocket->GetRemoteDevice();
    GTEST_LOG_(INFO) << "SppClientSocket::GetRemoteDevice ends";

    GTEST_LOG_(INFO) << "SppClientSocket::IsConnected starts";
    bool IsConnected = false;
    IsConnected = pfd_SppClientSocket->IsConnected();
    EXPECT_EQ(IsConnected, true);
    GTEST_LOG_(INFO) << "SppClientSocket::IsConnected ends";

    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_001 end";
}

/**
 * @tc.number:Spp_UnitTest002
 * @tc.name: Spp_Server
 * @tc.desc: 
 */
HWTEST_F(SocketTest, Spp_UnitTest_Server, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_002 start";

    SppServerSocket *server_ = nullptr;
    UUID uuid_ = UUID::FromString("11111111-0000-1000-8000-00805F9B34FB");
    if (!server_)
        GTEST_LOG_(INFO) << "SocketFactory::DataListenRfcommByServiceRecord starts";
    server_ = SocketFactory::DataListenRfcommByServiceRecord("server", uuid_);

    GTEST_LOG_(INFO) << "SppServerSocket::GetStringTag starts";
    std::string returnStr{""};
    returnStr = server_->GetStringTag();
    GTEST_LOG_(INFO) << "SppServerSocket::GetStringTag ends";

    GTEST_LOG_(INFO) << "SppServerSocket::Accept starts";
    int SERTIMEOUT = 10;
    std::unique_ptr<SppClientSocket> preturn_SppClientSocket = server_->Accept(SERTIMEOUT);
    GTEST_LOG_(INFO) << "SppServerSocket::Accept ends";

    GTEST_LOG_(INFO) << "SppServerSocket::Close starts";
    server_->Close();
    GTEST_LOG_(INFO) << "SppServerSocket::Close ends";

    GTEST_LOG_(INFO) << "Spp_ModuleTest_Unittest_002 end";
}
}  // namespace Bluetooth
}  // namespace OHOS