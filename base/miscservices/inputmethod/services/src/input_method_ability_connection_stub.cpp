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

#include "input_method_ability_connection_stub.h"
#include "message_parcel.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
    InputMethodAbilityConnectionStub::InputMethodAbilityConnectionStub(const int index)
    {
        mIndex = index;
    }

    InputMethodAbilityConnectionStub::~InputMethodAbilityConnectionStub()
    {
    }

    void InputMethodAbilityConnectionStub::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
                                                                const sptr<IRemoteObject> &remoteObject, int resultCode)
    {
        IMSA_HILOGE("ConnectAbility: OnAbilityConnectDone.");
        if (messageHandler != nullptr) {
            MessageParcel *data = new MessageParcel();
            data->WriteParcelable(&element);
            data->WriteRemoteObject(remoteObject);
            data->WriteInt32(mIndex);
            Message *msg = new Message(MessageID::MSG_ID_ABILITY_CONNECT_DONE, data);
            messageHandler->SendMessage(msg);
        }
    }

    void InputMethodAbilityConnectionStub::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
                                                                   int resultCode)
    {
        IMSA_HILOGE("ConnectAbility: OnAbilityDisconnectDone.");
        if (messageHandler != nullptr) {
            MessageParcel *data = new MessageParcel();
            data->WriteParcelable(&element);
            data->WriteInt32(mIndex);
            Message *msg = new Message(MessageID::MSG_ID_ABILITY_DISCONNECT_DONE, data);
            messageHandler->SendMessage(msg);
        }
    }

    void InputMethodAbilityConnectionStub::SetHandler(MessageHandler *handler)
    {
        messageHandler = handler;
    }
}
}