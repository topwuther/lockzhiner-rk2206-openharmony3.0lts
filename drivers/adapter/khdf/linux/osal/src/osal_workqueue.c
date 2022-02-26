/*
 * osal_workqueue.c
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

#include "hdf_workqueue.h"
#include <linux/workqueue.h>
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG hdf_workqueue

struct WorkWrapper {
	struct delayed_work work;
	HdfWorkFunc workFunc;
	void *para;
};

int32_t HdfWorkQueueInit(HdfWorkQueue *queue, char *name)
{
	HDF_LOGD("%s  entry", __func__);

	if (queue == NULL || name == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	queue->realWorkQueue = create_singlethread_workqueue(name);
	if (queue->realWorkQueue == NULL) {
		HDF_LOGE("%s create queue fail", __func__);
		return HDF_FAILURE;
	}

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(HdfWorkQueueInit);

static void WorkEntry(struct work_struct *work)
{
	struct WorkWrapper *wrapper = NULL;
	if (work != NULL) {
		wrapper = (struct WorkWrapper *)work;
		if (wrapper->workFunc != NULL)
			wrapper->workFunc(wrapper->para);
		else
			HDF_LOGE("%s routine null", __func__);
	} else {
		HDF_LOGE("%s work null", __func__);
	}
}

int32_t HdfWorkInit(HdfWork *work, HdfWorkFunc func, void *para)
{
	struct work_struct *realWork = NULL;
	struct WorkWrapper *wrapper = NULL;

	if (work == NULL || func == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}
	work->realWork = NULL;

	wrapper = (struct WorkWrapper *)OsalMemCalloc(sizeof(*wrapper));
	if (wrapper == NULL) {
		HDF_LOGE("%s malloc fail", __func__);
		return HDF_ERR_MALLOC_FAIL;
	}
	realWork = &(wrapper->work.work);
	wrapper->workFunc = func;
	wrapper->para = para;

	INIT_WORK(realWork, WorkEntry);
	work->realWork = wrapper;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(HdfWorkInit);

int32_t HdfDelayedWorkInit(HdfWork *work, HdfWorkFunc func, void *para)
{
	struct delayed_work *realWork = NULL;
	struct WorkWrapper *wrapper = NULL;

	if (work == NULL || func == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	work->realWork = NULL;

	wrapper = (struct WorkWrapper *)OsalMemCalloc(sizeof(*wrapper));
	if (wrapper == NULL) {
		HDF_LOGE("%s malloc fail", __func__);
		return HDF_ERR_MALLOC_FAIL;
	}
	realWork = &(wrapper->work);
	wrapper->workFunc = func;
	wrapper->para = para;

	INIT_DELAYED_WORK(realWork, WorkEntry);
	work->realWork = wrapper;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(HdfDelayedWorkInit);

void HdfWorkDestroy(HdfWork *work)
{
	if (work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return;
	}

	OsalMemFree(work->realWork);
	work->realWork = NULL;

	return;
}
EXPORT_SYMBOL(HdfWorkDestroy);

void HdfDelayedWorkDestroy(HdfWork *work)
{
	if (work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return;
	}

	return HdfWorkDestroy(work);
}
EXPORT_SYMBOL(HdfDelayedWorkDestroy);

void HdfWorkQueueDestroy(HdfWorkQueue *queue)
{
	if (queue == NULL || queue->realWorkQueue == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return;
	}

	destroy_workqueue(queue->realWorkQueue);

	return;
}
EXPORT_SYMBOL(HdfWorkQueueDestroy);

bool HdfAddWork(HdfWorkQueue *queue, HdfWork *work)
{
	if (queue == NULL || queue->realWorkQueue == NULL || work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return false;
	}

	return queue_work(queue->realWorkQueue, &((struct WorkWrapper *)work->realWork)->work.work);
}
EXPORT_SYMBOL(HdfAddWork);

bool HdfAddDelayedWork(HdfWorkQueue *queue, HdfWork *work, unsigned long ms)
{
	if (queue == NULL || queue->realWorkQueue == NULL || work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return false;
	}

	return queue_delayed_work(queue->realWorkQueue, &((struct WorkWrapper *)work->realWork)->work,
		msecs_to_jiffies(ms));
}
EXPORT_SYMBOL(HdfAddDelayedWork);

unsigned int HdfWorkBusy(HdfWork *work)
{
	if (work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return 0;
	}

	return work_busy(&((struct WorkWrapper *)work->realWork)->work.work);
}
EXPORT_SYMBOL(HdfWorkBusy);

bool HdfCancelWorkSync(HdfWork *work)
{
	if (work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return false;
	}

	return cancel_work_sync(&((struct WorkWrapper *)work->realWork)->work.work);
}
EXPORT_SYMBOL(HdfCancelWorkSync);

bool HdfCancelDelayedWorkSync(HdfWork *work)
{
	if (work == NULL || work->realWork == NULL) {
		HDF_LOGE("%s invalid para", __func__);
		return false;
	}

	return cancel_delayed_work_sync(&((struct WorkWrapper *)work->realWork)->work);
}
EXPORT_SYMBOL(HdfCancelDelayedWorkSync);

