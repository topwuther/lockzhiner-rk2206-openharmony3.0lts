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

#ifndef STARTUP_TRIGER_MANAGER_H
#define STARTUP_TRIGER_MANAGER_H
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "param_manager.h"
#include "trigger_checker.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define TRIGGER_CMD "trigger "
#define TRIGGER_ARR_NAME_IN_JSON "jobs"
#define CMDS_ARR_NAME_IN_JSON "cmds"
#define MAX_CMD_IN_JOBS 1024

#define TRIGGER_NODE_IN_QUEUE(trigger) \
    (atomic_load_explicit(&(trigger)->serial, memory_order_relaxed) & 0x01)
#define TRIGGER_NODE_SET_QUEUE_FLAG(trigger) \
    atomic_store_explicit(&(trigger)->serial, (trigger)->serial | 0x01, memory_order_relaxed)
#define TRIGGER_NODE_CLEAR_QUEUE_FLAG(trigger) \
    atomic_store_explicit(&(trigger)->serial, (trigger)->serial & ~0x01, memory_order_relaxed)

typedef enum {
    TRIGGER_BOOT = 0,
    TRIGGER_PARAM,
    TRIGGER_UNKNOW,
    TRIGGER_MAX
}TriggerType;

// Command对象列表，主要存储每个triger需要执行那些Command操作。
typedef struct CommandNode {
    atomic_uint_least32_t next;
    char name[MAX_TRIGGER_CMD_NAME_LEN];
    char content[0];
} CommandNode;

typedef struct tagTriggerNode {
    atomic_uint_least32_t serial;
    atomic_uint_least32_t next;
    atomic_uint_least32_t firstCmd;
    atomic_uint_least32_t lastCmd;
    int type;
    char name[MAX_TRIGGER_NAME_LEN];
    char condition[0];
} TriggerNode;

typedef struct {
    atomic_uint_least32_t serial;
    u_int32_t dataSize;
    u_int32_t startSize;
    u_int32_t currOffset;
    char data[0];
} TriggerArea;

typedef struct {
    atomic_uint_least32_t firstTrigger;
    atomic_uint_least32_t lastTrigger;
} TriggerHeader;

typedef struct {
    u_int32_t *executeQueue;
    u_int32_t queueCount;
    u_int32_t startIndex;
    u_int32_t endIndex;
    pthread_mutex_t mutex;
} TriggerExecuteQueue;

typedef struct TriggerWorkSpace {
    TriggerExecuteQueue executeQueue;
    TriggerHeader header[TRIGGER_MAX];
    TriggerArea *area;
} TriggerWorkSpace;

int InitTriggerWorkSpace(TriggerWorkSpace *workSpace);
int ParseTrigger(TriggerWorkSpace *workSpace, const cJSON *triggerItem);

typedef int (*TRIGGER_MATCH)(LogicCalculator *calculator, TriggerNode *trigger, const char *content,
    u_int32_t contentSize);
typedef int (*PARAM_CHECK_DONE)(TriggerNode *trigger, u_int32_t index);
typedef int (*CMD_EXECUTE) (const TriggerNode *trigger, const char *cmdName, const char *command);

TriggerNode *GetTriggerByName(const TriggerWorkSpace *workSpace, const char *triggerName, u_int32_t *triggerIndex);
int ExecuteTrigger(TriggerWorkSpace *workSpace, TriggerNode *trigger, CMD_EXECUTE cmdExecuter);
int CheckTrigger(const TriggerWorkSpace *workSpace,
    int type, void *content, u_int32_t contentSize, PARAM_CHECK_DONE triggerExecuter);
int CheckParamTrigger(TriggerWorkSpace *workSpace,
    const char *content, u_int32_t contentSize, PARAM_CHECK_DONE triggerExecuter);
int CheckAndExecuteTrigger(const TriggerWorkSpace *workSpace, const char *content, PARAM_CHECK_DONE triggerExecuter);

TriggerNode *ExecuteQueuePop(TriggerWorkSpace *workSpace);
int ExecuteQueuePush(TriggerWorkSpace *workSpace, TriggerNode *trigger, u_int32_t index);
int ExecuteQueueSize(const TriggerWorkSpace *workSpace);

u_int32_t AddTrigger(TriggerWorkSpace *workSpace, int type, const char *name, const char *condition);
u_int32_t AddCommand(TriggerWorkSpace *workSpace, TriggerNode *trigger, const char *cmdName, const char *content);

TriggerWorkSpace *GetTriggerWorkSpace();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // STARTUP_TRIGER_MANAGER_H
