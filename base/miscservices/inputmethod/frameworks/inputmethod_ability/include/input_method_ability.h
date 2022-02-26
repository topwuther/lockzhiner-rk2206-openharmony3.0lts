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

#ifndef FM_IMMS_PROJECT_INPUTMETHODABILITY_H
#define FM_IMMS_PROJECT_INPUTMETHODABILITY_H

#include <thread>
#include "iremote_object.h"
#include "i_input_control_channel.h"
#include "i_input_method_core.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "input_method_core_stub.h"
#include "input_control_channel_proxy.h"
#include "input_attribute.h"
#include "message_handler.h"
#include "input_channel.h"
#include "message.h"
#include "utils.h"
#include "input_method_system_ability_proxy.h"

namespace OHOS {
namespace MiscServices {
    class EventTarget;
    class InputMethodAbility : public RefBase {
    public:
        InputMethodAbility();
        ~InputMethodAbility();
        static sptr<InputMethodAbility> GetInstance();
        sptr<IInputMethodCore> OnConnect();
        bool InsertText(const std::string text);
        void setEventTarget(sptr<EventTarget> &eventTarget);
        void DeleteBackward(int32_t length);
        void HideKeyboardSelf();

    private:
        std::thread workThreadHandler;
        MessageHandler *msgHandler;
        bool mSupportPhysicalKbd = false;
        InputAttribute *editorAttribute;
        int32_t displyId = 0;
        sptr<IRemoteObject> startInputToken;
        InputChannel *writeInputChannel;
        bool stop_;

        // communicating with IMSA
        sptr<IInputControlChannel> inputControlChannel;

        // communicating with IMC
        sptr<IInputDataChannel> inputDataChannel;
        sptr<IInputMethodAgent> inputMethodAgent;
        sptr<EventTarget> eventTarget_;
        static std::mutex instanceLock_;
        static sptr<InputMethodAbility> instance_;
        sptr<InputMethodSystemAbilityProxy> mImms;
        sptr<InputMethodSystemAbilityProxy> GetImsaProxy();

        void Initialize();
        void WorkThread();
        void CreateInputMethodAgent(bool supportPhysicalKbd);

        // the message from IMSA
        void OnInitialInput(Message *msg);
        void OnStartInput(Message *msg);
        void OnStopInput(Message *msg);
        void OnSetKeyboardType(Message *msg);
        void OnShowKeyboard(Message *msg);
        void OnHideKeyboard(Message *msg);

        // the message from IMC
        bool DispatchKey(Message *msg);

        // control inputwindow
        void InitialInputWindow();
        void ShowInputWindow();
        void DissmissInputWindow();
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTMETHODABILITY_H