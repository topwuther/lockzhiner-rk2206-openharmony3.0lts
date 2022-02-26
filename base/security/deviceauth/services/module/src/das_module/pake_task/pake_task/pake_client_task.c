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

#include "pake_client_task.h"
#include "das_common.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "pake_client_protocol_task.h"
#include "pake_protocol_task_common.h"
#include "pake_task_common.h"
#include "standard_client_bind_exchange_task.h"
#include "standard_client_unbind_exchange_task.h"

static int GetPakeClientTaskType(const struct SubTaskBaseT *task)
{
    PakeClientTask *realTask = (PakeClientTask *)task;
    if (realTask->curTask == NULL) {
        LOGE("cur Task is null");
        return TASK_TYPE_NONE;
    }
    return realTask->curTask->getCurTaskType();
}

static void DestroyPakeClientTask(struct SubTaskBaseT *task)
{
    PakeClientTask *innerTask = (PakeClientTask *)task;
    if (innerTask == NULL) {
        return;
    }

    DestroyDasPakeParams(&(innerTask->params));
    if (innerTask->curTask != NULL) {
        innerTask->curTask->destroyTask(innerTask->curTask);
    }
    HcFree(innerTask);
}

static int CreateAndProcessNextBindTask(PakeClientTask *realTask, const CJson *in, CJson *out, int *status)
{
    realTask->curTask->destroyTask(realTask->curTask);
    realTask->curTask = CreateStandardBindExchangeClientTask();
    if (realTask->curTask == NULL) {
        LOGE("CreateBindExchangeTask failed");
        return HC_ERROR;
    }
    int res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("process StandardBindExchangeClientTask failed.");
    }
    return res;
}

static int CreateAndProcessNextUnbindTask(PakeClientTask *realTask, const CJson *in, CJson *out, int *status)
{
    realTask->curTask->destroyTask(realTask->curTask);
    realTask->curTask = CreateStandardUnbindExchangeClientTask();
    if (realTask->curTask == NULL) {
        LOGE("CreateBindExchangeTask failed");
        return HC_ERROR;
    }
    int res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("process StandardUnbindExchangeClientTask failed.");
    }
    return res;
}

static int CreateNextTask(PakeClientTask *realTask, const CJson *in, CJson *out, int *status)
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
        LOGE("Create and process next task failed, opcode: %d, res: %d.", realTask->params.opCode, res);
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
    LOGD("End client task successfully.");
    return res;
}

static int Process(struct SubTaskBaseT *task, const CJson *in, CJson *out, int *status)
{
    if (task == NULL || in == NULL || out == NULL || status == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    PakeClientTask *realTask = (PakeClientTask *)task;
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

SubTaskBase *CreatePakeClientTask(const CJson *in, CJson *out)
{
    PakeClientTask *task = (PakeClientTask *)HcMalloc(sizeof(PakeClientTask), 0);
    if (task == NULL) {
        LOGE("Malloc for PakeClientTask failed.");
        return NULL;
    }

    task->taskBase.getTaskType = GetPakeClientTaskType;
    task->taskBase.destroyTask = DestroyPakeClientTask;
    task->taskBase.process = Process;

    int res = InitDasPakeParams(&(task->params), in);
    if (res != HC_SUCCESS) {
        LOGE("Init das pake params failed, res: %d.", res);
        DestroyPakeClientTask((struct SubTaskBaseT *)task);
        return NULL;
    }
    task->curTask = CreatePakeProtocolClientTask();
    if (task->curTask == NULL) {
        LOGE("Create pake protocol client task failed, res: %d.", res);
        DestroyPakeClientTask((struct SubTaskBaseT *)task);
        return NULL;
    }
    return (SubTaskBase *)task;
}
