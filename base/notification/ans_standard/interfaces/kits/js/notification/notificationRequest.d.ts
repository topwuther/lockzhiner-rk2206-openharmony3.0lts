/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http?://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import notification from '../@ohos.notification';
import { WantAgent } from '../@ohos.wantAgent';
import { NotificationContent } from './notificationContent';
import { NotificationActionButton } from './notificationActionButton';

/**
 * Carries notifications.
 *
 * @name NotificationRequest
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationRequest {
  content: NotificationContent;
  id?: number;
  slotType?: notification.SlotType;
  isOngoing?: boolean;
  isUnremovable?: boolean;
  deliveryTime?: number;
  tapDismissed?: boolean;
  autoDeletedTime?: number;
  wantAgent?: WantAgent;
  extraInfo?: {[key: string]: any};
  color?: number;
  colorEnabled?: boolean;
  isAlertOnce?: boolean;
  isStopwatch?: boolean;
  isCountDown?: boolean;
  isFloatingIcon?: boolean;
  label?: string;
  badgeIconStyle?: number;
  showDeliveryTime?: boolean;
  actionButtons?: Array<NotificationActionButton>;
  readonly creatorBundleName?: string;
  readonly creatorUid?: number;
  readonly creatorPid?: number;

  /**
   * Obtains the classification of this notification.
   *
   * @systemapi Hide this for inner system use.
   * @since 7
   */
  classification?: string;

  /**
   * Obtains the unique hash code of a notification in the current application.
   *
   * @since 7
   */
  readonly hashCode?: string;
}
