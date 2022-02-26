/*
 * osal_uaccess.h
 *
 * osal driver
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

#ifndef OSAL_UACCESS_DEF_H
#define OSAL_UACCESS_DEF_H

#include <linux/kernel.h>
#include <linux/uaccess.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline size_t CopyToUser (void __user *to, const void *from, size_t len)
{
    return (size_t)copy_to_user(to, from, (unsigned long)len);
}

static inline size_t CopyFromUser (void *to, const void __user *from, size_t len)
{
    return (size_t)copy_from_user(to, from, (unsigned long)len);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OSAL_UACCESS_DEF_H */

