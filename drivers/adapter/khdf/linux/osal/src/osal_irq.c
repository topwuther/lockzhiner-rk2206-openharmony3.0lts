/*
 * osal_irq.c
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

#include "osal_irq.h"
#include <linux/export.h>
#include <linux/interrupt.h>
#include "hdf_log.h"

#define HDF_LOG_TAG osal_irq

int32_t OsalRegisterIrq(uint32_t irq,
	uint32_t config, OsalIRQHandle handle, const char *name, void *data)
{
	uint32_t ret;
	const char *irq_name = NULL;

	irq_name = (name != NULL) ? name : "hdf_irq";

	ret = request_threaded_irq(irq, NULL, (irq_handler_t)handle,
		config | IRQF_ONESHOT | IRQF_NO_SUSPEND, irq_name, data);
	if (ret != 0) {
		HDF_LOGE("%s fail %u", __func__, ret);
		return HDF_FAILURE;
	}

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalRegisterIrq);

int32_t OsalUnregisterIrq(uint32_t irq, void *dev)
{
	disable_irq(irq);

	free_irq(irq, dev);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalUnregisterIrq);

int32_t OsalEnableIrq(uint32_t irq)
{
	enable_irq(irq);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalEnableIrq);

int32_t OsalDisableIrq(uint32_t irq)
{
	disable_irq(irq);

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalDisableIrq);

