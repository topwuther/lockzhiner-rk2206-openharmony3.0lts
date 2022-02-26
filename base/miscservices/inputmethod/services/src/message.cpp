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

#include "message.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    \param msgId a message Id
    \param msgContent the content of a message
    */
    Message::Message(int32_t msgId, MessageParcel *msgContent)
    {
        msgId_ = msgId;
        msgContent_ = msgContent;
        if (msgContent_) {
            msgContent_->RewindRead(0);
        }
    }

    /*! Constructor
    \param msg a source message
    */
    Message::Message(const Message& msg)
    {
        msgId_ = msg.msgId_;
        if (msgContent_ != nullptr) {
            delete msgContent_;
            msgContent_ = nullptr;
        }
        MessageParcel *src = msg.msgContent_;
        if (src) {
            msgContent_ = new MessageParcel();
            msgContent_->ParseFrom(src->GetData(), src->GetDataSize());
        }
    }

    Message& Message::operator =(const Message& msg)
    {
        if (this == &msg) {
            return *this;
        }
        msgId_ = msg.msgId_;
        if (msgContent_ != nullptr) {
            delete msgContent_;
            msgContent_ = nullptr;
        }
        if (msg.msgContent_) {
            msgContent_ = new MessageParcel();
            msgContent_->ParseFrom(msg.msgContent_->GetData(), msg.msgContent_->GetDataSize());
            msgContent_->RewindRead(0);
        }
        return *this;
    }

    Message::~Message()
    {
        if (msgContent_) {
            delete msgContent_;
            msgContent_ = nullptr;
        }
    }
}
}