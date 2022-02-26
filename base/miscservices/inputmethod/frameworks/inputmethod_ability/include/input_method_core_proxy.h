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

#ifndef FM_IMC_PROJECT_INPUTMETHODCOREPROXY_H
#define FM_IMC_PROJECT_INPUTMETHODCOREPROXY_H

#include "iremote_proxy.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "i_input_method_core.h"
#include "i_input_control_channel.h"
#include "i_input_data_channel.h"
#include "input_attribute.h"

namespace OHOS {
namespace MiscServices {
    class InputMethodCoreProxy : public IRemoteProxy<IInputMethodCore> {
    public:
        explicit InputMethodCoreProxy(const sptr<IRemoteObject>& object);
        ~InputMethodCoreProxy();

        DISALLOW_COPY_AND_MOVE(InputMethodCoreProxy);

        virtual int32_t initializeInput(sptr<IRemoteObject>& startInputToken, int32_t displayId,
            sptr<IInputControlChannel>& inputControlChannel) override;
        virtual bool startInput(const sptr<IInputDataChannel>& inputDataChannel,
                                const InputAttribute& editorAttribute,
                                bool supportPhysicalKbd) override;
        virtual int32_t stopInput() override;
        virtual bool showKeyboard(int32_t flags) override;
        virtual bool hideKeyboard(int32_t flags) override;
        virtual int32_t setKeyboardType(const KeyboardType& type) override;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) override;

    private:
        static inline BrokerDelegator<InputMethodCoreProxy> delegator_;
    };
}
}
#endif // FM_IMC_PROJECT_INPUTMETHODCOREPROXY_H