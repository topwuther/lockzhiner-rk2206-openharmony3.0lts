/*
 * osal_spinlock.c
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

#include "osal_spinlock.h"
#include <linux/export.h>
#include <linux/spinlock.h>
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG osal_spinlock

int32_t OsalSpinInit(OsalSpinlock *spinlock)
{
	spinlock_t *spin_tmp = NULL;

	if (spinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_tmp = (spinlock_t *)OsalMemCalloc(sizeof(*spin_tmp));
	if (spin_tmp == NULL) {
		HDF_LOGE("malloc fail");
		spinlock->realSpinlock = NULL;
		return HDF_ERR_MALLOC_FAIL;
	}
	spin_lock_init(spin_tmp);
	spinlock->realSpinlock = (void *)spin_tmp;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinInit);

int32_t OsalSpinDestroy(OsalSpinlock *spinlock)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	OsalMemFree(spinlock->realSpinlock);
	spinlock->realSpinlock = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinDestroy);

int32_t OsalSpinLock(OsalSpinlock *spinlock)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_lock((spinlock_t *)spinlock->realSpinlock);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinLock);

int32_t OsalSpinUnlock(OsalSpinlock *spinlock)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_unlock((spinlock_t *)spinlock->realSpinlock);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinUnlock);

int32_t OsalSpinLockIrq(OsalSpinlock *spinlock)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_lock_irq((spinlock_t *)spinlock->realSpinlock);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinLockIrq);

int32_t OsalSpinUnlockIrq(OsalSpinlock *spinlock)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_unlock_irq((spinlock_t *)spinlock->realSpinlock);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinUnlockIrq);

int32_t OsalSpinLockIrqSave(OsalSpinlock *spinlock, uint32_t *flags)
{
	unsigned long temp = 0;

	if (spinlock == NULL || spinlock->realSpinlock == NULL || flags == NULL) {
		HDF_LOGE("%s invalid param %d", __func__, __LINE__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_lock_irqsave((spinlock_t *)spinlock->realSpinlock, temp);
	*flags = temp;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinLockIrqSave);

int32_t OsalSpinUnlockIrqRestore(OsalSpinlock *spinlock, uint32_t *flags)
{
	if (spinlock == NULL || spinlock->realSpinlock == NULL || flags == NULL) {
		HDF_LOGE("%s invalid param %d", __func__, __LINE__);
		return HDF_ERR_INVALID_PARAM;
	}

	spin_unlock_irqrestore((spinlock_t *)spinlock->realSpinlock, *flags);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSpinUnlockIrqRestore);

