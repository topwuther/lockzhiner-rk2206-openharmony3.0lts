/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef STORAGE_CHAR_H
#define STORAGE_CHAR_H

#include "mtd_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CHAR_NAME_LEN 32

int32_t MtdCharInit(struct MtdDevice *mtdDevice);
void MtdCharUninit(struct MtdDevice *mtdDevice);

/* these too functions gona implemented by specific os */
extern int32_t MtdCharOsInit(struct MtdDevice *mtdDevice);
extern void MtdCharOsUninit(struct MtdDevice *mtdDevice);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* STORAGE_CHAR_H */
