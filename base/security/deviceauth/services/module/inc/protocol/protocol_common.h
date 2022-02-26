/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROTOCOL_COMMON_H
#define PROTOCOL_COMMON_H

#include "common_util.h"

typedef enum {
    UNSUPPORTED_ALG = 0x0000,
    DL_SPEKE = 0x0001,
    EC_SPEKE = 0x0002,
    STS_ALG = 0x0004,
    PSK_SPEKE = 0x0008,
    ISO_ALG = 0x0010,
    NEW_DL_SPEKE = 0x0020,
    NEW_EC_SPEKE = 0x0040,
} AlgType;

typedef enum {
    UNSUPPORTED,
    ISO,
    PAKE,
    NEW_PAKE,
    STS,
} ProtocolType;

void FreeAndCleanKey(Uint8Buff *key);

#endif
