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
#ifndef FM_IMMS_PROJECT_MESSAGE_H
#define FM_IMMS_PROJECT_MESSAGE_H

#include "global.h"
#include "message_parcel.h"
namespace OHOS {
namespace MiscServices {
    class Message {
    public:
        int32_t msgId_; // message id
        MessageParcel *msgContent_ = nullptr; // message content
        Message(int32_t msgId, MessageParcel *msgContent);
        explicit Message(const Message& msg);
        Message& operator =(const Message& msg);
        ~Message();
    private:
        Message(const Message&&);
        Message& operator =(const Message&&);
    };
}
}

#endif // FM_IMMS_PROJECT_MESSAGE_H