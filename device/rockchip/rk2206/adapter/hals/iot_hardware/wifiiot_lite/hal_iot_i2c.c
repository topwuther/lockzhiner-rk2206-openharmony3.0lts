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

#include "iot_errno.h"
#include "iot_i2c.h"
#include "lz_hardware.h"

#define TEST_ID 0
I2cBusIo i2c = {
        .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .id = FUNC_ID_I2C0,
        .mode = FUNC_MODE_M2,
    };



unsigned int IoTI2cWrite(unsigned int id, unsigned short deviceAddr, const unsigned char *data, unsigned int dataLen)
{
    id = TEST_ID;
    return LzI2cWrite(id, deviceAddr, data, dataLen);
}

unsigned int IoTI2cRead(unsigned int id, unsigned short deviceAddr, unsigned char *data, unsigned int dataLen)
{
    id = TEST_ID;
    return LzI2cRead(id, deviceAddr, data, dataLen);
}

unsigned int IoTI2cInit(unsigned int id, unsigned int baudrate)
{
    unsigned int ret = 0;

    id = TEST_ID;
    ret = I2cIoInit(i2c);
    if(ret != 0)
    {
        return ret;
    }
    return LzI2cInit(id, baudrate);
}

unsigned int IoTI2cDeinit(unsigned int id)
{
    id = TEST_ID;
    return LzI2cDeinit(id);
}

unsigned int IoTI2cSetBaudrate(unsigned int id, unsigned int baudrate)
{
    id = TEST_ID;
    return LzI2cSetFreq(id, baudrate);
}
