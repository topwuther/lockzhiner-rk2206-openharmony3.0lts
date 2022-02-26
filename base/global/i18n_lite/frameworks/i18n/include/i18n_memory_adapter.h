/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef I18N_MEMORY_ADAPTER_H
#define I18N_MEMORY_ADAPTER_H

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#ifndef I18N_PRODUCT
// memory operator define
#include <stdlib.h>

#define I18nMalloc(a) malloc(a)
#define I18nFree(a) \
    do { \
        if (a != nullptr) { \
            (void) free((void *) a); \
            a = nullptr; \
        } \
    } while (0)

#else // I18N_PRODUCT
#include "stdint.h"
#include "ohos_mem_pool.h"

#define I18nMalloc(a) OhosMalloc(MEM_TYPE_I18N_LSRAM, a)
#define I18nFree(a) \
    do { \
        if (a != nullptr) { \
            (void) OhosFree((void *) a); \
            a = nullptr; \
        } \
    } while (0)

#endif // I18N_PRODUCT
#endif // I18N_MEMORY_ADAPTER_H