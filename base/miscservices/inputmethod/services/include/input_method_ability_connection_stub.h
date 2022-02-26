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
#ifndef SERVICES_INPUTMETHODABILITYCONNECTIONSTUB_H
#define SERVICES_INPUTMETHODABILITYCONNECTIONSTUB_H
#include "ability_connect_callback_stub.h"
#include "want.h"
#include "element_name.h"
#include "message_handler.h"
#include "iremote_object.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    class InputMethodAbilityConnectionStub : public AAFwk::AbilityConnectionStub {
    public:
        enum MessageID {
            MSG_ID_ABILITY_CONNECT_DONE = 1,
            MSG_ID_ABILITY_DISCONNECT_DONE,
        };

        InputMethodAbilityConnectionStub(const int index);
        ~InputMethodAbilityConnectionStub();
        void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
                                  const sptr<IRemoteObject> &remoteObject, int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
        void SetHandler(MessageHandler *handler);

    private:
        MessageHandler *messageHandler;
        int mIndex;
    };
}
}
#endif // SERVICES_INPUTMETHODABILITYCONNECTIONSTUB_H