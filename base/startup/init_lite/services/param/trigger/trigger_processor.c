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

#include "trigger_processor.h"
#include <pthread.h>
#include <unistd.h>

#include "init_cmds.h"
#include "param_manager.h"
#include "trigger_checker.h"
#include "uv.h"

#define LABEL "Trigger"
#define MAX_TRIGGER_COUNT_RUN_ONCE 20
#define SYS_POWER_CTRL "sys.powerctrl="
#define OHOS_CTL_START "ohos.ctl.start="
#define OHOS_CTL_STOP "ohos.ctl.stop="

static TriggerWorkSpace g_triggerWorkSpace = {};

static int DoCmdExecute(const TriggerNode *trigger, const char *cmdName, const char *command)
{
    PARAM_CHECK(trigger != NULL && cmdName != NULL && command != NULL, return -1, "Invalid param");
    PARAM_LOGD("DoCmdExecute trigger %s cmd %s %s", trigger->name, cmdName, command);
    if (strncmp(cmdName, TRIGGER_CMD, strlen(TRIGGER_CMD)) == 0) {
        DoTriggerExec(command);
        return 0;
    }
    DoCmdByName(cmdName, command);
    return 0;
}

static int DoTiggerCheckResult(TriggerNode *trigger, u_int32_t triggerIndex)
{
    if (TRIGGER_NODE_IN_QUEUE(trigger)) {
        PARAM_LOGI("DoTiggerExecute trigger %s has been waiting execute", trigger->name);
        return 0;
    }
    TRIGGER_NODE_SET_QUEUE_FLAG(trigger);
    PARAM_LOGI("Waiting to exec trigger %s", trigger->name);
    ExecuteQueuePush(&g_triggerWorkSpace, trigger, triggerIndex);
    return 0;
}

static int ExecuteTiggerImmediately(const TriggerNode *trigger, u_int32_t triggerIndex)
{
    return ExecuteTrigger(&g_triggerWorkSpace, trigger, DoCmdExecute);
}

void ExecuteQueueWork(u_int32_t maxCount)
{
    u_int32_t executeCount = 0;
    TriggerNode *trigger = ExecuteQueuePop(&g_triggerWorkSpace);
    while (trigger != NULL) {
        ExecuteTrigger(&g_triggerWorkSpace, trigger, DoCmdExecute);
        TRIGGER_NODE_CLEAR_QUEUE_FLAG(trigger);
        CheckAndExecuteTrigger(&g_triggerWorkSpace, trigger->name, ExecuteTiggerImmediately);

        executeCount++;
        if (executeCount > maxCount) {
            break;
        }
        PARAM_LOGI("ExecuteQueueWork %u", executeCount);
        trigger = ExecuteQueuePop(&g_triggerWorkSpace);
    }
}

static void CheckTriggers(int type, const void *content, u_int32_t contentLen)
{
    switch (type) {
        case EVENT_PROPERTY: {
            CheckParamTrigger(&g_triggerWorkSpace, content, contentLen, DoTiggerCheckResult);
            break;
        }
        case EVENT_BOOT: {
            CheckTrigger(&g_triggerWorkSpace, TRIGGER_BOOT, content, contentLen, DoTiggerCheckResult);
            break;
        }
        default:
            PARAM_LOGI("CheckTriggers: %d", type);
            break;
    }
}

static void ProcessAfterEvent(uv_work_t *req, int status)
{
    free(req);
    ExecuteQueueWork(MAX_TRIGGER_COUNT_RUN_ONCE);
}

static void ProcessEvent(uv_work_t *req)
{
    TriggerDataEvent *event = (TriggerDataEvent *)req;
    CheckTriggers(event->type, event->content, event->contentSize);
}

static const char *GetCmdInfo(const char *content, u_int32_t contentSize, char **cmdParam)
{
    static const char *ctrlCmds[][2] = {
        {"reboot", "reboot "}
    };
    for (size_t i = 0; i < sizeof(ctrlCmds) / sizeof(ctrlCmds[0]); i++) {
        if (strncmp(content, ctrlCmds[i][0], strlen(ctrlCmds[i][0])) == 0) {
            *cmdParam = (char *)content;
            return GetMatchCmd(ctrlCmds[i][1]);
        }
    }
    return NULL;
}

static void SendTriggerEvent(TriggerDataEvent *event)
{
    if (event == NULL) {
        return;
    }
    int ctrlSize = strlen(SYS_POWER_CTRL);
    if (strncmp(event->content, SYS_POWER_CTRL, ctrlSize) == 0) {
        char *cmdParam = NULL;
        const char *matchCmd = GetCmdInfo(event->content + ctrlSize, event->contentSize - ctrlSize, &cmdParam);
        if (matchCmd != NULL) {
            DoCmdByName(matchCmd, cmdParam);
        } else {
            PARAM_LOGE("SendTriggerEvent cmd %s not found", event->content);
        }
    } else if (strncmp(event->content, OHOS_CTL_START, strlen(OHOS_CTL_START)) == 0) {
        DoCmdByName("start ", event->content + strlen(OHOS_CTL_START));
    } else if (strncmp(event->content, OHOS_CTL_STOP, strlen(OHOS_CTL_STOP)) == 0) {
        DoCmdByName("stop ", event->content + strlen(OHOS_CTL_STOP));
    } else {
        uv_queue_work(uv_default_loop(), &event->request, ProcessEvent, ProcessAfterEvent);
        event = NULL;
    }
    if (event != NULL) {
        free(event);
    }
}

void PostParamTrigger(const char *name, const char *value)
{
    PARAM_CHECK(name != NULL && value != NULL, return, "Invalid param");
    PARAM_LOGD("PostParamTrigger %s ", name);
    PARAM_CHECK(strlen(name) <= PARAM_VALUE_LEN_MAX, return, "Invalid param");
    PARAM_CHECK(strlen(value) <= PARAM_NAME_LEN_MAX, return, "Invalid param");
    const int offset = 2;
    int contentLen = strlen(name) + strlen(value) + offset;
    TriggerDataEvent *event = (TriggerDataEvent *)malloc(sizeof(TriggerDataEvent) + contentLen);
    PARAM_CHECK(event != NULL, return, "Failed to alloc memory");
    event->type = EVENT_PROPERTY;
    event->request.data = (void *)((char*)event + sizeof(uv_work_t));
    event->contentSize = BuildParamContent(event->content, contentLen, name, value);
    PARAM_CHECK(event->contentSize > 0, return, "Failed to copy porperty");
    SendTriggerEvent(event);
    PARAM_LOGI("PostParamTrigger %s success", name);
}

void PostTrigger(EventType type, const char *content, u_int32_t contentLen)
{
    PARAM_LOGD("PostTrigger %d %s", type, content);
    PARAM_CHECK(content != NULL && contentLen > 0, return, "Invalid param");
    TriggerDataEvent *event = (TriggerDataEvent *)malloc(sizeof(TriggerDataEvent) + contentLen + 1);
    PARAM_CHECK(event != NULL, return, "Failed to alloc memory");
    event->type = type;
    event->request.data = (void *)((char*)event + sizeof(uv_work_t));
    event->contentSize = contentLen;
    PARAM_CHECK(memcpy_s(event->content, contentLen + 1, content, contentLen) == 0, return, "Failed to copy content");
    event->content[contentLen] = '\0';
    SendTriggerEvent(event);
    PARAM_LOGD("PostTrigger %d success", type);
}

int ParseTriggerConfig(const cJSON *fileRoot)
{
    PARAM_CHECK(fileRoot != NULL, return -1, "Invalid file");
    int ret = InitTriggerWorkSpace(&g_triggerWorkSpace);
    PARAM_CHECK(ret == 0, return -1, "Failed to init trigger");

    cJSON *triggers = cJSON_GetObjectItemCaseSensitive(fileRoot, TRIGGER_ARR_NAME_IN_JSON);
    PARAM_CHECK(cJSON_IsArray(triggers), return -1, "Trigger item must array");

    int size = cJSON_GetArraySize(triggers);
    PARAM_CHECK(size > 0, return -1, "Trigger array size must positive");

    for (int i = 0; i < size; ++i) {
        cJSON *item = cJSON_GetArrayItem(triggers, i);
        ParseTrigger(&g_triggerWorkSpace, item);
    }
    return 0;
}

void DoTriggerExec(const char *content)
{
    PARAM_CHECK(content != NULL, return, "Invalid trigger content");
    u_int32_t triggerIndex = 0;
    TriggerNode *trigger = GetTriggerByName(&g_triggerWorkSpace, content, &triggerIndex);
    if (trigger != NULL && !TRIGGER_NODE_IN_QUEUE(trigger)) { // 不在队列中
        PARAM_LOGI("DoTriggerExec trigger %s", trigger->name);
        TRIGGER_NODE_SET_QUEUE_FLAG(trigger);
        ExecuteQueuePush(&g_triggerWorkSpace, trigger, triggerIndex);
    }
}

TriggerWorkSpace *GetTriggerWorkSpace()
{
    return &g_triggerWorkSpace;
}
