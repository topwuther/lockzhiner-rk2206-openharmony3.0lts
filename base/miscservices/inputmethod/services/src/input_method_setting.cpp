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

#include "input_method_setting.h"
#include "utils.h"

namespace OHOS {
namespace MiscServices {
    const std::u16string InputMethodSetting::CURRENT_INPUT_METHOD_TAG = u"imms_current_input_method";
    const std::u16string InputMethodSetting::ENABLED_INPUT_METHODS_TAG = u"imms_enabled_input_methods";
    const std::u16string InputMethodSetting::CURRENT_KEYBOARD_TYPE_TAG = u"imms_current_keyboard_type";
    const std::u16string InputMethodSetting::CURRENT_SYS_KEYBOARD_TYPE_TAG = u"imms_current_sys_keyboard_type";
    const std::u16string InputMethodSetting::SYSTEM_LOCALE_TAG = u"system_locales";

    /*! Constructor
    */
    InputMethodSetting::InputMethodSetting()
    {
    }

    /*! Destructor
    */
    InputMethodSetting::~InputMethodSetting()
    {
        setting.clear();
    }

    /*! Constructor
    \param inputMethodSetting the source InputMethodSetting copied to this instance
    */
    InputMethodSetting::InputMethodSetting(const InputMethodSetting& inputMethodSetting)
    {
        setting = inputMethodSetting.setting;
    }

    /*! operator=
    \param inputMethodSetting the source InputMethodSetting copied to this instance
    \return return this instance
    */
    InputMethodSetting& InputMethodSetting::operator =(const InputMethodSetting& inputMethodSetting)
    {
        if (this == &inputMethodSetting) {
            return *this;
        }
        setting = inputMethodSetting.setting;
        return *this;
    }

    /*! Write setting data to parcel
    \param[out] parcel the setting data is written to parcel returned to caller.
    \return ErrorCode::NO_ERROR
    */
    bool InputMethodSetting::Marshalling(OHOS::Parcel &parcel) const
    {
        int32_t size = setting.size();
        parcel.WriteInt32(size);
        std::map<std::u16string, std::u16string>::const_iterator it;
        for (it = setting.cbegin(); it != setting.cend(); ++it) {
            parcel.WriteString16(it->first);
            parcel.WriteString16(it->second);
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Read setting data from parcel
    \param parcel read the setting data from the given parcel
    \return ErrorCode::NO_ERROR
    */
    InputMethodSetting *InputMethodSetting::Unmarshalling(OHOS::Parcel &parcel)
    {
        auto ims = new InputMethodSetting();
        int32_t size = parcel.ReadInt32();
        for (int i = 0; i < size; i++) {
            std::u16string key = parcel.ReadString16();
            std::u16string value = parcel.ReadString16();
            ims->setting.insert(std::pair<std::u16string, std::u16string>(key, value));
        }
        return ims;
    }

    /*! Set setting data for an item
    \param key the name of setting item
    \param value the value of setting item
    */
    void InputMethodSetting::SetValue(const std::u16string& key, const std::u16string& value)
    {
        setting.insert_or_assign(key, value);
    }

    /*! Get setting data for an item
    \param key the name of setting item
    \return the value of setting item if key is found
    \return an empty string if key is not found
    */
    std::u16string InputMethodSetting::GetValue(const std::u16string& key) const
    {
        std::map<std::u16string, std::u16string>::const_iterator it = setting.find(key);
        if (it == setting.end()) {
            return u"";
        }
        return it->second;
    }

    /*! Get the default input method engine
    \return the imeId of the default input method engine
    */
    std::u16string InputMethodSetting::GetCurrentInputMethod() const
    {
        return GetValue(CURRENT_INPUT_METHOD_TAG);
    }

    /*! Set the default input method engine
    \param imeId the ime Id of the given input method engine
    */
    void InputMethodSetting::SetCurrentInputMethod(const std::u16string& imeId)
    {
        SetValue(CURRENT_INPUT_METHOD_TAG, imeId);
    }

    /*! Get enabled input method engine list
    \return a vector of ImeId
    */
    std::vector<std::u16string> InputMethodSetting::GetEnabledInputMethodList()
    {
        std::u16string value = GetValue(ENABLED_INPUT_METHODS_TAG);
        std::vector<std::u16string> tmp1 = Split(value, DELIM_IME);
        std::vector<std::u16string> imeList;
        for (int i = 0; i < (int)tmp1.size(); i++) {
            std::vector<std::u16string> tmp2 = Split(tmp1[i], DELIM_KBD_TYPE);
            imeList.push_back(tmp2[0]);
            tmp2.clear();
        }
        return imeList;
    }

    /*! Add an input method engine to enabled ime list
    \param imeId the ime id of the added input method engine
    \param types a vector of hashCode of keyboard types which are supported by the added input method engine
    \return true - added successfully.
    \return false - the given input method engine is already enabled.
    */
    bool InputMethodSetting::AddEnabledInputMethod(const std::u16string& imeId, const std::vector<int32_t>& types)
    {
        std::u16string str = GetValue(ENABLED_INPUT_METHODS_TAG);
        std::vector<std::u16string> imeList = Split(str, DELIM_IME);

        std::u16string typeStr;
        for (int i = 0; i < (int)types.size(); i++) {
            typeStr = typeStr + u";" + Utils::to_utf16(std::to_string(types[i]));
        }
        std::u16string imeStr = imeId + typeStr;
        bool flag = false;
        for (int i = 0; i < (int)imeList.size(); i++) {
            if (imeList[i] == imeStr) {
                return false;
            }
            if (imeList[i].find_first_of(imeId)) {
                imeList[i] = imeStr;
                flag = true;
                break;
            }
        }
        if (flag == false) {
            imeList.push_back(imeStr);
        }

        std::u16string value = BuildString(imeList, DELIM_IME);
        SetValue(ENABLED_INPUT_METHODS_TAG, value);
        imeList.clear();
        return true;
    }

    /*! Remove an input method engine from enabled ime list.
    \param imeId the ime id of the given input method engine
    \return true - removed successfully.
    \return false - ime id is not found in enabled ime list.
    */
    bool InputMethodSetting::RemoveEnabledInputMethod(const std::u16string& imeId)
    {
        std::u16string str = GetValue(ENABLED_INPUT_METHODS_TAG);
        std::vector<std::u16string> imeList = Split(str, DELIM_IME);
        bool flag = false;
        std::vector<std::u16string>::iterator it;
        for (it = imeList.begin(); it < imeList.end(); ++it) {
            if (it->find_first_of(imeId)) {
                imeList.erase(it);
                flag = true;
                break;
            }
        }
        if (flag == true) {
            std::u16string value = BuildString(imeList, DELIM_IME);
            SetValue(ENABLED_INPUT_METHODS_TAG, value);
        }
        imeList.clear();
        return flag;
    }

    /*! Get the keyboard type list of the given input method engine
    \param imeId the ime id of the given input method engine
    \return a vector of hashCodes
    */
    std::vector<int32_t> InputMethodSetting::GetEnabledKeyboardTypes(const std::u16string& imeId)
    {
        std::vector<int32_t> retValue;
        std::u16string value = GetValue(ENABLED_INPUT_METHODS_TAG);
        std::vector<std::u16string> tmpVector = Split(value, DELIM_IME);
        bool flag = false;
        std::u16string imeStr;
        for (int i = 0; i < (int)tmpVector.size(); i++) {
            if (tmpVector[i].find_first_of(imeId) != std::u16string::npos) {
                flag = true;
                imeStr = tmpVector[i];
                break;
            }
        }
        tmpVector.clear();
        if (flag == false) {
            return retValue;
        }

        std::vector<std::u16string> tmp2 = Split(imeStr, DELIM_KBD_TYPE);
        for (int i = 1; i < (int)tmp2.size(); i++) {
            std::u16string str = tmp2[i];
            retValue.push_back(std::atoi(Utils::to_utf8(str).c_str()));
        }
        tmp2.clear();

        return retValue;
    }

    /*! Get the default keyboard type
    \return return the hashCode value of the default keyboard type.
    */
    int32_t InputMethodSetting::GetCurrentKeyboardType()
    {
        std::u16string value = GetValue(CURRENT_KEYBOARD_TYPE_TAG);
        return std::atoi(Utils::to_utf8(value).c_str());
    }

    /*! Set the default keyboard type
    \param type hashCode of the given keyboard type
    */
    void InputMethodSetting::SetCurrentKeyboardType(int32_t type)
    {
        std::u16string str = Utils::to_utf16(std::to_string(type));
        SetValue(CURRENT_KEYBOARD_TYPE_TAG, str);
    }

    /*! Get the default keyboard type for security input method engine
    \return return hashCode of the default keyboard type of security IME
    */
    int32_t InputMethodSetting::GetCurrentSysKeyboardType()
    {
        std::u16string value = GetValue(CURRENT_SYS_KEYBOARD_TYPE_TAG);
        return std::atoi(Utils::to_utf8(value).c_str());
    }

    /*! Set the default keyboard type for security IME
    \param type the hashCode of the given keyboard type of security IME
    */
    void InputMethodSetting::SetCurrentSysKeyboardType(int32_t type)
    {
        std::u16string str = Utils::to_utf16(std::to_string(type));
        SetValue(CURRENT_SYS_KEYBOARD_TYPE_TAG, std::u16string(str));
    }

    /*! Clear setting data
    */
    void InputMethodSetting::ClearData()
    {
        setting.clear();
    }

    /*! Find if the key is in the setting
    \param key the name of setting item
    \return true - key is found
    \return false - key is not found
    */
    bool InputMethodSetting::FindKey(const std::u16string& key) const
    {
        std::map<std::u16string, std::u16string>::const_iterator it = setting.find(key);
        if (it == setting.end()) {
            return false;
        }
        return true;
    }

    /*! Split a string into a vector
    \param str the string to be split
    \param delim the alphabet to split the string.
    \return a vector of string
    */
    std::vector<std::u16string> InputMethodSetting::Split(const std::u16string& str, char16_t delim)
    {
        std::vector<std::u16string> retValue;
        std::u16string::size_type left, right;
        left = 0;
        right = str.find(delim, 0);
        while (right != std::u16string::npos) {
            if (right - left) {
                retValue.emplace_back(str.substr(left, right - left));
            }
            left = right + 1;
            right = str.find(delim, left);
        }

        if (left != str.size()) {
            retValue.emplace_back(str.substr(left));
        }
        return retValue;
    }

    /*! Build a string from a vector
    \param vector a vector of string
    \delim a separator
    \return return a string
    */
    std::u16string InputMethodSetting::BuildString(const std::vector<std::u16string>& vector, char16_t delim)
    {
        std::u16string retValue = u"";
        char16_t delimStr[] = {delim, 0};
        for (int i = 0; i < (int)vector.size(); i++) {
            retValue += vector[i];
            if (i < (int)vector.size()-1) {
                retValue += std::u16string(delimStr);
            }
        }
        return retValue;
    }
}
}