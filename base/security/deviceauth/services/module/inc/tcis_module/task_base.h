/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCIS_DEVICE_AUTH_TASK_BASE_H
#define TCIS_DEVICE_AUTH_TASK_BASE_H

#include "alg_defs.h"
#include "tcis_module_defines.h"

typedef struct _TaskBase {
    TcisTaskType (*getTaskType)(void);
    void (*destroyTask)(struct _TaskBase *);
    int (*process)(struct _TaskBase *, CJson *in, CJson *out, int *status);
    int taskStatus;
    int taskId;
} TaskBase;

typedef TaskBase *(*CreateTaskFunc)(int *, const CJson *, CJson *, AlgLoader *);

typedef struct {
    int taskTypeId;
    CreateTaskFunc createTask;
} TaskCreateInfo;

#endif
