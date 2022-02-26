/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import { AsyncCallback } from "./basic";

/**
 * Provides methods to operate or manage Wi-Fi.
 *
 * @SysCap SystemCapability.Communication.WifiDevice
 * @devices phone, tablet
 * @since 6
 */
declare namespace wifi {
    /**
     * Enables Wi-Fi.
     *
     * @return Returns {@code true} if the operation is successful; returns {@code false} otherwise.
     *
     * @since 6
     * @hide SystemApi
     */
    function enableWifi(): boolean;

    /**
     * Disables Wi-Fi.
     *
     * @return Returns {@code true} if the operation is successful; returns {@code false} otherwise.
     *
     * @since 6
     * @hide SystemApi
     */
    function disableWifi(): boolean;

    /**
     * Queries the Wi-Fi status
     *
     * @return Returns {@code true} if the Wi-Fi is active; returns {@code false} otherwise.
     *
     * @since 6
     */
    function isWifiActive(): boolean;

    /**
     * Scans Wi-Fi hotspots with parameters.
     *
     * <p>This API works in asynchronous mode.</p>
     *
     * @return Returns {@code true} if the scanning is successful; returns {@code false} otherwise.
     *
     * @since 6
     */
    function scan(): boolean;

    /**
     * Obtains the hotspot information that scanned.
     *
     * @return Returns information about scanned Wi-Fi hotspots if any.
     *
     * @since 6
     */
    function getScanInfos(): Promise<Array<WifiScanInfo>>;
    function getScanInfos(callback: AsyncCallback<Array<WifiScanInfo>>): void;

    /**
     * Adds Wi-Fi connection configuration to the device.
     *
     * <p>The configuration will be updated when the configuration is added.</p>
     *
     * @return Returns {@code networkId} if the configuration is added; returns {@code -1} otherwise.
     *
     * @devices phone, tablet
     * @since 6
     * @hide SystemApi
     */
    function addDeviceConfig(config: WifiDeviceConfig): Promise<number>;
    function addDeviceConfig(config: WifiDeviceConfig, callback: AsyncCallback<number>): void;

    /**
     * Connects to Wi-Fi network.
     *
     * @param networkId ID of the connected network.
     * @return Returns {@code true} if the network connection is successful; returns {@code false} otherwise.
     *
     * @since 6
     * @hide SystemApi
     */
    function connectToNetwork(networkId: number): boolean;

    /**
     * Connects to Wi-Fi network.
     *
     * @param config Indicates the device configuration for connection to the Wi-Fi network.
     * @return Returns {@code true} if the network connection is successful; returns {@code false} otherwise.
     *
     * @devices phone, tablet
     * @since 6
     * @hide SystemApi
     */
    function connectToDevice(config: WifiDeviceConfig): boolean;

    /**
     * Disconnects Wi-Fi network.
     *
     * @return Returns {@code true} for disconnecting network success, returns {@code false} otherwise.
     *
     * @since 6
     * @hide SystemApi
     */
    function disconnect(): boolean;

    /**
     * Calculates the Wi-Fi signal level based on the Wi-Fi RSSI and frequency band.
     *
     * @param rssi Indicates the Wi-Fi RSSI.
     * @band Indicates the Wi-Fi frequency band.
     * @return Returns Wi-Fi signal level ranging from 0 to 4.
     *
     * @since 6
     */
    function getSignalLevel(rssi: number, band: number): number;

    /**
     * Wi-Fi device configuration information.
     *
     * @devices phone, tablet
     * @since 6
     * @hide SystemApi
     */
    interface WifiDeviceConfig {
        /** Wi-Fi SSID: the maximum length is 32 */
        ssid: string;

        /** Wi-Fi bssid(MAC): the length is 6 */
        bssid: string;

        /** Wi-Fi key: maximum length is 64 */
        preSharedKey: string;

        /** Hide SSID or not, false(default): not hide */
        isHiddenSsid: boolean;

        /** Security type: reference definition of WifiSecurityType */
        securityType: number;
    }

    /**
     * Describes the scanned Wi-Fi information.
     *
     * @devices phone, tablet
     * @since 6
     */
    interface WifiScanInfo {
        /** Wi-Fi SSID: the maximum length is 32 */
        ssid: string;

        /** Wi-Fi bssid(MAC): the length is 6 */
        bssid: string;

        /** Security type: reference definition of WifiSecurityType */
        securityType: number;

        /** Received signal strength indicator (RSSI) */
        rssi: number;

        /** Frequency band */
        band: number;

        /** Frequency */
        frequency: number;

        /** Time stamp */
        timestamp: number;
    }

    /**
     * Describes the wifi security type.
     *
     * @devices phone, tablet
     * @since 6
     */
    enum WifiSecurityType {
        WIFI_SEC_TYPE_INVALID = 0, /* Invalid security type */
        WIFI_SEC_TYPE_OPEN = 1, /* Open */
        WIFI_SEC_TYPE_WEP = 2, /* Wired Equivalent Privacy (WEP) */
        WIFI_SEC_TYPE_PSK = 3, /* Pre-shared key (PSK) */
        WIFI_SEC_TYPE_SAE = 4, /* Simultaneous Authentication of Equals (SAE) */
    }
}

export default wifi;
