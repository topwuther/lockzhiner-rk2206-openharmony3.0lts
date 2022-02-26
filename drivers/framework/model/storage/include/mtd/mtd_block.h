/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef MTD_BLOCK_H
#define MTD_BLOCK_H

#include "mtd_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t MtdBlockInit(struct MtdDevice *mtd);
void MtdBlockUninit(struct MtdDevice *mtd);

/* these too functions gona implemented by specific os */
extern int32_t MtdBlockOsInit(struct MtdDevice *mtd);
extern void MtdBlockOsUninit(struct MtdDevice *mtd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* MTD_BLOCK_H */
