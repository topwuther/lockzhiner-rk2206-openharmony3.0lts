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

#ifndef FM_IMMS_PROJECT_IINPUTCLIENT_H
#define FM_IMMS_PROJECT_IINPUTCLIENT_H

#include "iremote_broker.h"
#include "i_input_method_agent.h"
#include "input_channel.h"
#include "global.h"

/**
 * brief Definition of interface IInputClient
 * It defines the remote calls from input method management service to input client.
 */
namespace OHOS {
namespace MiscServices {
    class IInputClient : public IRemoteBroker {
    public:
        enum {
            ON_INPUT_READY = 0,
            ON_INPUT_RELEASED,
            SET_DISPLAY_MODE,
        };

        DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.InputClient");

        virtual int32_t onInputReady(int32_t retValue, const sptr<IInputMethodAgent>& agent,
            const InputChannel *channel) = 0;
        virtual int32_t onInputReleased(int32_t retValue) = 0;
        virtual int32_t setDisplayMode(int32_t mode) = 0;
    };
} 
}
#endif // FM_IMMS_PROJECT_IINPUTCLIENT_H