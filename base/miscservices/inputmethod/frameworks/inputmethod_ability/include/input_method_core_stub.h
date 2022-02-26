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

#ifndef FM_IMMS_PROJECT_INPUTCONTROLCHANNEL_SK_H
#define FM_IMMS_PROJECT_INPUTCONTROLCHANNEL_SK_H

#include "iremote_broker.h"
#include "iremote_stub.h"
#include "i_input_method_agent.h"
#include "input_channel.h"
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include "message_parcel.h"
#include "input_attribute.h"
#include "i_input_data_channel.h"
#include "i_input_method_core.h"
#include "i_input_control_channel.h"
#include "keyboard_type.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
    class InputMethodCoreStub : public IRemoteStub<IInputMethodCore> {
    public:
        DISALLOW_COPY_AND_MOVE(InputMethodCoreStub);
        explicit InputMethodCoreStub(int userId);
        virtual ~InputMethodCoreStub();
        virtual int OnRemoteRequest(uint32_t code,
                                    MessageParcel &data,
                                    MessageParcel &reply,
                                    MessageOption &option) override;

        virtual int32_t initializeInput(sptr<IRemoteObject>& startInputToken, int32_t displayId,
                                        sptr<IInputControlChannel>& inputControlChannel) override;
        virtual bool startInput(const sptr<IInputDataChannel>& inputDataChannel,
                                const InputAttribute& editorAttribute,
                                bool supportPhysicalKbd) override;
        virtual int32_t stopInput() override;
        virtual bool showKeyboard(int32_t flags) override;
        virtual bool hideKeyboard(int32_t flags)override;
        virtual int32_t setKeyboardType(const KeyboardType& type) override;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) override;
        void SetMessageHandler(MessageHandler *msgHandler);

    private:
        int userId_;
        MessageHandler *msgHandler_;
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTCONTROLCHANNEL_SK_H
