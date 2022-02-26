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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_MAP_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_MAP_H

#include <map>
#include <string>
#include <vector>

#include "notification_sorting.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {

class NotificationSortingMap final : public Parcelable {
public:
    /**
     * @brief Default constructor used to create an empty NotificationMap instance.
     */
    NotificationSortingMap();

    /**
     * @brief Default deconstructor used to deconstruct.
     */
    ~NotificationSortingMap();

    /**
     * @brief A constructor used to create a NotificationSortingMap instance by copying parameters from an existing one.
     *
     * @param Indicates the NotificationSortingMap object.
     */
    NotificationSortingMap(const std::vector<NotificationSorting> &sortingList);

    /**
     * @brief Obtains the list of sorted hash codes.
     *
     * @return Returns the list of sorted hash codes.
     */
    inline std::vector<std::string> GetKey()
    {
        return sortedKey_;
    }

    /**
     * @brief Obtains NotificationSorting based on the hash codes.
     *
     * @param key Indicates the hash codes obtained by using getkey().
     *
     * @param sorting Indicates the sorting information about the hash codes.
     *
     */
    bool GetNotificationSorting(const std::string &key, NotificationSorting &sorting) const;

    /**
     * @brief Marshals a NotificationSortingMap object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     *
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals a NotificationSortingMap object from a Parcel.
     *
     * @param Indicates the Parcel object for unmarshalling.
     */
    static NotificationSortingMap *Unmarshalling(Parcel &parcel);

    /**
     * @brief Dump sorting map info
     *
     * @return Return sorting map info to string.
     */
    std::string Dump() const;

private:
    void SetKey(const std::string &key);
    void SetNotificationSorting(const std::vector<NotificationSorting> &sortingList);

private:
    std::vector<std::string> sortedKey_ {};
    std::map<std::string, NotificationSorting> sortings_ {};
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_MAP_H