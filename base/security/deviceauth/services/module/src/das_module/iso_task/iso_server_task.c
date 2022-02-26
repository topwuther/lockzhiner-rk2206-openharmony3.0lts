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

#include "iso_server_task.h"
#include "das_common.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_server_bind_exchange_task.h"
#include "iso_server_protocol_task.h"
#include "iso_server_unbind_exchange_task.h"
#include "iso_task_common.h"

static int GetIsoServerTaskType(const struct SubTaskBaseT *task)
{
    IsoServerTask *realTask = (IsoServerTask *)task;
    if (realTask->curTask == NULL) {
        LOGE("cur Task is null");
        return TASK_TYPE_NONE;
    }
    return realTask->curTask->getCurTaskType();
}

static void DestroyIsoServerTask(struct SubTaskBaseT *task)
{
    IsoServerTask *innerTask = (IsoServerTask *)task;
    if (innerTask == NULL) {
        return;
    }
    DestroyIsoParams(&(innerTask->params));
    if (innerTask->curTask != NULL) {
        innerTask->curTask->destroyTask(innerTask->curTask);
    }
    HcFree(innerTask);
}

static int CreateNextTask(IsoServerTask *realTask, const CJson *in, CJson *out, int *status)
{
    int message = 0;
    int res = GetIntFromJson(in, FIELD_MESSAGE, &message);
    if (res != 0) {
        LOGE("GetIntFromJson failed, res:%d", res);
        return res;
    }
    switch (realTask->params.opCode) {
        case OP_BIND:
            if (message != ISO_CLIENT_BIND_EXCHANGE_CMD) {
                LOGI("The message is repeated, ignore it message :%d.", message);
                *status = IGNORE_MSG;
                break;
            }
            realTask->curTask = CreateServerBindExchangeTask(&(realTask->params), in, out, status);
            if (realTask->curTask == NULL) {
                LOGE("CreateBindExchangeTask failed");
                res = HC_ERROR;
            }
            break;
        case OP_UNBIND:
            if (message != ISO_CLIENT_UNBIND_EXCHANGE_CMD) {
                LOGI("The message is repeated, ignore it message :%d.", message);
                *status = IGNORE_MSG;
                break;
            }
            realTask->curTask = CreateServerUnbindExchangeTask(&(realTask->params), in, out, status);
            if (realTask->curTask == NULL) {
                LOGE("CreateBindExchangeTask failed");
                res = HC_ERROR;
            }
            break;
        default:
            res = CheckEncResult(&(realTask->params), in, RESULT_AAD);
            if (res != 0) {
                LOGE("CheckEncResult failed, res:%d", res);
                break;
            }
            res = SendResultToFinalSelf(&(realTask->params), out, true);
            if (res != 0) {
                LOGE("SendResultToFinalSelf failed, res:%d", res);
                break;
            }
            LOGD("Authenticate task end.");
            *status = FINISH;
    }
    if (res != 0) {
        SendErrorToOut(out, realTask->params.opCode, res);
    }
    return res;
}

static int Process(struct SubTaskBaseT *task, const CJson *in, CJson *out, int *status)
{
    if (task == NULL || in == NULL || out == NULL || status == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    IsoServerTask *realTask = (IsoServerTask *)task;
    int res;
    if (realTask->curTask != NULL) {
        res = realTask->curTask->process(realTask->curTask, &(realTask->params), in, out, status);
        if (*status != FINISH) {
            if (res != 0) {
                SendErrorToOut(out, realTask->params.opCode, res);
            }
            return res;
        } else {
            if (realTask->curTask->getCurTaskType() == TASK_TYPE_ISO_PROTOCOL) {
                realTask->curTask->destroyTask(realTask->curTask);
                realTask->curTask = NULL;
                *status = CONTINUE;
            }
        }
        if (res != 0) {
            LOGE("process failed, res:%d", res);
            SendErrorToOut(out, realTask->params.opCode, res);
            return res;
        }
    } else {
        res = CreateNextTask(realTask, in, out, status);
    }
    if (res != 0) {
        SendErrorToOut(out, realTask->params.opCode, res);
    }
    return res;
}

SubTaskBase *CreateIsoServerTask(const CJson *in, CJson *out)
{
    IsoServerTask *task = (IsoServerTask *)HcMalloc(sizeof(IsoServerTask), 0);
    if (task == NULL) {
        return NULL;
    }

    task->taskBase.getTaskType = GetIsoServerTaskType;
    task->taskBase.destroyTask = DestroyIsoServerTask;
    task->taskBase.process = Process;

    int res = InitIsoParams(&(task->params), in);
    if (res != 0) {
        SendErrorToOut(out, task->params.opCode, res);
        DestroyIsoServerTask((struct SubTaskBaseT *)task);
        return NULL;
    }

    task->curTask = CreateProtocolServerTask();
    if (task->curTask == NULL) {
        SendErrorToOut(out, task->params.opCode, res);
        DestroyIsoServerTask((struct SubTaskBaseT *)task);
        return NULL;
    }
    return (SubTaskBase *)task;
}
