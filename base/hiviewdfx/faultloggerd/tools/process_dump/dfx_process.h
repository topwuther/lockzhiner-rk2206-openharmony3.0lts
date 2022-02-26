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
#ifndef DFX_PROCESS_H
#define DFX_PROCESS_H

#include <inttypes.h>

#include "dfx_define.h"
#include "dfx_elf.h"
#include "dfx_maps.h"
#include "dfx_thread.h"

typedef struct DfxThreadsNode {
    DfxThread *thread;
    struct DfxThreadsNode *next;
} DfxThreadsNode;

typedef struct {
    pid_t pid;
    pid_t uid;
    char *processName;
    DfxElfMapsNode *maps;
    DfxThreadsNode *threads;
} DfxProcess;

BOOL InitProcessWithKeyThread(DfxProcess **process, pid_t pid, DfxThread *keyThread);
BOOL InitProcessMapsNode(DfxProcess *process);
BOOL InitProcessThreadsNode(DfxProcess *process, DfxThread *keyThread);
BOOL InitOtherThreads(DfxProcess *process);
void InsertThreadNode(DfxProcess *process, pid_t tid);
void DestroyProcess(DfxProcess *process);
void DestroyProcessThreadsNode(DfxProcess *process);
void DestroyProcessMapsNode(DfxProcess *process);
void PrintProcess(const DfxProcess *process, int32_t fd);
void PrintProcessWithSiginfo(const DfxProcess *process, const siginfo_t *info, int32_t fd);

#endif