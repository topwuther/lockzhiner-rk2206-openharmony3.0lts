/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

/**
 * @devices phone, tablet, wearable, liteWearable
 */
export interface AccelerometerResponse {
  /**
   * X-coordinate
   * @devices phone, tablet, wearable
   * @since 3
   */
  x: number;

  /**
   * Y-coordinate
   * @devices phone, tablet, wearable
   * @since 3
   */
  y: number;

  /**
   * Z-coordinate
   * @devices phone, tablet, wearable
   * @since 3
   */
  z: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface subscribeAccelerometerOptions {
  /**
   * Execution frequency of the callback function for listening to acceleration sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet, wearable
   * @since 3
   */
  interval: string;

  /**
   * Called when acceleration sensor data changes.
   * @devices phone, tablet, wearable
   * @since 3
   */
  success: (data: AccelerometerResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface CompassResponse {
  /**
   * Direction of the device (in degrees).
   * @devices phone, tablet, wearable
   * @since 3
   */
  direction: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeCompassOptions {
  /**
   * Called when compass sensor data changes.
   * @devices phone, tablet, wearable
   * @since 3
   */
  success: (data: CompassResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet
 */
export interface ProximityResponse {
  /**
   * Distance between a visible object and the device screen
   * @devices phone, tablet
   * @since 3
   */
  distance: number;
}

/**
 * @devices phone, tablet
 */
export interface SubscribeProximityOptions {
  /**
   * Called when distance sensor data changes.
   * @devices phone, tablet
   * @since 3
   */
  success: (data: ProximityResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface LightResponse {
  /**
   * Light intensity, in lux.
   * @devices phone, tablet, wearable
   * @since 3
   */
  intensity: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeLightOptions {
  /**
   * Called when ambient light sensor data changes.
   * @devices phone, tablet, wearable
   * @since 3
   */
  success: (data: LightResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable, liteWearable
 */
export interface StepCounterResponse {
  /**
   * Number of steps counted.
   * Each time the device restarts, the value is recalculated from 0 in phone, tablet.
   * @devices phone, tablet, wearable, liteWearable
   * @since 3
   */
  steps: number;
}

/**
 * @devices phone, tablet, liteWearable
 */
export interface SubscribeStepCounterOptions {
  /**
   * Called when step counter sensor data changes.
   * @devices phone, tablet, liteWearable
   * @since 3
   */
  success: (data: StepCounterResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, liteWearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable, liteWearable
 */
export interface BarometerResponse {
  /**
   * Pressure, in pascal.
   * @devices phone, tablet, wearable, liteWearable
   * @since 3
   */
  pressure: number;
}

/**
 * @devices phone, tablet, wearable, liteWearable
 */
export interface SubscribeBarometerOptions {
  /**
   * Called when the barometer sensor data changes.
   * @devices phone, tablet, wearable, liteWearable
   * @since 3
   */
  success: (data: BarometerResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable, liteWearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices wearable, liteWearable
 */
export interface HeartRateResponse {
  /**
   * Heart rate.
   * 255 indicates an invalid value in lite wearable.
   * @devices wearable, liteWearable
   * @since 3
   */
  heartRate: number;
}

/**
 * @devices wearable, liteWearable
 */
export interface SubscribeHeartRateOptions {
  /**
   * Called when the heart rate sensor data changes.
   * @devices wearable, liteWearable
   * @since 3
   */
  success: (data: HeartRateResponse) => void;

  /**
   * Called when the listening fails
   * @devices wearable, liteWearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices liteWearable
 */
export interface OnBodyStateResponse {
  /**
   * Whether the sensor is worn.
   * @devices liteWearable
   * @since 3
   */
  value: boolean;
}

/**
 * @devices liteWearable
 */
export interface SubscribeOnBodyStateOptions {
  /**
   * Called when the wearing status changes.
   * @devices liteWearable
   * @since 3
   */
  success: (data: OnBodyStateResponse) => void;

  /**
   * Called when the listening fails.
   * @devices liteWearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices liteWearable
 */
export interface GetOnBodyStateOptions {
  /**
   * Called when the sensor wearing state is obtained
   * @devices liteWearable
   * @since 3
   */
  success: (data: OnBodyStateResponse) => void;

  /**
   * Called when the sensor wearing state fails to be obtained
   * @devices liteWearable
   * @since 3
   */
  fail?: (data: string, code: number) => void;

  /**
   * Called when the execution is completed
   * @devices liteWearable
   * @since 3
   */
  complete?: () => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface DeviceOrientationResponse {
  /**
   * alpha
   * @devices phone, tablet, wearable
   * @since 6
   */
  alpha: number;

  /**
   * beta
   * @devices phone, tablet, wearable
   * @since 6
   */
  beta: number;

  /**
   * gamma
   * @devices phone, tablet, wearable
   * @since 6
   */
  gamma: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeDeviceOrientationOptions {
  /**
   * Execution frequency of the callback function for listening to device orientation sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet, wearable
   * @since 6
   */
  interval: string;

  /**
   * Called when device orientation sensor data changes.
   * @devices phone, tablet, wearable
   * @since 6
   */
  success: (data: DeviceOrientationResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 6
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface GyroscopeResponse {
  /**
   * X-coordinate
   * @devices phone, tablet, wearable
   * @since 6
   */
  x: number;

  /**
   * Y-coordinate
   * @devices phone, tablet, wearable
   * @since 6
   */
  y: number;

  /**
   * Z-coordinate
   * @devices phone, tablet, wearable
   * @since 6
   */
  z: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeGyroscopeOptions {
  /**
   * Execution frequency of the callback function for listening to gyroscope sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet, wearable
   * @since 6
   */
  interval: string;

  /**
   * Called when gyroscope sensor data changes.
   * @devices phone, tablet, wearable
   * @since 6
   */
  success: (data: GyroscopeResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 6
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface GravityResponse {
  /**
   * X-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  x: number;

  /**
   * Y-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  y: number;

  /**
   * Z-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  z: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeGravityOptions {
  /**
   * Execution frequency of the callback function for listening to gravity sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet, wearable
   * @since 7
   */
  interval: string;

  /**
   * Called when gravity sensor data changes.
   * @devices phone, tablet, wearable
   * @since 7
   */
  success: (data: GravityResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 7
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export interface MagneticResponse {
  /**
   * X-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  x: number;

  /**
   * Y-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  y: number;

  /**
   * Z-coordinate
   * @devices phone, tablet, wearable
   * @since 7
   */
  z: number;
}

/**
 * @devices phone, tablet, wearable
 */
export interface SubscribeMagneticOptions {
  /**
   * Execution frequency of the callback function for listening to magnetic sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet, wearable
   * @since 7
   */
  interval: string;

  /**
   * Called when magnetic sensor data changes.
   * @devices phone, tablet, wearable
   * @since 7
   */
  success: (data: MagneticResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet, wearable
   * @since 7
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet
 */
export interface HallResponse {
  /**
   * Indicates the hall sensor data.
   * @devices phone, tablet
   * @since 7
   */
  value: number;
}

/**
 * @devices phone, tablet
 */
export interface SubscribeHallOptions {
  /**
   * Execution frequency of the callback function for listening to hall sensor data.
   * Available values are as follows:
   *   1. game: Extremely high frequency (20 ms per callback), which is applicable to gaming.
   *   2. ui: High frequency (60 ms per callback), which is applicable to UI updating.
   *   3. normal: Regular frequency (200 ms per callback), which is application to low power consumption.
   * The default value is normal.
   * @devices phone, tablet
   * @since 7
   */
  interval: string;

  /**
   * Called when hall sensor data changes.
   * @devices phone, tablet
   * @since 7
   */
  success: (data: HallResponse) => void;

  /**
   * Called when the listening fails.
   * @devices phone, tablet
   * @since 7
   */
  fail?: (data: string, code: number) => void;
}

/**
 * @devices phone, tablet, wearable
 */
export default class Sensor {
  /**
   * Listens to acceleration sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeAccelerometer(options: subscribeAccelerometerOptions): void;

  /**
   * Cancels listening to acceleration sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeAccelerometer(): void;

  /**
   * Listens to compass sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeCompass(options: SubscribeCompassOptions): void;

  /**
   * Cancels listening to compass sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeCompass(): void;

  /**
   * Listens to distance sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet
   */
  static subscribeProximity(options: SubscribeProximityOptions): void;

  /**
   * Cancels listening to distance sensor data.
   * @param options Options.
   * @devices phone, tablet
   */
  static unsubscribeProximity(): void;

  /**
   * Listens to ambient light sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeLight(options: SubscribeLightOptions): void;

  /**
   * Cancels listening to ambient light sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeLight(): void;

  /**
   * Listens to step counter sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable, liteWearable
   */
  static subscribeStepCounter(options: SubscribeStepCounterOptions): void;

  /**
   * Cancels listening to step counter sensor data.
   * @devices phone, tablet, wearable, liteWearable
   */
  static unsubscribeStepCounter(): void;

  /**
   * Listens to barometer sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable, liteWearable
   */
  static subscribeBarometer(options: SubscribeBarometerOptions): void;

  /**
   * Cancels listening to barometer sensor data.
   * @devices phone, tablet, wearable, liteWearable
   */
  static unsubscribeBarometer(): void;

  /**
   * Listens to changes of heart rate sensor data.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices wearable, liteWearable
   */
  static subscribeHeartRate(options: SubscribeHeartRateOptions): void;

  /**
   * Cancels listening to heart rate sensor data.
   * @devices wearable, liteWearable
   */
  static unsubscribeHeartRate(): void;

  /**
   * Listens to whether a sensor is worn.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices wearable, liteWearable
   */
  static subscribeOnBodyState(options: SubscribeOnBodyStateOptions): void;

  /**
   * Cancels listening to whether the sensor is worn.
   * @devices wearable, liteWearable
   */
  static unsubscribeOnBodyState(): void;

  /**
   * Listens to device orientation sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeDeviceOrientation(options: SubscribeDeviceOrientationOptions): void;

  /**
   * Cancels listening to device orientation sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeDeviceOrientation(): void;

  /**
   * Listens to gyroscope sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeGyroscope(options: SubscribeGyroscopeOptions): void;

  /**
   * Cancels listening to gyroscope sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeGyroscope(): void;

  /**
   * Listens to gravity sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeGravity(options: SubscribeGravityOptions): void;

  /**
   * Cancels listening to gravity sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeGravity(): void;

  /**
   * Listens to magnetic sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeMagnetic(options: SubscribeMagneticOptions): void;

  /**
   * Cancels listening to magnetic sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeMagnetic(): void;

  /**
   * Listens to hall sensor data changes.
   * If this API is called multiple times, the last call takes effect.
   * @param options Options.
   * @devices phone, tablet, wearable
   */
  static subscribeHall(options: SubscribeHallOptions): void;

  /**
   * Cancels listening to hall sensor data.
   * @devices phone, tablet, wearable
   */
  static unsubscribeHall(): void;

  /**
   * Obtains the sensor wearing state.
   * @param options Options.
   * @devices wearable, liteWearable
   */
  static getOnBodyState(options: GetOnBodyStateOptions): void;
}
