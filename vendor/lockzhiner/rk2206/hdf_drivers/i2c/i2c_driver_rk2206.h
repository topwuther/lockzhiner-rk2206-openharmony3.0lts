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

#ifndef _I2C_DRIVER_H__
#define _I2C_DRIVER_H__

#include "lz_hardware.h"
#include "device_resource_if.h"
#include "osal_mutex.h"
#ifdef __cplusplus
extern "C" {
#endif

struct I2cResource {
    uint32_t port;
    uint32_t mode;
    uint32_t sclPin;
    uint32_t sdaPin;
    uint32_t useSync;
    uint32_t useDma;
    uint32_t asMaster;
    uint32_t speed;
    uint32_t addrAsSlave;
    uint32_t addressWidth;
};

struct I2cDevice {
    uint16_t devAddr;      /**< slave device addr */
    uint32_t addressWidth; /**< Addressing mode: 7 bit or 10 bit */
    struct OsalMutex mutex;
    uint32_t port;
    struct I2cResource resource;
};

#ifdef __cplusplus
}
#endif

#endif