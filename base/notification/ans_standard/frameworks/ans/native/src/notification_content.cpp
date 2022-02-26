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

#include "notification_content.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
NotificationContent::NotificationContent(const std::shared_ptr<NotificationNormalContent> &normalContent)
{
    if (!normalContent) {
        ANS_LOGE("NotificationNormalContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::BASIC_TEXT;
    content_ = normalContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationLongTextContent> &longTextContent)
{
    if (!longTextContent) {
        ANS_LOGE("NotificationLongTextContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::LONG_TEXT;
    content_ = longTextContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationPictureContent> &pictureContent)
{
    if (!pictureContent) {
        ANS_LOGE("NotificationPictureContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::PICTURE;
    content_ = pictureContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationConversationalContent> &conversationContent)
{
    if (!conversationContent) {
        ANS_LOGE("NotificationConversationalContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::CONVERSATION;
    content_ = conversationContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationMultiLineContent> &multiLineContent)
{
    if (!multiLineContent) {
        ANS_LOGE("NotificationMultiLineContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::MULTILINE;
    content_ = multiLineContent;
}

NotificationContent::NotificationContent(const std::shared_ptr<NotificationMediaContent> &mediaContent)
{
    if (!mediaContent) {
        ANS_LOGE("NotificationMediaContent can not be null");
        return;
    }

    contentType_ = NotificationContent::Type::MEDIA;
    content_ = mediaContent;
}

NotificationContent::~NotificationContent()
{}

NotificationContent::Type NotificationContent::GetContentType() const
{
    return contentType_;
}

std::shared_ptr<NotificationBasicContent> NotificationContent::GetNotificationContent() const
{
    return content_;
}

std::string NotificationContent::Dump()
{
    std::string contentTypeStr =   (contentType_ == NotificationContent::Type::BASIC_TEXT)   ? "BASIC_TEXT"
                                 : (contentType_ == NotificationContent::Type::CONVERSATION) ? "CONVERSATION"
                                 : (contentType_ == NotificationContent::Type::LONG_TEXT)    ? "LONG_TEXT"
                                 : (contentType_ == NotificationContent::Type::MEDIA)        ? "MEDIA"
                                 : (contentType_ == NotificationContent::Type::MULTILINE)    ? "MULTILINE"
                                 : (contentType_ == NotificationContent::Type::PICTURE)      ? "PICTURE" : "NONE";

    return "NotificationContent[ contentType = " + contentTypeStr +
           ", content = " + (content_ ? "not null" : "null") + "]";
}

bool NotificationContent::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(contentType_))) {
        ANS_LOGE("Failed to write contentType");
        return false;
    }

    auto valid = content_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether content is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(content_.get())) {
            ANS_LOGE("Failed to write content");
            return false;
        }
    }

    return true;
}

NotificationContent *NotificationContent::Unmarshalling(Parcel &parcel)
{
    auto pContent = new NotificationContent();
    if ((nullptr != pContent) && !pContent->ReadFromParcel(parcel)) {
        delete pContent;
        pContent = nullptr;
    }

    return pContent;
}

bool NotificationContent::ReadFromParcel(Parcel &parcel)
{
    contentType_ = static_cast<NotificationContent::Type>(parcel.ReadInt32());

    auto valid = parcel.ReadBool();
    if (!valid) {
        return true;
    }

    switch (contentType_) {
        case NotificationContent::Type::BASIC_TEXT: {
            std::shared_ptr<NotificationNormalContent> normalContent(
                parcel.ReadParcelable<NotificationNormalContent>());
            if (!normalContent) {
                ANS_LOGE("Failed to read normal content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(normalContent);
        } break;
        case NotificationContent::Type::CONVERSATION: {
            std::shared_ptr<NotificationConversationalContent> conversationalContent(
                parcel.ReadParcelable<NotificationConversationalContent>());
            if (!conversationalContent) {
                ANS_LOGE("Failed to read conversational content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(conversationalContent);
        } break;
        case NotificationContent::Type::LONG_TEXT: {
            std::shared_ptr<NotificationLongTextContent> longTextContent(
                parcel.ReadParcelable<NotificationLongTextContent>());
            if (!longTextContent) {
                ANS_LOGE("Failed to read long text content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(longTextContent);
        } break;
        case NotificationContent::Type::MEDIA: {
            std::shared_ptr<NotificationMediaContent> mediaContent(parcel.ReadParcelable<NotificationMediaContent>());
            if (!mediaContent) {
                ANS_LOGE("Failed to read media content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(mediaContent);
        } break;
        case NotificationContent::Type::MULTILINE: {
            std::shared_ptr<NotificationMultiLineContent> multiLineContent(
                parcel.ReadParcelable<NotificationMultiLineContent>());
            if (!multiLineContent) {
                ANS_LOGE("Failed to read multiLine content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(multiLineContent);
        } break;
        case NotificationContent::Type::PICTURE: {
            std::shared_ptr<NotificationPictureContent> pictureContent(
                parcel.ReadParcelable<NotificationPictureContent>());
            if (!pictureContent) {
                ANS_LOGE("Failed to read picture content");
                return false;
            }
            content_ = std::dynamic_pointer_cast<NotificationBasicContent>(pictureContent);
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}
}  // namespace Notification
}  // namespace OHOS
