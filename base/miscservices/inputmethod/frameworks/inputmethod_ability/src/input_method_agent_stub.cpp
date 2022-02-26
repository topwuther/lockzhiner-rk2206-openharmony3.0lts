/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "input_method_agent_stub.h"
#include "global.h"
#include "message_handler.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;

    InputMethodAgentStub::InputMethodAgentStub()
    {
    }

    InputMethodAgentStub::~InputMethodAgentStub()
    {
    }

    int32_t InputMethodAgentStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                                  MessageParcel &reply, MessageOption &option)
    {
        IMSA_HILOGI("InputMethodAgentStub::OnRemoteRequest code = %{public}d", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }

        switch (code) {
            case DISPATCH_KEY: {
                int32_t key = data.ReadInt32();
                int32_t status = data.ReadInt32();
                int32_t result = DispatchKey(key, status);
                if (result == ErrorCode::NO_ERROR) {
                    reply.WriteNoException();
                } else {
                    reply.WriteInt32(result);
                }
                return result;
            }
            default: {
                return IRemoteStub::OnRemoteRequest(code, data, reply, option);
            }
        }
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodAgentStub::DispatchKey(int32_t key, int32_t status)
    {
        IMSA_HILOGI("InputMethodAgentStub::DispatchKey key = %{public}d, status = %{public}d", key, status);
        if (msgHandler_ == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        MessageParcel *data = new MessageParcel();
        data->WriteInt32(key);
        data->WriteInt32(status);
        Message *message = new Message(MessageID::MSG_ID_DISPATCH_KEY, data);
        msgHandler_->SendMessage(message);
        return ErrorCode::NO_ERROR;
    }

    void InputMethodAgentStub::SetMessageHandler(MessageHandler *msgHandler)
    {
        msgHandler_ = msgHandler;
    }
}
}