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

#include "notification_slot.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {

const int MAX_TEXT_LENGTH = 1000;

NotificationSlot::NotificationSlot(NotificationConstant::SlotType type) : sound_("")
{
    SetType(type);
}

NotificationSlot::~NotificationSlot()
{}

bool NotificationSlot::CanEnableLight() const
{
    return isLightEnabled_;
}

void NotificationSlot::SetEnableLight(bool isLightEnabled)
{
    isLightEnabled_ = isLightEnabled;
}

bool NotificationSlot::CanVibrate() const
{
    return isVibrationEnabled_;
}

void NotificationSlot::SetEnableVibration(bool vibration)
{
    isVibrationEnabled_ = vibration;
}

std::string NotificationSlot::GetDescription() const
{
    return description_;
}

void NotificationSlot::SetDescription(const std::string &description)
{
    description_ = TruncateString(description);
}

std::string NotificationSlot::GetId() const
{
    return id_;
}

int32_t NotificationSlot::GetLedLightColor() const
{
    return lightColor_;
}

void NotificationSlot::SetLedLightColor(int32_t color)
{
    lightColor_ = color;
}

NotificationSlot::NotificationLevel NotificationSlot::GetLevel() const
{
    return level_;
}

void NotificationSlot::SetLevel(NotificationLevel level)
{
    level_ = level;
}

NotificationConstant::SlotType NotificationSlot::GetType() const
{
    return type_;
}

void NotificationSlot::SetType(NotificationConstant::SlotType type)
{
    type_ = NotificationConstant::SlotType::CUSTOM;
    switch (type) {
        case NotificationConstant::SlotType::SOCIAL_COMMUNICATION:
            id_ = "SOCIAL_COMMUNICATION";
            SetName("SOCIAL_COMMUNICATION");
            SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
            SetEnableVibration(true);
            SetLevel(LEVEL_HIGH);
            break;
        case NotificationConstant::SlotType::SERVICE_REMINDER:
            id_ = "SERVICE_REMINDER";
            SetName("SERVICE_REMINDER");
            SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
            SetEnableVibration(true);
            SetLevel(LEVEL_DEFAULT);
            break;
        case NotificationConstant::SlotType::CONTENT_INFORMATION:
            id_ = "CONTENT_INFORMATION";
            SetName("CONTENT_INFORMATION");
            SetLockscreenVisibleness(NotificationConstant::VisiblenessType::SECRET);
            SetEnableVibration(false);
            SetLevel(LEVEL_LOW);
            break;
        case NotificationConstant::SlotType::OTHER:
            id_ = "OTHER";
            SetName("OTHER");
            SetLockscreenVisibleness(NotificationConstant::VisiblenessType::SECRET);
            SetEnableVibration(false);
            SetLevel(LEVEL_MIN);
            break;
        default:
            break;
    }
    type_ = type;
}

NotificationConstant::VisiblenessType NotificationSlot::GetLockScreenVisibleness() const
{
    return lockScreenVisibleness_;
}

void NotificationSlot::SetLockscreenVisibleness(NotificationConstant::VisiblenessType visibleness)
{
    lockScreenVisibleness_ = visibleness;
}

std::string NotificationSlot::GetName() const
{
    return name_;
}

void NotificationSlot::SetName(const std::string &name)
{
    name_ = TruncateString(name);
}

std::string NotificationSlot::GetSlotGroup() const
{
    return groupId_;
}

void NotificationSlot::SetSlotGroup(const std::string &groupId)
{
    groupId_ = groupId;
}

Uri NotificationSlot::GetSound() const
{
    return sound_;
}

void NotificationSlot::SetSound(const Uri &sound)
{
    sound_ = sound;
}

std::vector<int64_t> NotificationSlot::GetVibrationStyle() const
{
    return vibrationValues_;
}

void NotificationSlot::SetVibrationStyle(const std::vector<int64_t> &vibrationValues)
{
    isVibrationEnabled_ = (vibrationValues.size() > 0);
    vibrationValues_ = vibrationValues;
}

bool NotificationSlot::IsEnableBypassDnd() const
{
    return isBypassDnd_;
}

void NotificationSlot::EnableBypassDnd(bool isBypassDnd)
{
    isBypassDnd_ = isBypassDnd;
}

bool NotificationSlot::IsShowBadge() const
{
    return isShowBadge_;
}

void NotificationSlot::EnableBadge(bool isShowBadge)
{
    isShowBadge_ = isShowBadge;
}

std::string NotificationSlot::Dump() const
{
    return "NotificationSlot[ id = " + id_ +
           ", name = " + name_ +
           ", description = " + description_ +
           ", type = " + std::to_string(static_cast<int32_t>(type_)) +
           ", level = " + std::to_string(static_cast<int32_t>(level_)) +
           ", isBypassDnd = " + (isBypassDnd_ ? "true" : "false") +
           ", visibleness = " + std::to_string(static_cast<int32_t>(lockScreenVisibleness_)) +
           ", sound = " + sound_.ToString() +
           ", isLightEnabled = " + (isLightEnabled_ ? "true" : "false") +
           ", lightColor = " + std::to_string(lightColor_) +
           ", isVibrate = " + (isVibrationEnabled_ ? "true" : "false") +
           ", vibration = " + MergeVectorToString(vibrationValues_) +
           ", isShowBadge = " + (isShowBadge_ ? "true" : "false") + ", groupId = " + groupId_ + "]";
}

bool NotificationSlot::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(id_)) {
        ANS_LOGE("Failed to write id");
        return false;
    }

    if (!parcel.WriteString(name_)) {
        ANS_LOGE("Failed to write name");
        return false;
    }

    if (!parcel.WriteBool(isLightEnabled_)) {
        ANS_LOGE("Failed to write isLightEnabled");
        return false;
    }

    if (!parcel.WriteBool(isVibrationEnabled_)) {
        ANS_LOGE("Failed to write isVibrationEnabled");
        return false;
    }

    if (!parcel.WriteBool(isShowBadge_)) {
        ANS_LOGE("Failed to write isShowBadge");
        return false;
    }

    if (!parcel.WriteBool(isBypassDnd_)) {
        ANS_LOGE("Failed to write isBypassDnd");
        return false;
    }

    if (!parcel.WriteString(description_)) {
        ANS_LOGE("Failed to write description");
        return false;
    }

    if (!parcel.WriteInt32(lightColor_)) {
        ANS_LOGE("Failed to write lightColor");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(level_))) {
        ANS_LOGE("Failed to write level");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(type_))) {
        ANS_LOGE("Failed to write type");
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(lockScreenVisibleness_))) {
        ANS_LOGE("Failed to write lockScreenVisibleness");
        return false;
    }

    if (!parcel.WriteString(groupId_)) {
        ANS_LOGE("Failed to write groupId");
        return false;
    }

    if (sound_.ToString().empty()) {
        if (!parcel.WriteInt32(VALUE_NULL)) {
            ANS_LOGE("Failed to write int");
            return false;
        }
    } else {
        if (!parcel.WriteInt32(VALUE_OBJECT)) {
            ANS_LOGE("Failed to write int");
            return false;
        }
        if (!parcel.WriteString((sound_.ToString()))) {
            ANS_LOGE("Failed to write sound");
            return false;
        }
    }

    if (!parcel.WriteInt64Vector(vibrationValues_)) {
        ANS_LOGE("Failed to write vibrationValues");
        return false;
    }

    return true;
}

bool NotificationSlot::ReadFromParcel(Parcel &parcel)
{
    id_ = parcel.ReadString();
    name_ = parcel.ReadString();
    isLightEnabled_ = parcel.ReadBool();
    isVibrationEnabled_ = parcel.ReadBool();
    isShowBadge_ = parcel.ReadBool();
    isBypassDnd_ = parcel.ReadBool();
    description_ = parcel.ReadString();
    lightColor_ = parcel.ReadInt32();
    level_ = static_cast<NotificationLevel>(parcel.ReadInt32());
    type_ = static_cast<NotificationConstant::SlotType>(parcel.ReadInt32());
    lockScreenVisibleness_ = static_cast<NotificationConstant::VisiblenessType>(parcel.ReadInt32());
    groupId_ = parcel.ReadString();

    int empty = VALUE_NULL;
    if (!parcel.ReadInt32(empty)) {
        ANS_LOGE("Failed to read int");
        return false;
    }

    if (empty == VALUE_OBJECT) {
        sound_ = Uri((parcel.ReadString()));
    }

    parcel.ReadInt64Vector(&vibrationValues_);
    return true;
}

NotificationSlot *NotificationSlot::Unmarshalling(Parcel &parcel)
{
    NotificationSlot *notificationSlot = new NotificationSlot(NotificationConstant::SlotType::CUSTOM);

    if (notificationSlot && !notificationSlot->ReadFromParcel(parcel)) {
        delete notificationSlot;
        notificationSlot = nullptr;
    }

    return notificationSlot;
}

std::string NotificationSlot::MergeVectorToString(const std::vector<int64_t> &mergeVector) const
{
    std::string contents;
    for (auto it = mergeVector.begin(); it != mergeVector.end(); ++it) {
        contents += std::to_string(*it);
        if (it != mergeVector.end() - 1) {
            contents += ", ";
        }
    }
    return contents;
}

std::string NotificationSlot::TruncateString(const std::string &in)
{
    std::string temp = in;
    if (in.length() > MAX_TEXT_LENGTH) {
        temp = in.substr(0, MAX_TEXT_LENGTH);
    }
    return temp;
}
}  // namespace Notification
}  // namespace OHOS