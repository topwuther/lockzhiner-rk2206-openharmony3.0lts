/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIO_ADAPTER_INFO_COMMON_H
#define AUDIO_ADAPTER_INFO_COMMON_H

#include <stdio.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "securec.h"
#include "audio_types.h"
#include "audio_internal.h"

struct AudioAdapterDescriptor *AudioAdapterGetConfigOut(void);
struct AudioAdapterDescriptor *AudioAdapterGetConfigDescs(void);
int32_t AudioAdapterGetAdapterNum(void);
int32_t AudioAdaptersForUser(struct AudioAdapterDescriptor **descs, int *size);
int32_t AudioAdapterExist(const char *adapterName);
int32_t HdmiPortInit(struct AudioPort portIndex, struct AudioPortCapability *capabilityIndex);
int32_t KeyValueListToMap(const char *keyValueList, struct ParamValMap mParamValMap[], int32_t *count);
int32_t AddElementToList(char *keyValueList, int32_t listLenth, const char *key, void *value);
int32_t GetErrorReason(int reason, char* reasonDesc);
int32_t GetCurrentTime(char *time);
int32_t CheckAttrRoute(long param);
int32_t CheckAttrChannel(unsigned long param);
int32_t TransferRoute(char *value, int32_t *route);
int32_t TransferFormat(char *value, int32_t *format);
int32_t TransferChannels(char *value, uint32_t *channels);
int32_t TransferFrames(char *value, uint64_t *frames);
int32_t TransferSampleRate(char *value, uint32_t *sampleRate);
int32_t FormatToBits(enum AudioFormat format, uint32_t *formatBits);
int32_t BitsToFormat(enum AudioFormat *format, long formatBits);
int32_t SetExtParam(const char *key, char *value, struct ExtraParams *mExtraParams);
#endif
