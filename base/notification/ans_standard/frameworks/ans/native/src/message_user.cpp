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

#include "message_user.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {

MessageUser::MessageUser()
    : uri_("")
{}

MessageUser::~MessageUser()
{}

void MessageUser::SetKey(const std::string &key)
{
    key_ = key;
}

std::string MessageUser::GetKey() const
{
    return key_;
}

void MessageUser::SetName(const std::string &name)
{
    name_ = name;
}

std::string MessageUser::GetName() const
{
    return name_;
}

void MessageUser::SetPixelMap(const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    pixelMap_ = pixelMap;
}

const std::shared_ptr<Media::PixelMap> MessageUser::GetPixelMap() const
{
    return pixelMap_;
}

void MessageUser::SetUri(const Uri &uri)
{
    uri_ = uri;
}

Uri MessageUser::GetUri() const
{
    return uri_;
}

void MessageUser::SetMachine(bool machine)
{
    isMachine_ = machine;
}

bool MessageUser::IsMachine() const
{
    return isMachine_;
}

void MessageUser::SetUserAsImportant(bool userImportant)
{
    isUserImportant_ = userImportant;
}

bool MessageUser::IsUserImportant() const
{
    return isUserImportant_;
}

std::string MessageUser::Dump() const
{
    return "MessageUser[key = " + key_ +
                        ", name = " + name_ +
                        ", pixelMap = " + (pixelMap_ ? "not null" : "null") +
                        ", uri = " + uri_.ToString() +
                        ", isMachine = " + (isMachine_ ? "true" : "false") +
                        ", isUserImportant = " + (isUserImportant_ ? "true" : "false") + "]";
}

bool MessageUser::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(key_)) {
        ANS_LOGE("Failed to write key");
        return false;
    }

    if (!parcel.WriteString(name_)) {
        ANS_LOGE("Failed to write name");
        return false;
    }

    if (!parcel.WriteBool(isMachine_)) {
        ANS_LOGE("Failed to write isMachine");
        return false;
    }

    if (!parcel.WriteBool(isUserImportant_)) {
        ANS_LOGE("Failed to write isUserImportant");
        return false;
    }

    if (uri_.ToString().empty()) {
        if (!parcel.WriteInt32(VALUE_NULL)) {
            ANS_LOGE("Failed to write VALUE_NULL");
            return false;
        }
    } else {
        if (!parcel.WriteInt32(VALUE_OBJECT)) {
            ANS_LOGE("Failed to write VALUE_OBJECT");
            return false;
        }
        if (!parcel.WriteString((uri_.ToString()))) {
            ANS_LOGE("Failed to write uri");
            return false;
        }
    }

    bool valid = pixelMap_ ? true : false;
    if (!parcel.WriteBool(valid)) {
        ANS_LOGE("Failed to write the flag which indicate whether pixelMap is null");
        return false;
    }

    if (valid) {
        if (!parcel.WriteParcelable(pixelMap_.get())) {
            ANS_LOGE("Failed to write pixelMap");
            return false;
        }
    }

    return true;
}

bool MessageUser::ReadFromParcel(Parcel &parcel)
{
    key_ = parcel.ReadString();
    name_ = parcel.ReadString();
    isMachine_ = parcel.ReadBool();
    isUserImportant_ = parcel.ReadBool();

    int empty = VALUE_NULL;
    if (!parcel.ReadInt32(empty)) {
        ANS_LOGE("Failed to read VALUE");
        return false;
    }

    if (empty == VALUE_OBJECT) {
        uri_ = Uri((parcel.ReadString()));
    }

    bool valid = parcel.ReadBool();
    if (valid) {
        pixelMap_ = std::shared_ptr<Media::PixelMap>(parcel.ReadParcelable<Media::PixelMap>());
        if (!pixelMap_) {
            ANS_LOGE("Failed to read pixelMap");
            return false;
        }
    }

    return true;
}

MessageUser *MessageUser::Unmarshalling(Parcel &parcel)
{
    MessageUser *messageUser = new MessageUser();

    if (messageUser && !messageUser->ReadFromParcel(parcel)) {
        delete messageUser;
        messageUser = nullptr;
    }

    return messageUser;
}
}  // namespace Notification
}  // namespace OHOS