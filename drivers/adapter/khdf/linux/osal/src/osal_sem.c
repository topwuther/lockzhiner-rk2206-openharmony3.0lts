/*
 * osal_sem.c
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

#include "osal_sem.h"
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG osal_sem

int32_t OsalSemInit(struct OsalSem *sem, uint32_t value)
{
	struct semaphore *sem_tmp = NULL;

	if (sem == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	sem_tmp = (struct semaphore *)OsalMemCalloc(sizeof(*sem_tmp));
	if (sem_tmp == NULL) {
		HDF_LOGE("%s malloc fail", __func__);
		return HDF_ERR_MALLOC_FAIL;
	}
	sema_init(sem_tmp, value);
	sem->realSemaphore = (void *)sem_tmp;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSemInit);

int32_t OsalSemWait(struct OsalSem *sem, uint32_t millisec)
{
	int32_t ret;

	if (sem == NULL || sem->realSemaphore == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	if (millisec == HDF_WAIT_FOREVER) {
		down((struct semaphore *)sem->realSemaphore);
	} else {
		ret = down_timeout((struct semaphore *)sem->realSemaphore, msecs_to_jiffies(millisec));
		if (ret != 0) {
			if (ret == -ETIME) {
				return HDF_ERR_TIMEOUT;
			} else {
				HDF_LOGE("%s time_out %d %d", __func__, millisec, ret);
				return HDF_FAILURE;
			}
		}
	}

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSemWait);

int32_t OsalSemPost(struct OsalSem *sem)
{
	if (sem == NULL || sem->realSemaphore == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	up((struct semaphore *)sem->realSemaphore);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSemPost);

int32_t OsalSemDestroy(struct OsalSem *sem)
{
	if (sem == NULL || sem->realSemaphore == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	OsalMemFree(sem->realSemaphore);
	sem->realSemaphore = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSemDestroy);

