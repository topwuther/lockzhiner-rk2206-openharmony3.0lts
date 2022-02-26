/*
 * osal_firmware.c
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

#include "osal_firmware.h"
#include <linux/export.h>
#include <linux/firmware.h>
#include <linux/string.h>
#include "hdf_log.h"

#define HDF_LOG_TAG osal_fw

int32_t OsalRequestFirmware(struct OsalFirmware *fwPara,
	const char *fwName, void *device)
{
	const struct firmware *fw = NULL;
	int ret;

	if (fwPara == NULL || fwName == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	ret = request_firmware(&fw, fwName, device);
	if (ret < 0) {
		fwPara->fwSize = 0;
		fwPara->para = NULL;
		HDF_LOGE("%s failure to request firmware file", __func__);
		return HDF_FAILURE;
	}
	fwPara->fwSize = fw->size;
	fwPara->para = (void *)fw;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalRequestFirmware);

int32_t OsalSeekFirmware(struct OsalFirmware *fwPara, uint32_t offset)
{
	(void)fwPara;
	(void)offset;
	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalSeekFirmware);

int32_t OsalReadFirmware(struct OsalFirmware *fwPara, struct OsalFwBlock *block)
{
	struct firmware *fw = NULL;

	if (fwPara == NULL || fwPara->para == NULL || block == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	fw = (struct firmware *)fwPara->para;
	block->data = (uint8_t *)fw->data;
	block->dataSize = fwPara->fwSize;
	block->curOffset = 0;
	block->endFlag = true;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalReadFirmware);

int32_t OsalReleaseFirmware(struct OsalFirmware *fwPara)
{
	if (fwPara == NULL || fwPara->para == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	release_firmware((struct firmware *)fwPara->para);
	fwPara->para = NULL;

	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalReleaseFirmware);
