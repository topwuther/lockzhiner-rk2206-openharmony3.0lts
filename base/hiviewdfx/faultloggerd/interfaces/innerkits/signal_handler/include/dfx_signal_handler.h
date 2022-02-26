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
#ifndef DFX_SIGNAL_HANDLER_H
#define DFX_SIGNAL_HANDLER_H
#include <signal.h>
#include <ucontext.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifndef DFX_LOCAL_UNWIND
struct ProcessDumpRequest {
    int32_t type;
    int32_t tid;
    int32_t pid;
    int32_t uid;
    uint64_t reserved;
    uint64_t timeStamp;
    siginfo_t siginfo;
    ucontext_t context;
};
#endif
void DFX_InstallSignalHandler();
#ifdef __cplusplus
}
#endif
#endif