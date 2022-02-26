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
#ifndef DFX_SIGNAL_H
#define DFX_SIGNAL_H

#include <signal.h>
#include <stdint.h>

void PrintSignal(const siginfo_t *info, int32_t fd);

int32_t IsSignalInfoAvaliable(int32_t signal);
int32_t IsSignalAddrAvaliable(int32_t signal);
int32_t IsSignalPidAvaliable(int32_t sigCode);
const char *FormatSignalName(int32_t signal);
const char *FormatCodeName(int32_t signal, int32_t signalCode);

const char *FormatSIGBUSCodeName(int32_t signalCode);
const char *FormatSIGILLCodeName(int32_t signalCode);
const char *FormatSIGFPECodeName(int32_t signalCode);
const char *FormatSIGSEGVCodeName(int32_t signalCode);
const char *FormatSIGTRAPCodeName(int32_t signalCode);
const char *FormatCommonSignalCodeName(int32_t signalCode);

#endif