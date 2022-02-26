/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CORE_TEST_H
#define AUDIO_CORE_TEST_H

#include "hdf_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t AudioSocTestRegisterDai(void);
int32_t AudioSocTestRegisterPlatform(void);
int32_t AudioTestRegisterCodec(void);
int32_t AudioTestBindDaiLink(void);
int32_t AudioTestSocDeviceRegister(void);
int32_t AudioTestSocRegisterDsp(void);
int32_t AudioTestRegisterAccessory(void);
int32_t AudioTestUpdateRegBits(void);
int32_t AudioTestAiaoUpdateRegBits(void);
int32_t AudioTestKcontrolGetCodec(void);
int32_t AudioTestAddControls(void);
int32_t AudioTestAddControl(void);
int32_t AudioTestDeviceReadReg(void);
int32_t AudioTestAiaoDeviceReadReg(void);
int32_t AudioTestInfoCtrlSw(void);
int32_t AudioTestGetCtrlSw(void);
int32_t AudioTestPutCtrlSw(void);
int32_t AiaoTestGetCtrlSw(void);
int32_t AiaoTestPutCtrlSw(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_CORE_TEST_H */
