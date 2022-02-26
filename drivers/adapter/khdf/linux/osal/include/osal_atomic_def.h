/*
 * osal_atomic_def.h
 *
 * osal driver
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#ifndef OSAL_ATOMIC_DEF_H
#define OSAL_ATOMIC_DEF_H

#include <linux/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OsalAtomicReadWrapper(v) atomic_read((const atomic_t *)(v))
#define OsalAtomicSetWrapper(v, value) atomic_set((atomic_t *)(v), value)
#define OsalAtomicIncWrapper(v) atomic_inc((atomic_t *)(v))
#define OsalAtomicIncRetWrapper(v) atomic_inc_return((atomic_t *)(v))
#define OsalAtomicDecWrapper(v) atomic_dec((atomic_t *)(v))
#define OsalAtomicDecRetWrapper(v) atomic_dec_return((atomic_t *)(v))

#define OsalTestBitWrapper(nr, addr) test_bit(nr, addr)
#define OsalTestSetBitWrapper(nr, addr) test_and_change_bit(nr, addr)
#define OsalTestClearBitWrapper(nr, addr) test_and_clear_bit(nr, addr)
#define OsalClearBitWrapper(nr, addr) clear_bit(nr, addr)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OSAL_ATOMIC_DEF_H */

