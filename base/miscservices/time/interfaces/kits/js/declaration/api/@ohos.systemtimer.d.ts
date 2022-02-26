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
import { AsyncCallback, ErrorCallback } from './basic';
import { WantAgent } from './@ohos.wantAgent';

/**
 * Provides js api for systemTimer
 *
 * @since 7
 * @devices phone, tablet, tv, wearable
 * @systemapi Hide this for inner system use.
 */
declare namespace systemTimer {
  /**
   * Indicates the timing policy the timer use, which can be REALTIME or UTC.
   */
  const TIMER_TYPE_REALTIME: number;

  /**
   * Describes whether a timer will wake the device up.
   */
  const TIMER_TYPE_WAKEUP: number;

  /**
   * Describes whether a timer will be delivered precisely at a scheduled time.
   */
  const TIMER_TYPE_EXACT: number;

  /**
   * Indicates whether the timer waking up the system is supported in low-power mode.
   */
  const TIMER_TYPE_IDLE: number;

  /**
   * Creates a timer.
   * @since 7
   * @Param options Indicates the timer options.
   * @Return timer ID.
   * 
   * @systemapi Hide this for inner system use.
   */
  function createTimer(options: TimerOptions, callback: AsyncCallback<number>): void;
  function createTimer(options: TimerOptions): Promise<number>;

  /**
   * Starts a timer.
   *@since 7
   * @Param timer The timer ID.
   * @Param triggerTime Indicates the time at which the timer is triggered for the first time, in milliseconds.
   * The time will be automatically set to 5000 milliseconds after the current time if the passed
   * value is smaller than the current time plus 5000 milliseconds.
   * @systemapi Hide this for inner system use.
   */
  function startTimer(timer: number, triggerTime: number, callback: AsyncCallback<void>): void;
  function startTimer(timer: number, triggerTime: number): Promise<void>;

  /**
   * Stops a timer.
   * @since 7
   * @Param timer The timer ID.
   * @systemapi Hide this for inner system use.
   */
  function stopTimer(timer: number, callback: AsyncCallback<void>):  void;
  function stopTimer(timer: number): Promise<void>;

  /**
   * Clears a timer.
   * @since 7
   * @Param timer The timer ID.
   * @systemapi Hide this for inner system use.
   */
  function destroyTimer(timer: number, callback: AsyncCallback<void>): void;
  function destroyTimer(timer: number): Promise<void>;

  interface TimerOptions {
    /**
     * timer type.
     */
    type: number;
    
    /**
     * Indicates a repeating timer
     */
    repeat: boolean;

    /**
     * Indicates the interval between two consecutive triggers, in milliseconds.
     * The interval will be set to 5000 milliseconds automatically if the passed value is smaller than 5000.
     */
    interval?: number;

    /**
     * Indicates the intent to send when the timer goes off.
     */
    wantAgent?: WantAgent;

    /**
     * Called back when the timer goes off.
     */
    callback?: () => void;
  }
}

export default systemTimer;