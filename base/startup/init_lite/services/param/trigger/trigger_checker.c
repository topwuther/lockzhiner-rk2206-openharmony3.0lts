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

#include "trigger_checker.h"
#include <ctype.h>
#include "trigger_manager.h"
#include "init_param.h"

#define LABEL "Trigger"
// 申请整块内存作为计算的节点
int CalculatorInit(LogicCalculator *calculator, int dataNumber, int dataUnit, int needCondition)
{
    PARAM_CHECK(calculator != NULL, return -1, "Invalid param");
    PARAM_CHECK(dataUnit <= sizeof(LogicData), return -1, "Invalid dataUnit");
    PARAM_CHECK(dataNumber <= MAX_DATA_NUMBER, return -1, "Invalid dataNumber");
    int dataSize = dataUnit * dataNumber;
    const int multiple = 5;
    if (needCondition) {
        dataSize += multiple * SUPPORT_DATA_BUFFER_MAX;
    }
    calculator->data = (char *)malloc(dataSize);
    PARAM_CHECK(calculator->data != NULL, return -1, "Failed to malloc for calculator");
    calculator->dataNumber = dataNumber;
    calculator->endIndex = 0;
    calculator->dataUnit = dataUnit;

    dataSize = dataUnit * dataNumber;
    calculator->conditionName = calculator->data + dataSize;
    dataSize += SUPPORT_DATA_BUFFER_MAX;
    calculator->conditionContent = calculator->data + dataSize;
    dataSize += SUPPORT_DATA_BUFFER_MAX;
    calculator->inputName = calculator->data + dataSize;
    dataSize += SUPPORT_DATA_BUFFER_MAX;
    calculator->inputContent = calculator->data + dataSize;
    dataSize += SUPPORT_DATA_BUFFER_MAX;
    calculator->readContent = calculator->data + dataSize;
    return memset_s(calculator->triggerContent,
        sizeof(calculator->triggerContent), 0, sizeof(calculator->triggerContent));
}

void CalculatorFree(LogicCalculator *calculator)
{
    PARAM_CHECK(calculator != NULL, return, "Invalid param");
    free(calculator->data);
    calculator->data = NULL;
}

static void CalculatorClear(LogicCalculator *calculator)
{
    PARAM_CHECK(calculator != NULL, return, "Invalid param");
    calculator->endIndex = 0;
}

static int CalculatorPushChar(LogicCalculator *calculator, char data)
{
    PARAM_CHECK(calculator != NULL, return -1, "Invalid param");
    PARAM_CHECK(calculator->endIndex < calculator->dataNumber, return -1, "More data for calculator support");
    PARAM_CHECK(sizeof(char) == calculator->dataUnit, return -1, "More data for calculator support");
    calculator->data[calculator->endIndex++] = data;
    return 0;
}

static int CalculatorPopChar(LogicCalculator *calculator, char *data)
{
    PARAM_CHECK(calculator != NULL, return -1, "Invalid param");
    PARAM_CHECK(calculator->endIndex < calculator->dataNumber, return -1, "More data for calculator support");
    if (calculator->endIndex == 0) {
        return -1;
    }
    *data = calculator->data[--calculator->endIndex];
    return 0;
}

static int CalculatorPush(LogicCalculator *calculator, const void *data)
{
    PARAM_CHECK(calculator != NULL, return -1, "Invalid param");
    PARAM_CHECK(calculator->endIndex < calculator->dataNumber, return -1, "More data for calculator support");
    char *tmpData = (calculator->data + calculator->dataUnit * calculator->endIndex);
    int ret = memcpy_s(tmpData, calculator->dataUnit, data, calculator->dataUnit);
    PARAM_CHECK(ret == 0, return -1, "Failed to copy logic data");
    calculator->endIndex++;
    return 0;
}

static int CalculatorPop(LogicCalculator *calculator, void *data)
{
    PARAM_CHECK(calculator != NULL && data != NULL, return -1, "Invalid param");
    PARAM_CHECK(calculator->endIndex < calculator->dataNumber, return -1, "More data for calculator support");
    if (calculator->endIndex == 0) {
        return -1;
    }
    char *tmpData = calculator->data + calculator->dataUnit * (calculator->endIndex - 1);
    int ret = memcpy_s(data, calculator->dataUnit, tmpData, calculator->dataUnit);
    PARAM_CHECK(ret == 0, return -1, "Failed to copy logic data");
    calculator->endIndex--;
    return 0;
}

static int CalculatorLength(const LogicCalculator *calculator)
{
    PARAM_CHECK(calculator != NULL, return 0, "Invalid param");
    return calculator->endIndex;
}

static int PrefixAdd(char *prefix, u_int32_t *prefixIndex, u_int32_t prefixLen, char op)
{
    const u_int32_t offset = 3;
    if ((*prefixIndex + offset) >= prefixLen) {
        return -1;
    }
    prefix[(*prefixIndex)++] = ' ';
    prefix[(*prefixIndex)++] = op;
    prefix[(*prefixIndex)++] = ' ';
    return 0;
}

static int HandleOperationOr(LogicCalculator *calculator, char *prefix, u_int32_t *prefixIndex, u_int32_t prefixLen)
{
    char e;
    prefix[(*prefixIndex)++] = ' ';
    if (CalculatorLength(calculator) == 0) {
        CalculatorPushChar(calculator, '|');
    } else {
        do {
            CalculatorPopChar(calculator, &e);
            if (e == '(') {
                CalculatorPushChar(calculator, e);
            } else {
                int ret = PrefixAdd(prefix, prefixIndex, prefixLen, e);
                PARAM_CHECK(ret == 0, return -1, "Invalid prefix");
            }
        } while (CalculatorLength(calculator) > 0 && e != '(');
        CalculatorPushChar(calculator, '|');
    }
    return 0;
}

static int ComputeSubCondition(LogicCalculator *calculator, LogicData *data, const char *condition)
{
    if (!LOGIC_DATA_TEST_FLAG(data, LOGIC_DATA_FLAGS_ORIGINAL)) {
        return LOGIC_DATA_TEST_FLAG(data, LOGIC_DATA_FLAGS_TRUE);
    }
    // 解析条件
    char *subStr = strstr(condition + data->startIndex, "=");
    if (subStr != NULL && ((u_int32_t)(subStr - condition) > data->endIndex)) {
        if (strncmp(condition + data->startIndex, calculator->triggerContent,
            strlen(calculator->triggerContent)) == 0) {
            return 1;
        }
    } else {
        int ret = GetValueFromContent(condition + data->startIndex,
            data->endIndex - data->startIndex, 0, calculator->conditionName, SUPPORT_DATA_BUFFER_MAX);
        PARAM_CHECK(ret == 0, return -1, "Failed parse content name");
        ret = GetValueFromContent(condition + data->startIndex, data->endIndex - data->startIndex,
            strlen(calculator->conditionName) + 1, calculator->conditionContent, SUPPORT_DATA_BUFFER_MAX);
        PARAM_CHECK(ret == 0, return -1, "Failed parse content value");
        // check name
        if (calculator->inputName && strcmp(calculator->conditionName, calculator->inputName) == 0) {
            if (strcmp(calculator->conditionContent, "*") == 0) {
                return 1;
            }
            if (strcmp(calculator->conditionContent, calculator->inputContent) == 0) {
                return 1;
            }
        } else {
            u_int32_t len = SUPPORT_DATA_BUFFER_MAX;
            ret = SystemReadParam(calculator->conditionName, calculator->readContent, &len);
            if (ret == 0 && (strcmp(calculator->conditionContent, "*") == 0 ||
                strcmp(calculator->conditionContent, calculator->readContent) == 0)) {
                return 1;
            }
        }
    }
    return 0;
}

int GetValueFromContent(const char *content, u_int32_t contentSize, u_int32_t start, char *value, u_int32_t valueSize)
{
    u_int32_t contentIndex = start;
    u_int32_t currIndex = 0;
    PARAM_CHECK(content != NULL && value != NULL, return -1, "Invalid arguments");
    while (contentIndex < contentSize && currIndex < valueSize) {
        if (content[contentIndex] == '=') {
            value[currIndex++] = '\0';
            return 0;
        }
        value[currIndex++] = content[contentIndex++];
    }
    if (currIndex < valueSize) {
        value[currIndex] = '\0';
        return 0;
    }
    return -1;
}

int ComputeCondition(LogicCalculator *calculator, const char *condition)
{
    u_int32_t currIndex = 0;
    u_int32_t start = 0;
    int noneOper = 1;
    CalculatorClear(calculator);
    PARAM_CHECK(condition != NULL, return -1, "Invalid condition");
    LogicData data1 = {};
    LogicData data2 = {};
    while (currIndex < strlen(condition)) {
        if (condition[currIndex] == '|' || condition[currIndex] == '&') {
            noneOper = 0;
            int ret = CalculatorPop(calculator, (void*)&data2);
            int ret1 = CalculatorPop(calculator, (void*)&data1);
            PARAM_CHECK((ret == 0 && ret1 == 0), return -1, "Failed to pop data");

            ret = ComputeSubCondition(calculator, &data1, condition);
            data1.flags = 0;
            if (condition[currIndex] == '|' && ret == 1) {
                LOGIC_DATA_SET_FLAG(&data1, LOGIC_DATA_FLAGS_TRUE);
            } else if (condition[currIndex] == '|' || ret == 1) {
                PARAM_CHECK(ComputeSubCondition(calculator, &data2, condition) != 1,
                    LOGIC_DATA_SET_FLAG(&data1, LOGIC_DATA_FLAGS_TRUE));
            }
            ret = CalculatorPush(calculator, (void*)&data1);
            PARAM_CHECK(ret == 0, return -1, "Failed to push data");
            start = currIndex + 1; // 跳过符号
        } else if (isspace(condition[currIndex])) {
            if (start == currIndex) {
                start = ++currIndex;
                continue;
            }
            data1.flags = LOGIC_DATA_FLAGS_ORIGINAL;
            data1.startIndex = start;
            data1.endIndex = currIndex;
            int ret = CalculatorPush(calculator, (void*)&data1);
            PARAM_CHECK(ret == 0, return -1, "Failed to push data");
            start = currIndex + 1;
        }
        currIndex++;
    }
    if (noneOper) {
        data1.flags = LOGIC_DATA_FLAGS_ORIGINAL;
        data1.startIndex = start;
        data1.endIndex = strlen(condition);
    } else {
        int ret = CalculatorPop(calculator, &data1);
        PARAM_CHECK(ret == 0, return -1, "Invalid calculator");
    }
    return ComputeSubCondition(calculator, &data1, condition);
}

int ConvertInfixToPrefix(const char *condition, char *prefix, u_int32_t prefixLen)
{
    char e = 0;
    int ret = 0;
    u_int32_t curr = 0;
    u_int32_t prefixIndex = 0;

    PARAM_CHECK(condition != NULL, return -1, "Invalid condition");
    LogicCalculator calculator;
    CalculatorInit(&calculator, MAX_DATA_NUMBER, 1, 0);

    while (curr < strlen(condition)) {
        PARAM_CHECK(prefixIndex < prefixLen, CalculatorFree(&calculator); return -1, "Invalid prefixIndex");
        if (condition[curr] == ')') {
            CalculatorPopChar(&calculator, &e);
            while (e != '(') {
                ret = PrefixAdd(prefix, &prefixIndex, prefixLen, e);
                PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Invalid prefix");
                ret = CalculatorPopChar(&calculator, &e);
                PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Invalid calculator");
            }
        } else if (condition[curr] == '|') {
            PARAM_CHECK(condition[curr + 1] == '|', CalculatorFree(&calculator); return -1, "Invalid condition");
            ret = HandleOperationOr(&calculator, prefix, &prefixIndex, prefixLen);
            PARAM_CHECK(ret == 0, CalculatorFree(&calculator); return -1, "Invalid prefix");
            curr++;
        } else if (condition[curr] == '&') {
            PARAM_CHECK(condition[curr + 1] == '&', CalculatorFree(&calculator); return -1, "Invalid condition");
            prefix[prefixIndex++] = ' ';
            CalculatorPushChar(&calculator, condition[curr]);
            curr++;
        } else if (condition[curr] == '(') {
            CalculatorPushChar(&calculator, condition[curr]);
        } else {
            prefix[prefixIndex++] = condition[curr];
        }
        curr++;
    }

    while (CalculatorLength(&calculator) > 0) {
        CalculatorPopChar(&calculator, &e);
        ret = PrefixAdd(prefix, &prefixIndex, prefixLen, e);
        PARAM_CHECK(ret == 0, CalculatorFree(&calculator);
            return -1, "Invalid prefix %u %u", prefixIndex, prefixLen);
    }
    prefix[prefixIndex] = '\0';
    CalculatorFree(&calculator);
    return 0;
}

char *GetMatchedSubCondition(const char *condition, const char *input, int length)
{
    PARAM_CHECK(condition != NULL && input != NULL, return NULL, "Invalid arguments");
    const char *p = condition;
    for (; (p = strchr(p, *input)) != 0; p++) {
        if (strncmp(p, input, length) == 0) {
            return (char*)p;
        }
    }
    return NULL;
}
