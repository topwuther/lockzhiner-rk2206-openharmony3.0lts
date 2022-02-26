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

#include "input_data_channel_stub.h"

namespace OHOS {
namespace MiscServices {
    InputDataChannelStub::InputDataChannelStub() : msgHandler(nullptr)
    {
    }

    InputDataChannelStub::~InputDataChannelStub()
    {
        if (msgHandler != nullptr) {
            delete msgHandler;
            msgHandler = nullptr;
        }
    }

    int32_t InputDataChannelStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option)
    {
        IMSA_HILOGI("InputDataChannelStub::OnRemoteRequest code = %{public}d", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }
        switch (code) {
            case INSERT_TEXT: {
                auto text = data.ReadString16();
                InsertText(text);
                break;
            }
            case DELETE_BACKWARD: {
                auto length = data.ReadInt32();
                DeleteBackward(length);
                break;
            }
            case CLOSE: {
                Close();
                break;
            }
            default:
                return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
        return NO_ERROR;
    }

    bool InputDataChannelStub::InsertText(const std::u16string& text)
    {
        IMSA_HILOGI("InputDataChannelStub::InsertText");
        if (msgHandler != nullptr) {
            MessageParcel *parcel = new MessageParcel;
            parcel->WriteString16(text);
            Message *msg = new Message(MessageID::MSG_ID_INSERT_CHAR, parcel);
            msgHandler->SendMessage(msg);
            IMSA_HILOGI("InputDataChannelStub::InsertText return true");
            return true;
        }
        return false;
    }

    bool InputDataChannelStub::DeleteBackward(int32_t length)
    {
        IMSA_HILOGI("InputDataChannelStub::DeleteBackward");
        if (msgHandler != nullptr) {
            MessageParcel *parcel = new MessageParcel;
            parcel->WriteInt32(length);
            Message *msg = new Message(MessageID::MSG_ID_DELETE_BACKWARD, parcel);
            msgHandler->SendMessage(msg);
            return true;
        }
        return false;
    }

    void InputDataChannelStub::Close()
    {
    }

    void InputDataChannelStub::SetHandler(MessageHandler *handler)
    {
        msgHandler = handler;
    }
}
}