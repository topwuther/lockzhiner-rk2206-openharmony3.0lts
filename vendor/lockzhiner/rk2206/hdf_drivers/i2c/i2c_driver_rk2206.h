/*
 * Copyright (c) 2021 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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