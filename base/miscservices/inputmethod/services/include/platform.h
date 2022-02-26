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

#ifndef FM_IMMS_PROJECT_PLATFORMAPI_H
#define FM_IMMS_PROJECT_PLATFORMAPI_H

#include <vector>
#include <string>
#include <memory>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "i_platform_api.h"
#include "i_input_method_core.h"
#include "input_method_property.h"
#include "input_method_setting.h"

namespace OHOS {
namespace MiscServices {
    class Platform {
    public:
        static Platform *Instance();
        void SetPlatform(const sptr<IPlatformApi>& platformApi);
        sptr<IInputMethodCore> BindInputMethodService(int userId, const std::u16string& packageName,
                                                      const std::u16string& intention);
        int UnbindInputMethodService(int userId, const std::u16string& packageName);
        sptr<IRemoteObject> CreateWindowToken(int userId, int displayId, const std::u16string& packageName);
        int DestroyWindowToken(int userId, const std::u16string& packageName);
        int ListInputMethod(int userId, std::vector<InputMethodProperty*> *properties);
        int GetInputMethodProperty(int userId, const std::u16string& packageName,
                                   InputMethodProperty *inputMethodProperty);
        int GetInputMethodSetting(int userId, InputMethodSetting *inputMethodSetting);
        int SetInputMethodSetting(int userId, const InputMethodSetting& inputMethodSetting);
        bool CheckPhysicalKeyboard();
        bool IsValidWindow(int uid, int pid, int displayId);
        bool IsWindowFocused(int uid, int pid, int displayId);

        static inline sptr<IRemoteObject> RemoteBrokerToObject(const sptr<IRemoteBroker>& broker)
        {
            return broker->AsObject();
        }

    private:
        sptr<IPlatformApi> platformApi;
        Platform();
        ~Platform();
        Platform(const Platform&);
        Platform& operator =(const Platform&);
        Platform(const Platform&&);
        Platform& operator =(const Platform&&);
    };
}
}
#endif // FM_IMMS_PROJECT_PLATFORMAPI_H
