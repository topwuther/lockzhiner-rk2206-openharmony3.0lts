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


#ifndef FM_IMC_PROJECT_INPUTCLIENTSTUB_H
#define FM_IMC_PROJECT_INPUTCLIENTSTUB_H

#include "iremote_stub.h"
#include "i_input_client.h"
#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
    class InputClientStub : public IRemoteStub<IInputClient> {
    public:
        DISALLOW_COPY_AND_MOVE(InputClientStub);
        int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
        InputClientStub();
        ~InputClientStub();
        void SetHandler(MessageHandler *handler);

        int32_t onInputReady(int32_t retValue, const sptr<IInputMethodAgent>& agent,
        const InputChannel *channel) override;
        int32_t onInputReleased(int32_t retValue) override;
        int32_t setDisplayMode(int32_t mode) override;
    private:
        MessageHandler *msgHandler = nullptr;
    };
}
}
#endif