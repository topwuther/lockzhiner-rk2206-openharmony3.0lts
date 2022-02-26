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

    (*regs)->r[0] = context->uc_mcontext.arm_r0;   // 0:r0
    (*regs)->r[1] = context->uc_mcontext.arm_r1;   // 1:r1
    (*regs)->r[2] = context->uc_mcontext.arm_r2;   // 2:r2
    (*regs)->r[3] = context->uc_mcontext.arm_r3;   // 3:r3
    (*regs)->r[4] = context->uc_mcontext.arm_r4;   // 4:r4
    (*regs)->r[5] = context->uc_mcontext.arm_r5;   // 5:r5
    (*regs)->r[6] = context->uc_mcontext.arm_r6;   // 6:r6
    (*regs)->r[7] = context->uc_mcontext.arm_r7;   // 7:r7
    (*regs)->r[8] = context->uc_mcontext.arm_r8;   // 8:r8
    (*regs)->r[9] = context->uc_mcontext.arm_r9;   // 9:r9
    (*regs)->r[10] = context->uc_mcontext.arm_r10; // 10:r10
    (*regs)->r[11] = context->uc_mcontext.arm_fp;  // 11:fp
    (*regs)->r[12] = context->uc_mcontext.arm_ip;  // 12:ip
    (*regs)->r[13] = context->uc_mcontext.arm_sp;  // 13:sp
    (*regs)->r[14] = context->uc_mcontext.arm_lr;  // 14:lr
    (*regs)->r[15] = context->uc_mcontext.arm_pc;  // 15:pc
    return TRUE;
}

void PrintRegs(const DfxRegs *regs, int32_t fd)
{
    if (regs == NULL || fd < 0) {
        return;
    }

    WriteLog(fd, "r0:%08x r1:%08x r2:%08x r3:%08x\n", regs->r[0], regs->r[1], regs->r[2], regs->r[3]);
    WriteLog(fd, "r4:%08x r5:%08x r6:%08x r7:%08x\n", regs->r[4], regs->r[5], regs->r[6], regs->r[7]);
    WriteLog(fd, "r8:%08x r9:%08x r10:%08x\n", regs->r[8], regs->r[9], regs->r[10]);
    WriteLog(fd, "fp:%08x ip:%08x sp:%08x lr:%08x pc:%08x \n", regs->r[11], regs->r[12], regs->r[13], regs->r[14],
        regs->r[15]);
    WriteLog(fd, "\n");
}