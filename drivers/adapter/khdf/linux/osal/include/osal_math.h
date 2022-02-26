/*
 * osal_math.h
 *
 * osal mathematics
 *
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef OSAL_MATH_H
#define OSAL_MATH_H

#include <linux/math64.h>
#include "hdf_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline int64_t OsalDivS64(int64_t dividend, int32_t divisor)
{
    return (int64_t)div_s64(dividend, divisor);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OSAL_MATH_H */

