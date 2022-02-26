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


#ifndef FM_IMC_PROJECT_INPUTCLIENTPROXY_H
#define FM_IMC_PROJECT_INPUTCLIENTPROXY_H

#include "iremote_proxy.h"
#include "i_input_client.h"

namespace OHOS {
namespace MiscServices {
class InputClientProxy : public IRemoteProxy<IInputClient> {
    public:
        explicit InputClientProxy(const sptr<IRemoteObject> &object);
        ~InputClientProxy() = default;
        DISALLOW_COPY_AND_MOVE(InputClientProxy);

        int32_t onInputReady(int32_t retValue, const sptr<IInputMethodAgent>& agent, const InputChannel *channel) override;
        int32_t onInputReleased(int32_t retValue) override;
        int32_t setDisplayMode(int32_t mode) override;

    private:
        static inline BrokerDelegator<InputClientProxy> delegator_;
    };
}
}
#endif