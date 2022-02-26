/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HAL_WRAPPER_H
#define HAL_WRAPPER_H

#include "bluetooth_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    HalInitFunc halInit;
    HalSendHciPacketFunc halSendHciPacket;
    HalCloseFunc halClose;

    void *lib;
} HALLib;

HALLib *LoadHalLib();
void UnloadHALLib(HALLib *lib);

#ifdef __cplusplus
}
#endif

#endif