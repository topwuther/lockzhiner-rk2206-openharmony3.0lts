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

#include "pake_server_task.h"
#include "das_common.h"
#include "hc_log.h"
#include "hc_types.h"
#include "pake_protocol_task_common.h"
#include "pake_server_protocol_task.h"
#include "pake_task_common.h"
#include "standard_server_bind_exchange_task.h"
#include "standard_server_unbind_exchange_task.h"

static int GetPakeServerTaskType(const struct SubTaskBaseT *task)
{
    PakeServerTask *realTask = (PakeServerTask *)task;
    if (realTask->curTask == NULL) {
        LOGE("cur Task is null");
        return TASK_TYPE_NONE;
    }
    return realTask->curTask->getCurTaskType();
}

static void DestroyPakeServerTask(struct SubTaskBaseT *task)
{
    PakeServerTask *innerTask = (PakeServerTask *)task;
    if (innerTask == NULL) {
        return;
    }

    DestroyDasPakeParams(&(innerTask->params));
    if (innerTask->curTask != NULL) {
        innerTask->curTask->destroyTask(innerTask->curTask);
    }
    HcFree(innerTask);
}

static int CreateAndProcessNextBindTask(PakeServerTask *realTask, const CJson *in, CJson *out, int *status)
{
    realTask->curTask->destroyTask(realTask->curTask);
    realTask->curTask = CreateStandardBindExchangeServerTask();
    if (realTask->curTask == NULL) {
        LOGE("CreateStandardBindExchangeServerTask failed");
        return HC_ERROR;
    }
    int res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("Process StandardBindExchangeServerTask failed.");
    }
    return res;
}

static int CreateAndProcessNextUnbindTask(PakeServerTask *realTask, const CJson *in, CJson *out, int *status)
{
    realTask->curTask->destroyTask(realTask->curTask);
    realTask->curTask = CreateStandardUnbindExchangeServerTask();
    if (realTask->curTask == NULL) {
        LOGE("CreateBindExchangeTask failed");
        return HC_ERROR;
    }
    int res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("process StandardUnbindExchangeServerTask failed.");
    }
    return res;
}

static int CreateNextTask(PakeServerTask *realTask, const CJson *in, CJson *out, int *status)
{
    int res = HC_SUCCESS;
    switch (realTask->params.opCode) {
        case OP_BIND:
            if (realTask->curTask->getCurTaskType() == TASK_TYPE_BIND_STANDARD_EXCHANGE) {
                break;
            }
            res = CreateAndProcessNextBindTask(realTask, in, out, status);
            break;
        case OP_UNBIND:
            if (realTask->curTask->getCurTaskType() == TASK_TYPE_UNBIND_STANDARD_EXCHANGE) {
                break;
            }
            res = CreateAndProcessNextUnbindTask(realTask, in, out, status);
            break;
        case AUTH_KEY_AGREEMENT:
        case AUTHENTICATE:
            break;
        default:
            LOGE("error opcode: %d", realTask->params.opCode);
            res = HC_ERR_INVALID_PARAMS;
    }
    if (res != HC_SUCCESS) {
        SendErrorToOut(out, realTask->params.opCode, res);
        return res;
    }
    if (*status != FINISH) {
        return res;
    }
    res = SendResultToSelf(&realTask->params, out);
    if (res != HC_SUCCESS) {
        LOGE("SendResultToSelf failed, res: %d", res);
        SendErrorToOut(out, realTask->params.opCode, res);
        return res;
    }
    LOGD("End server task successfully.");
    return res;
}

static int Process(struct SubTaskBaseT *task, const CJson *in, CJson *out, int *status)
{
    if (task == NULL || in == NULL || out == NULL || status == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    PakeServerTask *realTask = (PakeServerTask *)task;
    if (realTask->curTask == NULL) {
        LOGE("cur Task is null");
        return HC_ERROR;
    }

    realTask->params.baseParams.supportedPakeAlg = GetSupportedPakeAlg(&(realTask->taskBase.curVersion));
    int res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
    if (*status != FINISH) {
        return res;
    }
    if (res != HC_SUCCESS) {
        LOGE("process failed, res:%d", res);
        return res;
    }
    return CreateNextTask(realTask, in, out, status);
}

SubTaskBase *CreatePakeServerTask(const CJson *in, CJson *out)
{
    PakeServerTask *task = (PakeServerTask *)HcMalloc(sizeof(PakeServerTask), 0);
    if (task == NULL) {
        return NULL;
    }

    task->taskBase.getTaskType = GetPakeServerTaskType;
    task->taskBase.destroyTask = DestroyPakeServerTask;
    task->taskBase.process = Process;

    int res = InitDasPakeParams(&(task->params), in);
    if (res != HC_SUCCESS) {
        LOGE("Init das pake params failed, res: %d.", res);
        SendErrorToOut(out, task->params.opCode, HC_ERR_ALLOC_MEMORY);
        DestroyPakeServerTask((struct SubTaskBaseT *)task);
        return NULL;
    }
    task->curTask = CreatePakeProtocolServerTask();
    if (task->curTask == NULL) {
        LOGE("Create pake protocol server task failed, res: %d.", res);
        SendErrorToOut(out, task->params.opCode, HC_ERR_ALLOC_MEMORY);
        DestroyPakeServerTask((struct SubTaskBaseT *)task);
        return NULL;
    }
    return (SubTaskBase *)task;
}
