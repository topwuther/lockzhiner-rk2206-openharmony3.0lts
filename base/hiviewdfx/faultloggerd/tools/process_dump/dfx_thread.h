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
#ifndef DFX_THREAD_H
#define DFX_THREAD_H

#include <sys/types.h>

#include "dfx_define.h"
#include "dfx_frames.h"
#include "dfx_regs.h"

typedef struct {
    pid_t pid;
    pid_t tid;
    char *threadName;
    DfxRegs *regs;
    DfxFramesNode *head;
} DfxThread;

BOOL InitThread(DfxThread **thread, pid_t pid, pid_t tid);
BOOL InitThreadByContext(DfxThread **thread, pid_t pid, pid_t tid, ucontext_t *context);
DfxFrame *GetAvaliableFrame(DfxThread *thread);
void DestroyThread(DfxThread *thread);
void PrintThread(const DfxThread *thread, int32_t fd);
void SkipFramesInSignalHandler(DfxThread *thread);

#endif