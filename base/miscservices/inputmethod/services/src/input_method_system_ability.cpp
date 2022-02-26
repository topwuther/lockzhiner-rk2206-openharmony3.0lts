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

#include "input_method_system_ability.h"
#include "message_handler.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "ipc_skeleton.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    REGISTER_SYSTEM_ABILITY_BY_ID(InputMethodSystemAbility, INPUT_METHOD_SYSTEM_ABILITY_ID, true);
    const std::int32_t INIT_INTERVAL = 10000L;
    std::mutex InputMethodSystemAbility::instanceLock_;
    sptr<InputMethodSystemAbility> InputMethodSystemAbility::instance_;

    std::shared_ptr<AppExecFwk::EventHandler> InputMethodSystemAbility::serviceHandler_;

    /**
     * constructor
     * @param systemAbilityId
     * @param runOnCreate
     */
    InputMethodSystemAbility::InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate)
        : SystemAbility(systemAbilityId, runOnCreate), state_(ServiceRunningState::STATE_NOT_START)
    {
    }

    /**
     * constructor
     */
    InputMethodSystemAbility::InputMethodSystemAbility() : state_(ServiceRunningState::STATE_NOT_START)
    {
    }

    /**
     * Destructor
     */
    InputMethodSystemAbility::~InputMethodSystemAbility()
    {
        if (workThreadHandler.joinable()) {
            workThreadHandler.join();
        }

        std::map<int32_t, PerUserSession*>::const_iterator it;
        for (it = userSessions.cbegin(); it != userSessions.cend();) {
            PerUserSession *session = it->second;
            it = userSessions.erase(it);
            delete session;
            session = nullptr;
        }
        userSessions.clear();
        std::map<int32_t, PerUserSetting*>::const_iterator it1;
        for (it1 = userSettings.cbegin(); it1 != userSettings.cend();) {
            PerUserSetting *setting = it1->second;
            it1 = userSettings.erase(it1);
            delete setting;
            setting = nullptr;
        }
        userSettings.clear();
        std::map<int32_t, MessageHandler*>::const_iterator it2;
        for (it2 = msgHandlers.cbegin(); it2 != msgHandlers.cend();) {
            MessageHandler *handler = it2->second;
            it2 = msgHandlers.erase(it2);
            delete handler;
            handler = nullptr;
        }
        msgHandlers.clear();
    }

    sptr<InputMethodSystemAbility> InputMethodSystemAbility::GetInstance()
    {
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> autoLock(instanceLock_);
            if (instance_ == nullptr) {
                instance_ = new InputMethodSystemAbility;
            }
        }
        return instance_;
    }

    void InputMethodSystemAbility::OnStart()
    {
        IMSA_HILOGI("Enter OnStart.");
        if (state_ == ServiceRunningState::STATE_RUNNING) {
            IMSA_HILOGI("ImsaService is already running.");
            return;
        }
        Initialize();
        InitServiceHandler();
        if (Init() != ErrorCode::NO_ERROR) {
            auto callback = [=]() { Init(); };
            serviceHandler_->PostTask(callback, INIT_INTERVAL);
            IMSA_HILOGE("Init failed. Try again 10s later");
            return;
        }

        IMSA_HILOGI("Start ImsaService ErrorCode::NO_ERROR.");
        return;
    }

    int32_t InputMethodSystemAbility::Init()
    {
        bool ret = Publish(InputMethodSystemAbility::GetInstance());
        if (!ret) {
            IMSA_HILOGE("Publish failed.");
            return -1;
        }
        IMSA_HILOGI("Publish ErrorCode::NO_ERROR.");
        state_ = ServiceRunningState::STATE_RUNNING;
        return ErrorCode::NO_ERROR;
    }

    void InputMethodSystemAbility::OnStop()
    {
        IMSA_HILOGI("OnStop started.");
        if (state_ != ServiceRunningState::STATE_RUNNING) {
            return;
        }
        serviceHandler_ = nullptr;

        state_ = ServiceRunningState::STATE_NOT_START;
        IMSA_HILOGI("OnStop end.");
    }

    void InputMethodSystemAbility::InitServiceHandler()
    {
        IMSA_HILOGI("InitServiceHandler started.");
        if (serviceHandler_ != nullptr) {
            IMSA_HILOGI("InitServiceHandler already init.");
            return;
        }
        std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("InputMethodSystemAbility");
        serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

        IMSA_HILOGI("InitServiceHandler succeeded.");
    }

    /*! Initialization of Input method management service
    \n It's called after the service starts, before any transaction.
    */
    void InputMethodSystemAbility::Initialize()
    {
        IMSA_HILOGI("InputMethodSystemAbility::Initialize");
        // init work thread to handle the messages
        workThreadHandler = std::thread([this] {
            WorkThread();
        });
        PerUserSetting *setting = new PerUserSetting(0);
        PerUserSession *session = new PerUserSession(0);
        userSettings.insert(std::pair<int32_t, PerUserSetting*>(0, setting));
        userSessions.insert(std::pair<int32_t, PerUserSession*>(0, session));

        setting->Initialize();
    }

    /*! Get the state of user
    \n This API is added for unit test.
    \param userID the id of given user
    \return user state can be one of the values of UserState
    */
    int32_t InputMethodSystemAbility::GetUserState(int32_t userId)
    {
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr) {
            return UserState::USER_STATE_NOT_AVAILABLE;
        }
        return setting->GetUserState();
    }

    /*! Handle the transaction from the remote binder
    \n Run in binder thread
    \param code transaction code number
    \param data the params from remote binder
    \param[out] reply the result of the transaction replied to the remote binder
    \param flags the flags of handling transaction
    \return int32_t
    */
    int32_t InputMethodSystemAbility::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option)
    {
        return InputMethodSystemAbilityStub::OnRemoteRequest(code, data, reply, option);
    }

    /*! Get the display mode of keyboard
    \n Run in binder thread
    \param[out] retMode the display mode returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::getDisplayMode(int32_t retMode)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        retMode = GetUserSession(userId)->GetDisplayMode();
        return ErrorCode::NO_ERROR;
    }

    /*! Get the window height of the current keyboard
    \n Run in binder thread
    \param[out] retHeight the window height returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::getKeyboardWindowHeight(int32_t retHeight)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        int32_t ret = GetUserSession(userId)->GetKeyboardWindowHeight(retHeight);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGE("Failed to get keyboard window height. ErrorCode=%d\n", ret);
        }
        return ret;
    }

    /*! Get the  current keyboard type
    \n Run in binder thread
    \param[out] retType the current keyboard type returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_NULL_POINTER current keyboard type is null
    */
    int32_t InputMethodSystemAbility::getCurrentKeyboardType(KeyboardType *retType)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        PerUserSession *userSession = GetUserSession(userId);
        if (userSession == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        KeyboardType *type = userSession->GetCurrentKeyboardType();
        if (type == nullptr) {
            return ErrorCode::ERROR_NULL_POINTER;
        }
        *retType = *type;
        return ErrorCode::NO_ERROR;
    }

    /*! Get the enabled input method engine list
    \n Run in binder thread
    \param[out] properties input method engine list returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::listInputMethodEnabled(std::vector<InputMethodProperty*> *properties)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        setting->ListInputMethodEnabled(properties);

        std::vector<InputMethodProperty*>::iterator it;
        for (it = properties->begin(); it != properties->end();) {
            if (*it && (*it)->isSystemIme) {
                it = properties->erase(it);
            } else {
                ++it;
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Get all of the input method engine list installed in the system
    \n Run in binder thread
    \param[out] properties input method engine list returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::listInputMethod(std::vector<InputMethodProperty*> *properties)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        setting->ListInputMethod(properties);
        std::vector<InputMethodProperty*>::iterator it;
        for (it = properties->begin(); it != properties->end();) {
            if (*it && (*it)->isSystemIme) {
                it = properties->erase(it);
            } else {
                ++it;
            }
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Get the keyboard type list for the given input method engine
    \n Run in binder thread
    \param imeId the id of the given input method engine
    \param[out] types keyboard type list returned to the caller
    \return ErrorCode::NO_ERROR no error
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::listKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types)
    {
        int32_t uid = IPCSkeleton::GetCallingUid();
        int32_t userId = getUserId(uid);
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("%s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        return setting->ListKeyboardType(imeId, types);
    }

    /*! Get the instance of PerUserSetting for the given user
    \param userId the user id of the given user
    \return a pointer of the instance if the user is found
    \return null if the user is not found
    */
    PerUserSetting *InputMethodSystemAbility::GetUserSetting(int32_t userId)
    {
        std::map<int32_t, PerUserSetting*>::iterator it = userSettings.find(userId);
        if (it == userSettings.end()) {
            return nullptr;
        }
        return it->second;
    }

    /*! Get the instance of PerUserSession for the given user
    \param userId the user id of the given user
    \return a pointer of the instance if the user is found
    \return null if the user is not found
    */
    PerUserSession *InputMethodSystemAbility::GetUserSession(int32_t userId)
    {
        std::map<int32_t, PerUserSession*>::iterator it = userSessions.find(userId);
        if (it == userSessions.end()) {
            return nullptr;
        }
        return it->second;
    }

    /*! Work Thread of input method management service
    \n Remote commands which may change the state or data in the service will be handled sequentially in this thread.
    */
    void InputMethodSystemAbility::WorkThread()
    {
        while (1) {
            Message *msg = MessageHandler::Instance()->GetMessage();
            switch (msg->msgId_) {
                case MSG_ID_USER_START : {
                    OnUserStarted(msg);
                    break;
                }
                case MSG_ID_USER_STOP: {
                    OnUserStopped(msg);
                    break;
                }
                case MSG_ID_USER_UNLOCK: {
                    OnUserUnlocked(msg);
                    break;
                }
                case MSG_ID_USER_LOCK : {
                    OnUserLocked(msg);
                    break;
                }
                case MSG_ID_PACKAGE_ADDED: {
                    OnPackageAdded(msg);
                    break;
                }
                case MSG_ID_PACKAGE_REMOVED: {
                    OnPackageRemoved(msg);
                    break;
                }
                case MSG_ID_SETTING_CHANGED: {
                    OnSettingChanged(msg);
                    break;
                }
                case MSG_ID_PREPARE_INPUT: {
                    OnPrepareInput(msg);
                    break;
                }
                case MSG_ID_RELEASE_INPUT:
                case MSG_ID_START_INPUT:
                case MSG_ID_STOP_INPUT:
                case MSG_ID_SET_INPUT_METHOD_CORE:
                case MSG_ID_HIDE_KEYBOARD_SELF:
                case MSG_ID_SET_DISPLAY_MODE:
                case MSG_ID_CLIENT_DIED:
                case MSG_ID_IMS_DIED:
                case MSG_ID_RESTART_IMS: {
                    OnHandleMessage(msg);
                    break;
                }
                case MSG_ID_DISABLE_IMS: {
                    OnDisableIms(msg);
                    break;
                }
                case MSG_ID_ADVANCE_TO_NEXT: {
                    OnAdvanceToNext(msg);
                    break;
                }
                case MSG_ID_EXIT_SERVICE: {
                    std::map<int32_t, MessageHandler*>::const_iterator it;
                    for (it = msgHandlers.cbegin(); it != msgHandlers.cend();) {
                        MessageHandler *handler = it->second;
                        Message *destMsg = new Message(MSG_ID_EXIT_SERVICE, nullptr);
                        handler->SendMessage(destMsg);
                        PerUserSession *userSession = GetUserSession(it->first);
                        if (userSession == nullptr) {
                            IMSA_HILOGE("getUserSession fail.");
                            return;
                        }
                        userSession->JoinWorkThread();
                        it = msgHandlers.erase(it);
                        delete handler;
                        handler = nullptr;
                    }
                    delete msg;
                    msg = nullptr;
                    return;
                }
                default: {
                    break;
                }
            }
        }
    }

    /*! Called when a user is started. (EVENT_USER_STARTED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserStarted(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (msg->msgContent_ == nullptr) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting != nullptr) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_ALREADY_STARTED), userId);
            return ErrorCode::ERROR_USER_ALREADY_STARTED;
        }

        setting = new PerUserSetting(userId);
        PerUserSession *session = new PerUserSession(userId);

        userSettings.insert(std::pair<int32_t, PerUserSetting*>(userId, setting));
        userSessions.insert(std::pair<int32_t, PerUserSession*>(userId, session));
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is stopped. (EVENT_USER_STOPPED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserStopped(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (msg->msgContent_ == nullptr) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        PerUserSession *session = GetUserSession(userId);
        if (setting == nullptr || session == nullptr) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_STARTED), userId);
            return ErrorCode::ERROR_USER_NOT_STARTED;
        }
        if (setting->GetUserState() == UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_LOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_LOCKED;
        }
        std::map<int32_t, PerUserSession*>::iterator itSession = userSessions.find(userId);
        userSessions.erase(itSession);
        delete session;
        session = nullptr;

        std::map<int32_t, PerUserSetting*>::iterator itSetting = userSettings.find(userId);
        userSettings.erase(itSetting);
        delete setting;
        setting = nullptr;
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    int32_t InputMethodSystemAbility::setInputMethodCore(sptr<IInputMethodCore> &core)
    {
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is unlocked. (EVENT_USER_UNLOCKED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserUnlocked(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (msg->msgContent_ == nullptr) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        PerUserSession *session = GetUserSession(userId);
        if (setting == nullptr || session == nullptr) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_STARTED), userId);
            return ErrorCode::ERROR_USER_NOT_STARTED;
        }
        if (setting->GetUserState() == UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_ALREADY_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_ALREADY_UNLOCKED;
        }

        setting->Initialize();

        InputMethodProperty *ime = setting->GetSecurityInputMethod();
        session->SetSecurityIme(ime);
        ime = setting->GetCurrentInputMethod();
        session->SetCurrentIme(ime);
        session->SetInputMethodSetting(setting->GetInputMethodSetting());
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a user is locked. (EVENT_USER_LOCKED is received)
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode
    */
    int32_t InputMethodSystemAbility::OnUserLocked(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        if (msg->msgContent_ == nullptr) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        int32_t userId = msg->msgContent_->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        std::map<int32_t, MessageHandler*>::iterator it = msgHandlers.find(userId);
        if (it != msgHandlers.end()) {
            MessageHandler *handler = it->second;
            Message *destMsg = new Message(MSG_ID_USER_LOCK , nullptr);
            if (destMsg != nullptr) {
                handler->SendMessage(destMsg);
                PerUserSession *userSession = GetUserSession(userId);
                if (userSession != nullptr) {
                    userSession->JoinWorkThread();
                }
                msgHandlers.erase(it);
                delete handler;
                handler = nullptr;
            }
        }
        setting->OnUserLocked();
        IMSA_HILOGI("End...[%d]\n", userId);
        return ErrorCode::NO_ERROR;
    }

    /*! Prepare input. Called by an input client.
    \n Run in work thread of input method management service
    \param msg the parameters from remote client are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnPrepareInput(Message *msg)
    {
        IMSA_HILOGI("InputMethodSystemAbility::OnPrepareInput");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();

        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("InputMethodSystemAbility::OnPrepareInput errorCode = ErrorCode::ERROR_USER_NOT_UNLOCKED");
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(userId);
        if (it == msgHandlers.end()) {
            PerUserSession *session = GetUserSession(userId);
            MessageHandler *handler = new MessageHandler();
            if (session == nullptr) {
                IMSA_HILOGE("InputMethodSystemAbility::OnPrepareInput session is nullptr");
            }
            session->CreateWorkThread(*handler);
            msgHandlers.insert(std::pair<int32_t, MessageHandler*>(userId, handler));
            it = msgHandlers.find(userId);
        }

        if (it != msgHandlers.end()) {
            MessageHandler *handler = it->second;
            handler->SendMessage(msg);
        }
        return 0;
    }

    /*! Handle message
    \param msgId the id of message to run
    \msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnHandleMessage(Message *msg)
    {
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(userId);
        if (it != msgHandlers.end()) {
            MessageHandler *handler = it->second;
            handler->SendMessage(msg);
        }
        return ErrorCode::NO_ERROR;
    }

    /*! Called when a package is installed.
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnPackageAdded(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();

        if (size <= 0) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string packageName = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        bool securityImeFlag = false;
        int32_t ret = setting->OnPackageAdded(packageName, securityImeFlag);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("End...\n");
            return ret;
        }
        if (securityImeFlag == true) {
            InputMethodProperty *securityIme = setting->GetSecurityInputMethod();
            InputMethodProperty *defaultIme = setting->GetCurrentInputMethod();
            GetUserSession(userId)->ResetIme(defaultIme, securityIme);
        }
        IMSA_HILOGI("End...\n");
        return 0;
    }

    /*! Called when a package is removed.
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnPackageRemoved(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        if (data == nullptr) {
            IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved data is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();

        if (size <= 0) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string packageName = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        PerUserSession *session = GetUserSession(userId);
        if (session == nullptr) {
            IMSA_HILOGI("InputMethodSystemAbility::OnPackageRemoved session is nullptr");
            return ErrorCode::ERROR_NULL_POINTER;
        }
        session->OnPackageRemoved(packageName);
        bool securityImeFlag = false;
        int32_t ret = setting->OnPackageRemoved(packageName, securityImeFlag);
        if (ret != ErrorCode::NO_ERROR) {
            IMSA_HILOGI("End...\n");
            return ret;
        }
        if (securityImeFlag == true) {
            InputMethodProperty *securityIme = setting->GetSecurityInputMethod();
            InputMethodProperty *defaultIme = setting->GetCurrentInputMethod();
            GetUserSession(userId)->ResetIme(defaultIme, securityIme);
        }
        return 0;
    }

    /*! Called when input method setting data is changed.
    \n Run in work thread of input method management service
    \param msg the parameters from remote binder are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    \return ErrorCode::ERROR_BAD_PARAMETERS bad parameter
    */
    int32_t InputMethodSystemAbility::OnSettingChanged(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        int32_t size = data->ReadInt32();
        if (size < 2) {
            IMSA_HILOGE("Aborted! %s\n", ErrorCode::ToString(ErrorCode::ERROR_BAD_PARAMETERS));
            return ErrorCode::ERROR_BAD_PARAMETERS;
        }
        std::u16string updatedKey = data->ReadString16();
        std::u16string updatedValue = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        PerUserSession *session = GetUserSession(userId);
        int32_t ret = session->OnSettingChanged(updatedKey, updatedValue);
        if (ret == ErrorCode::ERROR_SETTING_SAME_VALUE) {
            IMSA_HILOGI("End...No need to update\n");
            return ret;
        }

        // PerUserSetting does not need handle keyboard type change notification
        if (updatedKey == InputMethodSetting::CURRENT_KEYBOARD_TYPE_TAG ||
            updatedKey == InputMethodSetting::CURRENT_SYS_KEYBOARD_TYPE_TAG) {
            IMSA_HILOGI("End...\n");
            return ErrorCode::NO_ERROR;
        }

        ret = setting->OnSettingChanged(updatedKey, updatedValue);
        if (ret != 0) {
            IMSA_HILOGI("End...No need to update\n");
            return ret;
        }

        InputMethodProperty *securityIme = setting->GetSecurityInputMethod();
        InputMethodProperty *defaultIme = setting->GetCurrentInputMethod();
        session->ResetIme(defaultIme, securityIme);
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }

    /*! Disable input method service. Called from PerUserSession module
    \n Run in work thread of input method management service
    \param msg the parameters are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnDisableIms(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        std::u16string imeId = data->ReadString16();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }

        InputMethodSetting tmpSetting;
        std::u16string key = InputMethodSetting::ENABLED_INPUT_METHODS_TAG;
        tmpSetting.SetValue(key, setting->GetInputMethodSetting()->GetValue(key));
        tmpSetting.RemoveEnabledInputMethod(imeId);
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }

    /*! Switch to next ime or next keyboard type. It's called by input method service
    \n Run in work thread of input method management service or the work thread of PerUserSession
    \param msg the parameters from remote binder are saved in msg->msgContent_
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_USER_NOT_UNLOCKED user not unlocked
    */
    int32_t InputMethodSystemAbility::OnAdvanceToNext(const Message *msg)
    {
        IMSA_HILOGI("Start...\n");
        MessageParcel *data = msg->msgContent_;
        int32_t userId = data->ReadInt32();
        bool isCurrentIme = data->ReadBool();
        PerUserSetting *setting = GetUserSetting(userId);
        if (setting == nullptr || setting->GetUserState() != UserState::USER_STATE_UNLOCKED) {
            IMSA_HILOGE("Aborted! %s %d\n", ErrorCode::ToString(ErrorCode::ERROR_USER_NOT_UNLOCKED), userId);
            return ErrorCode::ERROR_USER_NOT_UNLOCKED;
        }
        if (isCurrentIme) {
            std::map<int32_t, MessageHandler*>::const_iterator it = msgHandlers.find(userId);
            if (it != msgHandlers.end()) {
                Message *destMsg = new Message(msg->msgId_, nullptr);
                it->second->SendMessage(destMsg);
            }
        } else {
            setting->OnAdvanceToNext();
        }
        IMSA_HILOGI("End...\n");
        return ErrorCode::NO_ERROR;
    }
}
}