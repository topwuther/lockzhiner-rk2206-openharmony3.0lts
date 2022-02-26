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

#ifndef FM_IMMS_PROJECT_INPUTMETHODSYSTEMABILITY_H
#define FM_IMMS_PROJECT_INPUTMETHODSYSTEMABILITY_H

#include <thread>
#include <map>
#include "system_ability.h"
#include "input_method_system_ability_stub.h"
#include "peruser_setting.h"
#include "peruser_session.h"
#include "event_handler.h"
#include "input_method_ability.h"

namespace OHOS {
namespace MiscServices {
    enum class ServiceRunningState {
        STATE_NOT_START,
        STATE_RUNNING
    };

    class InputMethodSystemAbility : public SystemAbility, public InputMethodSystemAbilityStub {
        DECLARE_SYSTEM_ABILITY(InputMethodSystemAbility);
    public:
        DISALLOW_COPY_AND_MOVE(InputMethodSystemAbility);
        InputMethodSystemAbility(int32_t systemAbilityId, bool runOnCreate);
        InputMethodSystemAbility();
        ~InputMethodSystemAbility();
        static sptr<InputMethodSystemAbility> GetInstance();
        
        int32_t GetUserState(int32_t userId);

        int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                MessageOption &option) override;
        virtual int32_t getDisplayMode(int32_t retMode) override;
        virtual int32_t getKeyboardWindowHeight(int32_t retHeight) override;
        virtual int32_t getCurrentKeyboardType(KeyboardType *retType) override;
        virtual int32_t listInputMethodEnabled(std::vector<InputMethodProperty*> *properties) override;
        virtual int32_t listInputMethod(std::vector<InputMethodProperty*> *properties) override;
        virtual int32_t listKeyboardType(const std::u16string& imeId, std::vector<KeyboardType*> *types) override;
        virtual  int32_t setInputMethodCore(sptr<IInputMethodCore> &core) override;
    protected:
        void OnStart() override;
        void OnStop() override;
    private:
        int32_t Init();
        void Initialize();
        
        std::thread workThreadHandler; /*!< thread handler of the WorkThread */

        std::map<int32_t, PerUserSetting*> userSettings;

        std::map<int32_t, PerUserSession*> userSessions;
        std::map<int32_t, MessageHandler*> msgHandlers;

        void WorkThread();
        PerUserSetting *GetUserSetting(int32_t userId);
        PerUserSession *GetUserSession(int32_t userId);
        int32_t OnUserStarted(const Message *msg);
        int32_t OnUserStopped(const Message *msg);
        int32_t OnUserUnlocked(const Message *msg);
        int32_t OnUserLocked(const Message *msg);
        int32_t OnPrepareInput(Message *msg);
        int32_t OnHandleMessage(Message *msg);
        int32_t OnRemotePeerDied(const Message *msg);
        int32_t OnSettingChanged(const Message *msg);
        int32_t OnPackageRemoved(const Message *msg);
        int32_t OnPackageAdded(const Message *msg);
        int32_t OnDisableIms(const Message *msg);
        int32_t OnAdvanceToNext(const Message *msg);

        ServiceRunningState state_;
        sptr<InputMethodAbility> inputMethodAbility_;
        void InitServiceHandler();
        static std::mutex instanceLock_;
        static sptr<InputMethodSystemAbility> instance_;
        static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTMETHODSYSTEMABILITY_H
