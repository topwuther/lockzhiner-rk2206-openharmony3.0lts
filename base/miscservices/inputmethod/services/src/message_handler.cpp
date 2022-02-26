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

#include "message_handler.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    */
    MessageHandler::MessageHandler()
    {
    }

    /*! Destructor
    */
    MessageHandler::~MessageHandler()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while (! mQueue.empty()) {
            Message *msg = mQueue.front();
            mQueue.pop();
            delete msg;
            msg = nullptr;
        }
    }

    /*! Send a message
      \param msg a message to be sent
      \note the msg pointer should not be freed by the caller
    */
    void MessageHandler::SendMessage(Message *msg)
    {
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mQueue.push(msg);
        }
      mCV.notify_one();
    }

    /*! Get a message
      \return a pointer referred to an object of message
      \note the returned pointer should be freed by the caller.
    */
    Message *MessageHandler::GetMessage()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCV.wait(lock, [this]{
            return !this->mQueue.empty();
        });

        Message *msg = (Message*) mQueue.front();
        mQueue.pop();
        return msg;
    }

    /*! The single instance of MessageHandler in the service
      \return the pointer referred to an object.
    */
    MessageHandler *MessageHandler::Instance()
    {
        static MessageHandler *handler = nullptr;
        if (handler == nullptr) {
            handler = new MessageHandler();
        }
        return handler;
    }
}
}

