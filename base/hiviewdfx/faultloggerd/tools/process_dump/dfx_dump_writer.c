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

#include "dfx_dump_writer.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <faultloggerd_client.h>
#include <securec.h>

#include "dfx_log.h"
#include "dfx_process.h"

void WriteProcessDump(DfxProcess *process, struct ProcessDumpRequest *request, int32_t fromSignalHandler)
{
    if (process == NULL || request == NULL) {
        return;
    }

    if (fromSignalHandler == 0) {
        PrintProcess(process, STDOUT_FILENO);
    } else {
        struct FaultLoggerdRequest faultloggerdRequest;
        memset_s(&faultloggerdRequest, sizeof(faultloggerdRequest), 0, sizeof(struct FaultLoggerdRequest));
        faultloggerdRequest.type = (request->siginfo.si_signo == SIGDUMP) ? CPP_STACKTRACE : CPP_CRASH;
        faultloggerdRequest.pid = request->pid;
        faultloggerdRequest.tid = request->tid;
        faultloggerdRequest.uid = request->uid;
        if (strncpy_s(faultloggerdRequest.module, sizeof(faultloggerdRequest.module), process->processName,
            strlen(process->processName)) != 0) {
            DfxLogWarn("Failed to set process name.");
            return;
        }

        int32_t targetFd = RequestFileDescriptorEx(&faultloggerdRequest);
        if (targetFd < 0) {
            DfxLogWarn("Failed to request fd from faultloggerd.");
            return;
        }
        PrintProcessWithSiginfo(process, &(request->siginfo), targetFd);
        close(targetFd);
    }
}
