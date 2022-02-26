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

#ifndef FM_IMMS_PROJECT_IINPUTMETHODAGENT_H
#define FM_IMMS_PROJECT_IINPUTMETHODAGENT_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "global.h"

namespace OHOS {
    namespace MiscServices {
        class IInputMethodProxy : public IRemoteBroker {
        public:
            enum {
                DISPATCH_KEY = FIRST_CALL_TRANSACTION,
            };

            DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IInputMethodProxy");

            virtual int32_t dispatchKey(int key, int status) = 0;
        };
    }
}
#endif // FM_IMMS_PROJECT_IINPUTMETHODAGENT_H
