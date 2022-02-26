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

#ifndef DAS_ASY_TOKEN_MANAGER
#define DAS_ASY_TOKEN_MANAGER

#include "common_defs.h"
#include "pake_base_cur_task.h"

typedef struct TokenManagerT {
    int (*registerLocalIdentity)(const char *, const char *, Uint8Buff *, int);
    int (*unregisterLocalIdentity)(const char *, const char *, Uint8Buff *, int);
    int (*deletePeerAuthInfo)(const char *, const char *, Uint8Buff *, int);
    int (*computeAndSavePsk)(const PakeParams *params);
} TokenManager;

const TokenManager *GetAsyTokenManagerInstance(void);

#endif
