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

#include "itest_transaction_service.h"

using namespace std;

namespace OHOS {
namespace {
constexpr int32_t REVERSE_INT = 1;
constexpr int32_t RESULT_STEP = 10;
}
int32_t TestTransactionServiceProxy::ReverseInt(int32_t data, int32_t& rep)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("TestTransactionServiceProxy remote null!");
        return -1;
    }
    HILOGI("TestTransactionServiceProxy:send to server data = %d", data);
    int32_t result = remote->SendRequest(REVERSE_INT, dataParcel, replyParcel, option);
    if (result != ERR_NONE) {
        HILOGE("TestTransactionServiceProxy SendRequest failed, errno:%d!", result);
        return -1;
    }
    result = replyParcel.ReadInt32();
    HILOGI("TestTransactionServiceProxy:get result from server data = %d", result);
    return ERR_NONE;
}

int32_t TestTransactionServiceStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    HILOGI("TestTransactionServiceStub::OnRemoteRequest, code = %d, flags = %d", code, option.GetFlags());
    switch (code) {
        case REVERSE_INT: {
            int32_t result = 0;
            int32_t value = data.ReadInt32();
            int32_t ret = ReverseInt(value, result);
            reply.WriteInt32(result);
            HILOGI("ReverseInt result = %d", result);
            return ret;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t TestTransactionService::ReverseInt(int32_t data, int32_t& rep)
{
    int32_t result = 0;
    while (data != 0) {
        result = result * RESULT_STEP + data % RESULT_STEP;
        data = data / RESULT_STEP;
    }
    rep = result;
    return ERR_NONE;
}
} // namespace OHOS

