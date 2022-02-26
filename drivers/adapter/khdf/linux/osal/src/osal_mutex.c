/*
 * osal_mutex.c
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

#include "osal_mutex.h"
#include <linux/export.h>
#include <linux/mutex.h>
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG osal_mutex

int32_t OsalMutexInit(struct OsalMutex *mutex)
{
	struct mutex *mutex_tmp = NULL;

	if (mutex == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	mutex_tmp = (struct mutex *)OsalMemCalloc(sizeof(*mutex_tmp));
	if (mutex_tmp == NULL) {
		HDF_LOGE("malloc fail");
		return HDF_ERR_MALLOC_FAIL;
	}
	mutex_init(mutex_tmp);
	mutex->realMutex = (void *)mutex_tmp;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalMutexInit);

int32_t OsalMutexDestroy(struct OsalMutex *mutex)
{
	if (mutex == NULL || mutex->realMutex == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	mutex_destroy((struct mutex *)mutex->realMutex);
	OsalMemFree(mutex->realMutex);
	mutex->realMutex = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalMutexDestroy);

int32_t OsalMutexLock(struct OsalMutex *mutex)
{
	if (mutex == NULL || mutex->realMutex == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	mutex_lock((struct mutex *)mutex->realMutex);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalMutexLock);

int32_t OsalMutexTimedLock(struct OsalMutex *mutex, uint32_t mSec)
{
	if (mutex == NULL || mutex->realMutex == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	(void)mSec;
	mutex_lock((struct mutex *)mutex->realMutex);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalMutexTimedLock);

int32_t OsalMutexUnlock(struct OsalMutex *mutex)
{
	if (mutex == NULL || mutex->realMutex == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	mutex_unlock((struct mutex *)mutex->realMutex);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalMutexUnlock);

