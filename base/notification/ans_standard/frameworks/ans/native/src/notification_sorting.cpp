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

#include "notification_sorting.h"

#include "string_ex.h"

namespace OHOS {
namespace Notification {
NotificationSorting::NotificationSorting()
{}

NotificationSorting::~NotificationSorting()
{}

NotificationSorting::NotificationSorting(const NotificationSorting &sorting)
{
    key_ = sorting.key_;
    ranking_ = sorting.ranking_;
    importance_ = sorting.importance_;
    isDisplayBadge_ = sorting.isDisplayBadge_;
    isHiddenNotification_ = sorting.isHiddenNotification_;
    visiblenessOverride_ = sorting.visiblenessOverride_;
    groupKeyOverride_ = sorting.groupKeyOverride_;
    slot_ = sorting.slot_;
}

void NotificationSorting::SetSlot(const sptr<NotificationSlot> &slot)
{
    slot_ = slot;
}

void NotificationSorting::SetGroupKeyOverride(const std::string &groupKeyOverride)
{
    groupKeyOverride_ = groupKeyOverride;
}

std::string NotificationSorting::Dump() const
{
    std::string contents {""};
    if (slot_ != nullptr) {
        contents = slot_->Dump();
    } else {
        contents = "nullptr";
    }
    return "NotificationSorting[key =" + key_ + ", ranking =" + std::to_string(ranking_) +
           ", importance =" + std::to_string(importance_) +
           ", visiblenessOverride =" + std::to_string(visiblenessOverride_) +
           ", isDisplayBadge =" + (isDisplayBadge_ ? "true" : "false") +
           ", isHiddenNotification =" + (isHiddenNotification_ ? "true" : "false") +
           ", groupKeyOverride =" + groupKeyOverride_ + ", slot_ =" + contents + "]";
}

bool NotificationSorting::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(key_)) {
        ANS_LOGE("Can't write key");
        return false;
    }

    if (!parcel.WriteInt32(ranking_)) {
        ANS_LOGE("Can't write ranking");
        return false;
    }

    if (!parcel.WriteInt32(importance_)) {
        ANS_LOGE("Can't write importance");
        return false;
    }

    if (!parcel.WriteInt32(visiblenessOverride_)) {
        ANS_LOGE("Can't write importance");
        return false;
    }

    if (!parcel.WriteBool(isDisplayBadge_)) {
        ANS_LOGE("Can't write isDisplayBadge");
        return false;
    }

    if (!parcel.WriteBool(isHiddenNotification_)) {
        ANS_LOGE("Can't write is HiddenNotification");
        return false;
    }

    if (!parcel.WriteString(groupKeyOverride_)) {
        ANS_LOGE("Can't write groupKey groupKeyOverride");
        return false;
    }

    if (!parcel.WriteStrongParcelable(slot_)) {
        ANS_LOGE("Can't write slot");
        return false;
    }

    return true;
}

bool NotificationSorting::ReadFromParcel(Parcel &parcel)
{
    // read key_
    key_ = parcel.ReadString();

    // read ranking_
    ranking_ = parcel.ReadInt32();

    // read importance_
    importance_ = parcel.ReadInt32();

    // read visiblenessOverride_
    visiblenessOverride_ = parcel.ReadInt32();

    // read isDisplayBadge_
    isDisplayBadge_ = parcel.ReadBool();

    // read isHiddenNotification_
    isHiddenNotification_ = parcel.ReadBool();

    // read groupKeyOverride_
    groupKeyOverride_ = parcel.ReadString();

    // read slot_
    slot_ = parcel.ReadStrongParcelable<NotificationSlot>();

    return true;
}

NotificationSorting *NotificationSorting::Unmarshalling(Parcel &parcel)
{
    NotificationSorting *sorting = new NotificationSorting();
    if (sorting && !sorting->ReadFromParcel(parcel)) {
        delete sorting;
        sorting = nullptr;
    }
    return sorting;
}

void NotificationSorting::SetKey(const std::string &key)
{
    key_ = key;
}

void NotificationSorting::SetImportance(const int32_t &importance)
{
    importance_ = importance;
}

void NotificationSorting::SetRanking(const int32_t &ranking)
{
    ranking_ = ranking;
}

void NotificationSorting::SetVisiblenessOverride(const int32_t &visibleness)
{
    visiblenessOverride_ = visibleness;
}

void NotificationSorting::SetDisplayBadge(const bool &isDisplayBadge)
{
    isDisplayBadge_ = isDisplayBadge;
}

void NotificationSorting::SetHiddenNotification(const bool &isHiddenNotification)
{
    isHiddenNotification_ = isHiddenNotification;
}
}  // namespace Notification
}  // namespace OHOS
