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
#include <hdf_remote_service.h>
#include <hdf_sbuf.h>
#include <gtest/gtest.h>
#include <servmgr_hdi.h>
#include "sample_hdi.h"

#define HDF_LOG_TAG   service_manager_test

using namespace testing::ext;

constexpr const char *TEST_SERVICE_NAME = "sample_driver_service";
constexpr int PAYLOAD_NUM = 1234;

class HdfServiceMangerHdiCTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest001, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);
    HDIServiceManagerRelease(servmgr);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest002, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    bool ret = HdfSbufWriteString(data, "sample_service test call");
    ASSERT_EQ(ret, true);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_PING, data, reply);
    ASSERT_EQ(status, 0);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}

static int32_t g_callbackPayload = 0;

int ServiceManagerTestCallbackDispatch(struct HdfRemoteService *service, int code,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGI("ServiceManagerTestCallbackDispatch called, code = %{public}d", code);
    HdfSbufReadInt32(data, &g_callbackPayload);
    return HDF_SUCCESS;
}

struct HdfRemoteDispatcher g_callbackDispatcher {
    .Dispatch = ServiceManagerTestCallbackDispatch,
};

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest003, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfRemoteService *callback = HdfRemoteServiceObtain(NULL, &g_callbackDispatcher);
    ASSERT_NE(callback, nullptr);
    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    int32_t payload = PAYLOAD_NUM;
    HdfSbufWriteInt32(data, payload);
    HdfSBufWriteRemoteService(data, callback);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_CALLBACK, data, reply);
    ASSERT_EQ(status, 0);
    ASSERT_EQ(g_callbackPayload, payload);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest004, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);
    HdfSbufWriteInt32(data, PAYLOAD_NUM);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_SERVICE_SUM, data, reply);
    ASSERT_EQ(status, 0);
    int32_t result;
    bool ret = HdfSbufReadInt32(reply, &result);
    ASSERT_TRUE(ret);

    int32_t expRes = PAYLOAD_NUM + PAYLOAD_NUM;
    ASSERT_EQ(result, expRes);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest005, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    struct DataBlock dataBlock = { 1, 2, "dataBolck", 3};
    bool ret = DataBlockBlockMarshalling(&dataBlock, data);
    ASSERT_TRUE(ret);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_STRUCT_TRANS, data, reply);
    ASSERT_EQ(status, 0);

    struct DataBlock *dataBlock_ = DataBlockBlockUnmarshalling(reply);
    ASSERT_TRUE(dataBlock_ != nullptr);

    ASSERT_EQ(dataBlock_->a, dataBlock.a);
    ASSERT_EQ(dataBlock_->b, dataBlock.b);
    ASSERT_EQ(dataBlock_->c, dataBlock.c);
    ASSERT_TRUE(!strcmp(dataBlock_->str, dataBlock.str));
    DataBlockFree(dataBlock_);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}

HWTEST_F(HdfServiceMangerHdiCTest, ServMgrTest006, TestSize.Level1)
{
    struct HDIServiceManager *servmgr = HDIServiceManagerGet();
    ASSERT_TRUE(servmgr != nullptr);

    struct HdfRemoteService *sampleService = servmgr->GetService(servmgr, TEST_SERVICE_NAME);
    HDIServiceManagerRelease(servmgr);
    ASSERT_TRUE(sampleService != nullptr);

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    ASSERT_TRUE(data != nullptr);
    ASSERT_TRUE(reply != nullptr);

    constexpr int buffersize = 10;
    uint8_t dataBuffer[buffersize];
    for (int i = 0; i < buffersize; i++) {
        dataBuffer[i] = i;
    }

    bool ret = HdfSbufWriteUnpadBuffer(data, dataBuffer, sizeof(dataBuffer));
    ASSERT_TRUE(ret);

    int status = sampleService->dispatcher->Dispatch(sampleService, SAMPLE_BUFFER_TRANS, data, reply);
    ASSERT_EQ(status, 0);

    const uint8_t *retBuffer = HdfSbufReadUnpadBuffer(reply, buffersize);
    ASSERT_TRUE(retBuffer != nullptr);

    for (int i = 0; i < buffersize; i++) {
        ASSERT_EQ(retBuffer[i], i);
    }

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
}