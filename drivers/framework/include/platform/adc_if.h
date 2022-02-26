/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef ADC_IF_H
#define ADC_IF_H

#include "hdf_platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct AdcIoMsg {
    uint32_t number;
    uint32_t channel;
};

DevHandle AdcOpen(uint32_t num);

void AdcClose(DevHandle handle);

int32_t AdcRead(DevHandle handle, uint32_t channel, uint32_t *val);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* ADC_IF_H */
