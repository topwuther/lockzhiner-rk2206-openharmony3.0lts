/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "hdf_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STORAGE_MAX_BYTES ((size_t)(-1))

enum StorageType {
    MEDIA_MMC = 0,
    MEDIA_MTD = 1,
    MEDIA_ERR = 2,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* STORAGE_H */
