/*
 * osal_thread.c
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

#include "osal_thread.h"
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/types.h>
#include <uapi/linux/sched.h>
#include <uapi/linux/sched/types.h>
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG osal_thread
#define OSAL_INVALID_CPU_ID UINT_MAX

struct thread_wrapper {
	OsalThreadEntry thread_entry;
	void *entry_para;
	struct task_struct *task;
	uint32_t cpu_id;
};

enum {
	OSAL_PRIORITY_MIDDLE  = 50,
	OSAL_PRIORITY_HIGH    = 90,
	OSAL_PRIORITY_HIGHEST = 99,
};

static int osal_thread_entry(void *para)
{
	int ret = -1;

	struct thread_wrapper *wrapper = (struct thread_wrapper *)para;
	if (wrapper == NULL || wrapper->thread_entry == NULL) {
		HDF_LOGE("%s invalid param", __func__);
	} else {
		ret = wrapper->thread_entry(wrapper->entry_para);
	}

	do_exit(ret);
	return ret;
}

int32_t OsalThreadCreate(struct OsalThread *thread, OsalThreadEntry thread_entry, void *entry_para)
{
	struct thread_wrapper *wrapper = NULL;

	if (thread == NULL || thread_entry == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	thread->realThread = NULL;
	wrapper = (struct thread_wrapper *)OsalMemCalloc(sizeof(*wrapper));
	if (wrapper == NULL) {
		HDF_LOGE("%s malloc fail", __func__);
		return HDF_ERR_MALLOC_FAIL;
	}

	wrapper->entry_para = entry_para;
	wrapper->thread_entry = thread_entry;
	wrapper->cpu_id = OSAL_INVALID_CPU_ID;
	thread->realThread = wrapper;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalThreadCreate);

int32_t OsalThreadBind(struct OsalThread *thread, unsigned int cpu_id)
{
	struct thread_wrapper *wrapper = NULL;

	if (thread == NULL || thread->realThread == NULL) {
		HDF_LOGE("%s invalid parameter %d\n", __func__, __LINE__);
		return HDF_ERR_INVALID_PARAM;
	}
	wrapper = (struct thread_wrapper *)thread->realThread;
	wrapper->cpu_id = cpu_id;
	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalThreadBind);

int32_t OsalThreadStart(struct OsalThread *thread, const struct OsalThreadParam *param)
{
	int32_t ret;
	struct sched_param sched_para;
	int32_t policy = SCHED_FIFO;
	struct task_struct *task = NULL;
	struct thread_wrapper *wrapper = NULL;

	if (thread == NULL || thread->realThread == NULL || param == NULL || param->name == NULL) {
		HDF_LOGE("%s invalid parameter\n", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	(void)memset_s(&sched_para, sizeof(sched_para), 0, sizeof(sched_para));

	if (param->priority == OSAL_THREAD_PRI_HIGHEST)
		sched_para.sched_priority = OSAL_PRIORITY_HIGHEST;
	else if (param->priority == OSAL_THREAD_PRI_HIGH)
		sched_para.sched_priority = OSAL_PRIORITY_HIGH;
	else if (param->priority == OSAL_THREAD_PRI_DEFAULT)
		sched_para.sched_priority = OSAL_PRIORITY_MIDDLE;
	else
		policy = SCHED_NORMAL;

	wrapper = (struct thread_wrapper *)thread->realThread;
	task = kthread_create(osal_thread_entry, wrapper, param->name);
	if (IS_ERR(task)) {
		ret = PTR_ERR(task);
		HDF_LOGE("%s kthread_create fail %d", __func__, ret);
		return HDF_FAILURE;
	}
	if (wrapper->cpu_id != OSAL_INVALID_CPU_ID) {
		kthread_bind(task, wrapper->cpu_id);
	}
	wake_up_process(task);
	if (policy == SCHED_FIFO) {
		if (sched_setscheduler(task, policy, &sched_para)) {
			HDF_LOGE("%s sched_setscheduler fail", __func__);
			kthread_stop(task);
			return HDF_FAILURE;
		}
	}

	wrapper->task = task;

	return HDF_SUCCESS;
}

EXPORT_SYMBOL(OsalThreadStart);

int32_t OsalThreadSuspend(struct OsalThread *thread)
{
	return HDF_ERR_NOT_SUPPORT;
}
EXPORT_SYMBOL(OsalThreadSuspend);

int32_t OsalThreadDestroy(struct OsalThread *thread)
{
	if (thread == NULL || thread->realThread == NULL) {
		HDF_LOGE("%s invalid parameter\n", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	OsalMemFree(thread->realThread);
	thread->realThread = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalThreadDestroy);

int32_t OsalThreadResume(struct OsalThread *thread)
{
	return HDF_ERR_NOT_SUPPORT;
}
EXPORT_SYMBOL(OsalThreadResume);

