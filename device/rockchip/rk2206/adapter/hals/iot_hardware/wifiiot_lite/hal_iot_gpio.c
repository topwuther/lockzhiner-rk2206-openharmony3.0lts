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
#include "iot_gpio.h"
#include "lz_hardware.h"

#define TESTGPIO GPIO0_PA0

unsigned int IoTGpioInit(unsigned int id)
{
    id = TESTGPIO;
    if (HAL_PINCTRL_SetIOMUX(id / 32, 1 << (id % 32), (1UL << (31))) != 0)//设置复用为普通gpio功能
        return 1;

    return LzGpioInit(id);
}

unsigned int IoTGpioDeinit(unsigned int id)
{
    id = TESTGPIO;
    return LzGpioDeinit(id);
}

unsigned int IoTGpioSetDir(unsigned int id, IotGpioDir dir)
{
    id = TESTGPIO;
    return LzGpioSetDir(id, (LzGpioDir)dir);
}

unsigned int IoTGpioGetDir(unsigned int id, IotGpioDir *dir)
{
    id = TESTGPIO;
    return LzGpioGetDir(id, (LzGpioDir *)dir);
}

unsigned int IoTGpioSetOutputVal(unsigned int id, IotGpioValue val)
{
    id = TESTGPIO;
    return LzGpioSetVal(id, (LzGpioValue)val);
}

unsigned int IoTGpioGetOutputVal(unsigned int id, IotGpioValue *val)
{
    id = TESTGPIO;
    return LzGpioGetVal(id, (LzGpioValue *)val);
}

unsigned int IoTGpioGetInputVal(unsigned int id, IotGpioValue *val)
{
    id = TESTGPIO;
    return LzGpioGetVal(id, (LzGpioValue *)val);
}

unsigned int IoTGpioRegisterIsrFunc(unsigned int id, IotGpioIntType intType, IotGpioIntPolarity intPolarity,
                                    GpioIsrCallbackFunc func, char *arg)
{
    LzGpioIntType type;
    id = TESTGPIO;

    if (intType == IOT_INT_TYPE_LEVEL && intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW)
        type = LZGPIO_INT_LEVEL_LOW;
    else if (intType == IOT_INT_TYPE_LEVEL && intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH)
        type = LZGPIO_INT_LEVEL_HIGH;
    else if (intType == IOT_INT_TYPE_EDGE && intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW)
        type = LZGPIO_INT_EDGE_FALLING;
    else if (intType == IOT_INT_TYPE_EDGE && intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH)
        type = LZGPIO_INT_EDGE_RISING;
    else
        return LZ_HARDWARE_FAILURE;
    return LzGpioRegisterIsrFunc(id, type, (GpioIsrFunc)func, arg);
}

unsigned int IoTGpioUnregisterIsrFunc(unsigned int id)
{
    id = TESTGPIO;
    return LzGpioUnregisterIsrFunc(id);
}

unsigned int IoTGpioSetIsrMask(unsigned int id, unsigned char mask)
{
    id = TESTGPIO;
    if (mask)
        return LzGpioDisableIsr(id);
    else
        return LzGpioEnableIsr(id);
}

unsigned int IoTGpioSetIsrMode(unsigned int id, IotGpioIntType intType, IotGpioIntPolarity intPolarity)
{
    return LZ_HARDWARE_SUCCESS;
}
