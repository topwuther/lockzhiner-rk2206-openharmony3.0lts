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
 
#include "unistd.h"   //usleep
#include "peruser_setting.h"
#include "platform.h"
#include "utils.h"
namespace OHOS {
namespace MiscServices {
    /*! Constructor
    \param userId the id number of this user
    */
    PerUserSetting::PerUserSetting(int userId)
    {
        userId_ = userId;
        currentImeId = Utils::to_utf16("");
        userState = UserState::USER_STATE_STARTED;
    }

    /*! Destructor
    */
    PerUserSetting::~PerUserSetting()
    {
        if (userState == UserState::USER_STATE_UNLOCKED) {
            OnUserLocked();
        }
    }

    /* Initialize data for this user.
     * It's called when this user is unlocked. The work includes:
     * read all installed input method engine information from the system
     * read input method setting data from the system
     */
    void PerUserSetting::Initialize()
    {
        userState = UserState::USER_STATE_UNLOCKED;

        inputMethodProperties.clear();
        int ret = Platform::Instance()->ListInputMethod(userId_, &inputMethodProperties);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("Failed to listInputMethod [%d]\n", userId_);
        }
        int size = inputMethodProperties.size();
        if (size == 0) {
            currentImeId = Utils::to_utf16("");
        }

        ret = Platform::Instance()->GetInputMethodSetting(userId_, &inputMethodSetting);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("Failed to getInputMethodSetting [%d]\n", userId_);
        } else {
            if (size > 0) {
                InitInputMethodSetting();
            }
        }
    }

    /*! Add an input method engine.
    \n It's called when a package is installed in the system.
    \param packageName the package name of installed package.
    \param[out] isSecurityIme check if the added ime is a security ime.
    \return ErrorCode::NO_ERROR The installed package is an IME package, and is added in the input method management system
    \return ErrorCode::ERROR_NOT_IME_PACKAGE The installed package is not an IME package.
    \return ErrorCode::ERROR_IME_PACKAGE_DUPLICATED The installed package is duplicated.
    */
    int PerUserSetting::OnPackageAdded(std::u16string& packageName, bool isSecurityIme)
    {
        if (isSecurityIme) {
            isSecurityIme = false;
        }
        std::u16string imeId = GetImeId(packageName);
        if (imeId.size() != 0) {
            IMSA_HILOGI("%s [%d]\n", ErrorCode::ToString(ErrorCode::ERROR_IME_PACKAGE_DUPLICATED), userId_);
            return ErrorCode::ERROR_IME_PACKAGE_DUPLICATED;
        }
        // retake the input method list installed in the system.
        InputMethodProperty *property = new InputMethodProperty();
        int ret = Platform::Instance()->GetInputMethodProperty(userId_, packageName, property);
        if (ret != ErrorCode::NO_ERROR) {
            delete property;
            property = nullptr;
            IMSA_HILOGI("%s [%d]\n", ErrorCode::ToString(ErrorCode::ERROR_NOT_IME_PACKAGE), userId_);
            return ErrorCode::ERROR_NOT_IME_PACKAGE;
        }
        inputMethodProperties.push_back(property);
        if (CheckIfSecurityIme(*property)) {
            if (isSecurityIme) {
                isSecurityIme = true;
            }
            return ErrorCode::NO_ERROR;
        }

        std::vector<int> types;
        for (int i = 0; i < (int)property->mTypes.size(); i++) {
            types.push_back(property->mTypes[i]->getHashCode());
        }

        InputMethodSetting imSetting;
        std::u16string key = InputMethodSetting::ENABLED_INPUT_METHODS_TAG;
        imSetting.SetValue(key, inputMethodSetting.GetValue(key));
        imSetting.AddEnabledInputMethod(property->mImeId, types);
        types.clear();
        Platform::Instance()->SetInputMethodSetting(userId_, imSetting);
        return ErrorCode::NO_ERROR;
    }

    /*! Remove an input method engine.
    \n It's called when a package is removed from the system.
    \param packageName the package name of installed package.
    \param[out] isSecurityIme check if the removed ime is a security ime.
    \return ErrorCode::NO_ERROR The removed package is an IME package, and is removed from the input method management system
    \return ErrorCode::ERROR_NOT_IME_PACKAGE The removed package is not an IME package.
    */
    int PerUserSetting::OnPackageRemoved(std::u16string& packageName, bool isSecurityIme)
    {
        if (isSecurityIme) {
            isSecurityIme = false;
        }
        std::u16string imeId = GetImeId(packageName);
        if (imeId.size() == 0) {
            IMSA_HILOGI("%s [%d]\n", ErrorCode::ToString(ErrorCode::ERROR_NOT_IME_PACKAGE), userId_);
            return ErrorCode::ERROR_NOT_IME_PACKAGE;
        }
        bool securityFlag = false;
        std::vector<InputMethodProperty*>::iterator it;
        for (it = inputMethodProperties.begin(); it < inputMethodProperties.end(); ++it) {
            InputMethodProperty *node = (InputMethodProperty*)*it;
            if (node->mImeId == imeId) {
                if (CheckIfSecurityIme(*node) == true) {
                    securityFlag = true;
                }
                inputMethodProperties.erase(it);
                delete node;
                node = nullptr;
                break;
            }
        }
        if (securityFlag) {
            if (isSecurityIme) {
                isSecurityIme = true;
            }
            return ErrorCode::NO_ERROR;
        }

        InputMethodSetting imSetting;
        std::u16string key = InputMethodSetting::ENABLED_INPUT_METHODS_TAG;
        imSetting.SetValue(key, inputMethodSetting.GetValue(key));

        int flag = imSetting.RemoveEnabledInputMethod(imeId);
        if (flag == false) {
            IMSA_HILOGI("The package removed is not an enabled IME. [%d]\n", userId_);
            return ErrorCode::NO_ERROR;
        }
        Platform::Instance()->SetInputMethodSetting(userId_, imSetting);
        // wait for some time so that the setting change will not be overrided by the followed transact
        int32_t sleepTime = 100000;
        usleep(sleepTime);
        return ErrorCode::NO_ERROR;
    }

    /*! Update input method setting data.
    \n It's called when the input method setting data in the system is changed.
    \param key the name of setting item
    \param value the value of the setting item
    \return ErrorCode::NO_ERROR update the setting data to input method management system.
    \return ErrorCode::ERROR_SETTING_SAME_VALUE the current value is same as the one in the system.
    */
    int PerUserSetting::OnSettingChanged(const std::u16string& key, const std::u16string& value)
    {
        std::u16string currentValue = inputMethodSetting.GetValue(key);

        if (currentValue == value) {
            return ErrorCode::ERROR_SETTING_SAME_VALUE;
        }

        inputMethodSetting.SetValue(key, value);

        if (key == InputMethodSetting::CURRENT_INPUT_METHOD_TAG) {
            currentImeId = inputMethodSetting.GetCurrentInputMethod();
        } else if (key == InputMethodSetting::ENABLED_INPUT_METHODS_TAG) {
            if ((currentImeId.size() > 0 && value.find(currentImeId) == std::string::npos) ||
                currentImeId.size() == 0) {
                ResetCurrentInputMethod();
                InputMethodSetting tmpSetting;
                tmpSetting.ClearData();
                tmpSetting.SetCurrentInputMethod(currentImeId);
                tmpSetting.SetCurrentKeyboardType(-1);
                Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Switch to the next input method service.
    */
    void PerUserSetting::OnAdvanceToNext()
    {
        bool flag = false;
        std::u16string enabledInputMethods = inputMethodSetting.GetValue(InputMethodSetting::ENABLED_INPUT_METHODS_TAG);
        std::u16string imeId;
        std::u16string nextImeId = Utils::to_utf16("");
        InputMethodProperty *firstEnabledProperty = nullptr;
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            imeId = inputMethodProperties[i]->mImeId;
            if (imeId == currentImeId) {
                flag = true;
            } else if (enabledInputMethods.find(imeId) != std::string::npos) {
                if (flag == true) {
                    nextImeId = imeId;
                    break;
                } else if (firstEnabledProperty == nullptr) {
                    firstEnabledProperty = inputMethodProperties[i];
                }
            }
        }

        if (nextImeId.size() == 0 && firstEnabledProperty) {
            nextImeId = firstEnabledProperty->mImeId;
        }

        // next enabled ime is not available.
        if (nextImeId.size() == 0) {
            IMSA_HILOGW("No next IME is available. [%d]\n", userId_);
            return;
        }

        InputMethodSetting tmpSetting;
        tmpSetting.SetCurrentInputMethod(nextImeId);
        tmpSetting.SetCurrentKeyboardType(-1);
        Platform::Instance()->SetInputMethodSetting(userId_, tmpSetting);
    }

    /* It's Called when this user is locked.
     * Release data for this user including:
     * release input method engine information
     * release input method setting data
     */
    void PerUserSetting::OnUserLocked()
    {
        if (userState == UserState::USER_STATE_STARTED) {
            return;
        }
        userState = UserState::USER_STATE_STARTED;
        currentImeId = Utils::to_utf16("");

        // release input method properties
        std::vector<InputMethodProperty*>::iterator it;
        for (it = inputMethodProperties.begin(); it < inputMethodProperties.end();) {
            InputMethodProperty *node = (InputMethodProperty*)*it;
            if (node != nullptr) {
                it = inputMethodProperties.erase(it);
                delete node;
                node = nullptr;
            }
        }
        inputMethodProperties.clear();
        // release input method setting.
        inputMethodSetting.ClearData();
    }

    /* Print the related information for this user into the given stream
     * The information includes:
     * The user id and user state
     * The information of all input method engine installed in the system
     * The input method setting data of this user.
     * param fd the raw file descriptor that the dump is being sent to
     */
    void PerUserSetting::Dump(int fd)
    {
        int size = inputMethodProperties.size();
        dprintf(fd, "\n - User Setting State :\n");
        dprintf(fd, " * Installed IME count = %d\n", size);
        std::vector<std::u16string> imeList = inputMethodSetting.GetEnabledInputMethodList();
        size = imeList.size();
        dprintf(fd, "\n * Enabled IME count : %d\n", size);
        for (int i = 0; i < size; i++) {
            dprintf(fd, "  [%d] ImeId = %s\n", i, Utils::to_utf8(imeList[i]).c_str());
            std::vector<int> hashCodeList = inputMethodSetting.GetEnabledKeyboardTypes(imeList[i]);
            dprintf(fd, "      Enabled keyboard count = %d, hashcode list : ", hashCodeList.size());
            for (int j = 0; j < (int)hashCodeList.size(); j++) {
                dprintf(fd, "%d", hashCodeList[j]);
                if (j < (int)hashCodeList.size()-1) {
                    dprintf(fd, ", ");
                }
            }
            dprintf(fd, "\n");
            hashCodeList.clear();
        }
        imeList.clear();
    }

    /*! Get the state of this user
    \return UserState::USER_STATE_STARTED user is started
    \return UserState::USER_STATE_UNLOCKED user is unlocked
    */
    int PerUserSetting::GetUserState()
    {
        return userState;
    }

    /*! Get the current IME
    \return a pointer of InputMethodProperty when an IME is available.
    \return null when there is no enabled IME in the system.
    \note The returned pointer should NOT be freed by caller
    */
    InputMethodProperty *PerUserSetting::GetCurrentInputMethod()
    {
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            if (currentImeId == inputMethodProperties[i]->mImeId) {
                return inputMethodProperties[i];
            }
        }
        // if default ime is null, we use security ime as default ime.
        return GetSecurityInputMethod();
    }

    /*! Get the system security IME
    \return a pointer of InputMethodProperty when an system security IME is available.
    \return null when there is no security IME in the system.
    \note The returned pointer should NOT be freed by caller
    */
    InputMethodProperty *PerUserSetting::GetSecurityInputMethod()
    {
        InputMethodProperty *ime = nullptr;
        std::u16string systemLocales = inputMethodSetting.GetValue(InputMethodSetting::SYSTEM_LOCALE_TAG);
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            InputMethodProperty *imp = inputMethodProperties[i];
            if (CheckIfSecurityIme(*imp) == false) {
                continue;
            }
            // if systemLocales is not setting, return the first security ime
            if (systemLocales.size() == 0) {
                return imp;
            }
            if (ime == nullptr) {
                ime = imp;
            }
            for (int j = 0; j < (int)inputMethodProperties[i]->mTypes.size(); j++) {
                std::u16string language = inputMethodProperties[i]->mTypes[j]->getLanguage();
                // if the keyboard language is in the list of system locales, return the ime
                if (systemLocales.find(language) != std::string::npos) {
                    return imp;
                }
            }
        }
        return ime;
    }

    /*! Get the next enabled IME
    \return a pointer of InputMethodProperty when the next enabled IME is available.
    \return null when the next enabled IME is not available.
    \note The returned pointer should NOT be freed by caller
    */
    InputMethodProperty *PerUserSetting::GetNextInputMethod()
    {
        bool flag = false;
        std::u16string enabledInputMethods = inputMethodSetting.GetValue(InputMethodSetting::ENABLED_INPUT_METHODS_TAG);
        std::u16string imeId;
        InputMethodProperty *firstEnabledProperty = nullptr;
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            imeId = inputMethodProperties[i]->mImeId;
            if (imeId == currentImeId) {
                flag = true;
            } else if (enabledInputMethods.find(imeId) != std::string::npos) {
                if (flag == true) {
                    return inputMethodProperties[i];
                } else if (firstEnabledProperty == nullptr) {
                    firstEnabledProperty = inputMethodProperties[i];
                }
            }
        }
        return firstEnabledProperty;
    }

    /*! Get the input method setting data
    \return a pointer of InputMethodSetting.
    \note The returned pointer should NOT be freed by caller
    */
    InputMethodSetting *PerUserSetting::GetInputMethodSetting()
    {
        return &inputMethodSetting;
    }

    /*! list the details of all the enabled input method engine
    \param[out] properties the details will be written to the param properties
    \return ErrorCode::NO_ERROR
    */
    int32_t PerUserSetting::ListInputMethodEnabled(std::vector<InputMethodProperty*> *properties)
    {
        std::u16string enabledInputMethods = inputMethodSetting.GetValue(InputMethodSetting::ENABLED_INPUT_METHODS_TAG);
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            if (enabledInputMethods.find(inputMethodProperties[i]->mImeId) != std::string::npos) {
                properties->push_back(inputMethodProperties[i]);
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! List the details of all input method engine installed in the system
    \param[out] properties the details will be written to the param properties
    \return ErrorCode::NO_ERROR
    */
    int32_t PerUserSetting::ListInputMethod(std::vector<InputMethodProperty*> *properties)
    {
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            properties->push_back(inputMethodProperties[i]);
        }
        return ErrorCode::NO_ERROR;
    }

    /*! List the keyboard types of given input method engine
    \param imeId the id of the given IME
    \param[out] types the data of type list of the given IME will be written to types
    \return ErrorCode::NO_ERROR
    */
    int32_t PerUserSetting::ListKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types)
    {
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            if (imeId == inputMethodProperties[i]->mImeId) {
                for (int j = 0; j < (int)inputMethodProperties[i]->mTypes.size(); j++) {
                    types->push_back(inputMethodProperties[i]->mTypes[j]);
                }
                break;
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Get the input method engine details of the given imeId
    \param imeId the id of the given IME
    \return a pointer of InputMethodProperty when the given IME exists.
    \return null when the given IME is not available
    \note the returned pointer should not be freed by the caller.
    */
    InputMethodProperty *PerUserSetting::GetInputMethodProperty(const std::u16string& imeId)
    {
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            if (inputMethodProperties[i]->mImeId == imeId) {
                return inputMethodProperties[i];
            }
        }
        return nullptr;
    }

    /*! Get the language of keyboard type according to the given hashCode
    \param property a pointer to an IME
    \param hashCode the given hashCode
    \return language value when the given hashCode is found
    \return an empty string when the given hashCode is not found
    */
    std::u16string PerUserSetting::GetKeyboardTypeLanguage(const InputMethodProperty *property, int hashCode)
    {
        for (int i = 0; i < (int)property->mTypes.size(); i++) {
            if (property->mTypes[i]->getHashCode() == hashCode) {
                return property->mTypes[i]->getLanguage();
            }
        }
        return Utils::to_utf16("");
    }

    /*! Init input method setting data
    */
    void PerUserSetting::InitInputMethodSetting()
    {
        bool flag = inputMethodSetting.FindKey(InputMethodSetting::ENABLED_INPUT_METHODS_TAG);
        if (flag == false) {
            for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
                if (CheckIfSecurityIme(*inputMethodProperties[i]) == true) {
                    continue;
                }
                std::vector<int> types;
                for (int j = 0; j < (int)inputMethodProperties[i]->mTypes.size(); j++) {
                    types.push_back(inputMethodProperties[i]->mTypes[j]->getHashCode());
                }
                inputMethodSetting.AddEnabledInputMethod(inputMethodProperties[i]->mImeId, types);
                types.clear();
            }
        }

        flag = inputMethodSetting.FindKey(InputMethodSetting::CURRENT_INPUT_METHOD_TAG);
        std::u16string imeId = inputMethodSetting.GetCurrentInputMethod();
        if (flag == false) {
            ResetCurrentInputMethod();
        } else {
            currentImeId = imeId;
        }
        flag = inputMethodSetting.FindKey(InputMethodSetting::CURRENT_SYS_KEYBOARD_TYPE_TAG);
        if (flag == false) {
            inputMethodSetting.SetCurrentSysKeyboardType(-1);
        }
        Platform::Instance()->SetInputMethodSetting(userId_, inputMethodSetting);
    }

    /*! Reset the current (default) input method engine
    \li Look for the first keyboard language which is in the system locale list.
    \li If no keyboard language is in system locale list, we use the first system ime as current ime.
    \li If no system ime is there, we use the first enabled ime as current ime.
    \li If no enabled ime, set current ime as null.
    */
    void PerUserSetting::ResetCurrentInputMethod()
    {
        std::u16string systemLocales = inputMethodSetting.GetValue(InputMethodSetting::SYSTEM_LOCALE_TAG);
        std::u16string enabledInputMethods = inputMethodSetting.GetValue(InputMethodSetting::ENABLED_INPUT_METHODS_TAG);
        std::u16string imeId;
        InputMethodProperty *firstEnabledIme = nullptr;
        bool flag = false;

        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            imeId = inputMethodProperties[i]->mImeId;
            if (enabledInputMethods.find(imeId) == std::string::npos) {
                continue;
            }
            if (firstEnabledIme == nullptr) {
                firstEnabledIme = inputMethodProperties[i];
            }

            std::vector<int> hashCodeList = inputMethodSetting.GetEnabledKeyboardTypes(imeId);
            for (int j = 0; j < (int)hashCodeList.size(); j++) {
                std::u16string language = GetKeyboardTypeLanguage(inputMethodProperties[i], hashCodeList[j]);
                if (systemLocales.find(language) != std::string::npos) {
                    currentImeId = imeId;
                    flag = true;
                    break;
                }
            }
            if (flag) {
                break;
            }
        }

        // if we cannot find any keyboard type which belongs to system locales,
        // we will use the first enabled ime as current ime.
        if (flag == false) {
            if (firstEnabledIme) {
                currentImeId = firstEnabledIme->mImeId;
            } else {
                currentImeId = Utils::to_utf16("");
            }
        }
        inputMethodSetting.SetCurrentInputMethod(currentImeId);
        inputMethodSetting.SetCurrentKeyboardType(-1);
    }

    /*! Get the id of the given input method engine
    \param packageName the packageName of the given IME
    \return the id of the given IME
    */
    std::u16string PerUserSetting::GetImeId(const std::u16string& packageName)
    {
        for (int i = 0; i < (int)inputMethodProperties.size(); i++) {
            if (inputMethodProperties[i]->mPackageName == packageName) {
                return inputMethodProperties[i]->mImeId;
            }
        }
        return Utils::to_utf16("");
    }

    /*! Check if the InputMethodProperty object is a security ime
    \param property the InputMethodProperty object needed to be checked
    \return true - It's a security Ime
    \n      false - It's not a security Ime
    */
    bool PerUserSetting::CheckIfSecurityIme(const InputMethodProperty& property)
    {
        return property.isSystemIme;
    }
}
}