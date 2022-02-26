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

#include <hdf_log.h>
#include <iservmgr_hdi.h>
#include <ipc_object_stub.h>
#include <gtest/gtest.h>
#include "sample_hdi.h"

#define HDF_LOG_TAG   service_manager_test_cpp

using namespace testing::ext;
using OHOS::sptr;
using OHOS::IRemoteObject;
using OHOS::HDI::ServiceManager::V1_0::IServiceManager;

constexpr const char *TEST_SERVICE_NAME = "sample_driver_service";
constexpr int PAYLOAD_NUM = 1234;

class HdfServiceMangerHdiTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(HdfServiceMangerHdiTest, ServMgrTest001, TestSize.Level1)
{
    auto servmgr = IServiceManager::Get();
    ASSERT_TRUE(servmgr != nullptr);
}

HWTEST_F(HdfServiceMangerHdiTest, ServMgrTest002, TestSize.Level1)
{
    auto servmgr = IServiceManager::Get();
    ASSERT_TRUE(servmgr != nullptr);

    auto sampleService = servmgr->GetService(TEST_SERVICE_NAME);

    ASSERT_TRUE(sampleService != nullptr);

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    data.WriteCString("sample_service test call");

    OHOS::MessageOption option;
    int status = sampleService->SendRequest(SAMPLE_SERVICE_PING, data, reply, option);
    ASSERT_EQ(status, 0);
}

class IPCObjectStubTest : public OHOS::IPCObjectStub {
public:
    explicit IPCObjectStubTest() : OHOS::IPCObjectStub(u"") {};
    virtual ~IPCObjectStubTest() = default;
    int OnRemoteRequest(uint32_t code, OHOS::MessageParcel &data,
        OHOS::MessageParcel &reply, OHOS::MessageOption &option) override
    {
        HDF_LOGI("IPCObjectStubTest::OnRemoteRequest called, code = %{public}d", code);
        payload = data.ReadInt32();

        return HDF_SUCCESS;
    }

    static int32_t payload;
};

int32_t IPCObjectStubTest::payload = 0;

HWTEST_F(HdfServiceMangerHdiTest, ServMgrTest003, TestSize.Level1)
{
    auto servmgr = IServiceManager::Get();
    ASSERT_TRUE(servmgr != nullptr);

    auto sampleService = servmgr->GetService(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    sptr<IRemoteObject> callback = new IPCObjectStubTest();
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    int32_t payload = PAYLOAD_NUM;
    data.WriteInt32(payload);
    data.WriteRemoteObject(callback);

    OHOS::MessageOption option;
    int status = sampleService->SendRequest(SAMPLE_SERVICE_CALLBACK, data, reply, option);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(IPCObjectStubTest::payload, payload);
}

HWTEST_F(HdfServiceMangerHdiTest, ServMgrTest004, TestSize.Level1)
{
    auto servmgr = IServiceManager::Get();
    ASSERT_TRUE(servmgr != nullptr);

    auto sampleService = servmgr->GetService(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    data.WriteInt32(PAYLOAD_NUM);
    data.WriteInt32(PAYLOAD_NUM);

    OHOS::MessageOption option;
    int status = sampleService->SendRequest(SAMPLE_SERVICE_SUM, data, reply, option);
    ASSERT_EQ(status, 0);
    int32_t result = reply.ReadInt32();
    int32_t expRes = PAYLOAD_NUM + PAYLOAD_NUM;
    ASSERT_EQ(result, expRes);
}

HWTEST_F(HdfServiceMangerHdiTest, ServMgrTest006, TestSize.Level1)
{
    auto servmgr = IServiceManager::Get();
    ASSERT_TRUE(servmgr != nullptr);

    auto sampleService = servmgr->GetService(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleService != nullptr);

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;

    constexpr int buffersize = 10;
    uint8_t dataBuffer[buffersize];
    for (int i = 0; i < buffersize; i++) {
        dataBuffer[i] = i;
    }

    bool ret = data.WriteUnpadBuffer(dataBuffer, sizeof(dataBuffer));
    ASSERT_TRUE(ret);

    OHOS::MessageOption option;
    int status = sampleService->SendRequest(SAMPLE_BUFFER_TRANS, data, reply, option);
    ASSERT_EQ(status, 0);

    const uint8_t *retBuffer = reply.ReadUnpadBuffer(buffersize);
    ASSERT_TRUE(retBuffer != nullptr);

    for (int i = 0; i < buffersize; i++) {
        ASSERT_EQ(retBuffer[i], i);
    }
}