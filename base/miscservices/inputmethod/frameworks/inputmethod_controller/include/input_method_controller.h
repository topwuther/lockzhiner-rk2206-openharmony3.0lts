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


#ifndef FM_IMC_PROJECT_INPUTMETHODCONTROLLER_H
#define FM_IMC_PROJECT_INPUTMETHODCONTROLLER_H

#include <mutex>
#include <thread>
#include "input_data_channel_stub.h"
#include "input_client_stub.h"
#include "input_method_system_ability_proxy.h"
#include "input_method_agent_proxy.h"
#include "i_input_method_agent.h"
#include "message_handler.h"
#include "iremote_object.h"

namespace OHOS {
namespace MiscServices {
    class OnTextChangedListener : public virtual RefBase {
    public:
        virtual void InsertText(const std::u16string& text) = 0;
        virtual void DeleteBackward(int32_t length) = 0;
        virtual void SetKeyboardStatus(bool status) = 0;
    };

    class ImsaDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ImsaDeathRecipient();
        ~ImsaDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject> &object) override;
    };

    class InputMethodController : public RefBase {
    public:
        static sptr<InputMethodController> GetInstance();
        void Attach();
        void ShowTextInput(sptr<OnTextChangedListener> &listener);
        void HideTextInput();
        void Close();
        void OnRemoteSaDied(const wptr<IRemoteObject> &object);
    private:
        InputMethodController();
        ~InputMethodController();

        bool Initialize();
        sptr<InputMethodSystemAbilityProxy> GetImsaProxy();
        void PrepareInput(int32_t displayId, sptr<InputClientStub> &client, sptr<InputDataChannelStub> &channel,
                          InputAttribute &attribute);
        void StartInput(sptr<InputClientStub> &client);
        void StopInput(sptr<InputClientStub> &client);
        void ReleaseInput(sptr<InputClientStub> &client);
        void WorkThread();

        sptr<InputDataChannelStub> mInputDataChannel;
        sptr<InputClientStub> mClient;
        sptr<InputMethodSystemAbilityProxy> mImms;
        sptr<ImsaDeathRecipient> deathRecipient_;
        sptr<InputMethodAgentProxy> mAgent;
        OnTextChangedListener *textListener;
        InputAttribute mAttribute;

        static std::mutex instanceLock_;
        static sptr<InputMethodController> instance_;
        std::thread workThreadHandler;
        MessageHandler *msgHandler;
        bool stop_;
    };
}
}
#endif