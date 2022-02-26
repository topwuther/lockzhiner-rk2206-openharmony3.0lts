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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_H

#include <string>
#include <vector>

#include "ans_log_wrapper.h"
#include "notification_slot.h"
#include "parcel.h"
#include "uri.h"

namespace OHOS {
namespace Notification {

class NotificationSorting final : public Parcelable {
public:
    /**
     * @brief Default constructor used to create an empty NotificationSorting instance.
     */
    NotificationSorting();

    /**
     * @brief Default deconstructor used to deconstruct.
     */
    ~NotificationSorting();

    /**
     * @brief A constructor used to create a NotificationSorting instance by copying parameters from an existing one.
     *
     * @param sorting Indicates the NotificationSorting object.
     */
    NotificationSorting(const NotificationSorting &sorting);

    /**
     *  @brief Obtains the sequence number of a notification among all the active notifications.
     *
     *  @return Returns the sequence number of the notification.
     */
    inline int32_t GetRanking() const
    {
        return ranking_;
    };

    /**
     * @brief Obtains the notification hash code, which is unique in the current application.
     * Generally, the notification hash code is a string in the format
     * Notification ID_Creator package name_Creator UID_Owner package name.
     *
     * @return Returns the notification hash code.
     */
    inline std::string GetKey() const
    {
        return key_;
    };

    /**
     * @brief Obtains the importance level of the current notification set in the corresponding NotificationSlot.
     *
     * @return Returns the importance level of the notification.
     */
    inline int32_t GetImportance() const
    {
        return importance_;
    };

    /**
     * @brief Obtains the NotificationSlot the current notification belongs to.
     * Each notification must be in a particular NotificationSlot.
     *
     * @return Returns the NotificationSlot of the notification.
     */
    inline NotificationSlot GetSlot() const
    {
        return *slot_;
    };

    /**
     * @brief Obtains the visibility of the current notification on the lock screen set.
     *
     * @return Returns the visibility of the notification on the lock screen.
     */
    inline int32_t GetVisiblenessOverride() const
    {
        return visiblenessOverride_;
    };

    /**
     * @brief Checks whether the badge is displayed for the current notification.
     *
     * @return Returns true if the badge is displayed; returns false otherwise.
     */
    inline bool IsDisplayBadge() const
    {
        return isDisplayBadge_;
    };

    /**
     * @brief Checks whether the current notification is hidden.
     * A notification should be hidden if the application sending the notification is suspended.
     *
     * @return Returns true if the notification is hidden; returns false otherwise.
     */
    inline bool IsHiddenNotification() const
    {
        return isHiddenNotification_;
    };

    /**
     * @brief Obtains the overridden notification group key. If the system has overridden the group key,
     * a non-null value will be returned.
     *
     * @return Returns the overridden notification group key used to bind notifications.
     */
    inline std::string GetGroupKeyOverride() const
    {
        return groupKeyOverride_;
    };

    /**
     * @brief Marshals a NotificationSorting object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     *
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals a NotificationSorting object from a Parcel.
     *
     * @param Indicates the Parcel object for unmarshalling.
     *
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static NotificationSorting *Unmarshalling(Parcel &parcel);

    /**
     * @brief Dump sorting info
     *
     * @return Sorting info
     */
    std::string Dump() const;

private:
    void SetGroupKeyOverride(const std::string &str);
    void SetKey(const std::string &key);
    void SetImportance(const int32_t &importance);
    void SetRanking(const int32_t &ranking);
    void SetSlot(const sptr<NotificationSlot> &slot);
    void SetVisiblenessOverride(const int32_t &visibleness);
    void SetDisplayBadge(const bool &isDisplayBadge);
    void SetHiddenNotification(const bool &isHiddenNotfication);
    bool ReadFromParcel(Parcel &parcel);

private:
    std::string key_ {};
    int32_t ranking_ {-1};
    int32_t importance_ {-1};
    bool isDisplayBadge_ {true};
    bool isHiddenNotification_ {};
    std::string groupKeyOverride_ {};
    int32_t visiblenessOverride_ {};
    sptr<NotificationSlot> slot_ {nullptr};

    friend class AdvancedNotificationService;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SORTING_H