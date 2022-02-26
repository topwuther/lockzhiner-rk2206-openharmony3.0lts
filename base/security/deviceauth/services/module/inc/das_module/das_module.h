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

#ifndef DAS_MODULE_H
#define DAS_MODULE_H

#include "common_defs.h"
#include "common_util.h"

bool IsDasSupported(void);

typedef struct DasAuthModuleT {
    AuthModuleBase moduleBase;
    int (*registerLocalIdentity)(const char *, const char *, Uint8Buff *, int);
    int (*unregisterLocalIdentity)(const char *, const char *, Uint8Buff *, int);
    int (*deletePeerAuthInfo)(const char *, const char *, Uint8Buff *, int);
} DasAuthModule;

AuthModuleBase *CreateDasModule(void);

#endif
