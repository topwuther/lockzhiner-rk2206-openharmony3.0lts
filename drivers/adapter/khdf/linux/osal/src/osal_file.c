/*
 * osal_file.c
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

#include "osal_file.h"
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "hdf_log.h"

#define HDF_LOG_TAG osal_file

int32_t OsalFileOpen(OsalFile *file, const char *path, int flags, uint32_t rights)
{
	struct file *fp = NULL;

	if (file == NULL || path == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}

	file->realFile = (void *)fp;

	fp = filp_open(path, flags, rights);
	if (IS_ERR_OR_NULL(fp)) {
		HDF_LOGE("%s open file fail %d %d", __func__, flags, rights);
		return HDF_FAILURE;
	}

	file->realFile = (void *)fp;
	return HDF_SUCCESS;
}
EXPORT_SYMBOL(OsalFileOpen);

ssize_t OsalFileWrite(OsalFile *file, const void *string, uint32_t length)
{
	ssize_t ret;
	loff_t pos;
	mm_segment_t org_fs;
	struct file *fp = NULL;

	if (file == NULL || IS_ERR_OR_NULL(file->realFile) || string == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}
	fp = (struct file *)file->realFile;
	pos = fp->f_pos;
	org_fs = get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_write(fp, string, length, &pos);
	set_fs(org_fs);
	if (ret < 0) {
		HDF_LOGE("%s write file length %d fail %d", __func__, length, ret);
		return HDF_FAILURE;
	}

	return ret;
}
EXPORT_SYMBOL(OsalFileWrite);

void OsalFileClose(OsalFile *file)
{
	struct file *fp = NULL;

	if (file == NULL || IS_ERR_OR_NULL(file->realFile)) {
		HDF_LOGE("%s invalid param", __func__);
		return;
	}
	fp = (struct file *)file->realFile;
	filp_close(fp, NULL);
	file->realFile = NULL;
}
EXPORT_SYMBOL(OsalFileClose);

ssize_t OsalFileRead(OsalFile *file, void *buf, uint32_t length)
{
	ssize_t ret;
	mm_segment_t org_fs;
	loff_t pos;
	struct file *fp = NULL;

	if (file == NULL || IS_ERR_OR_NULL(file->realFile) || buf == NULL) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}
	fp = (struct file *)file->realFile;
	org_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = fp->f_pos;

	ret = vfs_read(fp, buf, length, &pos);
	set_fs(org_fs);
	if (ret < 0) {
		HDF_LOGE("%s read file length %d fail %d", __func__, length, ret);
		return HDF_FAILURE;
	}

	return ret;
}
EXPORT_SYMBOL(OsalFileRead);

off_t OsalFileLseek(OsalFile *file, off_t offset, int32_t whence)
{
	off_t ret;
	struct file *fp = NULL;

	if (file == NULL || IS_ERR_OR_NULL(file->realFile)) {
		HDF_LOGE("%s invalid param", __func__);
		return HDF_ERR_INVALID_PARAM;
	}
	fp = (struct file *)file->realFile;
	ret = vfs_llseek(fp, offset, whence);
	if (ret < 0) {
		HDF_LOGE("%s lseek file fail %ld %d", __func__, offset, whence);
		return HDF_FAILURE;
	}

	return ret;
}
EXPORT_SYMBOL(OsalFileLseek);

