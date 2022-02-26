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

#ifndef FM_IMMS_PROJECT_PERUSERSESSION_H
#define FM_IMMS_PROJECT_PERUSERSESSION_H

#include <thread>
#include <mutex>
#include <map>
#include <memory>

#include "iremote_object.h"
#include "i_input_control_channel.h"
#include "i_input_client.h"
#include "i_input_method_core.h"
#include "i_input_data_channel.h"
#include "i_input_method_agent.h"
#include "input_attribute.h"
#include "input_method_property.h"
#include "input_method_setting.h"
#include "input_control_channel_stub.h"
#include "message.h"
#include "message_handler.h"
#include "global.h"
#include "platform.h"
#include "keyboard_type.h"
#include "ability_manager_interface.h"
#include "ability_connect_callback_proxy.h"
#include "input_method_ability.h"

namespace OHOS {
namespace MiscServices {
    class RemoteObjectDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        RemoteObjectDeathRecipient(int userId, int msgId);
        ~RemoteObjectDeathRecipient();
        void OnRemoteDied(const wptr<IRemoteObject>& who) override;
    private:
        int userId_; // the id of the user to whom the object is linking
        int msgId_; // the message id can be  MessageID::MSG_ID_CLIENT_DIED and MessageID::MSG_ID_IMS_DIED
    };

    /*! \class ClientInfo
    \brief The class defines the details of an input client.
    */
    class ClientInfo {
    public:
        int pid; // the process id of the process in which the input client is running
        int uid; // the uid of the process in which the input client is running
        int userId; // the user if of the user under which the input client is running
        int displayId; // the display id on which the input client is showing
        sptr<IInputClient> client; // the remote object handler for the service to callback to the input client
        sptr<IInputDataChannel> channel; // the remote object handler for IMSA callback to input client
        InputAttribute attribute; // the input attribute of the input client

        ClientInfo(int pid, int uid, int userId, int displayId, const sptr<IInputClient>& client,
                   const sptr<IInputDataChannel>& channel, const InputAttribute& attribute)
        {
            this->pid = pid;
            this->uid = uid;
            this->userId = userId;
            this->displayId = displayId;
            this->client = client;
            this->channel = channel;
            this->attribute = attribute;
        };

        ~ClientInfo()
        {
            this->client = nullptr;
            this->channel = nullptr;
        };
    };

    /*! \class PerUserSession
        \brief The class provides session management in input method management service

        This class manages the sessions between input clients and input method engines for each unlocked user.
    */
    class PerUserSession {
    enum {
        DEFAULT_IME = 0,  // index for default input method service
        SECURITY_IME = 1, // index for security input method service
        MAX_IME = 2, // the maximum count of ims started for a user
    };

    public:
        explicit PerUserSession(int userId);
        ~PerUserSession();

        void SetCurrentIme(InputMethodProperty *ime);
        void SetSecurityIme(InputMethodProperty *ime);
        void SetInputMethodSetting(InputMethodSetting *setting);
        void ResetIme(InputMethodProperty *defaultIme, InputMethodProperty *securityIme);
        void OnPackageRemoved(const std::u16string& packageName);

        int GetDisplayMode();
        int GetKeyboardWindowHeight(int retHeight);
        KeyboardType *GetCurrentKeyboardType();

        int OnSettingChanged(const std::u16string& key, const std::u16string& value);
        void CreateWorkThread(MessageHandler& handler);
        void JoinWorkThread();
        void SetInputMethodAbility(sptr<InputMethodAbility> &inputMethodAbility);
        static void BindInputAbility();
    private:
        int userId_; // the id of the user to whom the object is linking
        int userState; // the state of the user to whom the object is linking
        int displayId; // the id of the display screen on which the user is
        int currentIndex;
        std::map<sptr<IRemoteObject>, ClientInfo*> mapClients;
        int MIN_IME = 2;

        InputMethodProperty *currentIme[MAX_IME]; // 0 - the default ime. 1 - security ime

        InputControlChannelStub *localControlChannel[MAX_IME];
        sptr<IInputControlChannel> inputControlChannel[MAX_IME];
        sptr<IInputMethodCore> imsCore[MAX_IME]; // the remote handlers of input method service
        sptr<IRemoteObject> inputMethodToken[MAX_IME]; // the window token of keyboard
        int currentKbdIndex[MAX_IME]; // current keyboard index
        int lastImeIndex; // The last ime which showed keyboard
        InputMethodSetting *inputMethodSetting; // The pointer referred to the object in PerUserSetting
        int currentDisplayMode; // the display mode of the current keyboard

        sptr<IInputMethodAgent> imsAgent;
        InputChannel *imsChannel; // the write channel created by input method service
        sptr<IInputClient> currentClient; // the current input client
        sptr<IInputClient> needReshowClient; // the input client for which keyboard need to re-show

        sptr<RemoteObjectDeathRecipient> clientDeathRecipient; // remote object death monitor for input client
        sptr<RemoteObjectDeathRecipient> imsDeathRecipient;
        MessageHandler *msgHandler = nullptr; // message handler working with Work Thread
        std::thread workThreadHandler; // work thread handler
        std::mutex mtx; // mutex to lock the operations among multi work threads
        sptr<AAFwk::AbilityConnectionProxy> connCallback;
        sptr<InputMethodAbility> inputMethodAbility_;

        PerUserSession(const PerUserSession&);
        PerUserSession& operator =(const PerUserSession&);
        PerUserSession(const PerUserSession&&);
        PerUserSession& operator =(const PerUserSession&&);
        int IncreaseOrResetImeError(bool resetFlag, int imeIndex);
        KeyboardType *GetKeyboardType(int imeIndex, int typeIndex);
        void ResetCurrentKeyboardType(int imeIndex);
        int OnCurrentKeyboardTypeChanged(int index, const std::u16string& value);
        void CopyInputMethodService(int imeIndex);
        ClientInfo *GetClientInfo(const sptr<IInputClient>& inputClient);
        void WorkThread();
        void OnPrepareInput(Message *msg);
        void OnReleaseInput(Message *msg);
        void OnStartInput(Message *msg);
        void OnStopInput(Message *msg);
        void OnClientDied(const wptr<IRemoteObject>& who);
        void OnImsDied(const wptr<IRemoteObject>& who);
        void OnHideKeyboardSelf(int flags);
        void OnAdvanceToNext();
        void OnSetDisplayMode(int mode);
        void OnRestartIms(int index, const std::u16string& imeId);
        void OnUserLocked();
        int AddClient(int pid, int uid, int displayId, const sptr<IInputClient>& inputClient,
                  const sptr<IInputDataChannel>& channel,
                  const InputAttribute& attribute);
        int RemoveClient(const sptr<IInputClient>& inputClient, int retClientNum);
        int StartInputMethod(int index);
        int StopInputMethod(int index);
        int ShowKeyboard(const sptr<IInputClient>& inputClient);
        int HideKeyboard(const sptr<IInputClient>& inputClient);
        void SetDisplayId(int displayId);
        int GetImeIndex(const sptr<IInputClient>& inputClient);
        static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();
        void onSetInputMethodCore(Message *msg);
    };
}
}
#endif // FM_IMMS_PROJECT_PERUSERSESSION_H
