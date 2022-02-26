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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_REQUEST_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_REQUEST_H

#include <memory>
#include <string>
#include <vector>
#include "message_user.h"
#include "notification_action_button.h"
#include "notification_content.h"
#include "want_agent.h"
#include "context.h"
#include "ohos/aafwk/content/want_params.h"
#include "pixel_map.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
class NotificationRequest : public Parcelable {
public:
    enum class BadgeStyle {
        /**
         * displays only numbers.
         */
        NONE,
        /**
         * displayed as a large icon.
         */
        BIG,
        /**
         * displayed as a small icon.
         */
        LITTLE
    };

    enum class GroupAlertType {
        /**
         * all notifications in a group have sound or vibration if sound or vibration is enabled
         * for the associated NotificationSlot objects.
         */
        ALL,
        /**
         * child notifications have sound or vibration but the overview notification is muted (no sound or vibration)
         * in a group if sound or vibration is enabled for the associated NotificationSlot objects.
         */
        CHILD,
        /**
         * the overview notification has sound or vibration but child notifications are muted (no sound or vibration)
         * in a group if sound or vibration is enabled for the associated NotificationSlot objects.
         */
        OVERVIEW
    };

    /**
     * Indicates the classification of notifications for alarms or timers.
     */
    static const std::string CLASSIFICATION_ALARM;
    /**
     * Indicates the classification of notifications for incoming calls or similar synchronous communication requests.
     */
    static const std::string CLASSIFICATION_CALL;
    /**
     * Indicates the classification of notifications for emails.
     */
    static const std::string CLASSIFICATION_EMAIL;
    /**
     * Indicates the classification of notifications for errors occurred during background operations or identity
     * authentication.
     */
    static const std::string CLASSIFICATION_ERROR;
    /**
     * Indicates the classification of notifications for calendar events.
     */
    static const std::string CLASSIFICATION_EVENT;
    /**
     * Indicates the classification of notifications for short messages or instant messages.
     */
    static const std::string CLASSIFICATION_MESSAGE;
    /**
     * Indicates the classification of notifications for map navigation.
     */
    static const std::string CLASSIFICATION_NAVIGATION;
    /**
     * Indicates the classification of notifications for processes that are operated in the background for a long time.
     */
    static const std::string CLASSIFICATION_PROGRESS;
    /**
     * Indicates the classification of notifications for advertisement or promotion information.
     */
    static const std::string CLASSIFICATION_PROMO;
    /**
     * Indicates the classification of notifications for specific and timely recommendations of a particular
     * transaction.
     */
    static const std::string CLASSIFICATION_RECOMMENDATION;
    /**
     * Indicates the classification of notifications for reminders previously set by the user.
     */
    static const std::string CLASSIFICATION_REMINDER;
    /**
     * Indicates the classification of notifications for ongoing background services.
     */
    static const std::string CLASSIFICATION_SERVICE;
    /**
     * Indicates the classification of notifications for social network or sharing updates.
     */
    static const std::string CLASSIFICATION_SOCIAL;
    /**
     * Indicates the classification of notifications for ongoing information about the device and contextual status.
     */
    static const std::string CLASSIFICATION_STATUS;
    /**
     * Indicates the classification of notifications for system or device status updates.
     */
    static const std::string CLASSIFICATION_SYSTEM;
    /**
     * Indicates the classification of notifications for media transport control during playback.
     */
    static const std::string CLASSIFICATION_TRANSPORT;

    /**
     * Indicates the default notification background color, which means that no color is displayed.
     */
    static const uint32_t COLOR_DEFAULT;

private:
    /**
     * Indicates the color mask, used for calculation with the ARGB value set by setColor(int32_t).
     */
    static const uint32_t COLOR_MASK;

    /**
     * the maximum number of user input history is 5.
     */
    static const std::size_t MAX_USER_INPUT_HISTORY;

    /**
     * the maximum number of action buttons is 3.
     */
    static const std::size_t MAX_ACTION_BUTTONS;

public:
    /**
     * Default constructor used to create a NotificationRequest instance.
     */
    NotificationRequest() = default;

    /**
     * A constructor used to create a NotificationRequest instance with the input parameter notificationId passed.
     * @param notificationId notification ID
     */
    explicit NotificationRequest(int32_t notificationId);

    /**
     * A constructor used to create a NotificationRequest instance based on the specified context and notification ID.
     * To publish a notification with a custom view, you must call such a constructor with the Context specified.
     * @param context the specified context
     * @param notificationId notification ID
     */
    NotificationRequest(const std::shared_ptr<AppExecFwk::Context> &context, int32_t notificationId);

    /**
     * A constructor used to create a NotificationRequest instance by copying parameters from an existing one.
     * @param other the existing object
     */
    NotificationRequest(const NotificationRequest &other);

    /**
     * A constructor used to create a NotificationRequest instance by copying parameters from an existing one.
     * @param other the existing object
     */
    NotificationRequest &operator=(const NotificationRequest &other);

    /**
     * Default deconstructor used to deconstruct.
     */
    virtual ~NotificationRequest();

    /**
     * Checks whether this notification is in progress.
     * @return true if this notification is in progress; returns false otherwise.
     */
    bool IsInProgress() const;

    /**
     * Sets whether this notification is in progress.
     * Users cannot directly dismiss notifications in progress because
     * they usually contain some ongoing background services such as music playback.
     * @param isOngoing Specifies whether this notification is in progress.
     */
    void SetInProgress(bool isOngoing);

    /**
     * Checks whether this notification is unremovable.
     * @return true if this notification is unremovable; returns false otherwise.
     */
    bool IsUnremovable() const;

    /**
     * Sets whether this notification is unremovable.
     * If it is set to be unremovable, it cannot be removed by users.
     * @param isUnremovable Specifies whether this notification is unremovable.
     */
    void SetUnremovable(bool isUnremovable);

    /**
     * Sets the number to be displayed for this notification.
     * @param number Indicates the number to set.
     */
    void SetBadgeNumber(int32_t number);

    /**
     * Obtains the number to be displayed for this notification.
     * @return the number to be displayed for this notification.
     */
    int32_t GetBadgeNumber() const;

    /**
     * Sets the current notification ID to uniquely identify the notification in the application.
     * After a notification is received, its ID is obtained by using the getNotificationId() method.
     * @param notificationId Indicates the ID of the notification to be set.
     */
    void SetNotificationId(int32_t notificationId);

    /**
     * Obtains the notification ID, which is unique in the current application.
     * @return the notification ID.
     */
    int32_t GetNotificationId() const;

    /**
     * Adds an WantAgent to this notification.
     * After a notification is tapped,
     * subsequent operations such as ability and common events will be triggered as set by WantAgent.
     * @param wantAgent Indicates the operation triggered by tapping the notification, which can be set by
     * WantAgent.
     */
    void SetWantAgent(const std::shared_ptr<WantAgent::WantAgent> &wantAgent);

    /**
     * Obtains the WantAgent contained in this notification.
     * @return the WantAgent contained in this notification.
     */
    const std::shared_ptr<WantAgent::WantAgent> GetWantAgent() const;

    /**
     * Sets an WantAgent object that is triggered when the user explicitly removes this notification.
     * @param wantAgent Indicates the WantAgent object to be triggered.
     */
    void SetRemovalWantAgent(const std::shared_ptr<WantAgent::WantAgent> &wantAgent);

    /**
     * Obtains the WantAgent object that is triggered when the user explicitly removes this notification.
     * @return the WantAgent object to be triggered.
     */
    const std::shared_ptr<WantAgent::WantAgent> GetRemovalWantAgent() const;

    /**
     * Sets the WantAgent to start when the device is not in use,
     * instead of showing this notification in the status bar.
     * When the device is in use, the system UI displays a pop-up notification
     * instead of starting the WantAgent specified by maxScreenWantAgent.
     * Your application must have the ohos.permission.USE_WHOLE_SCREEN permission to use this method.
     * @param wantAgent Indicates the WantAgent object containing information about the to-be-started ability that
     * uses the Page template.
     */
    void SetMaxScreenWantAgent(const std::shared_ptr<WantAgent::WantAgent> &wantAgent);

    /**
     * Obtains the full-screen WantAgent set by calling setMaxScreenWantAgent(WantAgent).
     * @return the full-screen WantAgent.
     */
    const std::shared_ptr<WantAgent::WantAgent> GetMaxScreenWantAgent() const;

    /**
     * Sets extra parameters that are stored as key-value pairs for the notification.
     * @param extras Indicates the WantParams object containing the extra parameters in key-value pair format.
     */
    void SetAdditionalData(const std::shared_ptr<AAFwk::WantParams> &extras);

    /**
     * Obtains the WantParams object set in the notification.
     * @return the WantParams object.
     */
    const std::shared_ptr<AAFwk::WantParams> GetAdditionalData() const;

    /**
     * Sets the time to deliver a notification.
     * @param deliveryTime Indicates the time in milliseconds.
     */
    void SetDeliveryTime(int64_t deliveryTime);

    /**
     * Obtains the time when a notification is delivered.
     * @return the time in milliseconds.
     */
    int64_t GetDeliveryTime() const;

    /**
     * Checks whether the notification delivery time is displayed for this notification.
     * @return true if the time is displayed; returns false otherwise.
     */
    bool IsShowDeliveryTime() const;

    /**
     * Sets whether to show the notification delivery time for this notification.
     * This method is valid only when the notification delivery time has been set by calling setDeliveryTime(int64_t).
     * @param showDeliveryTime Specifies whether to show the notification delivery time.
     */
    void SetShowDeliveryTime(bool showDeliveryTime);

    /**
     * Adds a NotificationActionButton to this notification.
     * An operation button is usually placed next to the notification content by the system. 
     * Each action button must contain an icon, a title, and an WantAgent. When a notification is expanded,
     * a maximum of three action buttons can be displayed from left to right in the order they were added.
     * When the notification is collapsed, no action buttons will be displayed.
     * @param actionButton Indicates the NotificationActionButton object to add.
     */
    void AddActionButton(const std::shared_ptr<NotificationActionButton> &actionButton);

    /**
     * Obtains the list of all NotificationActionButton objects included in this notification.
     * @return the list of NotificationActionButton objects.
     */
    const std::vector<std::shared_ptr<NotificationActionButton>> GetActionButtons() const;

    /**
     * Checks whether the platform is allowed to generate contextual NotificationActionButton objects for this
     * notification.
     * @return true if the platform is allowed to generate contextual NotificationActionButton objects;
     * returns false otherwise.
     */
    bool IsPermitSystemGeneratedContextualActionButtons() const;

    /**
     * Sets whether to allow the platform to generate contextual NotificationActionButton objects for this notification.
     * @param permitted Specifies whether to allow the platform to generate contextual NotificationActionButton objects.
     * The default value true indicates that the platform is allowed to generate contextual action buttons,
     * and the value false indicates not.
     */
    void SetPermitSystemGeneratedContextualActionButtons(bool permitted);

    /**
     * Adds a MessageUser object and associates it with this notification.
     * @param messageUser Indicates the MessageUser object to add.
     */
    void AddMessageUser(const std::shared_ptr<MessageUser> &messageUser);

    /**
     * Obtains all MessageUser objects associated with this notification.
     * @return the list of MessageUser objects associated with this notification.
     */
    const std::vector<std::shared_ptr<MessageUser>> GetMessageUsers() const;

    /**
     * Checks whether this notification is set to alert only once,
     * which means that sound or vibration will no longer be played
     * for notifications with the same ID upon their updates.
     * @return true if this notification is set to alert only once; returns false otherwise.
     */
    bool IsAlertOneTime() const;

    /**
     * Sets whether to have this notification alert only once.
     * If a notification alerts only once, sound or vibration will no longer be played
     * for notifications with the same ID upon their updates after they are published.
     * @param isAlertOnce Specifies whether to have this notification alert only once.
     */
    void SetAlertOneTime(bool isAlertOnce);

    /**
     * Sets the time to delete a notification.
     * @param deletedTime Indicates the time in milliseconds.
     * The default value is 0, indicating that the notification will not be automatically deleted.
     * To enable the notification to be automatically deleted, set this parameter to an integer greater than 0.
     */
    void SetAutoDeletedTime(int64_t deletedTime);

    /**
     * Obtains the period during which a notification is deleted.
     * @return the period in milliseconds.
     */
    int64_t GetAutoDeletedTime() const;

    /**
     * Sets the little icon of the notification.
     * @param littleIcon Indicates the icon of the notification.
     */
    void SetLittleIcon(const std::shared_ptr<Media::PixelMap> &littleIcon);

    /**
     * Obtains the icon of the notification.
     * @return the notification icon.
     */
    const std::shared_ptr<Media::PixelMap> GetLittleIcon() const;

    /**
     * Sets the large icon of this notification, which is usually displayed on the right of the notification.
     * @param bigIcon Indicates the large icon to set. It must be a PixelMap object.
     */
    void SetBigIcon(const std::shared_ptr<Media::PixelMap> &bigIcon);

    /**
     * Obtains the large icon of this notification.
     * @return the large icon of this notification.
     */
    const std::shared_ptr<Media::PixelMap> GetBigIcon() const;

    /**
     * Sets the classification of this notification, which describes the purpose of this notification.
     * Notification classifications are used to filter and sort notifications.
     * @param classification Indicates the notification classification predefined in the system,
     * such as CLASSIFICATION_CALL or CLASSIFICATION_NAVIGATION etc.
     */
    void SetClassification(const std::string &classification);

    /**
     * Obtains the classification of this notification.
     * @return the classification of this notification.
     */
    std::string GetClassification() const;

    /**
     * Sets the background color of this notification.
     * This method is valid only when background color has been enabled by calling setColorEnabled(bool).
     * @param color Indicates the background color to set. For details about the value range, see Color.
     */
    void SetColor(uint32_t color);

    /**
     * Obtains the background color of this notification.
     * The return value, except for the default color COLOR_DEFAULT,
     * is the bitwise OR operation result of 0xFF000000 and the ARGB value set by setColor(uint32_t).
     * @return the background color of this notification.
     */
    uint32_t GetColor() const;

    /**
     * Checks whether background color is enabled for this notification.
     * @return true if background color is enabled; returns false otherwise.
     */
    bool IsColorEnabled();

    /**
     * Sets whether to enable background color for this notification.
     * If colorEnabled is set to true, this method takes effect only
     * when the notification content type has been set to NotificationRequest.
     * NotificationMediaContent in the NotificationRequest object through
     * NotificationRequest::setContent(NotificationContent) and an AVToken has been attached to
     * that NotificationMediaContent object through NotificationMediaContent::setAVToken(AVToken).
     * @param colorEnabled Specifies whether to enable background color.
     */
    void SetColorEnabled(bool colorEnabled);

    /**
     * Sets the notification content type to NotificationNormalContent, NotificationLongTextContent,
     * or NotificationPictureContent etc.
     * Each content type indicates a particular notification content.
     * @param content Indicates the notification content type.
     */
    void SetContent(const std::shared_ptr<NotificationContent> &content);

    /**
     * Obtains the notification content set by calling the setContent(NotificationContent) method.
     * @return the notification content.
     */
    const std::shared_ptr<NotificationContent> GetContent() const;

    /**
     * Obtains the notification type.
     * @return the type of the current notification, which can be
     * NotificationContent::Type::BASIC_TEXT,
     * NotificationContent::Type::LONG_TEXT,
     * NotificationContent::Type::PICTURE,
     * NotificationContent::Type::CONVERSATION,
     * NotificationContent::Type::MULTILINE,
     * or NotificationContent::Type::MEDIA.
     */
    NotificationContent::Type GetNotificationType() const;

    /**
     * Checks whether the notification creation time is displayed as a countdown timer.
     * @return true if the time is displayed as a countdown timer; returns false otherwise.
     */
    bool IsCountdownTimer() const;

    /**
     * Sets whether to show the notification creation time as a countdown timer.
     * This method is valid only when setShowStopwatch(boolean) is set to true.
     * @param isCountDown Specifies whether to show the notification creation time as a countdown timer.
     */
    void SetCountdownTimer(bool isCountDown);

    /**
     * Sets the group alert type for this notification,
     * which determines how the group overview and other notifications in a group are published.
     * The group information must have been set by calling setGroupValue(string).
     * Otherwise, this method does not take effect.
     * @param type Indicates the group alert type to set. which can be GroupAlertType::ALL (default value),
     * GroupAlertType::OVERVIEW, or GroupAlertType::CHILD etc.
     */
    void SetGroupAlertType(NotificationRequest::GroupAlertType type);

    /**
     * Obtains the group alert type of this notification.
     * @return the group alert type of this notification.
     */
    NotificationRequest::GroupAlertType GetGroupAlertType() const;

    /**
     * Checks whether this notification is the group overview.
     * @return true if this notification is the group overview; returns false otherwise.
     */
    bool IsGroupOverview() const;

    /**
     * Sets whether to use this notification as the overview of its group.
     * This method helps display the notifications that are assigned the same group value by calling
     * setGroupValue(string) as one stack in the notification bar.
     * Each group requires only one group overview. After a notification is set as the group overview,
     * it becomes invisible if another notification in the same group is published.
     * @param overView Specifies whether to set this notification as the group overview.
     */
    void SetGroupOverview(bool overView);

    /**
     * Sets the group information for this notification.
     * If no groups are set for notifications, all notifications from the same application will appear
     * in the notification bar as one stack with the number of stacked notifications displayed.
     * If notifications are grouped and there are multiple groups identified by different groupValue,
     * notifications with different groupValue will appear in separate stacks.
     * Note that one of the notifications in a group must be set as the overview of its group by calling
     * setGroupOverview(bool), and other notifications are considered as child notifications.
     * Otherwise, notifications will not be displayed as one group even if they are assigned the same groupValue by
     * calling setGroupValue(string).
     * @param groupValue Specifies whether to set this notification as the group overview.
     */
    void SetGroupValue(const std::string &groupValue);

    /**
     * Obtains the group information about this notification.
     * @return the group information about this notification.
     */
    std::string GetGroupValue() const;

    /**
     * Checks whether this notification is relevant only to the local device and cannot be displayed on remote devices.
     * @return true if this notification is relevant only to the local device; returns false otherwise.
     */
    bool IsOnlyLocal() const;

    /**
     * Sets whether this notification is relevant only to the local device and cannot be displayed on remote devices.
     * This method takes effect only for notifications published by calling
     * NotificationHelper::publishNotification(NotificationRequest) or
     * NotificationHelper#publishNotification(string, NotificationRequest).
     * Notifications published using NotificationHelper::publishNotification(NotificationRequest, string)
     * in a distributed system will not be affected.
     * @param flag Specifies whether this notification can be displayed only on the local device.
     */
    void SetOnlyLocal(bool flag);

    /**
     * Sets the text that will be displayed as a link to the settings of the application.
     * Calling this method is invalid if the notification content type has been set to NotificationLongTextContent
     * or NotificationPictureContent in the NotificationRequest object through setContent(NotificationContent).
     * @param text Indicates the text to be included. You can set it to any valid link.
     */
    void SetSettingsText(const std::string &text);

    /**
     * Obtains the text that will be displayed as a link to the settings of the application.
     * @return the text displayed as the link to the application settings.
     */
    std::string GetSettingsText() const;

    /**
     * Deprecated.
     * Obtains the time when a notification is created.
     * @return the time in milliseconds.
     */
    int64_t GetCreateTime() const;

    /**
     * Checks whether the notification creation time is displayed as a stopwatch.
     * @return true if the time is displayed as a stopwatch; returns false otherwise.
     */
    bool IsShowStopwatch() const;

    /**
     * Sets whether to show the notification creation time as a stopwatch.
     * This method is valid only when the notification creation time has been set by calling setDeliveryTime(int64_t).
     * When the notification creation time is set to be shown as a stopwatch, the interval between the current time
     * and the creation time set by setDeliveryTime(int64_t) is dynamically displayed for this notification
     * in Minutes: Seconds format. If the interval is longer than 60 minutes, it will be displayed
     * in Hours: Minutes: Seconds format. If this method and setShowDeliveryTime(boolean) are both set to true, only
     * this method takes effect, that is, the notification creation time will be shown as a stopwatch.
     * @param isShow Specifies whether to show the notification creation time as a stopwatch.
     */
    void SetShowStopwatch(bool isShow);

    /**
     * Sets the slot type of a notification to bind the created NotificationSlot object.
     * You can use NotificationSlot to create a slot object,
     * then set the notification vibration and lock screen display, and use the current method to bind the slot.
     * The value must be the type of an existing NotificationSlot object.
     * @param slotType Indicates the unique type of the NotificationSlot object.
     */
    void SetSlotType(NotificationConstant::SlotType slotType);

    /**
     * Obtains the slot type of a notification set by calling the setSlotType(string) method.
     * @return the notification slot type.
     */
    NotificationConstant::SlotType GetSlotType() const;

    /**
     * Sets a key used for sorting notifications from the same application bundle.
     * @param key Indicates the key to set.
     */
    void SetSortingKey(const std::string &key);

    /**
     * Obtains the key used for sorting notifications from the same application bundle.
     * @return the key for sorting notifications.
     */
    std::string GetSortingKey() const;

    /**
     * Sets the scrolling text to be displayed in the status bar when this notification is received.
     * @param text Indicates the scrolling text to be displayed.
     */
    void SetStatusBarText(const std::string &text);

    /**
     * Obtains the scrolling text that will be displayed in the status bar when this notification is received.
     * @return the scrolling notification text.
     */
    std::string GetStatusBarText() const;

    /**
     * Checks whether the current notification will be automatically dismissed after being tapped.
     * @return true if the notification will be automatically dismissed; returns false otherwise.
     */
    bool IsTapDismissed() const;

    /**
     * Sets whether to automatically dismiss a notification after being tapped.
     * If you set tapDismissed to true,
     * you must call the setWantAgent(WantAgent) method to make the settings take effect.
     * @param isDismissed Specifies whether a notification will be automatically dismissed after being tapped.
     */
    void SetTapDismissed(bool isDismissed);

    /**
     * Sets the notification display effect, including whether to display this notification on the lock screen,
     * and how it will be presented if displayed.
     * For details, see NotificationSlot::setLockscreenVisibleness(int).
     * If the lock screen display effect is set for a NotificationRequest object
     * and its associated NotificationSlot object, the display effect set in the NotificationRequest object prevails.
     * @param type Indicates the notification display effect on the lock screen.
     */
    void SetVisibleness(NotificationConstant::VisiblenessType type);

    /**
     * Obtains the display effect of this notification on the lock screen.
     * @return the display effect of this notification on the lock screen.
     */
    NotificationConstant::VisiblenessType GetVisibleness() const;

    /**
     * Sets the badge icon style for this notification.
     * This method does not take effect if the home screen does not support badge icons.
     * @param style Indicates the type of the badge icon to be displayed for this notification.
     * The value must be BadgeStyle::NONE, BadgeStyle::LITTLE, or BadgeStyle::BIG.
     */
    void SetBadgeIconStyle(NotificationRequest::BadgeStyle style);

    /**
     * Obtains the badge icon style of this notification.
     * @return the badge icon style of this notification.
     */
    NotificationRequest::BadgeStyle GetBadgeIconStyle() const;

    /**
     * Sets the shortcut ID for this notification.
     * After a shortcut ID is set for a notification, the notification will be associated with the corresponding
     * home-screen shortcut, and the shortcut will be hidden when the Home application displays the badge or content
     * of the notification.
     * @param shortcutId Indicates the shortcut ID to set.
     */
    void SetShortcutId(const std::string &shortcutId);

    /**
     * Obtains the shortcut ID associated with this notification.
     * @return the shortcut ID of this notification.
     */
    std::string GetShortcutId() const;

    /**
     * Sets whether this notification is displayed as a floating icon on top of the screen.
     * @param floatingIcon Specifies whether a notification is displayed as a floating icon on top of the screen.
     */
    void SetFloatingIcon(bool floatingIcon);

    /**
     * Checks whether this notification is displayed as a floating icon on top of the screen.
     * @return true if this notification is displayed as a floating icon; returns false otherwise.
     */
    bool IsFloatingIcon() const;

    /**
     * Sets how the progress bar will be displayed for this notification.
     * A progress bar is usually used in notification scenarios such as download.
     * @param progress Indicates the current value displayed for the notification progress bar.
     * @param progressMax Indicates the maximum value displayed for the notification progress bar.
     * @param indeterminate Specifies whether the progress bar is indeterminate. The value true indicates that
     * the progress bar is indeterminate, and users cannot see its current and maximum values.
     */
    void SetProgressBar(int32_t progress, int32_t progressMax, bool indeterminate);

    /**
     * Obtains the maximum value displayed for the progress bar of this notification.
     * @return the maximum value of the notification progress bar.
     */
    int32_t GetProgressMax() const;

    /**
     * Obtains the current value displayed for the progress bar of this notification.
     * @return the current value of the notification progress bar.
     */
    int32_t GetProgressValue() const;

    /**
     * Checks whether the progress bar of this notification is indeterminate.
     * @return true if the notification progress bar is indeterminate; returns false otherwise.
     */
    bool IsProgressIndeterminate() const;

    /**
     * Sets the most recent NotificationUserInput records that have been sent through this notification.
     * The most recent input must be stored in index 0,
     * the second most recent input must be stored in index 1, and so on.
     * The system displays a maximum of five inputs.
     * @param text Indicates the list of inputs to set.
     */
    void SetNotificationUserInputHistory(const std::vector<std::string> &text);

    /**
     * Obtains the most recent NotificationUserInput records
     * @return the most recent NotificationUserInput records
     */
    std::vector<std::string> GetNotificationUserInputHistory() const;

    /**
     * Sets an alternative notification to be displayed on the lock screen for this notification.
     * The display effect (whether and how this alternative notification will be displayed) is subject to
     * the configuration in NotificationSlot::setLockscreenVisibleness(int).
     * @param other Indicates the alternative notification to be displayed on the lock screen.
     */
    void SetPublicNotification(const std::shared_ptr<NotificationRequest> &other);

    /**
     * Obtains the alternative notification to be displayed on the lock screen for this notification.
     * @return the alternative notification to be displayed on the lock screen for this notification.
     */
    const std::shared_ptr<NotificationRequest> GetPublicNotification() const;

    /**
     * Obtains the unique hash code of a notification in the current application.
     * To obtain a valid hash code, you must have subscribed to and received the notification.
     * A valid notification hash code is a string composed of multiple attributes separated by an underscore (_),
     * including the notification ID, creator bundle name, creator UID, and owner bundle name.
     * @return the hash code of the notification.
     */
    std::string GetNotificationHashCode() const;

    /**
     * Sets the bundle name of the notification owner.
     * The notification owner refers to the application that subscribes to the notification.
     * @param ownerName the bundle name of the notification owner.
     */
    void SetOwnerBundleName(const std::string &ownerName);

    /**
     * Obtains the bundle name of the notification owner.
     * The notification owner refers to the application that subscribes to the notification.
     * @return the bundle name of the notification owner.
     */
    std::string GetOwnerBundleName() const;

    /**
     * Sets the bundle name of the notification creator.
     * The notification creator refers to the application that publishes the notification.
     * @param creatorName the bundle name of the notification creator.
     */
    void SetCreatorBundleName(const std::string &creatorName);

    /**
     * Obtains the bundle name of the notification creator.
     * The notification creator refers to the application that publishes the notification.
     * @return the bundle name of the notification creator.
     */
    std::string GetCreatorBundleName() const;

    /**
     * Sets the PID of the notification creator.
     * @param pid the PID of the notification creator.
     */
    void SetCreatorPid(pid_t pid);

    /**
     * Obtains the PID of the notification creator.
     * @return the PID of the notification creator.
     */
    pid_t GetCreatorPid() const;

    /**
     * Sets the UID of the notification creator.
     * @param uid the UID of the notification creator.
     */
    void SetCreatorUid(pid_t uid);

    /**
     * Obtains the UID of the notification creator.
     * @return the UID of the notification creator.
     */
    pid_t GetCreatorUid() const;

    /**
     * Sets the label of this notification.
     * @param label the label of this notification.
     */
    void SetLabel(const std::string &label);

    /**
     * Obtains the label of this notification.
     * The label is set via NotificationHelper::publishNotification(string, NotificationRequest).
     * This method returns null if no specific label is set for this notification.
     * @return the label of this notification.
     */
    std::string GetLabel() const;

    /**
     * Returns a string representation of the object.
     * @return a string representation of the object.
     */
    std::string Dump();

    /**
     * Marshal a NotificationRequest object into a Parcel.
     * @param parcel the object into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal object from a Parcel.
     * @return the NotificationRequest
     */
    static NotificationRequest *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read a NotificationRequest object from a Parcel.
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * Obtains the current system time in milliseconds.
     * @return the current system time in milliseconds.
     */
    int64_t GetNowSysTime();

private:
    int32_t notificationId_ {0};
    uint32_t color_ {NotificationRequest::COLOR_DEFAULT};
    int32_t badgeNumber_ {0};
    int32_t progressValue_ {0};
    int32_t progressMax_ {0};
    int64_t createTime_ {0};
    int64_t deliveryTime_ {0};
    int64_t autoDeletedTime_ {0};

    pid_t creatorPid_ {0};
    pid_t creatorUid_ {0};

    std::string settingsText_ {};
    std::string creatorBundleName_ {};
    std::string ownerBundleName_ {};
    std::string groupValue_ {};
    std::string statusBarText_ {};
    std::string label_ {};
    std::string shortcutId_ {};
    std::string sortingKey_ {};
    std::string classification_ {};

    NotificationConstant::SlotType slotType_ {NotificationConstant::SlotType::OTHER};
    NotificationRequest::GroupAlertType groupAlertType_ {NotificationRequest::GroupAlertType::ALL};
    NotificationConstant::VisiblenessType visiblenessType_ {NotificationConstant::VisiblenessType::NO_OVERRIDE};
    NotificationRequest::BadgeStyle badgeStyle_ {NotificationRequest::BadgeStyle::NONE};
    NotificationContent::Type notificationContentType_ {NotificationContent::Type::NONE};

    bool showDeliveryTime_ {false};
    bool tapDismissed_ {false};
    bool colorEnabled_ {false};
    bool alertOneTime_ {false};
    bool showStopwatch_ {false};
    bool isCountdown_ {false};
    bool inProgress_ {false};
    bool groupOverview_ {false};
    bool progressIndeterminate_ {false};
    bool unremovable_ {false};
    bool floatingIcon_ {false};
    bool onlyLocal_ {false};
    bool permitted_ {true};

    std::shared_ptr<AppExecFwk::Context> context_ {};
    std::shared_ptr<WantAgent::WantAgent> wantAgent_ {};
    std::shared_ptr<WantAgent::WantAgent> removalWantAgent_ {};
    std::shared_ptr<WantAgent::WantAgent> maxScreenWantAgent_ {};
    std::shared_ptr<AAFwk::WantParams> additionalParams_ {};
    std::shared_ptr<Media::PixelMap> littleIcon_ {};
    std::shared_ptr<Media::PixelMap> bigIcon_ {};
    std::shared_ptr<NotificationContent> notificationContent_ {};
    std::shared_ptr<NotificationRequest> publicNotification_ {};

    std::vector<std::shared_ptr<NotificationActionButton>> actionButtons_ {};
    std::vector<std::shared_ptr<MessageUser>> messageUsers_ {};
    std::vector<std::string> userInputHistory_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_REQUEST_H
