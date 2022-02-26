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
#ifndef DFX_FAULTLOGGERD_CLIENT_H
#define DFX_FAULTLOGGERD_CLIENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
enum FaultLoggerType {
    JAVA_CRASH = 1,
    CPP_CRASH,
    JS_CRASH,
    APP_FREEZE,
    JAVA_STACKTRACE = 100, // unsupported yet
    CPP_STACKTRACE,
};

struct FaultLoggerdRequest {
    int32_t type;
    int32_t pid;
    int32_t tid;
    int32_t uid;
    char module[128];
} __attribute__((packed));

int32_t RequestFileDescriptor(int32_t type);
int32_t RequestFileDescriptorEx(const struct FaultLoggerdRequest *request);
int32_t LogThreadStacktraceToFile(const char *path, int32_t type, int32_t pid, int32_t tid, int32_t timeout);

#ifdef __cplusplus
}
#endif

#endif