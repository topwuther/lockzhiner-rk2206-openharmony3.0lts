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

#include "notification_sorting_map.h"

#include "parcel.h"

namespace OHOS {
namespace Notification {
NotificationSortingMap::NotificationSortingMap()
{}

NotificationSortingMap::~NotificationSortingMap()
{}

NotificationSortingMap::NotificationSortingMap(const std::vector<NotificationSorting> &sortingList)
{
    for (auto item : sortingList) {
        sortedKey_.push_back(item.GetKey());
        sortings_[item.GetKey()] = item;
    }
}

void NotificationSortingMap::SetKey(const std::string &key)
{
    auto it = std::find(sortedKey_.cbegin(), sortedKey_.cend(), key);
    if (it == sortedKey_.cend()) {
        sortedKey_.emplace_back(key);
    }
}

bool NotificationSortingMap::GetNotificationSorting(const std::string &key, NotificationSorting &sorting) const
{
    for (auto item : sortings_) {
        if (item.first == key) {
            sorting = item.second;
            return true;
        }
    }

    return false;
}

void NotificationSortingMap::SetNotificationSorting(const std::vector<NotificationSorting> &sortingList)
{
    for (auto item : sortingList) {
        sortedKey_.push_back(item.GetKey());
        sortings_[item.GetKey()] = item;
    }
}

bool NotificationSortingMap::Marshalling(Parcel &parcel) const
{
    bool ret = true;
    if (!parcel.WriteInt32(sortings_.size())) {
        ANS_LOGE("Can't write sorting size");
        return false;
    }
    for_each(sortings_.begin(), sortings_.end(), [&](std::pair<std::string, NotificationSorting> sorting) {
        if (!parcel.WriteParcelable(&sorting.second)) {
            ANS_LOGE("Can't write sorting");
            ret = false;
        }
    });

    return ret;
}

NotificationSortingMap *NotificationSortingMap::Unmarshalling(Parcel &parcel)
{
    std::vector<NotificationSorting> sortings;
    // read sorting num
    int32_t size = 0;
    parcel.ReadInt32(size);

    for (int i = 0; i < size; i++) {
        // read sorting
        NotificationSorting *sorting = parcel.ReadParcelable<NotificationSorting>();
        if (sorting != nullptr) {
            sortings.push_back(*sorting);
        }
    }

    NotificationSortingMap *sortingMap = new NotificationSortingMap(sortings);
    return sortingMap;
}
std::string NotificationSortingMap::Dump() const
{
    int keyNum = 0;
    std::string separator;
    std::string keys = " sortedkey = ";
    for (auto item : sortedKey_) {
        if (keyNum == 0) {
            separator = "";
        } else {
            separator = ", ";
        }
        keys = keys + separator + item;
        keyNum++;
    }
    return "NotificationSortingMap[" + keys + "]";
}
}  // namespace Notification
}  // namespace OHOS
