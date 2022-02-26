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
#include "input_method_ability.h"
#include "input_method_core_proxy.h"
#include "input_method_core_stub.h"
#include "input_method_agent_proxy.h"
#include "input_method_agent_stub.h"
#include "message_parcel.h"
#include "event_target.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "input_data_channel_proxy.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    sptr<InputMethodAbility> InputMethodAbility::instance_;
    std::mutex InputMethodAbility::instanceLock_;

    InputMethodAbility::InputMethodAbility() : stop_(false)
    {
        Initialize();
        OnConnect();
    }

    InputMethodAbility::~InputMethodAbility()
    {
        if (msgHandler != nullptr) {
            delete msgHandler;
            msgHandler = nullptr;
        }
        stop_ = true;
    }

    sptr<InputMethodAbility> InputMethodAbility::GetInstance()
    {
        IMSA_HILOGI("InputMethodAbility::GetInstance");
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> autoLock(instanceLock_);
            if (instance_ == nullptr) {
                IMSA_HILOGI("InputMethodAbility::GetInstance need new IMA");
                instance_ = new InputMethodAbility();
            }
        }
        return instance_;
    }

    sptr<InputMethodSystemAbilityProxy> InputMethodAbility::GetImsaProxy()
    {
        IMSA_HILOGI("InputMethodAbility::GetImsaProxy");
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            IMSA_HILOGI("InputMethodAbility::GetImsaProxy systemAbilityManager is nullptr");
            return nullptr;
        }

        auto systemAbility = systemAbilityManager->GetSystemAbility(INPUT_METHOD_SYSTEM_ABILITY_ID, "");
        if (systemAbility == nullptr) {
            IMSA_HILOGI("InputMethodAbility::GetImsaProxy systemAbility is nullptr");
            return nullptr;
        }

        sptr<InputMethodSystemAbilityProxy> iface = new InputMethodSystemAbilityProxy(systemAbility);
        return iface;
    }

    sptr<IInputMethodCore> InputMethodAbility::OnConnect()
    {
        IMSA_HILOGI("InputMethodAbility::OnConnect");
        mImms = GetImsaProxy();
        sptr<InputMethodCoreStub> stub = new InputMethodCoreStub(0);
        stub->SetMessageHandler(msgHandler);
        sptr<IInputMethodCore> stub2 = stub;
        if (mImms != nullptr) {
            mImms->setInputMethodCore(stub2);
        }
        IMSA_HILOGI("InputMethodAbility::OnConnect() mImms is nullptr");
        return nullptr;
    }

    void InputMethodAbility::Initialize()
    {
        IMSA_HILOGI("InputMethodAbility::Initialize");
        InitialInputWindow();
        msgHandler = new MessageHandler();
        workThreadHandler = std::thread([this]
        {
            WorkThread();
        });
    }

    void InputMethodAbility::setEventTarget(sptr<EventTarget> &eventTarget)
    {
        IMSA_HILOGI("InputMethodAbility::setEventTarget");
        eventTarget_ = eventTarget;
    }

    void InputMethodAbility::WorkThread()
    {
        while (!stop_) {
            Message *msg = msgHandler->GetMessage();
            switch (msg->msgId_) {
                case MSG_ID_INITIALIZE_INPUT: {
                    OnInitialInput(msg);
                    break;
                }

                case MSG_ID_START_INPUT: {
                    OnStartInput(msg);
                    break;
                }

                case MSG_ID_STOP_INPUT: {
                    OnStopInput(msg);
                    break;
                }

                case MSG_ID_SHOW_KEYBOARD: {
                    OnShowKeyboard(msg);
                    break;
                }

                case MSG_ID_HIDE_KEYBOARD: {
                    OnHideKeyboard(msg);
                    break;
                }

                case MSG_ID_DISPATCH_KEY: {
                    DispatchKey(msg);
                    break;
                }

                default: {
                    break;
                }
            }
            delete msg;
            msg = nullptr;
        }
    }

    void InputMethodAbility::OnInitialInput(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::OnInitialInput");
        MessageParcel *data = msg->msgContent_;
        displyId = data->ReadInt32();
        sptr<IRemoteObject> channelObject = data->ReadRemoteObject();
        if (channelObject == nullptr) {
            IMSA_HILOGI("InputMethodAbility::OnInitialInput channelObject is nullptr");
            return;
        }
        sptr<InputControlChannelProxy> channelProxy = new InputControlChannelProxy(channelObject);
        inputControlChannel = channelProxy;
        if (inputControlChannel == nullptr) {
            IMSA_HILOGI("InputMethodAbility::OnInitialInput inputControlChannel is nullptr");
            return;
        }
        InitialInputWindow();
    }

    void InputMethodAbility::OnStartInput(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::OnStartInput");
        MessageParcel *data = msg->msgContent_;
        sptr<InputDataChannelProxy> channalProxy = new InputDataChannelProxy(data->ReadRemoteObject());
        inputDataChannel = channalProxy;
        if (inputDataChannel == nullptr) {
            IMSA_HILOGI("InputMethodAbility::OnStartInput inputDataChannel is nullptr");
        }
        editorAttribute = data->ReadParcelable<InputAttribute>();
        if (editorAttribute == nullptr) {
            IMSA_HILOGI("InputMethodAbility::OnStartInput editorAttribute is nullptr");
        }
        mSupportPhysicalKbd = data->ReadBool();

        CreateInputMethodAgent(mSupportPhysicalKbd);
        if (inputControlChannel != nullptr) {
            IMSA_HILOGI("InputMethodAbility::OnStartInput inputControlChannel is not nullptr");
            inputControlChannel->onAgentCreated(inputMethodAgent, nullptr);
        }
    }

    void InputMethodAbility::OnShowKeyboard(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::OnShowKeyboard");
        ShowInputWindow();
    }

    void InputMethodAbility::OnHideKeyboard(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::OnHideKeyboard");
        DissmissInputWindow();
    }

    void InputMethodAbility::OnStopInput(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::OnStopInput");
        if (writeInputChannel != nullptr) {
            delete writeInputChannel;
            writeInputChannel = nullptr;
        }
    }

    bool InputMethodAbility::DispatchKey(Message *msg)
    {
        IMSA_HILOGI("InputMethodAbility::DispatchKey");
        MessageParcel *data = msg->msgContent_;
        int32_t key = data->ReadInt32();
        int32_t status = data->ReadInt32();
        IMSA_HILOGI("InputMethodAbility::DispatchKey: key = %{public}d, status = %{public}d", key, status);
        return true;
    }

    void InputMethodAbility::CreateInputMethodAgent(bool supportPhysicalKbd)
    {
        IMSA_HILOGI("InputMethodAbility::CreateInputMethodAgent");
        sptr<InputMethodAgentStub> inputMethodAgentStub(new InputMethodAgentStub());
        inputMethodAgentStub->SetMessageHandler(msgHandler);
        inputMethodAgent = sptr(new InputMethodAgentProxy(inputMethodAgentStub));
    }

    void InputMethodAbility::InitialInputWindow()
    {
        IMSA_HILOGI("InputMethodAbility::InitialInputWindow");
    }

    void InputMethodAbility::ShowInputWindow()
    {
        IMSA_HILOGI("InputMethodAbility::ShowInputWindow");
        eventTarget_->Emit("keyboardShow", nullptr);
    }

    void InputMethodAbility::DissmissInputWindow()
    {
        IMSA_HILOGI("InputMethodAbility::DissmissInputWindow");
        eventTarget_->Emit("keyboardHide", nullptr);
    }

    bool InputMethodAbility::InsertText(const std::string text)
    {
        IMSA_HILOGI("InputMethodAbility::InsertText");
        if (inputDataChannel == nullptr) {
            IMSA_HILOGI("InputMethodAbility::InsertText inputDataChanel is nullptr");
            return false;
        }

        return inputDataChannel->InsertText(Utils::to_utf16(text));
    }

    void InputMethodAbility::DeleteBackward(int32_t length)
    {
        IMSA_HILOGI("InputMethodAbility::DeleteBackward");
        if (inputDataChannel == nullptr) {
            IMSA_HILOGI("InputMethodAbility::DeleteBackward inputDataChanel is nullptr");
            return;
        }
        inputDataChannel->DeleteBackward(length);
    }

    void InputMethodAbility::HideKeyboardSelf()
    {
        IMSA_HILOGI("InputMethodAbility::HideKeyboardSelf");
        inputControlChannel->hideKeyboardSelf(1);
    }
}
}