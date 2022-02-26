/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef MMC_BLOCK_H
#define MMC_BLOCK_H

#include "mmc_corex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t MmcBlockAdd(struct MmcDevice *mmc);
void MmcBlockDel(struct MmcDevice *mmc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* MMC_BLOCK_H */
