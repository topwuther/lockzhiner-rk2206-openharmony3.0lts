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
#include "message.h"
#include "message_parcel.h"
#include "message_option.h"
#include "global.h"
#include "platform_callback_stub.h"

namespace OHOS {
namespace MiscServices {
    using namespace MessageID;
    PlatformCallbackStub::PlatformCallbackStub()
    {
    }

    PlatformCallbackStub::~PlatformCallbackStub()
    {
    }

    int PlatformCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
                                              MessageOption& option)
    {
        switch (code) {
            case NOTIFY_EVENT: {
                int eventId = data.ReadInt32();
                int userId = data.ReadInt32();
                std::vector < std::u16string > eventContent;
                int size = data.ReadInt32();
                for (int i = 0; i < size; i++) {
                    eventContent.push_back(data.ReadString16());
                }
                notifyEvent(eventId, userId, eventContent);
                break;
            }
            default: {
                return IRemoteStub::OnRemoteRequest(code, data, reply, option);
            }
        }
        return NO_ERROR;
    }

    void PlatformCallbackStub::notifyEvent(int eventId, int userId, const std::vector<std::u16string>& eventContent)
    {
        int msgId = 0;
        switch (eventId) {
            case CommonEvent::COMMON_EVENT_USER_STARTED: {
                msgId = MSG_ID_USER_START;
                break;
            }
            case CommonEvent::COMMON_EVENT_USER_STOPPED: {
                msgId = MSG_ID_USER_STOP;
                break;
            }
            case CommonEvent::COMMON_EVENT_USER_UNLOCKED: {
                msgId = MSG_ID_USER_UNLOCK;
                break;
            }
            case CommonEvent::COMMON_EVENT_USER_LOCKED: {
                msgId = MSG_ID_USER_LOCK;
                break;
            }
            case CommonEvent::COMMON_EVENT_SETTING_CHANGED: {
                msgId = MSG_ID_SETTING_CHANGED;
                break;
            }
            case CommonEvent::COMMON_EVENT_PACKAGE_ADDED: {
                msgId = MSG_ID_PACKAGE_ADDED;
                break;
            }
            case CommonEvent::COMMON_EVENT_PACKAGE_REMOVED: {
                msgId = MSG_ID_PACKAGE_REMOVED;
                break;
            }
            default: {
                return;
            }
        }

        MessageParcel *parcel = new MessageParcel();
        parcel->WriteInt32(userId);
        int size = eventContent.size();
        parcel->WriteInt32(size);
        for (int i = 0; i < size; i++) {
            parcel->WriteString16(eventContent[i]);
        }
        Message *msg = new Message(msgId, parcel);
        MessageHandler::Instance()->SendMessage(msg);
    }
}
}