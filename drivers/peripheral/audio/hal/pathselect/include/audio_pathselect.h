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

#ifndef AUDIO_PATHSELECT_H
#define AUDIO_PATHSELECT_H

#include "audio_types.h"
#include "audio_common.h"
#include "audio_internal.h"
#include "cJSON.h"

#ifdef __LITEOS__
#define CJSONFILE_CONFIG_PATH "/etc/parse.json"
#else
#define CJSONFILE_CONFIG_PATH "/system/etc/hdfconfig/parse.json"
#endif

int32_t AudioPathSelGetConfToJsonObj();
int32_t AudioPathSelAnalysisJson(void *adapterParam, enum AudioAdaptType adaptType);

#endif

