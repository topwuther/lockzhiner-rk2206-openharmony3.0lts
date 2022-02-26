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

/*! \file MessageHandler.h */
#ifndef FM_IMMS_PROJECT_MESSAGEHANDLER_H
#define FM_IMMS_PROJECT_MESSAGEHANDLER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include "global.h"
#include "message_parcel.h"
#include "message.h"

namespace OHOS {
namespace MiscServices {
namespace MessageID {
    enum {
        // for system broadcast
        MSG_ID_SYSTEM_START = 0, // system started
        MSG_ID_SYSTEM_STOP, // system stopped
        MSG_ID_USER_START, //  a user started
        MSG_ID_USER_STOP, // a user stopped
        MSG_ID_USER_UNLOCK, // a user unlocked
        MSG_ID_USER_LOCK, // a user locked
        MSG_ID_PACKAGE_ADDED, // a package is installed
        MSG_ID_PACKAGE_REMOVED, // a package is removed
        MSG_ID_SETTING_CHANGED, // input method setting is changed

        // the request from client
        MSG_ID_PREPARE_INPUT, // prepare input
        MSG_ID_START_INPUT, // start input
        MSG_ID_STOP_INPUT, // stop input
        MSG_ID_RELEASE_INPUT, // release input
        MSG_ID_SET_INPUT_METHOD_CORE,

        // the request to handle the condition that the remote object died
        MSG_ID_CLIENT_DIED, // input client died
        MSG_ID_IMS_DIED, // input method service died
        MSG_ID_DISABLE_IMS, // disable input method service
        MSG_ID_RESTART_IMS, // restart input method service
        MSG_ID_HIDE_KEYBOARD_SELF, // hide the current keyboard
        MSG_ID_ADVANCE_TO_NEXT, // switch to next
        MSG_ID_SET_DISPLAY_MODE, // set display mode

        MSG_ID_SHELL_COMMAND, // shell command
        MSG_ID_EXIT_SERVICE, // exit service

        //the request from IMSA to IMC
        MSG_ID_INSERT_CHAR,
        MSG_ID_DELETE_BACKWARD,
        MSG_ID_CLOSE,
        MSG_ID_ON_INPUT_READY,

        // the request from IMSA to IMA
        MSG_ID_SHOW_KEYBOARD,
        MSG_ID_INITIALIZE_INPUT,
        MSG_ID_HIDE_KEYBOARD,
        MSG_ID_SET_KEYBOARD_TYPE,
        MSG_ID_GET_KEYBOARD_WINDOW_HEIGHT,

        // the request from IMC to IMA
        MSG_ID_DISPATCH_KEY, // dispatch key from PhysicalKbd
    };
}

class MessageHandler {
    public:
        MessageHandler();
        ~MessageHandler();
        void SendMessage(Message *msg);
        Message *GetMessage();
        static MessageHandler *Instance();

    private:
        std::mutex mMutex; // a mutex to guard message queue
        std::condition_variable mCV; // condition variable to work with mMutex
        std::queue<Message*> mQueue; // Message queue, guarded by mMutex;

        MessageHandler(const MessageHandler&);
        MessageHandler& operator =(const MessageHandler&);
        MessageHandler(const MessageHandler&&);
        MessageHandler& operator =(const MessageHandler&&);
    };
}
}
#endif // FM_IMMS_PROJECT_MESSAGEHANDLER_H
