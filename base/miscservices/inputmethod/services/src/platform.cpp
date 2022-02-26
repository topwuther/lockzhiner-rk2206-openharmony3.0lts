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

#include "global.h"
#include "platform.h"
#include "platform_callback_stub.h"

namespace OHOS {
namespace MiscServices {
    void Platform::SetPlatform(const sptr < IPlatformApi >& platformApi)
    {
        this->platformApi = platformApi;
        sptr < IPlatformCallback > cb = new PlatformCallbackStub();
        this->platformApi->registerCallback(cb);
    }

    /*! Constructor
    */
    Platform::Platform()
    {
    }

    /*! Destructor
    */
    Platform::~Platform()
    {
    }

    /*! Single instance exists in the service
    */
    Platform *Platform::Instance()
    {
        static Platform *platform = nullptr;
        if (platform == nullptr) {
            platform = new Platform();
        }
        return platform;
    }

    /*! Start an input method service
      \param userId the id of the given user.
      \param packageName the packageName of the given input method engine which is going to start
      \param intention the intention to start the service
      \return the remote object handler of started input method service.
    */
    sptr<IInputMethodCore> Platform::BindInputMethodService(int userId, const std::u16string& packageName,
                                                            const std::u16string& intention)
    {
        if (platformApi == nullptr) {
            return nullptr;
        }
        return platformApi->bindInputMethodService(packageName, intention, userId);
    }

    /*! Stop an input method service
      \param userId the id of the given user.
      \param packageName the packageName of the given input method engine which is going to stop
      \return ErrorCode
    */
    int Platform::UnbindInputMethodService(int userId, const std::u16string& packageName)
    {
        if (platformApi == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return platformApi->unbindInputMethodService(userId, packageName);
    }

    /*! Create window token
      \param userId the id of the given user.
      \param displayId the id of display screen
      \param packageName the packageName of the given input method engine
      \return ErrorCode
    */
    sptr < IRemoteObject > Platform::CreateWindowToken(int userId, int displayId, const std::u16string& packageName)
    {
        if (platformApi == nullptr) {
            return nullptr;
        }
        return platformApi->createWindowToken(userId, displayId, packageName);
    }

    /*! Destroy window token
      \param userId the id of the given user.
      \param packageName the packageName of the given input method engine
      \return ErrorCode
    */
    int Platform::DestroyWindowToken(int userId, const std::u16string& packageName)
    {
        if (platformApi == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return platformApi->destroyWindowToken(userId, packageName);
    }

    /*! Get all the installed input method engines for the given user
      \param userId the id of the given user.
      \param[out] inputMethodProperties the input method engine list installed in the system for the given user
      \return ErrorCode
    */
    int Platform::ListInputMethod(int userId, std::vector<InputMethodProperty*> *inputMethodProperties)
    {
        return 0;
    }

    /*! Get input method engine information of the given package for the given user
      \param userId the id of the given user.
      \param packageName the given package name for which to get input method engine information
      \param[out] inputMethodProperty the input method engine information for the given package
      \return ErrorCode
    */
    int Platform::GetInputMethodProperty(int userId, const std::u16string& packageName,
                                         InputMethodProperty *inputMethodProperty)
    {
        if (platformApi == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return platformApi->getInputMethodProperty(userId, packageName, inputMethodProperty);
    }

    /*! Get the input method setting data for the given user
      \param userId the id of the given user
      \param[out] inputMethodSetting the input method setting data for the given user
      \return ErrorCode
    */

    int Platform::GetInputMethodSetting(int userId, InputMethodSetting *inputMethodSetting)
    {
        return 0;
    }

    /*! Set the input method setting data for the given user
      \param userId the id of the given user
      \param inputMethodSetting the input method setting data for the given user
      \return ErrorCode
    */
    int Platform::SetInputMethodSetting(int userId, const InputMethodSetting& inputMethodSetting)
    {
        if (platformApi == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        return platformApi->setInputMethodSetting(userId, inputMethodSetting);
    }

    /*! Check if a physical keyboard is connected to the system
      \return true - a physical keyboard is connected
      \n      false - no physical keyboard
    */
    bool Platform::CheckPhysicalKeyboard()
    {
        return true;
    }

    /*! Check if the remote caller is from a valid window
      \return true - the remote caller is from a valid window
      \n      false - the remote caller is not from a valid window
    */
    bool Platform::IsValidWindow(int uid, int pid, int displayId)
    {
        (void)uid;
        (void)pid;
        (void)displayId;
        return true;
    }

    /*! Check if the remote caller is from a focused window
      \return true - the remote caller is from a focused window
      \n      false - the remote caller is not from a focused window
    */
    bool Platform::IsWindowFocused(int uid, int pid, int displayId)
    {
        (void)uid;
        (void)pid;
        (void)displayId;
        return true;
    }
}
}