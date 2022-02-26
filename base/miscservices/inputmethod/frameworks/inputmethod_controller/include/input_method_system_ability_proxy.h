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

#ifndef FM_IMC_PROJECT_INPUTMETHODSYSTEMABILITYPROXY_H
#define FM_IMC_PROJECT_INPUTMETHODSYSTEMABILITYPROXY_H

#include <vector>
#include "iremote_proxy.h"
#include "i_input_method_system_ability.h"
#include "message_parcel.h"
#include "keyboard_type.h"
#include "input_method_property.h"
#include "input_client_stub.h"
#include "input_data_channel_stub.h"
#include "input_attribute.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
class InputMethodSystemAbilityProxy : public IRemoteProxy<IInputMethodSystemAbility> {
    public:
        explicit InputMethodSystemAbilityProxy(const sptr<IRemoteObject> &object);
        ~InputMethodSystemAbilityProxy() = default;
        DISALLOW_COPY_AND_MOVE(InputMethodSystemAbilityProxy);

        virtual void prepareInput(MessageParcel& data) override;
        virtual void releaseInput(MessageParcel& data) override;
        virtual void startInput(MessageParcel& data) override;
        virtual void stopInput(MessageParcel& data) override;
        virtual int32_t setInputMethodCore(sptr<IInputMethodCore> &core) override;

        int32_t Prepare(int32_t displayId, sptr<InputClientStub> &client, sptr<InputDataChannelStub> &channel,
                        InputAttribute &attribute);
        int32_t Release(sptr<InputClientStub> &client);
        int32_t Start(sptr<InputClientStub> &client);
        int32_t Stop(sptr<InputClientStub> &client);

        virtual int32_t getDisplayMode(int32_t retMode) override;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) override;
        virtual int32_t getCurrentKeyboardType(KeyboardType *retType) override;
        virtual int32_t listInputMethodEnabled(std::vector<InputMethodProperty*> *properties) override;
        virtual int32_t listInputMethod(std::vector<InputMethodProperty*> *properties) override;
        virtual int32_t listKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types) override;

    private:
        static inline BrokerDelegator<InputMethodSystemAbilityProxy> delegator_;
    };
}
}
#endif