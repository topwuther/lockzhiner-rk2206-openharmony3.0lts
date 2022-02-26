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
#include "dfx_signal.h"

#include <signal.h>
#include <stdio.h>

#include "dfx_log.h"

void PrintSignal(const siginfo_t *info, int32_t fd)
{
    WriteLog(fd, "Signal:%s(%s)", FormatSignalName(info->si_signo), FormatCodeName(info->si_signo, info->si_code));

    if (IsSignalAddrAvaliable(info->si_signo)) {
#if defined(__aarch64__)
        dprintf(fd, "@0x%016lx ", (uint64_t)info->si_addr);
#elif defined(__arm__)
        dprintf(fd, "@0x%08x ", (uint32_t)info->si_addr);
#elif defined(__x86_64__)
        dprintf(fd, "@0x%016lx ", (uint64_t)info->si_addr);
#else
#pragma message("Unsupport arch.")
#endif
    }

    if ((info->si_code <= 0) && (info->si_pid != 0)) {
        dprintf(fd, "from:%d:%d", info->si_pid, info->si_uid);
    }

    dprintf(fd, "\n");
}

int32_t IsSignalInfoAvaliable(int32_t signal)
{
    struct sigaction previousAction;
    if (sigaction(signal, NULL, &previousAction) < 0) {
        return 0;
    }
    return previousAction.sa_flags & SA_SIGINFO;
}

int32_t IsSignalAddrAvaliable(int32_t signal)
{
    switch (signal) {
        case SIGABRT:
        case SIGBUS:
        case SIGILL:
        case SIGSEGV:
        case SIGTRAP:
            return 1;
        default:
            return 0;
    }
}

int32_t IsSignalPidAvaliable(int32_t sigCode)
{
    switch (sigCode) {
        case SI_USER:
        case SI_QUEUE:
        case SI_TIMER:
        case SI_ASYNCIO:
        case SI_MESGQ:
            return 1;
        default:
            return 0;
    }
}

const char *FormatSignalName(int32_t signal)
{
    switch (signal) {
        case SIGABRT:
            return "SIGABRT";
        case SIGALRM:
            return "SIGALRM";
        case SIGBUS:
            return "SIGBUS";
        case SIGFPE:
            return "SIGFPE";
        case SIGILL:
            return "SIGILL";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGSYS:
            return "SIGSYS";
        case SIGTRAP:
            return "SIGTRAP";
        default:
            return "Uncare Signal";
    }
}

const char *FormatSIGBUSCodeName(int32_t signalCode)
{
    switch (signalCode) {
        case BUS_ADRALN:
            return "BUS_ADRALN";
        case BUS_ADRERR:
            return "BUS_ADRERR";
        case BUS_OBJERR:
            return "BUS_OBJERR";
        case BUS_MCEERR_AR:
            return "BUS_MCEERR_AR";
        case BUS_MCEERR_AO:
            return "BUS_MCEERR_AO";
        default:
            return FormatCommonSignalCodeName(signalCode);
    }
}

const char *FormatSIGILLCodeName(int32_t signalCode)
{
    switch (signalCode) {
        case ILL_ILLOPC:
            return "ILL_ILLOPC";
        case ILL_ILLOPN:
            return "ILL_ILLOPN";
        case ILL_ILLADR:
            return "ILL_ILLADR";
        case ILL_ILLTRP:
            return "ILL_ILLTRP";
        case ILL_PRVOPC:
            return "ILL_PRVOPC";
        case ILL_PRVREG:
            return "ILL_PRVREG";
        case ILL_COPROC:
            return "ILL_COPROC";
        case ILL_BADSTK:
            return "ILL_BADSTK";
        default:
            return FormatCommonSignalCodeName(signalCode);
    }
}

const char *FormatSIGFPECodeName(int32_t signalCode)
{
    switch (signalCode) {
        case FPE_INTDIV:
            return "FPE_INTDIV";
        case FPE_INTOVF:
            return "FPE_INTOVF";
        case FPE_FLTDIV:
            return "FPE_FLTDIV";
        case FPE_FLTOVF:
            return "FPE_FLTOVF";
        case FPE_FLTUND:
            return "FPE_FLTUND";
        case FPE_FLTRES:
            return "FPE_FLTRES";
        case FPE_FLTINV:
            return "FPE_FLTINV";
        case FPE_FLTSUB:
            return "FPE_FLTSUB";
        default:
            return FormatCommonSignalCodeName(signalCode);
    }
}

const char *FormatSIGSEGVCodeName(int32_t signalCode)
{
    switch (signalCode) {
        case SEGV_MAPERR:
            return "SEGV_MAPERR";
        case SEGV_ACCERR:
            return "SEGV_ACCERR";
        default:
            return FormatCommonSignalCodeName(signalCode);
    }
}

const char *FormatSIGTRAPCodeName(int32_t signalCode)
{
    switch (signalCode) {
        case TRAP_BRKPT:
            return "TRAP_BRKPT";
        case TRAP_TRACE:
            return "TRAP_TRACE";
        case TRAP_BRANCH:
            return "TRAP_BRANCH";
        case TRAP_HWBKPT:
            return "TRAP_HWBKPT";
        default:
            return FormatCommonSignalCodeName(signalCode);
    }
}

const char *FormatCommonSignalCodeName(int32_t signalCode)
{
    switch (signalCode) {
        case SI_USER:
            return "SI_USER";
        case SI_KERNEL:
            return "SI_KERNEL";
        case SI_QUEUE:
            return "SI_QUEUE";
        case SI_TIMER:
            return "SI_TIMER";
        case SI_MESGQ:
            return "SI_MESGQ";
        case SI_ASYNCIO:
            return "SI_ASYNCIO";
        case SI_SIGIO:
            return "SI_SIGIO";
        case SI_TKILL:
            return "SI_TKILL";
        default:
            return "UNKNOWN";
    }
}

const char *FormatCodeName(int32_t signal, int32_t signalCode)
{
    switch (signal) {
        case SIGILL:
            return FormatSIGILLCodeName(signalCode);
        case SIGBUS:
            return FormatSIGBUSCodeName(signalCode);
        case SIGFPE:
            return FormatSIGFPECodeName(signalCode);
        case SIGSEGV:
            return FormatSIGSEGVCodeName(signalCode);
        case SIGTRAP:
            return FormatSIGTRAPCodeName(signalCode);
        default:
            break;
    }
    return FormatCommonSignalCodeName(signalCode);
}