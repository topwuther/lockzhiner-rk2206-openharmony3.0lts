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

#ifndef PAKE_BASE_CUR_TASK_H
#define PAKE_BASE_CUR_TASK_H

#include <stdint.h>
#include "json_utils.h"
#include "das_module_defines.h"
#include "pake_defs.h"

#define PAKE_KEY_ALIAS_LEN 64
#define PAKE_ED25519_KEY_PAIR_LEN 32

typedef struct PakeParamsT {
    PakeBaseParams baseParams;

    Uint8Buff returnKey;
    Uint8Buff nonce;
    int opCode;
    int32_t userType;
    int32_t userTypePeer;
    char *packageName;
    char *serviceType;
} PakeParams;

typedef struct AsyBaseCurTaskT {
    CurTaskType (*getCurTaskType)(void);
    void (*destroyTask)(struct AsyBaseCurTaskT *);
    int(*process)(struct AsyBaseCurTaskT *, PakeParams *params, const CJson *in, CJson *out, int *status);
    int taskStatus;
} AsyBaseCurTask;

#endif
