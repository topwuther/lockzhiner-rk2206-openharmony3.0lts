/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "appspawn_message.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef __LINUX__
#include <linux/capability.h>
#else
#include <sys/capability.h>
#endif

#include "cJSON.h"
#include "log.h"
#include "ohos_errno.h"
#include "securec.h"

static const size_t MAX_BUNDLE_NAME_LEN = 127;
static const size_t MIN_BUNDLE_NAME_LEN = 7;
static const size_t MAX_IDENTITY_ID_LEN = 24;
static const size_t MIN_IDENTITY_ID_LEN = 1;
static const size_t MAX_CAPABILITY_COUNT = 10;

void FreeMessageSt(MessageSt* targetSt)
{
    if (targetSt != NULL) {
        if (targetSt->bundleName != NULL) {
            free(targetSt->bundleName);
            targetSt->bundleName = NULL;
        }

        if (targetSt->identityID != NULL) {
            free(targetSt->identityID);
            targetSt->identityID = NULL;
        }

        if (targetSt->caps != NULL) {
            free(targetSt->caps);
            targetSt->caps = NULL;
        }

        targetSt->uID = -1;
        targetSt->gID = -1;
        targetSt->capsCnt = 0;
    }
}

static enum OHOSLiteErrorCode ReadStringItem(cJSON* strItem, char** buf, size_t maxLen, size_t minLen)
{
    if (strItem == NULL || !cJSON_IsString(strItem)) {
        return EC_INVALID;
    }

    char* strPtr = cJSON_GetStringValue(strItem);
    if (strPtr == NULL) {
        return EC_PROTOCOL;
    }

    size_t strLength = strlen(strPtr);
    if (strLength > maxLen || strLength < minLen) {
        return EC_PROTOCOL;
    }

    char* bufTmp = (char*)malloc(strLength + 1);
    if (bufTmp == NULL) {
        return EC_NOMEMORY;
    }

    if (strLength > 0 && memcpy_s(bufTmp, strLength, strPtr, strLength) != EOK) {
        free(bufTmp);
        bufTmp = NULL;
        return EC_FAILURE;
    }

    bufTmp[strLength] = '\0';
    *buf = bufTmp;
    return EC_SUCCESS;
}

static double ReadNumberItem(cJSON* strItem)
{
    if (strItem == NULL || !cJSON_IsNumber(strItem)) {
        return -1;
    }

    return cJSON_GetNumberValue(strItem);
}

static int GetCaps(const cJSON* curItem, MessageSt* msgSt)
{
    msgSt->capsCnt = 0;
    msgSt->caps = NULL;
    cJSON* capItem = cJSON_GetObjectItem(curItem, "capability");
    if (!cJSON_IsArray(capItem)) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] GetCaps failed, no caps array found.");
        return EC_INVALID;
    }

    // caps array empty, means do not need any capability
    int capsCnt = cJSON_GetArraySize(capItem);
    if (capsCnt <= 0) {
        return EC_SUCCESS;
    }

    if (capsCnt > MAX_CAPABILITY_COUNT) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] GetCaps, too many caps[cnt %{public}d], max %{public}d",
            capsCnt, MAX_CAPABILITY_COUNT);
        return EC_INVALID;
    }

    msgSt->caps = (unsigned int*)malloc(sizeof(unsigned int) * capsCnt);
    if (msgSt->caps == NULL) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] GetCaps, malloc failed! capsCnt[cnt %{public}d].", capsCnt);
        return EC_NOMEMORY;
    }

    for (int i = 0; i < capsCnt; ++i) {
        cJSON* capJ = cJSON_GetArrayItem(capItem, i);
        if (!cJSON_IsNumber(capJ) || cJSON_GetNumberValue(capJ) < 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] GetCaps, invalid cap value detected!");
            free(msgSt->caps);
            msgSt->caps = NULL;
            return EC_INVALID;
        }
        msgSt->caps[i] = (unsigned int)cJSON_GetNumberValue(capJ);
        if (msgSt->caps[i] > CAP_LAST_CAP) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] GetCaps, invalid cap value %{public}u detected!",\
                msgSt->caps[i]);
            free(msgSt->caps);
            msgSt->caps = NULL;
            return EC_INVALID;
        }
    }
    msgSt->capsCnt = capsCnt;
    return EC_SUCCESS;
}

int SplitMessage(const char* msg, unsigned int msgLen, MessageSt* msgSt)
{
    if (msgSt == NULL) {
        return EC_INVALID;
    }

    if (msg == NULL || msgLen == 0) {
        FreeMessageSt(msgSt);
        return EC_INVALID;
    }

    cJSON* rootJ = cJSON_ParseWithLength(msg, msgLen);
    if (rootJ == NULL) {
        FreeMessageSt(msgSt);
        return EC_PROTOCOL;
    }

    cJSON* bundleNameItem = cJSON_GetObjectItem(rootJ, "bundleName");
    int ret = (int)ReadStringItem(bundleNameItem, &(msgSt->bundleName), MAX_BUNDLE_NAME_LEN, MIN_BUNDLE_NAME_LEN);
    if (ret != EC_SUCCESS) {
        FreeMessageSt(msgSt);
        cJSON_Delete(rootJ);
        return ret;
    }

    cJSON* identityIDItem = cJSON_GetObjectItem(rootJ, "identityID");
    ret = (int)ReadStringItem(identityIDItem, &(msgSt->identityID), MAX_IDENTITY_ID_LEN, MIN_IDENTITY_ID_LEN);
    if (ret != EC_SUCCESS) {
        FreeMessageSt(msgSt);
        cJSON_Delete(rootJ);
        return ret;
    }

    cJSON* uIDItem = cJSON_GetObjectItem(rootJ, "uID");
    cJSON* gIDItem = cJSON_GetObjectItem(rootJ, "gID");
    msgSt->uID = (int)ReadNumberItem(uIDItem);
    msgSt->gID = (int)ReadNumberItem(gIDItem);

    ret = GetCaps(rootJ, msgSt);
    if (ret != EC_SUCCESS) {
        FreeMessageSt(msgSt);
        cJSON_Delete(rootJ);
        return ret;
    }

    cJSON_Delete(rootJ);

    if (msgSt->uID <= 0 || msgSt->gID <= 0 || msgSt->uID == INT_MAX || msgSt->gID == INT_MAX) {
        FreeMessageSt(msgSt);
        return EC_PROTOCOL;
    }
    return EC_SUCCESS;
}
