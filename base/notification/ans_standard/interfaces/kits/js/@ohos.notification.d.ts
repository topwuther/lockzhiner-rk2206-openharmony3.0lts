/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { AsyncCallback } from './basic';
import { NotificationSlot } from './notification/notificationSlot';
import { NotificationSubscriber } from './notification/notificationSubscriber';
import { NotificationSubscribeInfo } from './notification/notificationSubscribeInfo';
import { NotificationRequest } from './notification/notificationRequest';

/**
 * Manages notifications.
 *
 * <p>Generally, only system applications have permissions on notification subscription and unsubscription.
 * You can specify the content of a notification to be published and the content is carried by
 * {@link NotificationRequest}. A notification ID is unique in an application and must be specified
 * when using {@link NotificationRequest} to carry the notification content. If a notification
 * with this ID has been published and you need to use this ID to publish another notification,
 * the original notification will be updated. In addition, the notification ID can be used to cancel
 * a notification by calling the {@link #cancel(int)} method.
 *
 * @name notification
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
declare namespace notification {
  /**
   * Publishes a notification.
   *
   * <p>If a notification with the same ID has been published by the current application and has not been deleted,
   * this method will update the notification.
   *
   * @param request Indicates the {@link NotificationRequest} object for setting the notification content.
   * This parameter must be specified.
   *
   * @since 7
   */
  function publish(request: NotificationRequest, callback: AsyncCallback<void>): void;
  function publish(request: NotificationRequest): Promise<void>;

  /**
   * Cancels a published notification.
   *
   * @param id Indicates the unique notification ID in the application. The value must be the ID
   * of a published notification. Otherwise, this method does not take effect.
   *
   * @since 7
   */
  function cancel(id: number, callback: AsyncCallback<void>): void;

  /**
   * Cancels a published notification matching the specified {@code label} and {@code notificationId}.
   *
   * @param label Indicates the label of the notification to cancel.
   * @param id Indicates the ID of the notification to cancel.
   *
   * @since 7
   */
  function cancel(id: number, label: string, callback: AsyncCallback<void>): void;
  function cancel(id: number, label?: string): Promise<void>;

  /**
   * Cancels all the published notifications.
   *
   * @since 7
   */
  function cancelAll(callback: AsyncCallback<void>): void;
  function cancelAll(): Promise<void>;

  /**
   * Creates a notification slot.
   *
   * @param slot Indicates the notification slot to be created, which is set by {@link NotificationSlot}.
   * This parameter must be specified.
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function addSlot(slot: NotificationSlot, callback: AsyncCallback<void>): void;
  function addSlot(slot: NotificationSlot): Promise<void>;

  /**
   * Creates a notification slot.
   *
   * @param slot Indicates the notification slot to be created, which is set by {@link SlotType}.
   * This parameter must be specified.
   *
   * @since 7
   */
  function addSlot(type: SlotType, callback: AsyncCallback<void>): void;
  function addSlot(type: SlotType): Promise<void>;

  /**
   * Creates a notification slot.
   *
   * @param slots Indicates the notification slots to be created, which is set by {@link NotificationSlot}.
   * This parameter must be specified.
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function addSlots(slots: Array<NotificationSlot>, callback: AsyncCallback<void>): void;
  function addSlots(slots: Array<NotificationSlot>): Promise<void>;

  /**
   * Queries a created notification slot.
   *
   * @param slotType Indicates the type of the slot, which is created by
   *
   * @return Returns the created {@link NotificationSlot}.
   *
   * @since 7
   */
  function getSlot(slotType: SlotType, callback: AsyncCallback<NotificationSlot>): void;
  function getSlot(slotType: SlotType): Promise<NotificationSlot>;

  /**
   * Obtains all notification slots of this application.
   *
   * @return Returns all notification slots of this application.
   *
   * @since 7
   */
  function getSlots(callback: AsyncCallback<Array<NotificationSlot>>): void;
  function getSlots(): Promise<Array<NotificationSlot>>;

  /**
   * Deletes a created notification slot based on the slot type.
   *
   * @param slotType Indicates the type of the slot.
   *
   * @since 7
   */
  function removeSlot(slotType: SlotType, callback: AsyncCallback<void>): void;
  function removeSlot(slotType: SlotType): Promise<void>;

  /**
   * Deletes all notification slots.
   *
   * @since 7
   */
  function removeAllSlots(callback: AsyncCallback<void>): void;
  function removeAllSlots(): Promise<void>;

  /**
   * Indicates the type of the slot
   *
   * @since 7
   */
  export enum SlotType {
    UNKNOWN_TYPE = 0,
    SOCIAL_COMMUNICATION = 1,
    SERVICE_INFORMATION = 2,
    CONTENT_INFORMATION = 3,
    OTHER_TYPES = 0xFFFF,
  }

  /**
   * Indicates the type of the content
   *
   * @name ContentType
   * @since 7
   * @sysCap SystemCapability.Notification.ANS
   * @devices phone, tablet, tv, wearable, car
   * @permission N/A
   */
  export enum ContentType {
    NOTIFICATION_CONTENT_BASIC_TEXT,
    NOTIFICATION_CONTENT_LONG_TEXT,
    NOTIFICATION_CONTENT_PICTURE,
    NOTIFICATION_CONTENT_MULTILINE,
  }

  /**
   * Indicates the level of the slot
   *
   * @since 7
   */
  export enum SlotLevel {
    LEVEL_NONE = 0,
    LEVEL_MIN = 1,
    LEVEL_LOW = 2,
    LEVEL_DEFAULT = 3,
    LEVEL_HIGH = 4,
  }

  /**
   * subscribe
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function subscribe(subscriber: NotificationSubscriber, callback: AsyncCallback<void>): void;
  function subscribe(subscriber: NotificationSubscriber, info: NotificationSubscribeInfo, callback: AsyncCallback<void>): void;
  function subscribe(subscriber: NotificationSubscriber, info?: NotificationSubscribeInfo): Promise<void>;

  /**
   * unsubscribe
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function unsubscribe(subscriber: NotificationSubscriber, callback: AsyncCallback<void>): void;
  function unsubscribe(subscriber: NotificationSubscriber): Promise<void>;

  /**
   * enableNotification
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function enableNotification(bundle: BundleOption, enable: boolean, callback: AsyncCallback<void>): void;
  function enableNotification(bundle: BundleOption, enable: boolean): Promise<void>;

  /**
   * isNotificationEnabled
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function isNotificationEnabled(bundle: BundleOption, callback: AsyncCallback<boolean>): void;
  function isNotificationEnabled(bundle: BundleOption): Promise<boolean>;

  /**
   * isNotificationEnabled
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function isNotificationEnabled(callback: AsyncCallback<boolean>): void;
  function isNotificationEnabled(): Promise<boolean>;

  /**
   * displayBadge
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function displayBadge(bundle: BundleOption, enable: boolean, callback: AsyncCallback<void>): void;
  function displayBadge(bundle: BundleOption, enable: boolean): Promise<void>;

  /**
   * isBadgeDisplayed
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function isBadgeDisplayed(bundle: BundleOption, callback: AsyncCallback<boolean>): void;
  function isBadgeDisplayed(bundle: BundleOption): Promise<boolean>;

  /**
   * setSlotByBundle
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function setSlotByBundle(bundle: BundleOption, slot: NotificationSlot, callback: AsyncCallback<void>): void;
  function setSlotByBundle(bundle: BundleOption, slot: NotificationSlot): Promise<void>;

  /**
   * getSlotsByBundle
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function getSlotsByBundle(bundle: BundleOption, callback: AsyncCallback<Array<NotificationSlot>>): void;
  function getSlotsByBundle(bundle: BundleOption): Promise<Array<NotificationSlot>>;

  /**
   * getSlotNumByBundle
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function getSlotNumByBundle(bundle: BundleOption, callback: AsyncCallback<number>): void;
  function getSlotNumByBundle(bundle: BundleOption): Promise<number>;

  /**
   * remove
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function remove(bundle: BundleOption, notificationKey: NotificationKey, callback: AsyncCallback<void>): void;
  function remove(bundle: BundleOption, notificationKey: NotificationKey): Promise<void>;

  /**
   * remove
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function remove(hashCode: string, callback: AsyncCallback<void>): void;
  function remove(hashCode: string): Promise<void>;

  /**
   * removeAll
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function removeAll(bundle: BundleOption, callback: AsyncCallback<void>): void;
  function removeAll(callback: AsyncCallback<void>): void;
  function removeAll(bundle?: BundleOption): Promise<void>;

  /**
   * Obtains all active notifications in the current system. The caller must have system permissions to
   * call this method.
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  function getAllActiveNotifications(callback: AsyncCallback<Array<NotificationRequest>>): void;
  function getAllActiveNotifications(): Promise<Array<NotificationRequest>>;

  /**
   * Obtains the number of active notifications of the current application in the system.
   *
   * @since 7
   */
  function getActiveNotificationCount(callback: AsyncCallback<number>): void;
  function getActiveNotificationCount(): Promise<number>;

  /**
   * Obtains active notifications of the current application in the system.
   *
   * @since 7
   */
  function getActiveNotifications(callback: AsyncCallback<Array<NotificationRequest>>): void;
  function getActiveNotifications(): Promise<Array<NotificationRequest>>;

  /**
   * BundleOption
   *
   * @since 7
   */
  export interface BundleOption {
    bundle: string;
    uid?: number;
  }

  /**
   * NotificationKey
   *
   * @since 7
   */
  export interface NotificationKey {
    id: number;
    label?: string;
  }

  /**
   * DisturbMode
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  export enum DoNotDisturbMode {
    ALLOW_UNKNOWN,

    /**
     * Indicates that all notifications are allowed to interrupt the user in Do Not Disturb mode.
     */
    ALLOW_ALL,

    /**
     * Indicates that only notifications meeting the specified priority criteria are allowed to interrupt
     * the user in Do Not Disturb mode.
     */
    ALLOW_PRIORITY,

    /**
     * Indicates that no notifications are allowed to interrupt the user in Do Not Disturb mode.
     */
    ALLOW_NONE,

    /**
     * Indicates that only notifications of the {@link NotificationRequest#CLASSIFICATION_ALARM} category
     * are allowed to interrupt the user in Do Not Disturb mode.
     */
    ALLOW_ALARMS
  }
}

export default notification;
