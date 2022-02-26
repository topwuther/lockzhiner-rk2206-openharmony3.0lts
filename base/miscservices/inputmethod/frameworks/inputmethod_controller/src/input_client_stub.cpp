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

#include "input_client_stub.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    InputClientStub::InputClientStub()
    {
    }

    InputClientStub::~InputClientStub()
    {
    }

    int32_t InputClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                             MessageOption &option)
    {
        IMSA_HILOGI("InputClientStub::OnRemoteRequest. code = %{public}u", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }
        switch (code) {
            case ON_INPUT_READY: {
                if (msgHandler == nullptr) {
                    break;
                }
                MessageParcel *parcel = new MessageParcel();
                parcel->WriteInt32(data.ReadInt32());
                if (data.ReadInt32() > 0) {
                    parcel->WriteRemoteObject(data.ReadRemoteObject());
                }
                if (data.ReadInt32() > 0) {
                    parcel->WriteParcelable(data.ReadParcelable<InputChannel>());
                }

                Message *msg = new Message(MessageID::MSG_ID_ON_INPUT_READY, parcel);
                msgHandler->SendMessage(msg);
                break;
            }
            case ON_INPUT_RELEASED: {
                if (msgHandler == nullptr) {
                    break;
                }
                MessageParcel *parcel = new MessageParcel();
                parcel->WriteInt32(data.ReadInt32());
                Message *msg = new Message(MessageID::MSG_ID_EXIT_SERVICE, parcel);
                msgHandler->SendMessage(msg);
                break;
            }
            case SET_DISPLAY_MODE: {
                if (msgHandler == nullptr) {
                    break;
                }
                MessageParcel *parcel = new MessageParcel();
                parcel->WriteInt32(data.ReadInt32());
                Message *msg = new Message(MessageID::MSG_ID_SET_DISPLAY_MODE, parcel);
                msgHandler->SendMessage(msg);
                break;
            }
            default:
                return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
        return NO_ERROR;
    }

    int32_t InputClientStub::onInputReady(int32_t retValue, const sptr<IInputMethodAgent>& agent,
        const InputChannel *channel)
    {
        return ErrorCode::NO_ERROR;
    }

    int32_t InputClientStub::onInputReleased(int32_t retValue)
    {
        return ErrorCode::NO_ERROR;
    }

    int32_t InputClientStub::setDisplayMode(int32_t mode)
    {
        return ErrorCode::NO_ERROR;
    }

    void InputClientStub::SetHandler(MessageHandler *handler)
    {
        msgHandler = handler;
    }
}
}