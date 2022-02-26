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

#ifndef FM_IMMS_PROJECT_PERUSERSETTING_H
#define FM_IMMS_PROJECT_PERUSERSETTING_H

#include <map>
#include <string>
#include <vector>
#include "input_method_property.h"
#include "input_method_setting.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    class PerUserSetting {
    public:
        explicit PerUserSetting(int32_t userId);

        ~PerUserSetting();

        void Initialize();
        int32_t GetUserState();
        InputMethodProperty *GetCurrentInputMethod();
        InputMethodProperty *GetSecurityInputMethod();
        InputMethodProperty *GetNextInputMethod();
        InputMethodSetting *GetInputMethodSetting();
        InputMethodProperty *GetInputMethodProperty(const std::u16string& imeId);

        int32_t OnPackageAdded(std::u16string& packageName, bool isSecurityIme);
        int32_t OnPackageRemoved(std::u16string& packageName, bool isSecurityIme);
        int32_t OnSettingChanged(const std::u16string& key, const std::u16string& value);
        void OnAdvanceToNext();
        void OnUserLocked();
        void Dump(int32_t fd);

        int32_t ListInputMethodEnabled(std::vector<InputMethodProperty*> *properties);
        int32_t ListInputMethod(std::vector<InputMethodProperty*> *properties);
        int32_t ListKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types);

        static bool CheckIfSecurityIme(const InputMethodProperty& property);

    private:
        int32_t userId_; // the id of the user to whom the object is linking
        int32_t userState; // the state of the user to whom the object is linking
        std::vector<InputMethodProperty*> inputMethodProperties; // a vector to save all IME installed for this user
        std::u16string currentImeId; // the id of the default input method engine.
        InputMethodSetting inputMethodSetting; // the object to manage the setting data for this user

        PerUserSetting(const PerUserSetting&);
        PerUserSetting& operator =(const PerUserSetting&);
        PerUserSetting(const PerUserSetting&&);
        PerUserSetting& operator =(const PerUserSetting&&);
        void InitInputMethodSetting();
        void ResetCurrentInputMethod();
        std::u16string GetKeyboardTypeLanguage(const InputMethodProperty *property, int32_t hashCode);
        std::u16string GetImeId(const std::u16string& packageName);
    };
}
}
#endif // FM_IMMS_PROJECT_PERUSERSETTING_H
