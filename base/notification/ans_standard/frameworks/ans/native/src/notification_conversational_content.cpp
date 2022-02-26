/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "notification_conversational_content.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
NotificationConversationalContent::NotificationConversationalContent(const MessageUser &messageUser)
    : messageUser_(messageUser)
{}

MessageUser NotificationConversationalContent::GetMessageUser() const
{
    return messageUser_;
}

void NotificationConversationalContent::SetConversationTitle(const std::string &conversationTitle)
{
    conversationTitle_ = conversationTitle;
}

std::string NotificationConversationalContent::GetConversationTitle() const
{
    return conversationTitle_;
}

bool NotificationConversationalContent::IsConversationGroup() const
{
    return isGroup_;
}

void NotificationConversationalContent::SetConversationGroup(bool isGroup)
{
    isGroup_ = isGroup;
}

void NotificationConversationalContent::AddConversationalMessage(
    const std::string &text, int64_t timestamp, const MessageUser &sender)
{
    auto message = std::make_shared<NotificationConversationalMessage>(text, timestamp, sender);
    if (!message) {
        ANS_LOGE("Failed to create message");
        return;
    }

    messages_.emplace_back(message);
}

void NotificationConversationalContent::AddConversationalMessage(const MessagePtr &message)
{
    if (!message) {
        ANS_LOGE("Message can not be null");
        return;
    }

    messages_.emplace_back(message);
}

NotificationConversationalContent::MessageVector NotificationConversationalContent::GetAllConversationalMessages() const
{
    return messages_;
}

std::string NotificationConversationalContent::Dump()
{
    return "NotificationConversationalContent[ " + NotificationBasicContent::Dump() +
           ", conversationTitle = " + conversationTitle_ + ", isGroup = " + (isGroup_ ? "true" : "false") +
           ", messageUser = " + messageUser_.Dump() +
           ", messages = " + (!messages_.empty() ? "not empty" : "empty") + " ]";
}

bool NotificationConversationalContent::Marshalling(Parcel &parcel) const
{
    if (!NotificationBasicContent::Marshalling(parcel)) {
        ANS_LOGE("Failed to write basic");
        return false;
    }

    if (!parcel.WriteString(conversationTitle_)) {
        ANS_LOGE("Failed to write conversation title");
        return false;
    }

    if (!parcel.WriteBool(isGroup_)) {
        ANS_LOGE("Failed to write flag group");
        return false;
    }

    if (!parcel.WriteParcelable(&messageUser_)) {
        ANS_LOGE("Failed to write messageUser");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(messages_.size()))) {
        ANS_LOGE("Failed to write the size of messages");
        return false;
    }

    for (auto it = messages_.begin(); it != messages_.end(); ++it) {
        auto valid = (*it) ? true : false;
        if (!parcel.WriteBool(valid)) {
            ANS_LOGE("Failed to write the flag which indicate whether message is null");
            return false;
        }

        if (!valid) {
            ANS_LOGE("Invalid message, write to parcel failed");
            return false;
        }

        if (!parcel.WriteParcelable(it->get())) {
            ANS_LOGE("Failed to write message");
            return false;
        }
    }

    return true;
}

NotificationConversationalContent *NotificationConversationalContent::Unmarshalling(Parcel &parcel)
{
    auto pContent = new NotificationConversationalContent();
    if ((nullptr != pContent) && !pContent->ReadFromParcel(parcel)) {
        delete pContent;
        pContent = nullptr;
    }

    return pContent;
}

bool NotificationConversationalContent::ReadFromParcel(Parcel &parcel)
{
    if (!NotificationBasicContent::ReadFromParcel(parcel)) {
        ANS_LOGE("Failed to read basic");
        return false;
    }

    if (!parcel.ReadString(conversationTitle_)) {
        ANS_LOGE("Failed to read conversation title");
        return false;
    }

    isGroup_ = parcel.ReadBool();

    auto pUser = parcel.ReadParcelable<MessageUser>();
    if (pUser == nullptr) {
        ANS_LOGE("Failed to read messageUser");
        return false;
    }
    messageUser_ = *pUser;

    auto vsize = parcel.ReadInt32();
    for (auto it = 0; it < vsize; ++it) {
        auto valid = parcel.ReadBool();
        if (!valid) {
            ANS_LOGE("Invalid message, read from parcel failed");
            return false;
        }

        auto member = parcel.ReadParcelable<NotificationConversationalMessage>();
        if (member == nullptr) {
            ANS_LOGE("Failed to read message");
            return false;
        }

        messages_.emplace_back(member);
    }

    return true;
}
}  // namespace Notification
}  // namespace OHOS
