/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_if.h"
#include "devsvc_manager_clnt.h"
#include "gpio_core.h"
#include "hdf_base.h"
#include "hdf_log.h"

#define HDF_LOG_TAG gpio_if

int32_t GpioRead(uint16_t gpio, uint16_t *val)
{
    return GpioCntlrRead(GpioGetCntlr(gpio), GpioToLocal(gpio), val);
}

int32_t GpioWrite(uint16_t gpio, uint16_t val)
{
    return GpioCntlrWrite(GpioGetCntlr(gpio), GpioToLocal(gpio), val);
}

int32_t GpioSetDir(uint16_t gpio, uint16_t dir)
{
    return GpioCntlrSetDir(GpioGetCntlr(gpio), GpioToLocal(gpio), dir);
}

int32_t GpioGetDir(uint16_t gpio, uint16_t *dir)
{
    return GpioCntlrGetDir(GpioGetCntlr(gpio), GpioToLocal(gpio), dir);
}

int32_t GpioSetIrq(uint16_t gpio, uint16_t mode, GpioIrqFunc func, void *arg)
{
    return GpioCntlrSetIrq(GpioGetCntlr(gpio), GpioToLocal(gpio), mode, func, arg);
}

int32_t GpioUnSetIrq(uint16_t gpio)
{
    return GpioCntlrUnsetIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}

int32_t GpioEnableIrq(uint16_t gpio)
{
    return GpioCntlrEnableIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}

int32_t GpioDisableIrq(uint16_t gpio)
{
    return GpioCntlrDisableIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}
