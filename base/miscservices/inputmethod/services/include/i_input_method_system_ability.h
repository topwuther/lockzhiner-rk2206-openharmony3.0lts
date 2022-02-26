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

#ifndef FM_IMMS_PROJECT_IINPUTMETHODSYSTEMABILITY_H
#define FM_IMMS_PROJECT_IINPUTMETHODSYSTEMABILITY_H

#include <vector>
#include <errors.h>
#include "iremote_broker.h"
#include "i_input_client.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"
#include "input_method_property.h"
#include "keyboard_type.h"
#include "global.h"
#include "message_parcel.h"
#include "i_input_method_core.h"

namespace OHOS {
namespace MiscServices {
    class IInputMethodSystemAbility : public IRemoteBroker {
    public:
        enum {
            PREPARE_INPUT = 0,
            RELEASE_INPUT,
            START_INPUT,
            STOP_INPUT,
            SET_INPUT_METHOD_CORE,
            GET_DISPLAY_MODE,
            GET_KEYBOARD_WINDOW_HEIGHT,
            GET_CURRENT_KEYBOARD_TYPE,
            LIST_INPUT_METHOD_ENABLED,
            LIST_INPUT_METHOD,
            LIST_KEYBOARD_TYPE,
        };

        DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodSystemAbility");

        virtual void prepareInput(MessageParcel& data) = 0;
        virtual void releaseInput(MessageParcel& data) = 0;
        virtual void startInput(MessageParcel& data) = 0;
        virtual void stopInput(MessageParcel& data) = 0;
        virtual int32_t setInputMethodCore(sptr<IInputMethodCore> &core) = 0;

        virtual int32_t getDisplayMode(int32_t retMode) = 0;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) = 0;
        virtual int32_t getCurrentKeyboardType(KeyboardType *retType) = 0;
        virtual int32_t listInputMethodEnabled(std::vector<InputMethodProperty*> *properties) = 0;
        virtual int32_t listInputMethod(std::vector<InputMethodProperty*> *properties) = 0;
        virtual int32_t listKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types) = 0;
    };
}
}
#endif // FM_IMMS_PROJECT_IINPUTMETHODSYSTEMABILITY_H
