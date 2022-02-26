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

#include "process_dump.h"

#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include "dfx_dump_writer.h"
#include "dfx_log.h"
#include "dfx_process.h"
#include "dfx_thread.h"
#include "dfx_unwind_remote.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

static void PrintCommandHelp()
{
    printf("usage:\n");
    printf("-t tid    dump the stacktrace of the thread with given tid.\n");
}

static BOOL ParseCommandOptions(int argc, char *argv[], struct ProcessDumpRequest *request)
{
    if (argc != 3) { // 3: currently supported argv size
        PrintCommandHelp();
        return FALSE;
    }

    if (strcmp("-t", argv[1]) == 0) {
        request->type = DUMP_TYPE_THREAD;
        request->tid = atoi(argv[2]); // 2 : second parameter
    } else if (strcmp("-p", argv[1]) == 0) {
        request->type = DUMP_TYPE_PROCESS;
        request->pid = atoi(argv[2]); // 2 : second parameter
    } else {
        PrintCommandHelp();
        return FALSE;
    }

    return TRUE;
}

static BOOL IsCallFromSignalHandler(int argc, char *argv[])
{
    if (argc != 1) { // 1 args processdump
        return FALSE;
    }

    if (strcmp("-signalhandler", argv[0]) != 0) {
        return FALSE;
    }
    return TRUE;
}

static void DumpProcessWithSignalContext(DfxProcess **process, struct ProcessDumpRequest *request)
{
    ssize_t readCount = read(STDIN_FILENO, request, sizeof(struct ProcessDumpRequest));
    if (readCount != sizeof(struct ProcessDumpRequest)) {
        return;
    }

    DfxThread *keyThread = NULL;
    if (!InitThreadByContext(&keyThread, request->pid, request->tid, &(request->context))) {
        DfxLogError("Fail to init key thread.");
        DestroyThread(keyThread);
        keyThread = NULL;
        return;
    }

    if (!InitProcessWithKeyThread(process, request->pid, keyThread)) {
        DfxLogError("Fail to init process with key thread.");
        DestroyThread(keyThread);
        keyThread = NULL;
        return;
    }

    if (request->siginfo.si_signo == SIGDUMP) {
        InitOtherThreads(*process);
    }

    UnwindProcess(*process);
}

static void DumpProcess(DfxProcess **process, struct ProcessDumpRequest *request)
{
    if (!InitProcessWithKeyThread(process, request->tid, NULL)) {
        DfxLogError("Fail to init key thread.");
        return;
    }

    InitOtherThreads(*process);
    UnwindProcess(*process);
}

void StartDumpProcess(int argc, char *argv[])
{
    struct ProcessDumpRequest *request = (struct ProcessDumpRequest *)calloc(1, sizeof(struct ProcessDumpRequest));
    if (request == NULL) {
        DfxLogError("Fail to create dump request.");
        return;
    }

    DfxProcess *process = NULL;
    int32_t fromSignalHandler = 0;
    if (IsCallFromSignalHandler(argc, argv)) { // read args from STDIN
        DumpProcessWithSignalContext(&process, request);
        fromSignalHandler = 1;
    } else if (ParseCommandOptions(argc, argv, request)) { // read args from shell
        DumpProcess(&process, request);
    } else {
        DfxLogError("Fail to parse dump request.");
        DestroyProcess(process);
        free(request);
        request = NULL;
        return;
    }

    WriteProcessDump(process, request, fromSignalHandler);
    DestroyProcess(process);
    free(request);
    request = NULL;
}