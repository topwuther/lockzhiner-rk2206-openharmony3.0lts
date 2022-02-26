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

import { AsyncCallback, Callback } from './basic';

declare namespace deviceManager {
  /**
   * DeviceInfo
   */
  interface DeviceInfo {
    /**
     * DeviceId ID.
     */
    deviceId: string;

    /**
     * Device name of the device.
     */
    deviceName: string;

    /**
     * Device type of the device.
     */
    deviceType: DeviceType;
  }

  /**
   * Device Type definitions
   */
  enum DeviceType {
    /**
     * Indicates an unknown device type.
     */
    UNKNOWN_TYPE = 0,

    /**
     * Indicates a speaker.
     */
    SPEAKER = 0x0A,

    /**
     * Indicates a smartphone.
     */
    PHONE = 0x0E,

    /**
     * Indicates a tablet.
     */
    TABLET = 0x11,

    /**
     * Indicates a smart watch.
     */
    WEARABLE = 0x6D,

    /**
     * Indicates a car.
     */
    CAR = 0x83,

    /**
     * Indicates a smart TV.
     */
    TV = 0x9C
  }

  /**
   * Device state change event definition
   */
  enum DeviceStateChangeAction {
    /**
     * device online action
     */
    ONLINE = 0,

    /**
     * device ready action, the device information synchronization was completed.
     */
    READY = 1,

    /**
     * device offline action
     */
    OFFLINE = 2,

    /**
     * device change action
     */
    CHANGE = 3
  }

  /**
   * Service subscribe info for device discover
   *
   * @systemapi this method can be used only by system applications.
   */
  interface SubscribeInfo {
    /**
     * Service subscribe ID, the value is in scope [0, 65535], should be unique for each discover process
     */
    subscribeId: number;

    /**
     * Discovery mode for service subscription.
     */
    mode: DiscoverMode;

    /**
     * Service subscription medium.
     */
    medium: ExchangeMedium;

    /**
     * Service subscription frequency.
     */
    freq: ExchangeFreq;

    /**
     * only find the device with the same account.
     */
    isSameAccount: boolean;

    /**
     * find the sleeping devices.
     */
    isWakeRemote: boolean;

    /**
     * Subscribe capability.
     */
    capability: SubscribeCap;
  }

  /**
   * device discover mode
   *
   * @systemapi this method can be used only by system applications.
   */
  enum DiscoverMode {
    /**
     * Passive
     */
    DISCOVER_MODE_PASSIVE = 0x55,

    /**
     * Proactive
     */
    DISCOVER_MODE_ACTIVE = 0xAA
  }

  /**
   * device discover medium
   *
   * @systemapi this method can be used only by system applications.
   */
  enum ExchangeMedium {
    /**
     * Automatic medium selection
     */
    AUTO = 0,

    /**
     * Bluetooth
     */
    BLE = 1,

    /**
     * Wi-Fi
     */
    COAP = 2,

    /**
     * USB
     */
    USB = 3
  }

  /**
   * device discover freq
   *
   * @systemapi this method can be used only by system applications.
   */
  enum ExchangeFreq {
    /**
     * Low
     */
    LOW = 0,

    /**
     * Medium
     */
    MID = 1,

    /**
     * High
     */
    HIGH = 2,

    /**
     * Super-high
     */
    SUPER_HIGH = 3
  }

  /**
   * device discover capability
   *
   * @systemapi this method can be used only by system applications.
   */
  enum SubscribeCap {
    /**
     * ddmpCapability
     */
    SUBSCRIBE_CAPABILITY_DDMP = 0
  }

  /**
   * Device Authentication param
   *
   * @systemapi this method can be used only by system applications
   */
  interface AuthParam {
    /**
     * Authentication type, 1 for pin code.
     */
    authType: number;

    /**
     * App application Icon.
     */
    appIcon?: Uint8Array;

    /**
     * App application thumbnail.
     */
    appThumbnail?: Uint8Array; 

    /**
     * Authentication extra infos.
     */
    extraInfo: {[key:string] : any};

  }

  /**
   * Device auth info.
   *
   * @systemapi this method can be used only by system applications
   */
  interface AuthInfo {
    /**
     * Authentication type, 1 for pin code.
     */
    authType: number;

    /**
     * the token used for this authentication.
     */
    token: number;
    
    /**
     * Authentication extra infos.
     */
    extraInfo: {[key:string] : any};
  }

  /**
   * User Operation Action from devicemanager Fa.
   *
   * @systemapi this method can be used only by system applications.
   */
  enum UserOperationAction {
    /**
     * allow authentication
     */
    ACTION_ALLOW_AUTH = 0,

    /**
     * cancel authentication
     */
    ACTION_CANCEL_AUTH = 1,

    /**
     * user operation timeout for authentication confirm
     */
    ACTION_AUTH_CONFIRM_TIMEOUT = 2,

    /**
     * cancel pincode display
     */
    ACTION_CANCEL_PINCODE_DISPLAY = 3,

    /**
     * cancel pincode input
     */
    ACTION_CANCEL_PINCODE_INPUT = 4,
  }

  /**
   * Creates a {@code DeviceManager} instance.
   *
   * <p>To manage devices, you must first call this method to obtain a {@code DeviceManager} instance and then
   * use this instance to call other device management methods.
   *
   * @param bundleName Indicates the bundle name of the application.
   * @param callback Indicates the callback to be invoked upon {@code DeviceManager} instance creation.
   */
  function createDeviceManager(bundleName: string, callback: AsyncCallback<DeviceManager>): void;

  /**
   * Provides methods for managing devices.
   */
  interface DeviceManager {
    /**
     * Releases the {@code DeviceManager} instance after the methods for device management are no longer used.
     */
    release(): void;

    /**
     * Obtains a list of trusted devices.
     *
     * @param options Indicates the extra parameters to be passed to this method for device filtering or sorting.
     * This parameter can be null. For details about available values, see {@link #TARGET_PACKAGE_NAME} and
     * {@link #SORT_TYPE}.
     * @return Returns a list of trusted devices.
     */
    getTrustedDeviceListSync(): Array<DeviceInfo>;

    /**
     * Start to discover device.
     *
     * @param bundleName Indicates the bundle name of the application.
     * @param subscribeInfo subscribe info to discovery device
     * @systemapi this method can be used only by system applications.
     */
    startDeviceDiscovery(subscribeInfo: SubscribeInfo): void;

    /**
     * Stop to discover device.
     *
     * @param bundleName Indicates the bundle name of the application.
     * @param subscribeId Service subscribe ID
     * @systemapi this method can be used only by system applications.
     */
    stopDeviceDiscovery(subscribeId: number): void;

    /**
     * Authenticate the specified device.
     *
     * @param deviceInfo deviceInfo of device to authenticate
     * @param authparam authparam of device to authenticate
     * @param callback Indicates the callback to be invoked upon authenticateDevice
     * @systemapi this method can be used only by system applications.
     */   
    authenticateDevice(deviceInfo: DeviceInfo, authparam: AuthParam, callback: AsyncCallback<{deviceId: string, pinTone ?: number}>): void;
    
     /**
     * verify auth info, such as pin code.
     *
     * @param authInfo device auth info o verify
     * @param callback Indicates the callback to be invoked upon verifyAuthInfo
     * @systemapi this method can be used only by system applications.
     */   
    verifyAuthInfo(authInfo: AuthInfo, callback: AsyncCallback<{deviceId: string, level: number}>): void;

    /**
     * Get authenticate parameters for peer device, this interface can only used by devicemanager Fa.
     *
     * @param authParam authparam for peer device
     * @systemapi this method can be used only by system applications.
     */  
    getAuthenticationParam(): AuthParam;

     /**
     * Set user Operation from devicemanager Fa, this interface can only used by devicemanager Fa.
     *
     * @param operateAction User Operation Actions.
     * @systemapi this method can be used only by system applications.
     */  
    setUserOperation(operateAction: UserOperationAction): void;

    /**
     * Register a callback from deviceManager service so that the devicemanager Fa can be notified when some events happen.
     * this interface can only used by devicemanager Fa.
     *
     * @param callback for devicemanager Fa to register.
     * @systemapi this method can be used only by system applications.
     */
    on(type: 'dmFaCallback', callback: Callback<{ param: string}>): void;

    /**
     * UnRegister dmFaCallback, this interface can only used by devicemanager Fa.
     *
     * @param callback for devicemanager Fa to register.
     * @systemapi this method can be used only by system applications.
     */
    off(type: 'dmFaCallback', callback?: Callback<{ param: string}>): void;

    /**
     * Register a device state callback so that the application can be notified upon device state changes based on
     * the application bundle name.
     *
     * @param bundleName Indicates the bundle name of the application.
     * @param callback Indicates the device state callback to register.
     */
    on(type: 'deviceStateChange', callback: Callback<{ action: DeviceStateChangeAction, device: DeviceInfo }>): void;

    /**
     * UnRegister device state callback based on the application bundle name.
     *
     * @param bundleName Indicates the bundle name of the application.
     * @param callback Indicates the device state callback to register.
     */
    off(type: 'deviceStateChange', callback?: Callback<{ action: DeviceStateChangeAction, device: DeviceInfo }>): void;

    /**
     * Register a device found callback so that the application can be notified when the device was found
     *
     * @param callback Indicates the device found callback to register.
     * @systemapi this method can be used only by system applications.
     */
    on(type: 'deviceFound', callback: Callback<{ subscribeId: number, device: DeviceInfo }>): void;

    /**
     * UnRegister a device found callback so that the application can be notified when the device was found
     *
     * @param callback Indicates the device found callback to register.
     * @systemapi this method can be used only by system applications.
     */
    off(type: 'deviceFound', callback?: Callback<{ subscribeId: number, device: DeviceInfo }>): void;

    /**
     * Register a device found result callback so that the application can be notified when the device discover was failed
     *
     * @param callback Indicates the device found result callback to register.
     * @systemapi this method can be used only by system applications.
     */
    on(type: 'discoverFail', callback: Callback<{ subscribeId: number, reason: number }>): void;

    /**
     * UnRegister a device found result callback so that the application can be notified when the device discover was failed
     *
     * @param callback Indicates the device found result callback to register.
     * @systemapi this method can be used only by system applications.
     */
    off(type: 'discoverFail', callback?: Callback<{ subscribeId: number, reason: number }>): void;

    /**
     * Register a serviceError callback so that the application can be notified when devicemanager service died
     *
     * @param callback Indicates the service error callback to register.
     */
    on(type: 'serviceDie', callback: () => void): void;

    /**
     * UnRegister a serviceError callback so that the application can be notified when devicemanager service died
     *
     * @param callback Indicates the service error callback to register.
     */
    off(type: 'serviceDie', callback?: () => void): void;
  }
}

export default deviceManager;
