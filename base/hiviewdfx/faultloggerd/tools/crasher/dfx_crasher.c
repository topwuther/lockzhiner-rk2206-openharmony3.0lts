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

#include "dfx_crasher.h"

#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/prctl.h>

#include <hilog/log_c.h>

#include "dfx_signal_handler.h"

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#define LOG_DOMAIN 0x2D11
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Unwind"
#endif

NOINLINE int TriggerTrapException()
{
    printf("test usage in testfunc3! \n");
    int a = 1;
    int *b = &a;
    b = NULL;
    *b = 1;
    return 0;
}

NOINLINE int TriggerSegmentFaultException()
{
    printf("test TriggerSegmentFaultException \n");
    // for crash test force cast the type
    int *a = (int *)(&RaiseAbort);
    *a = SIGSEGV;
    return 0;
}

NOINLINE int RaiseAbort()
{
    raise(SIGABRT);
    return 0;
}

NOINLINE int RaiseBusError()
{
    raise(SIGBUS);
    return 0;
}

NOINLINE int DumpStackTrace()
{
    raise(35); // 35:SIGDUMP
    return 0;
}

NOINLINE int RaiseFloatingPointException()
{
    raise(SIGFPE);
    return 0;
}

NOINLINE int RaiseIllegalInstructionException()
{
    raise(SIGILL);
    return 0;
}

NOINLINE int RaiseSegmentFaultException()
{
    printf("call RaiseSegmentFaultException \n");
    raise(SIGSEGV);
    return 0;
}

NOINLINE int RaiseTrapException()
{
    raise(SIGTRAP);
    return 0;
}

NOINLINE int MaxStackDepth()
{
    return TestFunc1();
}

NOINLINE int MaxMethodNameTest12345678901234567890123456789012345678901234567890ABC()
{
    printf("call MaxMethodNameTest12345678901234567890123456789012345678901234567890ABC \n");
    raise(SIGSEGV);
    return 0;
}

void PrintUsage()
{
    printf("  usage: crasher CMD\n");
    printf("\n");
    printf("  where CMD support:\n");
    printf("  SIGFPE                raise a SIGFPE\n");
    printf("  SIGILL                raise a SIGILL\n");
    printf("  SIGSEGV               raise a SIGSEGV\n");
    printf("  SIGTRAP               raise a SIGTRAP\n");
    printf("  SIGABRT               raise a SIGABRT\n");
    printf("  SIGBUS                raise a SIGBUS\n");
    printf("  STACKTRACE            raise a SIGDUMP\n");
    printf("  triSIGTRAP            trigger a SIGTRAP\n");
    printf("  triSIGSEGV            trigger a SIGSEGV\n");
    printf("  Loop                  trigger a ForeverLoop\n");
    printf("  MaxStack              trigger SIGSEGV after 64 function call\n");
    printf("  MaxMethod             trigger SIGSEGV after call a function with longer name\n");
    printf("  if you want the command execute in a sub thread\n");
    printf("  add thread Prefix, e.g crasher thread-SIGFPE\n");
    printf("\n");
}

void *DoCrashInThread(void *inputArg)
{
    prctl(PR_SET_NAME, "SubTestThread");
    const char *arg = (const char *)(inputArg);
    return (void *)((uint64_t)(ParseAndDoCrash(arg)));
}

uint64_t DoActionOnSubThread(const char *arg)
{
    pthread_t t;
    pthread_create(&t, NULL, DoCrashInThread, (char *)(arg));
    void *result = NULL;
    pthread_join(t, &result);
    return (uint64_t)(result);
}

uint64_t ParseAndDoCrash(const char *arg)
{
    // Prefix
    if (!strncmp(arg, "thread-", strlen("thread-"))) {
        return DoActionOnSubThread(arg + strlen("thread-"));
    }

    // Action
    if (!strcasecmp(arg, "SIGFPE")) {
        return RaiseFloatingPointException();
    }

    if (!strcasecmp(arg, "SIGILL")) {
        return RaiseIllegalInstructionException();
    }

    if (!strcasecmp(arg, "SIGSEGV")) {
        return RaiseSegmentFaultException();
    }

    if (!strcasecmp(arg, "SIGTRAP")) {
        return RaiseTrapException();
    }

    if (!strcasecmp(arg, "SIGABRT")) {
        return RaiseAbort();
    }

    if (!strcasecmp(arg, "SIGBUS")) {
        return RaiseBusError();
    }

    if (!strcasecmp(arg, "STACKTRACE")) {
        return DumpStackTrace();
    }

    if (!strcasecmp(arg, "triSIGTRAP")) {
        return TriggerTrapException();
    }

    if (!strcasecmp(arg, "triSIGSEGV")) {
        return TriggerSegmentFaultException();
    }

    if (!strcasecmp(arg, "Loop")) {
        int i = 0;
        while (1) {
            usleep(10000); // 10000:sleep 0.01 second
            HILOG_INFO(LOG_CORE, "LogTest %{public}d ", i);
            i++;
        }
    }

    if (!strcasecmp(arg, "MaxStack")) {
        return MaxStackDepth();
    }

    if (!strcasecmp(arg, "MaxMethod")) {
        return MaxMethodNameTest12345678901234567890123456789012345678901234567890ABC();
    }
    return 0;
}

NOINLINE int TestFunc70()
{
    raise(SIGSEGV);
    return 0;
}

int main(int argc, char *argv[])
{
    DFX_InstallSignalHandler();
    PrintUsage();
    if (argc <= 1) {
        printf("wrong usage!");
        PrintUsage();
        return 0;
    }

    printf("ParseAndDoCrash done: %" PRIu64 "!", ParseAndDoCrash(argv[1]));
    return 0;
}

// auto gen function
GEN_TEST_FUNCTION(0, 1)
GEN_TEST_FUNCTION(1, 2)
GEN_TEST_FUNCTION(2, 3)
GEN_TEST_FUNCTION(3, 4)
GEN_TEST_FUNCTION(4, 5)
GEN_TEST_FUNCTION(5, 6)
GEN_TEST_FUNCTION(6, 7)
GEN_TEST_FUNCTION(7, 8)
GEN_TEST_FUNCTION(8, 9)
GEN_TEST_FUNCTION(9, 10)

GEN_TEST_FUNCTION(10, 11)
GEN_TEST_FUNCTION(11, 12)
GEN_TEST_FUNCTION(12, 13)
GEN_TEST_FUNCTION(13, 14)
GEN_TEST_FUNCTION(14, 15)
GEN_TEST_FUNCTION(15, 16)
GEN_TEST_FUNCTION(16, 17)
GEN_TEST_FUNCTION(17, 18)
GEN_TEST_FUNCTION(18, 19)
GEN_TEST_FUNCTION(19, 20)

GEN_TEST_FUNCTION(20, 21)
GEN_TEST_FUNCTION(21, 22)
GEN_TEST_FUNCTION(22, 23)
GEN_TEST_FUNCTION(23, 24)
GEN_TEST_FUNCTION(24, 25)
GEN_TEST_FUNCTION(25, 26)
GEN_TEST_FUNCTION(26, 27)
GEN_TEST_FUNCTION(27, 28)
GEN_TEST_FUNCTION(28, 29)
GEN_TEST_FUNCTION(29, 30)

GEN_TEST_FUNCTION(30, 31)
GEN_TEST_FUNCTION(31, 32)
GEN_TEST_FUNCTION(32, 33)
GEN_TEST_FUNCTION(33, 34)
GEN_TEST_FUNCTION(34, 35)
GEN_TEST_FUNCTION(35, 36)
GEN_TEST_FUNCTION(36, 37)
GEN_TEST_FUNCTION(37, 38)
GEN_TEST_FUNCTION(38, 39)
GEN_TEST_FUNCTION(39, 40)

GEN_TEST_FUNCTION(40, 41)
GEN_TEST_FUNCTION(41, 42)
GEN_TEST_FUNCTION(42, 43)
GEN_TEST_FUNCTION(43, 44)
GEN_TEST_FUNCTION(44, 45)
GEN_TEST_FUNCTION(45, 46)
GEN_TEST_FUNCTION(46, 47)
GEN_TEST_FUNCTION(47, 48)
GEN_TEST_FUNCTION(48, 49)
GEN_TEST_FUNCTION(49, 50)

GEN_TEST_FUNCTION(50, 51)
GEN_TEST_FUNCTION(51, 52)
GEN_TEST_FUNCTION(52, 53)
GEN_TEST_FUNCTION(53, 54)
GEN_TEST_FUNCTION(54, 55)
GEN_TEST_FUNCTION(55, 56)
GEN_TEST_FUNCTION(56, 57)
GEN_TEST_FUNCTION(57, 58)
GEN_TEST_FUNCTION(58, 59)
GEN_TEST_FUNCTION(59, 60)

GEN_TEST_FUNCTION(60, 61)
GEN_TEST_FUNCTION(61, 62)
GEN_TEST_FUNCTION(62, 63)
GEN_TEST_FUNCTION(63, 64)
GEN_TEST_FUNCTION(64, 65)
GEN_TEST_FUNCTION(65, 66)
GEN_TEST_FUNCTION(66, 67)
GEN_TEST_FUNCTION(67, 68)
GEN_TEST_FUNCTION(68, 69)
GEN_TEST_FUNCTION(69, 70)