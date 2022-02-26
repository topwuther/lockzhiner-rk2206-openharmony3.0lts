/*
 * osal_timer.c
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

#include "osal_timer.h"
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/signal.h>
#include <linux/timer.h>
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_mutex.h"

#define HDF_LOG_TAG osal_timer

typedef enum {
	OSAL_TIMER_ONCE,
	OSAL_TIMER_LOOP,
} OsalTimerMode;

struct osal_ktimer {
	unsigned long arg;
	struct timer_list timer;
	OsalTimerFunc func;
	uint32_t msec;
	struct OsalMutex mutex;
	OsalTimerMode mode;
};

static void osal_timer_callback(struct timer_list *arg)
{
	struct osal_ktimer *ktimer = NULL;
	uint32_t msec;
	OsalTimerMode mode;

	if (arg == NULL) {
		HDF_LOGI("%s timer is stopped", __func__);
		return;
	}

	ktimer = from_timer(ktimer, arg, timer);

	OsalMutexTimedLock(&ktimer->mutex, HDF_WAIT_FOREVER);
	msec = ktimer->msec;
	mode = ktimer->mode;
	OsalMutexUnlock(&ktimer->mutex);

	if (msec != 0) {
		ktimer->func(ktimer->arg);
		if (mode == OSAL_TIMER_LOOP) {
			ktimer->timer.expires = jiffies + msecs_to_jiffies(msec);
			mod_timer(&ktimer->timer, ktimer->timer.expires);
		}
	} else {
		del_timer(&ktimer->timer);
		OsalMutexDestroy(&ktimer->mutex);
		OsalMemFree(ktimer);
		HDF_LOGI("%s timer is stop", __func__);
	}
}

int32_t OsalTimerCreate(OsalTimer *timer, uint32_t interval, OsalTimerFunc func, uintptr_t arg)
{
	struct osal_ktimer *ktimer = NULL;

	if (func == NULL || timer == NULL || interval == 0) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	ktimer = (struct osal_ktimer *)OsalMemCalloc(sizeof(*ktimer));
	if (ktimer == NULL) {
		HDF_LOGE("%s malloc fail", __func__);
		timer->realTimer = NULL;
		return HDF_ERR_MALLOC_FAIL;
	}

	ktimer->arg = arg;
	ktimer->func = func;
	ktimer->msec = interval;
	OsalMutexInit(&ktimer->mutex);
	timer->realTimer = (void *)ktimer;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalTimerCreate);

static int32_t OsalTimerStart(OsalTimer *timer, OsalTimerMode mode)
{
	struct osal_ktimer *ktimer = NULL;
	struct timer_list *timer_id = NULL;

	if (timer == NULL || timer->realTimer == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
    }

	ktimer = (struct osal_ktimer *)timer->realTimer;
	timer_id = &ktimer->timer;
	timer_setup(timer_id, osal_timer_callback, 0);
	ktimer->mode = mode;
	timer_id->expires = jiffies + msecs_to_jiffies(ktimer->msec);
	add_timer(timer_id);

	return HDF_SUCCESS;
}

int32_t OsalTimerStartOnce(OsalTimer *timer)
{
	return OsalTimerStart(timer, OSAL_TIMER_ONCE);
}
EXPORT_SYMBOL(OsalTimerStartOnce);

int32_t OsalTimerStartLoop(OsalTimer *timer)
{
	return OsalTimerStart(timer, OSAL_TIMER_LOOP);
}

EXPORT_SYMBOL(OsalTimerStartLoop);

int32_t OsalTimerSetTimeout(OsalTimer *timer, uint32_t interval)
{
	struct osal_ktimer *ktimer = NULL;

	if (timer == NULL || timer->realTimer == NULL || interval == 0) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	ktimer = (struct osal_ktimer *)timer->realTimer;
	if (ktimer->msec == interval)
		return HDF_SUCCESS;

	OsalMutexTimedLock(&ktimer->mutex, HDF_WAIT_FOREVER);
	ktimer->msec = interval;
	OsalMutexUnlock(&ktimer->mutex);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalTimerSetTimeout);

int32_t OsalTimerDelete(OsalTimer *timer)
{
	struct osal_ktimer *ktimer = NULL;

	if (timer == NULL || timer->realTimer == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	ktimer = (struct osal_ktimer *)timer->realTimer;
	OsalMutexTimedLock(&ktimer->mutex, HDF_WAIT_FOREVER);
	ktimer->msec = 0;
	OsalMutexUnlock(&ktimer->mutex);

	if (ktimer->mode == OSAL_TIMER_ONCE)
		mod_timer(&ktimer->timer, ktimer->timer.expires);

	timer->realTimer = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalTimerDelete);

