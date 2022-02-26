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

#include "parcel.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "message_handler.h"

#include "global.h"
#include "iremote_proxy.h"
#include "i_input_control_channel.h"
#include "i_input_method_agent.h"
#include "input_control_channel_proxy.h"

/*! \class InputControlChannelProxy
  \brief The proxy implementation of IInputControlChannel

  This class should be implemented by input method service
*/
namespace OHOS {
namespace MiscServices {
    InputControlChannelProxy::InputControlChannelProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<IInputControlChannel>(impl)
    {
    }

    InputControlChannelProxy::~InputControlChannelProxy()
    {
    }

    void InputControlChannelProxy::onAgentCreated(sptr<IInputMethodAgent> &agent, InputChannel *channel)
    {
        IMSA_HILOGI("InputControlChannelProxy::onAgentCreated start.");
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        data.WriteRemoteObject(agent->AsObject());
        if (channel == nullptr) {
            data.WriteInt32(0);
        } else {
            data.WriteInt32(1);
            data.WriteParcelable(channel);
        }
        Remote()->SendRequest(ON_AGENT_CREATED, data, reply, option);
        IMSA_HILOGI("InputMethodCoreStub::onAgentCreated end.");
    }

    void InputControlChannelProxy::hideKeyboardSelf(int flags)
    {
        IMSA_HILOGI("InputControlChannelProxy::hideKeyboardSelf");
        MessageParcel data, reply;
        MessageOption option;
        data.WriteInterfaceToken(GetDescriptor());
        data.WriteInt32(flags);
        Remote()->SendRequest(HIDE_KEYBOARD_SELF, data, reply, option);
    }

    bool InputControlChannelProxy::advanceToNext(bool isCurrentIme)
    {
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        data.WriteBool(isCurrentIme);
        Remote()->SendRequest(MessageID::MSG_ID_ADVANCE_TO_NEXT, data, reply, option);
        IMSA_HILOGI("InputControlChannelProxy::advanceToNext.");
        return true;
    }

    void InputControlChannelProxy::setDisplayMode(int mode)
    {
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        data.WriteInt32(mode);
        Remote()->SendRequest(MessageID::MSG_ID_SET_DISPLAY_MODE, data, reply, option);
        IMSA_HILOGI("InputControlChannelProxy::setDisplayMode.");
    }

    void InputControlChannelProxy::onKeyboardShowed()
    {
        MessageParcel data, reply;
        MessageOption option;

        data.WriteInterfaceToken(GetDescriptor());
        Remote()->SendRequest(ON_KEYBOARD_SHOWED, data, reply, option);
        IMSA_HILOGI("InputControlChannelProxy::onKeyboardShowed.");
    }
}
}
