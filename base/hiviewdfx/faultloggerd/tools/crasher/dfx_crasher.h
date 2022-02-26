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

#ifndef DFX_CRASHER_H
#define DFX_CRASHER_H

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#define NOINLINE __attribute__((noinline))

#define GEN_TEST_FUNCTION(FuncNumA, FuncNumB)          \
    __attribute__((noinline)) int TestFunc##FuncNumA() \
    {                                                  \
        return TestFunc##FuncNumB();                   \
    }


int RaiseAbort();
int RaiseBusError();
int RaiseFloatingPointException();
int RaiseIllegalInstructionException();
int RaiseSegmentFaultException();
int RaiseTrapException();
int DumpStackTrace();

void PrintUsage();
void *DoCrashInThread(void *inputArg);

uint64_t DoActionOnSubThread(const char *arg);
uint64_t ParseAndDoCrash(const char *arg);
int MaxStackDepth();

//           1         2         3         4         5         6         7
//  1234567890123456789012345678901234567890123456789012345678901234567890
int MaxMethodNameTest12345678901234567890123456789012345678901234567890ABC();

int TriggerSegmentFaultException();
int TriggerTrapException();

// test functions for callstack depth test
int TestFunc0();
int TestFunc1();
int TestFunc2();
int TestFunc3();
int TestFunc4();
int TestFunc5();
int TestFunc6();
int TestFunc7();
int TestFunc8();
int TestFunc9();
int TestFunc10();
int TestFunc11();
int TestFunc12();
int TestFunc13();
int TestFunc14();
int TestFunc15();
int TestFunc16();
int TestFunc17();
int TestFunc18();
int TestFunc19();
int TestFunc20();
int TestFunc21();
int TestFunc22();
int TestFunc23();
int TestFunc24();
int TestFunc25();
int TestFunc26();
int TestFunc27();
int TestFunc28();
int TestFunc29();
int TestFunc30();
int TestFunc31();
int TestFunc32();
int TestFunc33();
int TestFunc34();
int TestFunc35();
int TestFunc36();
int TestFunc37();
int TestFunc38();
int TestFunc39();
int TestFunc40();
int TestFunc41();
int TestFunc42();
int TestFunc43();
int TestFunc44();
int TestFunc45();
int TestFunc46();
int TestFunc47();
int TestFunc48();
int TestFunc49();
int TestFunc50();
int TestFunc51();
int TestFunc52();
int TestFunc53();
int TestFunc54();
int TestFunc55();
int TestFunc56();
int TestFunc57();
int TestFunc58();
int TestFunc59();
int TestFunc60();
int TestFunc61();
int TestFunc62();
int TestFunc63();
int TestFunc64();
int TestFunc65();
int TestFunc66();
int TestFunc67();
int TestFunc68();
int TestFunc69();
int TestFunc70();

#endif // DFX_CRASHER_H