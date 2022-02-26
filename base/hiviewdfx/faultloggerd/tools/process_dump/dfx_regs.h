/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DFX_REGS_H
#define DFX_REGS_H

#include <inttypes.h>
#include <ucontext.h>

#include <sys/types.h>

#include "dfx_define.h"

#if defined(__arm__)
#define USER_REG_NUM 16
#define REG_PC_NUM 15
#elif defined(__aarch64__)
#define USER_REG_NUM 34
#define REG_PC_NUM 32
#elif defined(__x86_64__)
#define USER_REG_NUM 27
#define REG_PC_NUM 16
#endif

typedef struct {
    uintptr_t r[USER_REG_NUM];
} DfxRegs;

BOOL InitRegsFromUcontext(DfxRegs **regs, ucontext_t *context);
void DestroyRegs(DfxRegs *regs);
void PrintRegs(const DfxRegs *regs, int32_t fd);

#endif