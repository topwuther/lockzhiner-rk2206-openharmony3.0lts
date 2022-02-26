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

#include "IPCStreamOperatorCallback_fuzzer.h"
#include "fuzz_base.h"

#include <cstddef>
#include <cstdint>

class IPCStreamOperatorCallbackFuzzer : public StreamOperatorCallbackStub {
public:
    virtual void OnCaptureStarted(int32_t captureId, const std::vector<int32_t> &streamIds) override {}
    virtual void OnCaptureEnded(int32_t captureId,
        const std::vector<std::shared_ptr<CaptureEndedInfo>> &infos) override {}
    virtual void OnCaptureError(int32_t captureId,
        const std::vector<std::shared_ptr<CaptureErrorInfo>> &infos) override {}
    virtual void OnFrameShutter(int32_t captureId,
        const std::vector<int32_t> &streamIds, uint64_t timestamp) override {}
};

static uint32_t U32_AT(const uint8_t *ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

static int32_t onRemoteRequest(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    IPCStreamOperatorCallbackFuzzer IPCStreamSerCall;
    auto ret = IPCStreamSerCall.OnRemoteRequest(code, data, reply, option);
    return ret;
}

static void fuzzAccountService(const uint8_t *data, size_t size)
{
    MessageParcel reply;
    MessageOption option;
    MessageParcel dataMessageParcel;
    if (size > sizeof(uint32_t)) {
        uint32_t code = U32_AT(data);
        data = data + sizeof(uint32_t);
        size = size - sizeof(uint32_t);
        dataMessageParcel.WriteInterfaceToken(StreamOperatorCallbackStub::GetDescriptor());
        dataMessageParcel.WriteBuffer(data, size);
        dataMessageParcel.RewindRead(0);
        onRemoteRequest(code, dataMessageParcel);
    }
}

static void OnRemoteRequestFunc(const uint8_t *data, size_t size)
{
    fuzzAccountService(data, size);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OnRemoteRequestFunc(data, size);
    return 0;
}