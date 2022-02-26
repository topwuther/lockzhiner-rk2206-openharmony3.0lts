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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_H

#include <memory>
#include <vector>

#include "notification_request.h"
#include "parcel.h"
#include "uri.h"

namespace OHOS {
namespace Notification {

class Notification final : public Parcelable {
public:
    /**
     * A constructor used to create a Notification instance by existing NotificationRequest object.
     *
     * @param request the existing NotificationRequest object.
     */
    Notification(const sptr<NotificationRequest> &request);

    /**
     * @brief A constructor used to create a Notification instance by copying parameters from an existing one.
     *
     * @param other Indicates the Notification object.
     */
    Notification(const Notification &other);

    /**
     * @brief Default destructor.
     */
    ~Notification();

    /**
     * @brief Obtains whether to enable the notification light when a notification is received on the device, provided
     * that this device has a notification light.
     *
     * @return Return true if led light color is set.
     */
    bool EnableLight() const;

    /**
     * @brief Obtains the sound enabled or not, set by ANS.
     *
     * @return Return true if sound is set.
     */
    bool EnableSound() const;

    /**
     * @brief Obtains the vibrate enabled or not, set by ANS.
     *
     * @return Return true if vibrate style is set.
     */
    bool EnableVibrate() const;

    /**
     * @brief Obtains the bundle's name which publish this notification.
     *
     * @return Return the bundle's name.
     */
    std::string GetBundleName() const;

    /**
     * @brief Obtains the bundle's name which create this notification.
     *
     * @return Return the creator bundle name.
     */
    std::string GetCreateBundle() const;

    /**
     * @brief Obtains the label of this notification.
     *
     * @return Return the label.
     */
    std::string GetLabel() const;

    /**
     * @brief Obtains the color of the notification light in a NotificationSlot object
     *
     * @return Returns the color of the notification light.
     */
    int32_t GetLedLightColor() const;
    /**
     * @brief Sets the notification display effect, including whether to display this notification on the lock screen,
     * and how it will be presented if displayed.
     *
     * @return Return the display effect of this notification on the lock screen.
     */
    NotificationConstant::VisiblenessType GetLockscreenVisibleness() const;

    /**
     * @brief The ID passed to setGroup(), or the override, or null.
     *
     * @return string of group.
     */
    std::string GetGroup() const;

    /**
     * @brief Obtains the id of the notification.
     *
     * @return The id supplied to NotificationManager::Notify(int, NotificationRequest).
     */
    int32_t GetId() const;

    /**
     * @brief A key for this notification record.
     *
     * @return A unique instance key.
     */
    std::string GetKey() const;

    /**
     * @brief Obtains the notificanton request set by ANS.
     *
     * @return Return sptr<NotificationRequest>.
     */
    NotificationRequest GetNotificationRequest() const;

    /**
     * @brief Obtains the time notification was posted.
     *
     * @return The time notificationRequest was posted.
     */
    int64_t GetPostTime() const;

    /**
     * @brief Obtains the sound uri.
     *
     * @return Return the sound set by ANS.
     */
    Uri GetSound() const;

    /**
     * @brief Obtains the UID of the notification creator.
     *
     * @return Return the UID of the notification creator.
     */
    pid_t GetUid() const;

    /**
     * @brief Obtains the PID of the notification creator.
     *
     * @return Return the PID of the notification creator.
     */
    pid_t GetPid() const;

    /**
     * @brief Obtains the vibration style for this notifications. if
     *
     * @return Return the vibration style.
     */
    std::vector<int64_t> GetVibrationStyle() const;

    /**
     * @brief This notification is part of a group or not.
     *
     * @return true if this notification is part of a group.
     */
    bool IsGroup() const;

    /**
     * @brief Checks whether this notification is displayed as a floating icon on top of the screen.
     *
     * @return true if this notification is displayed as a floating icon; returns false otherwise.
     */
    bool IsFloatingIcon() const;

    /**
     * @brief Dumps a string representation of the object.
     *
     * @return A string representation of the object.
     */
    std::string Dump() const;

    /**
     * @brief Marshals a Notification object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     *
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    bool Marshalling(Parcel &parcel) const;

    /**
     * @brief Unmarshals a Notification object from a Parcel.
     *
     * @param Indicates the Parcel object for unmarshalling.
     *
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static Notification *Unmarshalling(Parcel &parcel);

private:
    Notification();
    void SetEnableSound(const bool &enable);
    void SetEnableLight(const bool &enable);
    void SetEnableViration(const bool &enable);
    void SetLedLightColor(const int32_t &color);
    void SetLockScreenVisbleness(const NotificationConstant::VisiblenessType &visbleness);
    void SetPostTime(const int64_t &time);
    void SetSound(const Uri &sound);
    void SetVibrationStyle(const std::vector<int64_t> &style);
    bool ReadFromParcel(Parcel &parcel);
    std::string GenerateNotificationKey(int32_t uid, const std::string &label, int32_t id);

private:
    bool enableSound_ {false};
    bool enableLight_ {false};
    bool enableViration_ {false};
    std::string key_ {""};
    int32_t ledLightColor_ {0};
    NotificationConstant::VisiblenessType lockscreenVisibleness_ {NotificationConstant::VisiblenessType::NO_OVERRIDE};
    sptr<NotificationRequest> request_ {nullptr};
    int64_t postTime_ {0};
    std::shared_ptr<Uri> sound_ {nullptr};
    std::vector<int64_t> vibrationStyle_ {};

    friend class AdvancedNotificationService;
    friend class NotificationSlotFilter;
    friend class DisturbFilter;
};
}  // namespace Notification
}  // namespace OHOS
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_H