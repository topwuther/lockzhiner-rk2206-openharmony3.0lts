/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "mtd_core.h"

__attribute__((weak)) int32_t MtdBlockOsInit(struct MtdDevice *mtdDevice)
{
    (void)mtdDevice;
    return HDF_SUCCESS;
}

__attribute__ ((weak)) void MtdBlockOsUninit(struct MtdDevice *mtdDevice)
{
    (void)mtdDevice;
}

int32_t MtdBlockInit(struct MtdDevice *mtdDevice)
{
    return MtdBlockOsInit(mtdDevice);
}

void MtdBlockUninit(struct MtdDevice *mtdDevice)
{
    MtdBlockOsUninit(mtdDevice);
}
