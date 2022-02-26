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

#include "notification.h"

#include <sstream>

#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
Notification::Notification(){};

Notification::Notification(const sptr<NotificationRequest> &request)
{
    request_ = request;
    key_ = GenerateNotificationKey(GetUid(), GetLabel(), GetId());
}

Notification::Notification(const Notification &other)
{
    enableSound_ = other.enableSound_;
    enableLight_ = other.enableLight_;
    enableViration_ = other.enableViration_;
    key_ = other.key_;
    ledLightColor_ = other.ledLightColor_;
    lockscreenVisibleness_ = other.lockscreenVisibleness_;
    request_ = other.request_;
    postTime_ = other.postTime_;
    sound_ = other.sound_;
    vibrationStyle_ = other.vibrationStyle_;
}

Notification::~Notification()
{}

bool Notification::EnableLight() const
{
    return enableLight_;
}

bool Notification::EnableSound() const
{
    return enableSound_;
}

bool Notification::EnableVibrate() const
{
    return enableViration_;
}

std::string Notification::GetBundleName() const
{
    if (request_ == nullptr) {
        return "";
    }
    return request_->GetOwnerBundleName();
}

std::string Notification::GetCreateBundle() const
{
    if (request_ == nullptr) {
        return "";
    }
    return request_->GetCreatorBundleName();
}

std::string Notification::GetLabel() const
{
    if (request_ == nullptr) {
        return "";
    }
    return request_->GetLabel();
}

int32_t Notification::GetLedLightColor() const
{
    return ledLightColor_;
}

NotificationConstant::VisiblenessType Notification::GetLockscreenVisibleness() const
{
    return lockscreenVisibleness_;
}

std::string Notification::GetGroup() const
{
    if (request_ == nullptr) {
        return "";
    }
    return request_->GetGroupValue();
}

int32_t Notification::GetId() const
{
    if (request_ == nullptr) {
        return -1;
    }
    return request_->GetNotificationId();
}

std::string Notification::GetKey() const
{
    return key_;
}

NotificationRequest Notification::GetNotificationRequest() const
{
    return *request_;
}

int64_t Notification::GetPostTime() const
{
    return postTime_;
}

Uri Notification::GetSound() const
{
    if (enableSound_ && sound_ != nullptr) {
        return *sound_;
    }
    return Uri("");
}

pid_t Notification::GetUid() const
{
    if (request_ == nullptr) {
        return 0;
    }
    return request_->GetCreatorUid();
}

pid_t Notification::GetPid() const
{
    if (request_ == nullptr) {
        return 0;
    }
    return request_->GetCreatorPid();
}

std::vector<int64_t> Notification::GetVibrationStyle() const
{
    return vibrationStyle_;
}

bool Notification::IsGroup() const
{
    if (request_ == nullptr) {
        return false;
    }
    return !(request_->GetGroupValue() == "");
}

bool Notification::IsFloatingIcon() const
{
    if (request_ == nullptr) {
        return false;
    }
    return request_->IsFloatingIcon();
}

bool Notification::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(enableLight_)) {
        ANS_LOGE("Can't write enableLight_");
        return false;
    }

    if (!parcel.WriteBool(enableSound_)) {
        ANS_LOGE("Can't write enableSound_");
        return false;
    }

    if (!parcel.WriteBool(enableViration_)) {
        ANS_LOGE("Can't write enableViration_");
        return false;
    }

    if (!parcel.WriteString(key_)) {
        ANS_LOGE("Can't wirte key");
        return false;
    }
    if (!parcel.WriteInt32(ledLightColor_)) {
        ANS_LOGE("Can't write ledLigthColor");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(lockscreenVisibleness_))) {
        ANS_LOGE("Can't write visbleness");
        return false;
    }

    if (!parcel.WriteStrongParcelable(request_)) {
        ANS_LOGE("Can't write request");
        return false;
    }

    if (!parcel.WriteInt64(postTime_)) {
        ANS_LOGE("Can't write postTime");
        return false;
    }

    if (enableSound_ && sound_ != nullptr) {
        if (!parcel.WriteString(sound_->ToString())) {
            ANS_LOGE("Can't write sound");
            return false;
        }
    }

    if (!parcel.WriteInt64Vector(vibrationStyle_)) {
        ANS_LOGE("Can't write vibrationStyle");
        return false;
    }

    return true;
}

bool Notification::ReadFromParcel(Parcel &parcel)
{
    // Read enableLight_
    enableLight_ = parcel.ReadBool();

    // Read enableSound_
    enableSound_ = parcel.ReadBool();

    // Read enableViration_
    enableViration_ = parcel.ReadBool();

    // Read key_
    key_ = parcel.ReadString();

    // Read ledLightColor_
    ledLightColor_ = parcel.ReadInt32();

    // Read lockscreenVisibleness_
    lockscreenVisibleness_ = static_cast<NotificationConstant::VisiblenessType>(parcel.ReadInt32());

    // Read request_
    request_ = parcel.ReadStrongParcelable<NotificationRequest>();

    // Read postTime_
    postTime_ = parcel.ReadInt64();

    // Read sound_
    if (enableSound_) {
        sound_ = std::make_shared<Uri>(parcel.ReadString());
    }

    // Read vibrationStyle_
    parcel.ReadInt64Vector(&vibrationStyle_);

    return true;
}

Notification *Notification::Unmarshalling(Parcel &parcel)
{
    Notification *n = new Notification();
    if (n && !n->ReadFromParcel(parcel)) {
        ANS_LOGE("Read from parcel error");
        delete n;
        n = nullptr;
    }
    return n;
}

void Notification::SetEnableSound(const bool &enable)
{
    enableSound_ = enable;
}

void Notification::SetEnableLight(const bool &enable)
{
    enableLight_ = enable;
}

void Notification::SetEnableViration(const bool &enable)
{
    enableViration_ = enable;
}

void Notification::SetLedLightColor(const int32_t &color)
{
    ledLightColor_ = color;
}

void Notification::SetLockScreenVisbleness(const NotificationConstant::VisiblenessType &visbleness)
{
    lockscreenVisibleness_ = visbleness;
}

void Notification::SetPostTime(const int64_t &time)
{
    postTime_ = time;
}

void Notification::SetSound(const Uri &sound)
{
    sound_ = std::make_shared<Uri>(sound.ToString());
}

void Notification::SetVibrationStyle(const std::vector<int64_t> &style)
{
    vibrationStyle_ = style;
}

std::string Notification::GenerateNotificationKey(int32_t uid, const std::string &label, int32_t id)
{
    const char *KEY_SPLITER = "_";

    std::stringstream stream;
    stream << uid << KEY_SPLITER << label << KEY_SPLITER << id;

    return stream.str();
}

std::string Notification::Dump() const
{
    std::string dump = "Notification{ key = " + key_ + ", ledLightColor = " + std::to_string(ledLightColor_) +
                       ", lockscreenVisbleness = " + std::to_string(static_cast<int32_t>(lockscreenVisibleness_)) +
                       ",request = ";
    if (request_ == nullptr) {
        dump += "nullptr";
    } else {
        dump += request_->Dump();
    }
    dump = dump + ",postTime = " + std::to_string(postTime_) + ",sound = " + sound_->ToString() + "vibrationStyle = ";
    for (auto &style : vibrationStyle_) {
        dump += std::to_string(style);
        dump += ",";
    }
    return dump;
}

}  // namespace Notification
}  // namespace OHOS