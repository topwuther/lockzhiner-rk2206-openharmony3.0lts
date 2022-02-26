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

#include "trigger_manager.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "init_cmds.h"
#include "init_utils.h"
#include "trigger_checker.h"

#define LABEL "Trigger"
#define TRIGGER_AREA_SPACE 1024*128
#define TRIGGER_EXECUTE_QUEUE 64
#define BUFFER_SIZE 256
#define CHECK_INDEX_VALID(workSpace, index) \
    (u_int32_t)(index) < sizeof((workSpace)->header) / sizeof((workSpace)->header[0])

#ifdef STARTUP_LOCAL
#define TRIGGER_PATH "/media/sf_ubuntu/test/__trigger__/trigger"
#else
#define TRIGGER_PATH "/dev/__trigger__/trigger"
#endif

int InitTriggerWorkSpace(TriggerWorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL, return -1, "Invalid parm");
    if (workSpace->area != NULL) {
        return 0;
    }
    CheckAndCreateDir(TRIGGER_PATH);
    int fd = open(TRIGGER_PATH, O_CREAT | O_RDWR | O_TRUNC | O_CLOEXEC, S_IRUSR | S_IRGRP | S_IROTH);
    PARAM_CHECK(fd >= 0, return -1, "Open file fail error %s", strerror(errno));
    lseek(fd, TRIGGER_AREA_SPACE, SEEK_SET);
    write(fd, "", 1);

    void *areaAddr = (void *)mmap(NULL, TRIGGER_AREA_SPACE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    PARAM_CHECK(areaAddr != MAP_FAILED, close(fd); return -1,
        "Failed to map memory error %s", strerror(errno));
    close(fd);

    // 第一部分做执行队列
    workSpace->executeQueue.executeQueue = (u_int32_t *)areaAddr;
    workSpace->executeQueue.queueCount = TRIGGER_EXECUTE_QUEUE;
    workSpace->executeQueue.startIndex = 0;
    workSpace->executeQueue.endIndex = 0;
    pthread_mutex_init(&workSpace->executeQueue.mutex, NULL);

    // 动态数据保存
    workSpace->area = (TriggerArea *)(areaAddr + TRIGGER_EXECUTE_QUEUE * sizeof(u_int32_t));
    atomic_init(&workSpace->area->serial, ATOMIC_VAR_INIT(0));
    workSpace->area->dataSize = TRIGGER_AREA_SPACE - sizeof(TriggerArea) - TRIGGER_EXECUTE_QUEUE * sizeof(u_int32_t);
    workSpace->area->currOffset = sizeof(TriggerArea) + TRIGGER_EXECUTE_QUEUE * sizeof(u_int32_t);
    for (size_t i = 0; i < sizeof(workSpace->header) / sizeof(workSpace->header[0]); i++) {
        atomic_init(&workSpace->header[i].firstTrigger, ATOMIC_VAR_INIT(0));
        atomic_init(&workSpace->header[i].lastTrigger, ATOMIC_VAR_INIT(0));
    }
    return 0;
}

static CommandNode *GetCmdByIndex(const TriggerWorkSpace *workSpace, const TriggerNode *trigger, u_int32_t index)
{
    if (index == 0 || index == (u_int32_t)-1) {
        return NULL;
    }
    const u_int32_t offset = 2;
    u_int32_t size = sizeof(CommandNode) + offset;
    PARAM_CHECK((index + size) < workSpace->area->dataSize,
        return NULL, "Invalid index for cmd %u", index);
    return (CommandNode *)(workSpace->area->data + index);
}

u_int32_t AddCommand(TriggerWorkSpace *workSpace, TriggerNode *trigger, const char *cmdName, const char *content)
{
    PARAM_CHECK(workSpace != NULL && trigger != NULL && cmdName != NULL, return 0, "list is null");
    PARAM_CHECK(workSpace->area != NULL, return 0, "Invalid trigger workspace");
    u_int32_t size = sizeof(CommandNode) + strlen(cmdName) + 1;
    size += ((content == NULL) ? 1 : (strlen(content) + 1));
    size = (size + 0x03) & (~0x03);
    PARAM_CHECK((workSpace->area->currOffset + size) < workSpace->area->dataSize,
        return 0, "Not enough memory for cmd %u %u", size, workSpace->area->currOffset);

    CommandNode *node = (CommandNode *)(workSpace->area->data + workSpace->area->currOffset);
    PARAM_CHECK(node != NULL, return 0, "Failed to alloc memory for command");

    int ret = memcpy_s(node->name, sizeof(node->name) - 1, cmdName, strlen(cmdName));
    PARAM_CHECK(ret == 0, return 0, "Failed to copy command");
    node->name[strlen(cmdName)] = '\0';
    if (content != NULL) {
        ret = memcpy_s(node->content, size, content, strlen(content));
        node->content[strlen(content)] = '\0';
        PARAM_CHECK(ret == 0, return 0, "Failed to copy command");
    } else {
        node->content[0] = '\0';
    }

    u_int32_t offset = workSpace->area->currOffset;
    atomic_init(&node->next, ATOMIC_VAR_INIT(0));
    // 插入队列
    if (trigger->firstCmd == 0) {
        atomic_store_explicit(&trigger->firstCmd, offset, memory_order_release);
        atomic_store_explicit(&trigger->lastCmd, offset, memory_order_release);
    } else {
        CommandNode *lastNode = GetCmdByIndex(workSpace, trigger, trigger->lastCmd);
        if (lastNode != NULL) {
            atomic_store_explicit(&lastNode->next, offset, memory_order_release);
        }
        atomic_store_explicit(&trigger->lastCmd, offset, memory_order_release);
    }
    workSpace->area->currOffset += size;
    return offset;
}

static TriggerNode *GetTriggerByIndex(const TriggerWorkSpace *workSpace, u_int32_t index)
{
    if (index == 0 || index == (u_int32_t)-1) {
        return NULL;
    }
    u_int32_t size = sizeof(TriggerNode) + 1;
    PARAM_CHECK((index + size) < workSpace->area->dataSize,
        return NULL, "Invalid index for trigger %u", index);
    return (TriggerNode *)(workSpace->area->data + index);
}

u_int32_t AddTrigger(TriggerWorkSpace *workSpace, int type, const char *name, const char *condition)
{
    PARAM_CHECK(workSpace != NULL && name != NULL, return 0, "list is null");
    PARAM_CHECK(workSpace->area != NULL, return 0, "Invalid trigger workspace");
    const char *tmpCond = condition;
    if (type == TRIGGER_BOOT && condition == NULL) {
        tmpCond = name;
    }
    u_int32_t conditionSize = ((tmpCond == NULL) ? 1 : (strlen(tmpCond) + 1 + CONDITION_EXTEND_LEN));
    conditionSize = (conditionSize + 0x03) & (~0x03);
    PARAM_CHECK((workSpace->area->currOffset + sizeof(TriggerNode) + conditionSize) < workSpace->area->dataSize,
        return -1, "Not enough memory for cmd");

    TriggerNode *node = (TriggerNode *)(workSpace->area->data + workSpace->area->currOffset);
    PARAM_CHECK(node != NULL, return 0, "Failed to alloc memory for trigger");
    node->type = type;
    int ret = memcpy_s(node->name, sizeof(node->name) - 1, name, strlen(name));
    PARAM_CHECK(ret == 0, return 0, "Failed to memcpy_s for trigger");
    node->name[strlen(name)] = '\0';

    if (tmpCond != NULL) {
        ret = ConvertInfixToPrefix(tmpCond, node->condition, conditionSize);
        PARAM_CHECK(ret == 0, return 0, "Failed to memcpy_s for trigger");
    } else {
        node->condition[0] = '\0';
    }

    u_int32_t offset = workSpace->area->currOffset;
    atomic_init(&node->serial, ATOMIC_VAR_INIT(0));
    atomic_init(&node->next, ATOMIC_VAR_INIT(0));
    atomic_init(&node->firstCmd, ATOMIC_VAR_INIT(0));
    atomic_init(&node->lastCmd, ATOMIC_VAR_INIT(0));

    // 插入到trigger队列中
    if (workSpace->header[type].firstTrigger == 0) {
        atomic_store_explicit(&workSpace->header[type].firstTrigger, offset, memory_order_release);
        atomic_store_explicit(&workSpace->header[type].lastTrigger, offset, memory_order_release);
    } else {
        TriggerNode *lastNode = GetTriggerByIndex(workSpace, workSpace->header[type].lastTrigger);
        if (lastNode != NULL) {
            atomic_store_explicit(&lastNode->next, offset, memory_order_release);
        }
        atomic_store_explicit(&workSpace->header[type].lastTrigger, offset, memory_order_release);
    }
    workSpace->area->currOffset += conditionSize + sizeof(TriggerNode);
    return offset;
}

static int GetTriggerIndex(const char *type)
{
    if (strncmp("param:", type, strlen("param:")) == 0) {
        return TRIGGER_PARAM;
    }
    static const char *triggerType[] = {
        "pre-init", "boot", "early-init", "init", "late-init", "post-init",
        "early-fs", "post-fs", "late-fs", "post-fs-data"
    };
    for (size_t i = 0; i < sizeof(triggerType) / sizeof(char*); i++) {
        if (strcmp(triggerType[i], type) == 0) {
            return TRIGGER_BOOT;
        }
    }
    return TRIGGER_UNKNOW;
}

int ParseTrigger(TriggerWorkSpace *workSpace, const cJSON *triggerItem)
{
    PARAM_CHECK(triggerItem != NULL, return -1, "Invalid file");
    PARAM_CHECK(workSpace != NULL, return -1, "Failed to create trigger list");

    char *name = cJSON_GetStringValue(cJSON_GetObjectItem(triggerItem, "name"));
    PARAM_CHECK(name != NULL, return -1, "Can not get name from cfg");
    char *condition = cJSON_GetStringValue(cJSON_GetObjectItem(triggerItem, "condition"));

    int index = GetTriggerIndex(name);
    PARAM_CHECK(CHECK_INDEX_VALID(workSpace, index), return -1, "Failed to get trigger index");

    u_int32_t offset = 0;
    TriggerNode *trigger = GetTriggerByName(workSpace, name, &offset);
    if (trigger == NULL) {
        offset = AddTrigger(workSpace, index, name, condition);
        PARAM_CHECK(offset > 0, return -1, "Failed to create trigger %s", name);
        trigger = GetTriggerByIndex(workSpace, offset);
    } else {
        if (condition != NULL) {
            PARAM_LOGE("Warning parseTrigger %s %s", name, condition);
        }
    }
    PARAM_LOGD("ParseTrigger %s %u", name, offset);

    // 添加命令行
    cJSON* cmdItems = cJSON_GetObjectItem(triggerItem, CMDS_ARR_NAME_IN_JSON);
    PARAM_CHECK(cJSON_IsArray(cmdItems), return -1, "Command item must be array");
    int cmdLinesCnt = cJSON_GetArraySize(cmdItems);
    PARAM_CHECK(cmdLinesCnt > 0 && cmdLinesCnt <= MAX_CMD_IN_JOBS,
        return -1, "Command array size must positive %s", name);

    for (int i = 0; i < cmdLinesCnt; ++i) {
        char *cmdLineStr = cJSON_GetStringValue(cJSON_GetArrayItem(cmdItems, i));
        PARAM_CHECK(cmdLineStr != NULL, continue, "Command is null");

        size_t cmdLineLen = strlen(cmdLineStr);
        const char *matchCmd = GetMatchCmd(cmdLineStr);
        if (matchCmd == NULL && strncmp(cmdLineStr, TRIGGER_CMD, strlen(TRIGGER_CMD)) == 0) {
            matchCmd = TRIGGER_CMD;
        }
        PARAM_CHECK(matchCmd != NULL, continue, "Command not support %s", cmdLineStr);
        size_t matchLen = strlen(matchCmd);
        if (matchLen == cmdLineLen) {
            offset = AddCommand(workSpace, trigger, matchCmd, NULL);
        } else {
            offset = AddCommand(workSpace, trigger, matchCmd, cmdLineStr + matchLen);
        }
        PARAM_CHECK(offset > 0, continue, "Failed to add command %s", cmdLineStr);
    }
    return 0;
}

int ExecuteTrigger(TriggerWorkSpace *workSpace, TriggerNode *trigger, CMD_EXECUTE cmdExecuter)
{
    PARAM_CHECK(workSpace != NULL && trigger != NULL && cmdExecuter != NULL, return -1, "Invalid param");
    PARAM_LOGI("ExecuteTrigger trigger %s", trigger->name);
    CommandNode *cmd = GetCmdByIndex(workSpace, trigger, trigger->firstCmd);
    while (cmd != NULL) {
        cmdExecuter(trigger, cmd->name, cmd->content);
        cmd = GetCmdByIndex(workSpace, trigger, cmd->next);
    }
    return 0;
}

int ExecuteQueuePush(TriggerWorkSpace *workSpace, TriggerNode *trigger, u_int32_t triggerIndex)
{
    PARAM_CHECK(workSpace != NULL, return -1, "Invalid area");
    pthread_mutex_lock(&workSpace->executeQueue.mutex);
    u_int32_t index = workSpace->executeQueue.endIndex++ % workSpace->executeQueue.queueCount;
    workSpace->executeQueue.executeQueue[index] = triggerIndex;
    pthread_mutex_unlock(&workSpace->executeQueue.mutex);
    return 0;
}

TriggerNode *ExecuteQueuePop(TriggerWorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Invalid param");
    if (workSpace->executeQueue.endIndex <= workSpace->executeQueue.startIndex) {
        return NULL;
    }
    pthread_mutex_lock(&workSpace->executeQueue.mutex);
    u_int32_t currIndex = workSpace->executeQueue.startIndex % workSpace->executeQueue.queueCount;
    u_int32_t triggerIndex = workSpace->executeQueue.executeQueue[currIndex];
    workSpace->executeQueue.executeQueue[currIndex] = 0;
    workSpace->executeQueue.startIndex++;
    pthread_mutex_unlock(&workSpace->executeQueue.mutex);
    return GetTriggerByIndex(workSpace, triggerIndex);
}

int ExecuteQueueSize(const TriggerWorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL, return 0, "Invalid param");
    return workSpace->executeQueue.endIndex - workSpace->executeQueue.startIndex;
}

static int CheckBootTriggerMatch(const LogicCalculator *calculator,
    const TriggerNode *trigger, const char *content, u_int32_t contentSize)
{
    if (strncmp(trigger->name, (char *)content, contentSize) == 0) {
        return 1;
    }
    return 0;
}

static int CheckParamTriggerMatch(LogicCalculator *calculator,
    const TriggerNode *trigger, const char *content, u_int32_t contentSize)
{
    if (calculator->inputName != NULL) { // 存在input数据时，先过滤非input的
        if (GetMatchedSubCondition(trigger->condition, content, strlen(calculator->inputName) + 1) == NULL) {
            return 0;
        }
    }
    return ComputeCondition(calculator, trigger->condition);
}

static int CheckOtherTriggerMatch(LogicCalculator *calculator,
    const TriggerNode *trigger, const char *content, u_int32_t contentSize)
{
    return ComputeCondition(calculator, trigger->condition);
}

static int CheckTrigger_(TriggerWorkSpace *workSpace,
    LogicCalculator *calculator, int type, const char *content, u_int32_t contentSize)
{
    static TRIGGER_MATCH triggerCheckMatch[TRIGGER_MAX] = {
        CheckBootTriggerMatch, CheckParamTriggerMatch, CheckOtherTriggerMatch
    };
    PARAM_LOGD("CheckTrigger_ content %s ", content);
    PARAM_CHECK(calculator != NULL, return -1, "Failed to check calculator");
    PARAM_CHECK(CHECK_INDEX_VALID(workSpace, type), return -1, "Invalid type %d", type);
    PARAM_CHECK((u_int32_t)type < sizeof(triggerCheckMatch) / sizeof(triggerCheckMatch[0]),
        return -1, "Failed to get check function");
    PARAM_CHECK(triggerCheckMatch[type] != NULL, return -1, "Failed to get check function");

    u_int32_t index = workSpace->header[type].firstTrigger;
    TriggerNode *trigger = GetTriggerByIndex(workSpace, workSpace->header[type].firstTrigger);
    while (trigger != NULL) {
        if (triggerCheckMatch[type](calculator, trigger, content, contentSize) == 1) { // 等于1 则认为匹配
            calculator->triggerExecuter(trigger, index);
        }
        index = trigger->next;
        trigger = GetTriggerByIndex(workSpace, trigger->next);
    }
    return 0;
}

int CheckTrigger(const TriggerWorkSpace *workSpace,
    int type, void *content, u_int32_t contentSize, PARAM_CHECK_DONE triggerExecuter)
{
    PARAM_CHECK(workSpace != NULL && content != NULL && triggerExecuter != NULL,
        return -1, "Failed arg for trigger");

    LogicCalculator calculator = {};
    calculator.triggerExecuter = triggerExecuter;
    return CheckTrigger_(workSpace, &calculator, type, (char *)content, contentSize);
}

int CheckParamTrigger(TriggerWorkSpace *workSpace,
    const char *content, u_int32_t contentSize, PARAM_CHECK_DONE triggerExecuter)
{
    PARAM_CHECK(workSpace != NULL && content != NULL && triggerExecuter != NULL,
        return -1, "Failed arg for param trigger");
    LogicCalculator calculator = {};
    CalculatorInit(&calculator, MAX_DATA_NUMBER, sizeof(LogicData), 1);

    // 先解析content
    int ret = GetValueFromContent(content, contentSize, 0, calculator.inputName, SUPPORT_DATA_BUFFER_MAX);
    PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Failed parse content name");
    ret = GetValueFromContent(content, contentSize,
        strlen(calculator.inputName) + 1, calculator.inputContent, SUPPORT_DATA_BUFFER_MAX);
    PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Failed parse content value");

    calculator.triggerExecuter = triggerExecuter;
    CheckTrigger_(workSpace, &calculator, TRIGGER_PARAM, content, contentSize);
    CalculatorFree(&calculator);
    return 0;
}

int CheckAndExecuteTrigger(const TriggerWorkSpace *workSpace, const char *content, PARAM_CHECK_DONE triggerExecuter)
{
    PARAM_CHECK(workSpace != NULL && content != NULL && triggerExecuter != NULL,
        return -1, "Failed arg for param trigger");
    LogicCalculator calculator = {};
    CalculatorInit(&calculator, MAX_DATA_NUMBER, sizeof(LogicData), 1);

    int ret = memcpy_s(calculator.triggerContent, sizeof(calculator.triggerContent), content, strlen(content));
    PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Failed to memcpy");

    calculator.triggerExecuter = triggerExecuter;
    calculator.inputName = NULL;
    calculator.inputContent = NULL;
    // 执行完成后，对第三类trigger检查，执行必须是在本阶段执行的trigger
    CheckTrigger_(workSpace, &calculator, TRIGGER_UNKNOW, content, 0);
    CalculatorFree(&calculator);
    return 0;
}

TriggerNode *GetTriggerByName(const TriggerWorkSpace *workSpace, const char *triggerName, u_int32_t *triggerIndex)
{
    PARAM_CHECK(workSpace != NULL && triggerName != NULL && triggerIndex != NULL, return NULL, "Invalid param");
    for (size_t i = 0; i < sizeof(workSpace->header) / sizeof(workSpace->header[0]); i++) {
        u_int32_t index = workSpace->header[i].firstTrigger;
        TriggerNode *trigger = GetTriggerByIndex(workSpace, workSpace->header[i].firstTrigger);
        while (trigger != NULL) {
            if (strcmp(triggerName, trigger->name) == 0) {
                *triggerIndex = index;
                return trigger;
            }
            index = trigger->next;
            trigger = GetTriggerByIndex(workSpace, trigger->next);
        }
    }
    return NULL;
}
