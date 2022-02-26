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

#ifndef FM_IMMS_PROJECT_IPLATFORMAPI_H
#define FM_IMMS_PROJECT_IPLATFORMAPI_H

#include <vector>
#include <string>
#include "iremote_broker.h"
#include "ipc_types.h"

#include "i_platform_callback.h"
#include "input_method_property.h"
#include "input_method_setting.h"
#include "i_input_method_core.h"
#include "global.h"

namespace OHOS {
    namespace MiscServices {
    class IPlatformApi : public IRemoteBroker {
    public:
        enum {
            REGISTER_CALLBACK = FIRST_CALL_TRANSACTION,
            BIND_IMS,
            UNBIND_IMS,
            CREATE_WINDOW_TOKEN,
            DESTROY_WINDOW_TOKEN,
            LIST_INPUT_METHOD,
            GET_INPUT_METHOD_PROPERTY,
            GET_INPUT_METHOD_SETTING,
            SET_INPUT_METHOD_SETTING,
        };

        DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.inputmethod.IPlatformApi");

        virtual int32_t registerCallback(const sptr<IPlatformCallback>& cb) = 0;
        virtual sptr<IInputMethodCore> bindInputMethodService(const std::u16string& packageName,
                                                              const std::u16string& intention, int userId) = 0;
        virtual int32_t unbindInputMethodService(int userId, const std::u16string& packageName) = 0;
        virtual sptr<IRemoteObject> createWindowToken(int userId, int displayId, const std::u16string& packageName) = 0;
        virtual int32_t destroyWindowToken(int userId, const std::u16string& packageName) = 0;
        virtual int32_t listInputMethod(int userId, std::vector<InputMethodProperty*> *properties) = 0;
        virtual int32_t getInputMethodProperty(int userId, const std::u16string& packageName,
                                                InputMethodProperty *inputMethodProperty) = 0;
        virtual int32_t getInputMethodSetting(int userId, InputMethodSetting *inputMethodSetting) = 0;
        virtual int32_t setInputMethodSetting(int userId, const InputMethodSetting& inputMethodSetting) = 0;

    };
}
}
#endif // FM_IMMS_PROJECT_IPLATFORMAPI_H
