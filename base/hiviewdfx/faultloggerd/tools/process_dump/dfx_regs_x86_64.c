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

#include "dfx_regs.h"

#include <stdio.h>
#include <stdlib.h>

#include "dfx_define.h"

BOOL InitRegsFromUcontext(DfxRegs **regs, ucontext_t *context)
{
    if (regs == NULL || context == NULL) {
        return FALSE;
    }

    (*regs) = (DfxRegs *)calloc(1, sizeof(DfxRegs));
    if ((*regs) == NULL) {
        return FALSE;
    }

    (*regs)->r[0] = (uintptr_t)context->uc_mcontext.gregs[0];   // 0:rax
    (*regs)->r[1] = (uintptr_t)context->uc_mcontext.gregs[1];   // 1:rdx
    (*regs)->r[2] = (uintptr_t)context->uc_mcontext.gregs[2];   // 2:rcx
    (*regs)->r[3] = (uintptr_t)context->uc_mcontext.gregs[3];   // 3:rbx
    (*regs)->r[4] = (uintptr_t)context->uc_mcontext.gregs[4];   // 4:rsi
    (*regs)->r[5] = (uintptr_t)context->uc_mcontext.gregs[5];   // 5:rdi
    (*regs)->r[6] = (uintptr_t)context->uc_mcontext.gregs[6];   // 6:rbp
    (*regs)->r[7] = (uintptr_t)context->uc_mcontext.gregs[7];   // 7:rsp
    (*regs)->r[8] = (uintptr_t)context->uc_mcontext.gregs[8];   // 8:r8
    (*regs)->r[9] = (uintptr_t)context->uc_mcontext.gregs[9];   // 9:r9
    (*regs)->r[10] = (uintptr_t)context->uc_mcontext.gregs[10]; // 10:r10
    (*regs)->r[11] = (uintptr_t)context->uc_mcontext.gregs[11]; // 11:r11
    (*regs)->r[12] = (uintptr_t)context->uc_mcontext.gregs[12]; // 12:r12
    (*regs)->r[13] = (uintptr_t)context->uc_mcontext.gregs[13]; // 13:r13
    (*regs)->r[14] = (uintptr_t)context->uc_mcontext.gregs[14]; // 14:r14
    (*regs)->r[15] = (uintptr_t)context->uc_mcontext.gregs[15]; // 15:r15
    (*regs)->r[16] = (uintptr_t)context->uc_mcontext.gregs[16]; // 16:rip
    return TRUE;
}

void PrintRegs(const DfxRegs *regs, int32_t fd)
{
    if (regs == NULL || fd < 0) {
        return;
    }

    dprintf(fd, "  rax:%016lx rdx:%016lx rcx:%016lx rbx:%016lx\n", regs->r[0], regs->r[1], regs->r[2], regs->r[3]);
    dprintf(fd, "  rsi:%016lx rdi:%016lx rbp:%016lx rsp:%016lx\n", regs->r[4], regs->r[5], regs->r[6], regs->r[7]);
    dprintf(fd, "  r8:%016lx r9:%016lx r10:%016lx r11:%016lx\n", regs->r[8], regs->r[9], regs->r[10], regs->r[11]);
    dprintf(fd, "  r12:%016lx r13:%016lx r14:%016lx r15:%016lx rip:%016lx \n", regs->r[12], regs->r[13], regs->r[14],
        regs->r[15], regs->r16);
    dprintf(fd, "\n");
}