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
#include "dfx_log.h"

BOOL InitRegsFromUcontext(DfxRegs **regs, ucontext_t *context)
{
    if (regs == NULL || context == NULL) {
        return FALSE;
    }

    (*regs) = (DfxRegs *)calloc(1, sizeof(DfxRegs));
    if ((*regs) == NULL) {
        return FALSE;
    }

    (*regs)->r[0] = context->uc_mcontext.regs[0];   // 0:x0
    (*regs)->r[1] = context->uc_mcontext.regs[1];   // 1:x1
    (*regs)->r[2] = context->uc_mcontext.regs[2];   // 2:x2
    (*regs)->r[3] = context->uc_mcontext.regs[3];   // 3:x3
    (*regs)->r[4] = context->uc_mcontext.regs[4];   // 4:x4
    (*regs)->r[5] = context->uc_mcontext.regs[5];   // 5:x5
    (*regs)->r[6] = context->uc_mcontext.regs[6];   // 6:x6
    (*regs)->r[7] = context->uc_mcontext.regs[7];   // 7:x7
    (*regs)->r[8] = context->uc_mcontext.regs[8];   // 8:x8
    (*regs)->r[9] = context->uc_mcontext.regs[9];   // 9:x9
    (*regs)->r[10] = context->uc_mcontext.regs[10]; // 10:x10
    (*regs)->r[11] = context->uc_mcontext.regs[11]; // 11:x11
    (*regs)->r[12] = context->uc_mcontext.regs[12]; // 12:x12
    (*regs)->r[13] = context->uc_mcontext.regs[13]; // 13:x13
    (*regs)->r[14] = context->uc_mcontext.regs[14]; // 14:x14
    (*regs)->r[15] = context->uc_mcontext.regs[15]; // 15:x15
    (*regs)->r[16] = context->uc_mcontext.regs[16]; // 16:x16
    (*regs)->r[17] = context->uc_mcontext.regs[17]; // 17:x17
    (*regs)->r[18] = context->uc_mcontext.regs[18]; // 18:x18
    (*regs)->r[19] = context->uc_mcontext.regs[19]; // 19:x19
    (*regs)->r[20] = context->uc_mcontext.regs[20]; // 20:x20
    (*regs)->r[21] = context->uc_mcontext.regs[21]; // 21:x21
    (*regs)->r[22] = context->uc_mcontext.regs[22]; // 22:x22
    (*regs)->r[23] = context->uc_mcontext.regs[23]; // 23:x23
    (*regs)->r[24] = context->uc_mcontext.regs[24]; // 24:x24
    (*regs)->r[25] = context->uc_mcontext.regs[25]; // 25:x25
    (*regs)->r[26] = context->uc_mcontext.regs[26]; // 26:x26
    (*regs)->r[27] = context->uc_mcontext.regs[27]; // 27:x27
    (*regs)->r[28] = context->uc_mcontext.regs[28]; // 28:x28
    (*regs)->r[29] = context->uc_mcontext.regs[29]; // 29:x29
    (*regs)->r[30] = context->uc_mcontext.regs[30]; // 30:lr
    (*regs)->r[31] = context->uc_mcontext.sp;       // 31:sp
    (*regs)->r[32] = context->uc_mcontext.pc;       // 32:pc
    return TRUE;
}

void PrintRegs(const DfxRegs *regs, int32_t fd)
{
    if (regs == NULL || fd < 0) {
        return;
    }

    WriteLog(fd, "x0:%016lx x1:%016lx x2:%016lx x3:%016lx\n", regs->r[0], regs->r[1], regs->r[2], regs->r[3]);
    WriteLog(fd, "x4:%016lx x5:%016lx x6:%016lx x7:%016lx\n", regs->r[4], regs->r[5], regs->r[6], regs->r[7]);
    WriteLog(fd, "x8:%016lx x9:%016lx x10:%016lx x11:%016lx\n", regs->r[8], regs->r[9], regs->r[10], regs->r[11]);
    WriteLog(fd, "x12:%016lx x13:%016lx x14:%016lx x15:%016lx\n", regs->r[12], regs->r[13], regs->r[14], regs->r[15]);
    WriteLog(fd, "x16:%016lx x17:%016lx x18:%016lx x19:%016lx\n", regs->r[16], regs->r[17], regs->r[18], regs->r[19]);
    WriteLog(fd, "x20:%016lx x21:%016lx x22:%016lx x23:%016lx\n", regs->r[20], regs->r[21], regs->r[22], regs->r[23]);
    WriteLog(fd, "x24:%016lx x25:%016lx x26:%016lx x27:%016lx\n", regs->r[24], regs->r[25], regs->r[26], regs->r[27]);
    WriteLog(fd, "x28:%016lx x29:%016lx\n", regs->r[28], regs->r[29]);
    WriteLog(fd, "lr:%016lx sp:%016lx pc:%016lx\n", regs->r[30], regs->r[31], regs->r[32]);
    WriteLog(fd, "\n");
}