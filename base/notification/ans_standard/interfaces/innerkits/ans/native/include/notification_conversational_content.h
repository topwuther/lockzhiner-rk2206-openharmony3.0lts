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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_CONVERSATIONAL_CONTENT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_CONVERSATIONAL_CONTENT_H

#include <memory>
#include <string>
#include <vector>
#include "message_user.h"
#include "notification_basic_content.h"
#include "notification_conversational_message.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationConversationalContent : public NotificationBasicContent {
public:
    using MessagePtr = std::shared_ptr<NotificationConversationalMessage>;
    using MessageVector = std::vector<MessagePtr>;

    /**
     * A constructor used to create a NotificationConversationalContent instance with the MessageUser specified.
     * @param messageUser Indicates the MessageUser who sends all Message objects in this conversation-like
     * notification. This parameter cannot be null.
     */
    NotificationConversationalContent(const MessageUser &messageUser);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationConversationalContent() = default;

    /**
     * Obtains the message sender to be displayed for any messages
     * sent by the user in this conversation-like notification.
     * @return the message sender.
     */
    MessageUser GetMessageUser() const;

    /**
     * Sets the title to be displayed for the conversation.
     * The title set in this method will overwrite the one set by calling setTitle(std::tring).
     * @param conversationTitle Indicates the title to be displayed for the conversation.
     */
    void SetConversationTitle(const std::string &conversationTitle);

    /**
     * Obtains the title to be displayed for the conversation.
     * @return the title to be displayed for the conversation.
     */
    std::string GetConversationTitle() const;

    /**
     * Checks whether this notification represents a group conversation.
     * @return true if this notification represents a group conversation; returns false otherwise.
     */
    bool IsConversationGroup() const;

    /**
     * Sets whether this notification represents a group conversation.
     * The big icon, if any, set for this notification by calling NotificationRequest::setBigIcon(PixelMap)
     * will be displayed only when this method is set to true.
     * @param isGroup Specifies whether this notification represents a group conversation.
     */
    void SetConversationGroup(bool isGroup);

    /**
     * Adds a message to this conversation-like notification based on
     * the specified message content, timestamp, and MessageUser.
     * All messages will be displayed in the order they are added.
     * @param text Indicates the text to be displayed as the message content.
     * @param timestamp Indicates the time when the message arrived.
     * @param sender Indicates the MessageUser who sent the message.
     */
    void AddConversationalMessage(
        const std::string &text, int64_t timestamp, const MessageUser &sender);

    /**
     * Adds a specified message to this conversation-like notification.
     * All messages will be displayed in the order they are added.
     * @param message Indicates the ConversationalMessage object to add.
     */
    void AddConversationalMessage(const MessagePtr &message);

    /**
     * Obtains all messages included in this conversation-like notification.
     * @return the list of all Message objects included.
     */
    MessageVector GetAllConversationalMessages() const;

    /**
     * Returns a string representation of the object.
     * @return a string representation of the object.
     */
    std::string Dump() override;

    /**
     * Marshal a object into a Parcel.
     * @param parcel the object into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal object from a Parcel.
     * @return the NotificationConversationalContent
     */
    static NotificationConversationalContent *Unmarshalling(Parcel &parcel);

protected:
    /**
     * Read a NotificationConversationalContent object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel) override;

private:
    /**
     * Default constructor used to create an empty NotificationConversationalContent instance.
     */
    NotificationConversationalContent() = default;

private:
    MessageUser messageUser_ {};
    std::string conversationTitle_ {};
    bool isGroup_ {false};
    MessageVector messages_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_CONVERSATIONAL_CONTENT_H