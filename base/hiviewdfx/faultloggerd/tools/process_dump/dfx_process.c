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
#include "dfx_process.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#include <securec.h>

#include "dfx_define.h"
#include "dfx_log.h"
#include "dfx_maps.h"
#include "dfx_signal.h"
#include "dfx_thread.h"
#include "dfx_util.h"

void FillProcessName(DfxProcess *process)
{
    if (process == NULL) {
        return;
    }

    char path[NAME_LEN] = "\0";
    if (snprintf_s(path, sizeof(path), sizeof(path) - 1, "/proc/%d/cmdline", process->pid) <= 0) {
        return;
    }

    ReadStringFromFile(path, process->processName, NAME_LEN);
}

BOOL InitProcessWithKeyThread(DfxProcess **process, pid_t pid, DfxThread *keyThread)
{
    *process = calloc(1, sizeof(DfxProcess));
    if ((*process) == NULL) {
        DfxLogWarn("Fail to alloc process.");
        return FALSE;
    }

    (*process)->pid = pid;
    (*process)->processName = calloc(1, NAME_LEN);
    if ((*process)->processName == NULL) {
        DfxLogWarn("Fail to alloc processName.");
        free(*process);
        *process = NULL;
        return FALSE;
    }

    FillProcessName(*process);

    if (!InitProcessMapsNode(*process)) {
        DfxLogWarn("Fail to init process maps.");
        free((*process)->processName);
        free(*process);
        *process = NULL;
        return FALSE;
    }

    if (!InitProcessThreadsNode(*process, keyThread)) {
        DfxLogWarn("Fail to init threads.");
        DestroyProcessMapsNode(*process);
        free((*process)->processName);
        free(*process);
        *process = NULL;
        return FALSE;
    }

    DfxLogWarn("Init process dump with pid:%d.", (*process)->pid);
    return TRUE;
}

BOOL InitProcessMapsNode(DfxProcess *process)
{
    return InitElfMaps(&(process->maps), process->pid);
}

BOOL InitProcessThreadsNode(DfxProcess *process, DfxThread *keyThread)
{
    process->threads = (DfxThreadsNode *)calloc(1, sizeof(DfxThreadsNode));
    if (process->threads == NULL) {
        return FALSE;
    }

    if (keyThread != NULL) {
        process->threads->thread = keyThread;
        return TRUE;
    }

    if (!InitThread(&(process->threads->thread), process->pid, process->pid)) {
        free(process->threads);
        process->threads = NULL;
        return FALSE;
    }

    return TRUE;
}

BOOL InitOtherThreads(DfxProcess *process)
{
    if (process == NULL) {
        return FALSE;
    }

    char path[NAME_LEN] = {0};
    if (snprintf_s(path, sizeof(path), sizeof(path) - 1, "/proc/%d/task", process->pid) <= 0) {
        return FALSE;
    }

    char realPath[PATH_MAX];
    if (realpath(path, realPath) == NULL) {
        return FALSE;
    }

    DIR *dir = opendir(realPath);
    if (dir == NULL) {
        return FALSE;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0) {
            continue;
        }

        if (strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        pid_t tid = atoi(ent->d_name);
        if (tid == 0) {
            continue;
        }

        InsertThreadNode(process, tid);
    }
    closedir(dir);
    return TRUE;
}

void InsertThreadNode(DfxProcess *process, pid_t tid)
{
    if (process == NULL) {
        return;
    }

    DfxThreadsNode *curNode = process->threads;
    while ((curNode->thread != NULL) && (curNode->next != NULL)) {
        if (curNode->thread->tid == tid) {
            return;
        }
        curNode = curNode->next;
    }

    if (curNode->thread == NULL) {
        InitThread(&(curNode->thread), process->pid, tid);
        return;
    } else if (curNode->thread->tid == tid) {
        return;
    }

    curNode->next = (DfxThreadsNode *)calloc(1, sizeof(DfxThreadsNode));
    if (curNode->next != NULL) {
        InitThread(&(curNode->next->thread), process->pid, tid);
    }
}

void DestroyProcess(DfxProcess *process)
{
    DestroyProcessMapsNode(process);
    DestroyProcessThreadsNode(process);
    if (process != NULL) {
        free(process->processName);
        process->processName = NULL;
        free(process);
    }
}

void DestroyProcessMapsNode(DfxProcess *process)
{
    if (process != NULL && process->maps != NULL) {
        DestroyMaps(process->maps);
        process->maps = NULL;
    }
}

void DestroyProcessThreadsNode(DfxProcess *process)
{
    if (process == NULL) {
        return;
    }

    if (process->threads == NULL) {
        return;
    }
    DfxThreadsNode *cur = process->threads;
    while (cur != NULL && cur->next != NULL) {
        DfxThreadsNode *tmp = cur;
        cur = cur->next;
        DestroyThread(tmp->thread);
        free(tmp);
    }

    if ((cur != NULL) && (cur->thread != NULL)) {
        DestroyThread(cur->thread);
        free(cur);
        process->threads = NULL;
    }
}

void PrintProcessWithSiginfo(const DfxProcess *process, const siginfo_t *info, int32_t fd)
{
    if (process == NULL) {
        return;
    }

    WriteLog(fd, "Pid:%d\n", process->pid);
    WriteLog(fd, "Uid:%d\n", process->uid);
    WriteLog(fd, "Process name:%s\n", process->processName);
    if (info != NULL && info->si_signo != 35) { // 35:SIGDUMP
        WriteLog(fd, "Reason:");
        PrintSignal(info, fd);

        if (process->threads != NULL && process->threads->thread != NULL) {
            SkipFramesInSignalHandler(process->threads->thread);
            WriteLog(fd, "Fault thread Info:\n");
        }
    }

    PrintProcess(process, fd);

    DfxElfMapsNode *mapsNode = process->maps;
    if (mapsNode != NULL) {
        dprintf(fd, "Maps:\n");
    }

    while (mapsNode != NULL) {
        PrintMap(mapsNode->map, fd);
        mapsNode = mapsNode->next;
    }
}

void PrintProcess(const DfxProcess *process, int32_t fd)
{
    if (process == NULL) {
        return;
    }

    DfxThreadsNode *curNode = process->threads;
    while (curNode != NULL) {
        PrintThread(curNode->thread, fd);
        curNode = curNode->next;
    }
}