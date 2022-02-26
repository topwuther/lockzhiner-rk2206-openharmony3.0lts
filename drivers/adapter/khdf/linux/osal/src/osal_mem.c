/*
 * osal_mem.c
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

#include "osal_mem.h"
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include "hdf_log.h"
#include "securec.h"

#define HDF_LOG_TAG osal_mem

#define KMALLOC_SIZE 0x20000

struct mem_hdr {
	uint32_t type;
	uint32_t offset;
};

struct mem_block {
	struct mem_hdr hdr;
	uint8_t mem[0];
};

enum {
	TYPE_KMALLOC = 0xa5a5a5a1,
	TYPE_VMALLOC = 0xa5a5a5a2,
};

static void *osal_mem_alloc(size_t size, uint32_t *type)
{
	char *base = NULL;
	const uint32_t mng_size = sizeof(struct mem_block);
	if (size > (SIZE_MAX - mng_size)) {
		HDF_LOGE("%s invalid param %d", __func__, size);
		return NULL;
	}

	if (size > (KMALLOC_SIZE - mng_size)) {
		base = (char *)vmalloc(size + mng_size);
		*type = TYPE_VMALLOC;
	} else {
		base = (char *)kmalloc(size + mng_size, GFP_KERNEL);
		*type = TYPE_KMALLOC;
	}

	return base;
}

void *OsalMemAlloc(size_t size)
{
	void *buf = NULL;
	char *base = NULL;
	struct mem_block *block = NULL;
	const uint32_t mng_size = sizeof(*block);
	uint32_t type;

	if (size == 0) {
		HDF_LOGE("%s invalid param", __func__);
		return NULL;
	}

	base = osal_mem_alloc(size, &type);
	if (base == NULL) {
		HDF_LOGE("%s malloc fail %d", __func__, size);
		return NULL;
	}

	block = (struct mem_block *)base;
	block->hdr.type = type;
	block->hdr.offset = 0;

	buf = (void *)(base + mng_size);

	return buf;
}
EXPORT_SYMBOL(OsalMemAlloc);

void *OsalMemCalloc(size_t size)
{
	void *buf = NULL;

	if (size == 0) {
		HDF_LOGE("%s invalid param", __func__);
		return NULL;
	}

	buf = OsalMemAlloc(size);
	if (buf != NULL)
		(void)memset_s(buf, size, 0, size);

	return buf;
}
EXPORT_SYMBOL(OsalMemCalloc);

void *OsalMemAllocAlign(size_t alignment, size_t size)
{
	char *base = NULL;
	char *buf = NULL;
	struct mem_block *block = NULL;
	const uint32_t mng_size = sizeof(*block);
	uint32_t type;
	uint32_t offset;
	char *aligned_ptr = NULL;

	if (size == 0 || (alignment == 0) || ((alignment & (alignment - 1)) != 0) ||
		((alignment % sizeof(void *)) != 0) || size > (SIZE_MAX - alignment)) {
		HDF_LOGE("%s invalid param align:%d,size:%d", __func__, alignment, size);
		return NULL;
	}

	base = osal_mem_alloc(size + alignment, &type);
	if (base == NULL) {
		HDF_LOGE("%s malloc fail %d", __func__, size);
		return NULL;
	}

	buf = base + mng_size;
	aligned_ptr = (char *)(uintptr_t)(((size_t)(uintptr_t)buf + alignment - 1) & ~(alignment - 1));
	offset = aligned_ptr - buf;
	block = (struct mem_block *)(base + offset);
	block->hdr.type = type;
	block->hdr.offset = offset;

	return aligned_ptr;
}
EXPORT_SYMBOL(OsalMemAllocAlign);

void OsalMemFree(void *buf)
{
	uint32_t type;
	char *base = NULL;
	struct mem_block *block = NULL;

	if (buf == NULL) {
		HDF_LOGE("%s try to free null", __func__);
		return;
	}

	block = (struct mem_block *)((char *)buf - (char *)&((struct mem_block *)0)->mem);
	type = block->hdr.type;
	base = (char *)block - block->hdr.offset;

	if (type == TYPE_KMALLOC)
		kfree(base);
	else if (type == TYPE_VMALLOC)
		vfree(base);
	else
		HDF_LOGE("%s block : type %u fail", __func__, type);
}
EXPORT_SYMBOL(OsalMemFree);

