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

#include <chrono>
#include <cstdint>
#include "message_handler.h"
#include "input_control_channel_stub.h"
#include "i_input_control_channel.h"
#include "input_channel.h"
#include "input_method_agent_proxy.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    \param userId the id of the user to whom the object is linking
    */
    InputControlChannelStub::InputControlChannelStub(int userId)
    {
        userId_ = userId;
    }

    /*! Destructor
    */
    InputControlChannelStub::~InputControlChannelStub()
    {
    }

    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the params from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputControlChannelStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
                                                     MessageParcel& reply, MessageOption& option)
    {
        IMSA_HILOGI("InputControlChannelStub::OnRemoteRequest code = %{public}u", code);
        auto descriptorToken = data.ReadInterfaceToken();
        if (descriptorToken != GetDescriptor()) {
            return ErrorCode::ERROR_STATUS_UNKNOWN_TRANSACTION;
        }
        switch (code) {
            case ON_AGENT_CREATED: {
                sptr<InputMethodAgentProxy> proxy = new InputMethodAgentProxy(data.ReadRemoteObject());
                sptr<IInputMethodAgent> agent = proxy;

                InputChannel *channel = nullptr;
                if (data.ReadInt32() > 0) {
                    channel = data.ReadParcelable<InputChannel>();
                }
                onAgentCreated(agent, channel);
                reply.WriteNoException();
                break;
            }
            case HIDE_KEYBOARD_SELF: {
                int flag = data.ReadInt32();
                hideKeyboardSelf(flag);
                reply.WriteNoException();
                break;
            }
            case ADVANCE_TO_NEXT: {
                bool isCurrentIme = data.ReadInt32();
                bool ret = advanceToNext(isCurrentIme);
                reply.WriteNoException();
                reply.WriteBool(ret);
                break;
            }
            case SET_DISPLAY_MODE:  {
                int mode = data.ReadInt32();
                setDisplayMode(mode);
                reply.WriteNoException();
                break;
            }
            case ON_KEYBOARD_SHOWED: {
                onKeyboardShowed();
                reply.WriteNoException();
                break;
            }
            default: {
                return IRemoteStub::OnRemoteRequest(code, data, reply, option);
            }
        }
        return NO_ERROR;
    }


    /*! Called when input method service creates InputMethodAgentProxy
    \n This call is running in binder thread
    \param agent the remote handler from input method service, with which input client can remotely callback to
            input method service
    \param channel channel for sending physical keyboard event from input client to input method service
    */
    void InputControlChannelStub::onAgentCreated(sptr < IInputMethodAgent >& agent, InputChannel *channel)
    {
        IMSA_HILOGI("InputControlChannelStub::onAgentCreated");
        {
            std::unique_lock<std::mutex> lck(mtx);
            agentReadyFlag = true;
        }
        this->agent = agent;
        this->channel = channel;
        cv.notify_one();
    }

    /*! Called when input method service showed keyboard
    \n This call is running in binder thread
    */
    void InputControlChannelStub::onKeyboardShowed()
    {
        IMSA_HILOGI("InputControlChannelStub::onKeyboardShowed");
        {
            std::unique_lock<std::mutex> lck(mtx);
            keyboardReadyFlag = true;
        }
        cv.notify_one();
    }

    /*! Send hideKeyboardSelf command to work thread.
    \n This call is running in binder thread,
        but the handling of hideKeyboardSelf is in the work thread of PerUserSession.
    \see PerUserSession::OnHideKeyboardSelf
    \param flags the flag value of hiding keyboard
    */
    void InputControlChannelStub::hideKeyboardSelf(int flags)
    {
        IMSA_HILOGI("InputControlChannelStub::hideKeyboardSelf flags = %{public}d", flags);
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId_);
        parcel->WriteInt32(flags);

        Message *msg = new Message(MessageID::MSG_ID_HIDE_KEYBOARD_SELF, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Send advanceToNext command to work thread.
    \n This call is running in binder thread,
        but the handling of advanceToNext is in the work thread of InputMethodSystemAbility service
    \n or in the work thread of PerUserSession
    \see InputMethodSystemAbility::OnAdvanceToNext PerUserSession::OnAdvanceToNext
    \see PerUserSetting::OnAdvanceToNext
    \param isCurrentIme true - switch to next keyboard type within current input method engine
                    \n false - switch to next input method engine
    \return true
    */
    bool InputControlChannelStub::advanceToNext(bool isCurrentIme)
    {
        IMSA_HILOGI("InputControlChannelStub::advanceToNext");
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId_);
        parcel->WriteBool(isCurrentIme);

        Message *msg = new Message(MessageID::MSG_ID_ADVANCE_TO_NEXT, parcel);
        MessageHandler::Instance()->SendMessage(msg);
        return true;
    }

    /*! Send setDisplayMode command to work thread.
    \n This call is running in binder thread,
        but the handling of setDisplayMode is in the work thread of PerUserSession.
    \see PerUserSession::OnSetDisplayMode
    \param mode 0 - part screen mode, 1 - full screen mode
    */
    void InputControlChannelStub::setDisplayMode(int mode)
    {
        IMSA_HILOGI("InputControlChannelStub::setDisplayMode start");
        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId_);
        parcel->WriteInt32(mode);

        Message *msg = new Message(MessageID::MSG_ID_SET_DISPLAY_MODE, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }

    /*! Reset ready flag to be false
    \n This should be called before imsCore->startInput() in work thread of PerUserSession
    */
    void InputControlChannelStub::ResetFlag()
    {
        IMSA_HILOGI("InputControlChannelStub::ResetFlag");
        std::unique_lock<std::mutex> lck(mtx);
        keyboardReadyFlag = false;
        agentReadyFlag = false;
        if (agent) {
            agent = nullptr;
        }
        if (channel) {
            delete channel;
            channel = nullptr;
        }
    }
    /*! Get input method agent and input write channel
    \n This should be called in work thread of PerUserSession
    \param[out] retAgent remote handler of InputMethodAgentProxy returned to the caller
    \param[out] retChannel input write channel returned to caller
    \return true - onAgentCreated is called by input method service in time
    \n false - onAgentCreated is not called by input method service in time
    */
    bool InputControlChannelStub::GetAgentAndChannel(sptr<IInputMethodAgent> *retAgent, InputChannel **retChannel)
    {
        IMSA_HILOGI("InputControlChannelStub::GetAgentAndChannel");
        std::chrono::milliseconds millsec(sleepTime);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lck(mtx);
            ret = cv.wait_for(lck, millsec, [this] {
                return agentReadyFlag;
            });
        }
        if (ret) {
            if (retAgent) {
                *retAgent = agent;
                agent = nullptr;
            }
            if (retChannel) {
                *retChannel = channel;
                channel = nullptr;
            }
            return true;
        }
        return false;
    }

    /*! Wait for keyboard to be ready
    \n This should be called in work thread of PerUserSession
    \return true - onKeyboardShowed is called by input method service in time
    \n false - onKeyboardShowed is not called by input method service in time
    */
    bool InputControlChannelStub::WaitKeyboardReady()
    {
        IMSA_HILOGI("InputControlChannelStub::WaitKeyboardReady");
        std::chrono::milliseconds millsec(sleepTime);
        bool ret = false;
        {
            std::unique_lock<std::mutex> lck(mtx);
            ret = cv.wait_for(lck, millsec, [this] {
                return keyboardReadyFlag;
            });
        }
        return ret;
    }
}
}