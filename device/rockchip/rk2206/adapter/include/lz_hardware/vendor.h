/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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
 * @addtogroup Lockzhiner
 *
 * @file vendor.h
 */

#ifndef LZ_HARDWARE_VENDOR_H
#define LZ_HARDWARE_VENDOR_H

#define RK2206_SOFT_AP_SSID      "softap"
#define RK2206_SOFT_AP_PASSWORD  "12345678"
#define RK2206_SOAT_AP_NFC_MSG   "RK2206_AP_SETTING\r\n" RK2206_SOFT_AP_SSID "\r\n" RK2206_SOFT_AP_PASSWORD

#define VENDOR_ID_SIZE           64

typedef enum {
    /* 13Bytes Manufacture ID */
    VENDOR_ID_SN = 0,
    /* 6Bytes Mac Address */
    VENDOR_ID_MAC,
    /* Product ID */
    VENDOR_ID_PRODUCT,
    /* Factory Test flags */
    VENDOR_ID_FACTORY,
    /* Wifi hotspot mode: IP address */
    VENDOR_ID_NET_IP,
    /* Wifi hotspot mode: gateway */
    VENDOR_ID_NET_GW,
    /* Wifi hotspot mode: netmask */
    VENDOR_ID_NET_MASK,
    /* Wifi device mode: ssid */
    VENDOR_ID_WIFI_SSID,
    /* Wifi device mode: password */
    VENDOR_ID_WIFI_PASSWD,
    /* Route device mode: ssid */
    VENDOR_ID_WIFI_ROUTE_SSID,
    /* Route device mode: password */
    VENDOR_ID_WIFI_ROUTE_PASSWD,
    /* AP OR STA MODE */
    VENDOR_ID_WIFI_MODE,

    /* Custom definition ID */
    VENDOR_ID_CUSTOM = 32,
    VENDOR_ID_MAX = 64,  // Each ID has 64Bytes storage space, Total is 4096Btyes(4K)
} VendorID;

/**
 * @brief Get vendor information.
 *
 * This function get vendor information by ID.
 *
 * @param id Indicates the vendor ID.
 * @param buf Indicates the buffer to save vendor information.
 * @param len Indicates the length of vendor information to get, it should be less than 64.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the vendor information is got successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int VendorGet(VendorID id, unsigned char *buf, int len);

/**
 * @brief Set vendor information.
 *
 * This function set vendor information by ID.
 *
 * @param id Indicates the vendor ID.
 * @param buf Indicates the buffer to save vendor information.
 * @param len Indicates the length of vendor information to set, it should be less than 64.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the vendor information is set successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int VendorSet(VendorID id, unsigned char *buf, int len);

#endif
/** @} */
