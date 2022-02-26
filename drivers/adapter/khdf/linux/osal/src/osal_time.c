/*
 * osal_time.c
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

#include "osal_time.h"
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/rtc.h>
#include <linux/string.h>
#include <linux/timekeeping.h>
#include "hdf_log.h"
#include "osal_math.h"
#include "securec.h"

#define HDF_LOG_TAG osal_time

#define TM_SINCE_YEAR 1900
#define TM_MINUTE_UNIT 60

int32_t OsalGetTime(OsalTimespec *time)
{
	struct timespec64 ts;

	if (time == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	(void)memset_s(&ts, sizeof(ts), 0, sizeof(ts));
	ktime_get_ts64(&ts);
	time->sec = ts.tv_sec;
	time->usec = ts.tv_nsec / HDF_KILO_UNIT;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalGetTime);

int32_t OsalDiffTime(const OsalTimespec *start, const OsalTimespec *end, OsalTimespec *diff)
{
	uint32_t usec = 0;
	uint32_t sec = 0;
	if (start == NULL || end == NULL || diff == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	if (start->sec > end->sec) {
		HDF_LOGE("%s start time later then end time", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	if (end->usec < start->usec) {
		usec = (HDF_KILO_UNIT * HDF_KILO_UNIT);
		sec = 1;
	}
	diff->usec = usec + end->usec - start->usec;
	diff->sec = end->sec - start->sec - sec;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalDiffTime);

void OsalSleep(uint32_t sec)
{
	msleep(sec * HDF_KILO_UNIT);
}
EXPORT_SYMBOL(OsalSleep);

void OsalMSleep(uint32_t mSec)
{
	msleep(mSec);
}
EXPORT_SYMBOL(OsalMSleep);

void OsalUDelay(uint32_t us)
{
	udelay(us);
}
EXPORT_SYMBOL(OsalUDelay);

void OsalMDelay(uint32_t ms)
{
	mdelay(ms);
}
EXPORT_SYMBOL(OsalMDelay);

uint64_t OsalGetSysTimeMs()
{
	OsalTimespec time;

	(void)memset_s(&time, sizeof(time), 0, sizeof(time));
	(void)OsalGetTime(&time);

    return (time.sec * HDF_KILO_UNIT + OsalDivS64(time.usec, HDF_KILO_UNIT));
}
EXPORT_SYMBOL(OsalGetSysTimeMs);
