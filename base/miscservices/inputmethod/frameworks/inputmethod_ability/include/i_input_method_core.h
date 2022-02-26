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

#ifndef FM_IMMS_PROJECT_IINPUTMETHODCORE_H
#define FM_IMMS_PROJECT_IINPUTMETHODCORE_H
#include "iremote_broker.h"
#include "i_input_data_channel.h"
#include "i_input_control_channel.h"
#include "ipc_types.h"
#include "input_attribute.h"
#include "keyboard_type.h"
#include "input_channel.h"
#include "global.h"

/**
 * brief Definition of interface IInputMethodCore
 * It defines the remote calls from input method management service to input method service
 */
namespace OHOS {
namespace MiscServices {
    class IInputMethodCore : public IRemoteBroker {
    public:
        enum {
            INITIALIZE_INPUT = FIRST_CALL_TRANSACTION,
            START_INPUT,
            STOP_INPUT,
            SHOW_KEYBOARD,
            HIDE_KEYBOARD,
            SET_KEYBOARD_TYPE,
            GET_KEYBOARD_WINDOW_HEIGHT,
        };

        DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodCore");

        virtual int32_t initializeInput(sptr<IRemoteObject>& startInputToken, int32_t displayId,
                                        sptr<IInputControlChannel>& inputControlChannel) = 0;
        virtual bool startInput(const sptr<IInputDataChannel>& startInputToken,
                                const InputAttribute& editorAttribute,
                                bool supportPhysicalKbd) = 0;
        virtual int32_t stopInput() = 0;
        virtual bool showKeyboard(int32_t flags) = 0;
        virtual bool hideKeyboard(int32_t flags) = 0;
        virtual int32_t setKeyboardType(const KeyboardType& type) = 0;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) = 0;
    };
}
}
#endif // FM_IMMS_PROJECT_IINPUTMETHODCORE_H
