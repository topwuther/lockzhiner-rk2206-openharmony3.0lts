/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_SAPM_TEST_H
#define AUDIO_SAPM_TEST_H

#include "hdf_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
int32_t AudioSapmTestNewComponents(void);
int32_t AudioSapmTestAddRoutes(void);
int32_t AudioSapmTestNewControls(void);
int32_t AudioSapmTestPowerComponents(void);
int32_t AudioSapmTestRefreshTime(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
