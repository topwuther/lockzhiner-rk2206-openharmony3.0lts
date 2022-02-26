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

#define LOG_TAG "CommunicationProviderImplTest"

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "app_types.h"
#include "app_device_status_change_listener.h"
#include "communication_provider.h"
#include "log_print.h"
#include "app_device_handler.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::AppDistributedKv;
class CommunicationProviderImplTest : public testing::Test {
};

class AppDataChangeListenerImpl : public AppDataChangeListener {
    void OnMessage(const DeviceInfo &info, const uint8_t *ptr, const int size,
                   const struct PipeInfo &id) const override;
};
void AppDataChangeListenerImpl::OnMessage(const DeviceInfo &info, const uint8_t *ptr, const int size,
                                          const struct PipeInfo &id) const
{
    ZLOGI("data  %{public}s  %s", info.deviceName.c_str(), ptr);
}

class AppDeviceStatusChangeListenerImpl : public AppDeviceStatusChangeListener {
public:
    void OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const override;
    ~AppDeviceStatusChangeListenerImpl();
};

void AppDeviceStatusChangeListenerImpl::OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const
{
    ZLOGI("%{public}s  %{public}d", info.deviceName.c_str(), static_cast<int>(type));
}

AppDeviceStatusChangeListenerImpl::~AppDeviceStatusChangeListenerImpl()
{
}

/**
* @tc.name: CommunicationProvider003
* @tc.desc: Verify getting KvStore
* @tc.type: FUNC
* @tc.require: AR000CCPQ1 AR000CQDVE
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider003, TestSize.Level1)
{
    ZLOGI("begin.");
    const AppDataChangeListenerImpl* dataListener = new AppDataChangeListenerImpl();
    PipeInfo appId;
    appId.pipeId = "appId";
    appId.userId = "groupId";
    CommunicationProvider::GetInstance().StartWatchDataChange(dataListener, appId);
    auto secRegister = CommunicationProvider::GetInstance().StartWatchDataChange(dataListener, appId);
    EXPECT_EQ(Status::ERROR, secRegister);
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider004
* @tc.desc: Verify stop watch device change
* @tc.type: FUNC
* @tc.require: AR000CCPQ2 AR000CQS3F
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider004, TestSize.Level1)
{
    ZLOGD("CommunicationProvider004");
    const AppDataChangeListenerImpl* dataListener = new AppDataChangeListenerImpl();
    PipeInfo appId;
    appId.pipeId = "appId";
    appId.userId = "groupId";
    auto secRegister = CommunicationProvider::GetInstance().StopWatchDataChange(dataListener, appId);
    ZLOGD("CommunicationProvider004 %d", static_cast<int>(secRegister));
    EXPECT_EQ(Status::ERROR, secRegister);
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider005
* @tc.desc: Verify stop watch device change
* @tc.type: FUNC
* @tc.require: AR000CCPQ2 AR000CQS3G AR000CQS3F
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider005, TestSize.Level1)
{
    ZLOGD("CommunicationProvider005");
    const AppDeviceStatusChangeListenerImpl* dataListener = new AppDeviceStatusChangeListenerImpl();
    PipeInfo appId;
    appId.pipeId = "appId";
    appId.userId = "groupId";
    auto firRegister = CommunicationProvider::GetInstance().StartWatchDeviceChange(dataListener, appId);

    EXPECT_EQ(Status::SUCCESS, firRegister);
    auto secStop = CommunicationProvider::GetInstance().StopWatchDeviceChange(dataListener, appId);
    EXPECT_EQ(Status::SUCCESS, secStop);
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider006
* @tc.desc: Verify stop watch device change
* @tc.type: FUNC
* @tc.require: AR000CCPQ2
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider006, TestSize.Level1)
{
    ZLOGI("GetDeviceList");
    auto devices = CommunicationProvider::GetInstance().GetDeviceList();
    const unsigned long val = 0;
    ZLOGD("GetDeviceList size: %zu", devices.size());
    ASSERT_GE(devices.size(), val);
    for (const auto &device : devices) {
        ZLOGD("GetDeviceList id: %s, name:%s, type:%s",
              AppDeviceHandler::ToBeAnonymous(device.deviceId).c_str(),
              device.deviceName.c_str(), device.deviceType.c_str());
    }
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider007
* @tc.desc: Verify Local deviceId exist
* @tc.type: FUNC
* @tc.require: AR000CCPQ2 AR000CQS3I
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider007, TestSize.Level1)
{
    ZLOGI("CommunicationProvider007 GetLocalDevice");
    auto device = CommunicationProvider::GetInstance().GetLocalDevice();
    const unsigned long val = 0;
    ASSERT_GE(device.deviceName.length(), val);
    ZLOGD("GetLocalDevice id: %s, name:%s, type:%s",
          AppDeviceHandler::ToBeAnonymous(device.deviceId).c_str(),
          device.deviceName.c_str(), device.deviceType.c_str());
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider015
* @tc.desc: close pipe
* @tc.type: FUNC
* @tc.require: AR000CCPQ2
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider015, TestSize.Level1)
{
    ZLOGI("CommunicationProvider015 ");
    PipeInfo appId;
    appId.pipeId = "appId";
    appId.userId = "groupId";
    auto status = CommunicationProvider::GetInstance().Stop(appId);
    EXPECT_NE(Status::ERROR, status);
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider016
* @tc.desc: singleton pipe
* @tc.type: FUNC
* @tc.require: AR000CCPQ2
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider016, TestSize.Level1)
{
    ZLOGI("begin.");
    auto &provider = CommunicationProvider::GetInstance();
    auto &provider1 = CommunicationProvider::GetInstance();
    EXPECT_EQ(&provider, &provider1);
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}

/**
* @tc.name: CommunicationProvider017
* @tc.desc: parse sent data
* @tc.type: FUNC
* @tc.require: AR000CCPQ2 AR000CQS3M AR000CQSAI
* @tc.author: hongbo
*/
HWTEST_F(CommunicationProviderImplTest, CommunicationProvider017, TestSize.Level1)
{
    const AppDataChangeListenerImpl *dataListener17 = new AppDataChangeListenerImpl();
    PipeInfo id17;
    id17.pipeId = "appId";
    id17.userId = "groupId";
    CommunicationProvider::GetInstance().StartWatchDataChange(dataListener17, id17);
    CommunicationProvider::GetInstance().Start(id17);
    std::string content = "Helloworlds";
    const uint8_t *t = (uint8_t *)(content.c_str());
    DeviceId di17 = {"127.0.0.2"};
    Status status = CommunicationProvider::GetInstance().SendData(id17, di17, t, content.length());
    EXPECT_NE(status, Status::SUCCESS);
    CommunicationProvider::GetInstance().StopWatchDataChange(dataListener17, id17);
    CommunicationProvider::GetInstance().Stop(id17);
    delete dataListener17;
    sleep(1); // avoid thread dnet thread died, then will have pthread;
}
