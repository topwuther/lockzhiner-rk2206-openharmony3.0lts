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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_H

#include <string>

#include "notification_content.h"
#include "notification_request.h"
#include "parcel.h"
#include "uri.h"

namespace OHOS {
namespace Notification {
class NotificationSlot : public Parcelable {
public:
    enum NotificationLevel {
        LEVEL_NONE,       // the notification function is disabled.
        LEVEL_MIN,        // the notifications function is disabled on the notification panel,
                          // with no banner or prompt tone
        LEVEL_LOW,        // the notifications are displayed on the notification panel,
                          // with no banner or prompt tone
        LEVEL_DEFAULT,    // the notification function is enabled and notifications are displayed,
                          // on the notification panel, with a banner and a prompt tone.
        LEVEL_HIGH,       // the notifications are displayed on the notification panel,
                          // with a banner and a prompt tone
        LEVEL_UNDEFINED,  // the notification does not define an level.
    };

    /**
     * A constructor used to initialize the type of a NotificationSlot object.
     *
     * @param type  Specifies the type of the NotificationSlot object,
     */
    NotificationSlot(NotificationConstant::SlotType type = NotificationConstant::SlotType::CUSTOM);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~NotificationSlot();

    /**
     * Obtains whether the notification light is enabled in a NotificationSlot object,
     * which is set by SetEnableLight(bool).
     *
     * @return Returns true if the notification light is enabled; returns false otherwise.
     */
    bool CanEnableLight() const;

    /**
     * Specifies whether to enable the notification light when a notification is received on the device,
     * provided that this device has a notification light.
     * @note SetEnableLight must be set before the NotificationHelper:AddNotificationSlot(NotificationSlot) method is
     * called. Otherwise, the settings will not take effect.
     *
     * @param isLightEnabled Specifies whether to enable the pulse notification light.
     *                       The value true indicates to enable the notification light,
     *                       and the value false indicates not to enable it.
     */
    void SetEnableLight(bool isLightEnabled);

    /**
     * Obtains the vibration status of a NotificationSlot object,
     * which is set by SetEnableVibration(bool).
     *
     * @return Returns true if vibration is enabled; returns false otherwise.
     */
    bool CanVibrate() const;

    /**
     * Sets whether to enable vibration when a notification is received.
     * @note SetEnableVibration(bool) must be set before the NotificationHelper::AddNotificationSlot(NotificationSlot)
     * method is called. Otherwise, the settings will not take effect.
     *
     * @param vibration Indicates whether to enable vibration when a notification is received.
     *                  If the value is true, vibration is enabled; if the value is false, vibration is disabled.
     */
    void SetEnableVibration(bool vibration);

    /**
     * Obtains the description of a NotificationSlot object, which is set by SetDescription(string).
     *
     * @return Returns the description of the NotificationSlot object.
     */
    std::string GetDescription() const;

    /**
     * Sets the description for a NotificationSlot object.
     * @note The setting of setDescription is effective regardless of whether a NotificationSlot object has been created
     * by NotificationHelper::AddNotificationSlot(NotificationSlot).
     *
     * @param description Describes the NotificationSlot object.
     *                    The description is visible to users and its length must not exceed 1000 characters
     *                    (the excessive part is automatically truncated).
     */
    void SetDescription(const std::string &description);

    /**
     * Obtains the ID of a NotificationSlot object.
     *
     * @return Returns the ID of the NotificationSlot object,
     *         which is set by NotificationSlot(string, string, NotificationLevel).
     */
    std::string GetId() const;

    /**
     * Obtains the color of the notification light in a NotificationSlot object,
     * which is set by SetLedLightColor(int32_t).
     *
     * @return Returns the color of the notification light.
     */
    int32_t GetLedLightColor() const;

    /**
     * Sets the color of the notification light to flash when a notification is received on the device,
     * provided that this device has a notification light and setEnableLight is called with the value true.
     * @note SetLedLightColor must be set before the NotificationHelper::AddNotificationSlot(NotificationSlot) method is
     * called. Otherwise, the settings will not take effect.
     *
     * @param color Indicates the color of the notification light.
     */
    void SetLedLightColor(int32_t color);

    /**
     * Obtains the level of a NotificationSlot object, which is set by SetLevel(NotificationLevel).
     *
     * @return Returns the level of the NotificationSlot object.
     */
    NotificationLevel GetLevel() const;

    /**
     * Sets the level of a NotificationSlot object.
     * @note SetLevel must be set before the NotificationHelper::AddNotificationSlot(NotificationSlot) method is called.
     *       Otherwise, the settings will not take effect.
     *
     * @param level Specifies the level of the NotificationSlot object, which determines the notification display
     * effect. The value can be LEVEL_NONE, LEVEL_MIN, LEVEL_LOW, LEVEL_DEFAULT, or LEVEL_HIGH.
     */
    void SetLevel(NotificationLevel level);

    /**
     * Obtains the type of a NotificationSlot object, which is set by SetType(SlotType).
     *
     * @return Returns the Type of the NotificationSlot object.
     */
    NotificationConstant::SlotType GetType() const;

    /**
     * Sets the type of a NotificationSlot object.
     * @note Settype must be set before the NotificationHelper::AddNotificationSlot(NotificationSlot) method is called.
     *       Otherwise, the settings will not take effect.
     *
     * @param type Specifies the type of the NotificationSlot object, which determines the notification remind mode.
     *        The value can be DEFAULT, SOCIAL_COMMUNICATION, SERVICE_REMINDER, CONTENT_INFORMATION, or OTHER.
     */
    void SetType(NotificationConstant::SlotType type);

    /**
     * Obtains the notification display effect of a NotificationSlot object on the lock screen,
     * which is set by SetLockscreenVisibleness(int32_t).
     * @note This method specifies different effects for displaying notifications on the lock screen in order to protect
     * user privacy. The setting takes effect only when the lock screen notifications function is enabled for an
     * application in system notification settings.
     *
     * @return Returns the notification display effect of the NotificationSlot object on the lock screen.
     */
    NotificationConstant::VisiblenessType GetLockScreenVisibleness() const;

    /**
     * Sets whether and how to display notifications on the lock screen.
     *
     * @param visibleness Specifies the notification display effect on the lock screen, which can be set to
     *                    NO_OVERRIDE, PUBLIC, PRIVATE, or SECRET.
     */
    void SetLockscreenVisibleness(NotificationConstant::VisiblenessType visibleness);

    /**
     * Obtains the name of a NotificationSlot object.
     *
     * @return Returns the name of the NotificationSlot object, which is set by SetName(string).
     */
    std::string GetName() const;

    /**
     * Obtains the ID of the NotificationSlotGroup object to which this NotificationSlot object belongs,
     * which is set by SetSlotGroup(string).
     *
     * @return Returns the ID of the NotificationSlotGroup to which this NotificationSlot object belongs.
     */
    std::string GetSlotGroup() const;

    /**
     * Binds a NotificationSlot object to a specified NotificationSlotGroup.
     * @note SetSlotGroup must be called before the NotificationHelper::AddNotificationSlot(NotificationSlot) method is
     * called. Otherwise, this method will not take effect.
     * @param groupId Indicates the ID of the NotificationSlotGroup object to bind,
     *                which must have been created by calling
     * NotificationHelper::AddNotificationSlotGroup(NotificationSlotGroup).
     */
    void SetSlotGroup(const std::string &groupId);

    /**
     * Obtains the prompt tone of a NotificationSlot object, which is set by SetSound(Uri).
     *
     * @return Returns the prompt tone of the NotificationSlot object.
     */
    Uri GetSound() const;

    /**
     * Sets a prompt tone for a NotificationSlot object, which will be played after a notification is received.
     * @note SetSound must be set before the NotificationHelper:AddNotificationSlot(NotificationSlot) method is called.
     *       Otherwise, the settings will not take effect.
     *
     * @param sound Specifies the path for the prompt tone.
     */
    void SetSound(const Uri &sound);

    /**
     * Obtains the vibration style of notifications in this NotificationSlot.
     *
     * @return Returns the vibration style of this NotificationSlot.
     */
    std::vector<int64_t> GetVibrationStyle() const;

    /**
     * Sets the vibration style for notifications in this NotificationSlot.
     * @note If an empty array or null is passed to this method, the system then calls
     *       SetEnableVibration(bool) with the input parameter set to false.
     *       If a valid value is passed to this method, the system calls SetEnableVibration(bool) with the input
     * parameter set to true. This method takes effect only before
     * NotificationHelper::AddNotificationSlot(NotificationSlot) is called.
     *
     * @param vibration Indicates the vibration style to set.
     */
    void SetVibrationStyle(const std::vector<int64_t> &vibration);

    /**
     * Obtains whether DND mode is bypassed for a NotificationSlot object,
     * which is set by EnableBypassDnd(bool).
     *
     * @return Returns true if DND mode is bypassed; returns false otherwise.
     */
    bool IsEnableBypassDnd() const;

    /**
     * Sets whether to bypass Do not disturb (DND) mode in the system.
     * @note The setting of EnableBypassDnd takes effect only when the Allow interruptions function
     *       is enabled for an application in system notification settings.
     *
     * @param isBypassDnd Specifies whether to bypass DND mode for an application.
     *                    If the value is true, DND mode is bypassed;
     *                    if the value is false, DND mode is not bypassed.
     */
    void EnableBypassDnd(bool isBypassDnd);

    /**
     * Obtains the application icon badge status of a NotificationSlot object,
     * which is set by EnableBadge(bool).
     *
     * @return Returns true if the application icon badge is enabled; returns false otherwise.
     */
    bool IsShowBadge() const;

    /**
     * Sets whether to display application icon badges (digits or dots in the corner of the application icon)
     * on the home screen after a notification is received.
     * @note EnableBadge must be set before the NotificationHelper:AddNotificationSlot(NotificationSlot) method is
     * called. Otherwise, the settings will not take effect.
     *
     * @param isShowBadge Specifies whether to display the application icon badge.
     *                    If the value is true, the application icon badge is enabled;
     *                    if the value is false, the application icon badge is disabled..
     */
    void EnableBadge(bool isShowBadge);

    /**
     * Dumps a string representation of the object.
     *
     * @return A string representation of the object.
     */
    std::string Dump() const;

    /**
     * Marshals a NotificationSlot object into a Parcel.
     *
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshals a NotificationSlot object from a Parcel.
     *
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static NotificationSlot *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read NotificationSlot object from a Parcel.
     *
     * @param parcel the parcel
     * @return read from parcel success or fail
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * Merge the contents of vector and output a string
     *
     * @param mergeVector The vector which will be merged
     * @return Returns the string that has contents of the vector
     */
    std::string MergeVectorToString(const std::vector<int64_t> &mergeVector) const;

    /**
     * If string length exceed 1000 characters, the excessive part is automatically truncated.
     *
     * @param in The sting which will be truncated
     * @return Returns the string that has been truncated.
     */
    std::string TruncateString(const std::string &in);

    /**
     * Sets the name of a NotificationSlot object.
     * @note The setting of SetName is effective regardless of whether a NotificationSlot object has been created by
     *       NotificationHelper:AddNotificationSlot(NotificationSlot).
     *
     * @param name Specifies the name of the NotificationSlot object.
     *             The name is visible to users, and its length must not exceed 1000 characters
     *             (the excessive part is automatically truncated).
     */
    void SetName(const std::string &name);

private:
    std::string id_ {};
    std::string name_ {};
    bool isLightEnabled_ {false};
    bool isVibrationEnabled_ {false};
    bool isShowBadge_ {true};
    bool isBypassDnd_ {false};
    std::string description_ {};
    int32_t lightColor_ {0};
    NotificationLevel level_ {LEVEL_DEFAULT};
    NotificationConstant::SlotType type_ {};
    NotificationConstant::VisiblenessType lockScreenVisibleness_ {NotificationConstant::VisiblenessType::NO_OVERRIDE};
    std::string groupId_ {};
    Uri sound_;
    std::vector<int64_t> vibrationValues_ {};

    // no object in parcel
    static constexpr int VALUE_NULL = -1;
    // object exist in parcel
    static constexpr int VALUE_OBJECT = 1;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SLOT_H