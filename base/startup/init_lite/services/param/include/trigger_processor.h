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

#ifndef BASE_STARTUP_EVENT_MANAGER_H
#define BASE_STARTUP_EVENT_MANAGER_H

#include <stdio.h>

#include "sys_param.h"
#include "init_param.h"
#include "trigger_manager.h"
#include "uv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct TriggerEvent {
    uv_work_t request;
    EventType type;
} TriggerEvent;

typedef struct {
    uv_work_t request;
    EventType type;
    u_int32_t contentSize;
    char content[0];
} TriggerDataEvent;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif