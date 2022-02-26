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

#ifndef FM_IMMS_PROJECT_INPUTCONTROLCHANNELSTUB_H
#define FM_IMMS_PROJECT_INPUTCONTROLCHANNELSTUB_H

#include "iremote_broker.h"
#include "iremote_stub.h"
#include "i_input_method_agent.h"
#include "input_channel.h"
#include <mutex>
#include <condition_variable>
#include "message_parcel.h"
#include "i_input_control_channel.h"

namespace OHOS {
    namespace MiscServices {
    class InputControlChannelStub : public IRemoteStub<IInputControlChannel> {
    public:
        explicit InputControlChannelStub(int userId);
        virtual ~InputControlChannelStub();
        virtual int OnRemoteRequest(uint32_t code,
                                      MessageParcel &data,
                                      MessageParcel &reply,
                                      MessageOption &option) override;
        virtual void onAgentCreated(sptr<IInputMethodAgent>& agent, InputChannel *channel) override;
        virtual void hideKeyboardSelf(int flags) override;
        virtual bool advanceToNext(bool isCurrentIme) override;
        virtual void setDisplayMode(int mode) override;
        virtual void onKeyboardShowed() override;

        void ResetFlag();
        bool GetAgentAndChannel(sptr<IInputMethodAgent> *retAgent, InputChannel **retChannel);
        bool WaitKeyboardReady();
    private:
        int userId_;
        sptr<IInputMethodAgent> agent = nullptr;
        InputChannel *channel = nullptr;

        std::mutex mtx;
        std::condition_variable cv;
        bool agentReadyFlag = false;
        bool keyboardReadyFlag = false;
        const int32_t sleepTime = 300;
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTCONTROLCHANNELSTUB_H
