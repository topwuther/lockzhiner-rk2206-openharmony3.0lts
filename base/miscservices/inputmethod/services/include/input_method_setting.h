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

#ifndef FM_IMMS_PROJECT_INPUTMETHODSETTING_H
#define FM_IMMS_PROJECT_INPUTMETHODSETTING_H
#include <map>
#include <vector>
#include "string.h"
#include "global.h"
#include "parcel.h"

namespace OHOS {
namespace MiscServices {
    class InputMethodSetting : public Parcelable {
    public:

        const static std::u16string CURRENT_INPUT_METHOD_TAG; // default IME
        const static std::u16string ENABLED_INPUT_METHODS_TAG; // enabled IME list
        const static std::u16string CURRENT_KEYBOARD_TYPE_TAG; // default keyboard type
        const static std::u16string CURRENT_SYS_KEYBOARD_TYPE_TAG; // default keyboard type for security IME
        const static std::u16string SYSTEM_LOCALE_TAG;  // locale list supported in the system


        InputMethodSetting();
        ~InputMethodSetting();
        InputMethodSetting(const InputMethodSetting& inputMethodSetting);
        InputMethodSetting& operator =(const InputMethodSetting& inputMethodSetting);

        void SetValue(const std::u16string& key, const std::u16string& value);
        std::u16string GetValue(const std::u16string& key) const;

        std::u16string GetCurrentInputMethod() const;
        void SetCurrentInputMethod(const std::u16string& imeId);

        std::vector<std::u16string> GetEnabledInputMethodList();
        bool AddEnabledInputMethod(const std::u16string& imeId, const std::vector<int32_t>& types);
        bool RemoveEnabledInputMethod(const std::u16string& imdId);

        std::vector<int32_t> GetEnabledKeyboardTypes(const std::u16string& imeId);
        int32_t GetCurrentKeyboardType();
        void SetCurrentKeyboardType(int32_t type);
        int32_t GetCurrentSysKeyboardType();
        void SetCurrentSysKeyboardType(int32_t type);
        void ClearData();
        bool FindKey(const std::u16string& key) const;

        virtual bool Marshalling(Parcel &parcel) const override;
        static InputMethodSetting *Unmarshalling(Parcel &parcel);

    private:
        std::map<std::u16string, std::u16string> setting;
        const char16_t DELIM_IME = ':';
        const char16_t DELIM_KBD_TYPE = ';';
        std::vector<std::u16string> Split(const std::u16string& str, char16_t delim);
        std::u16string BuildString(const std::vector<std::u16string>& vector, char16_t delim);
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTMETHODSETTING_H
