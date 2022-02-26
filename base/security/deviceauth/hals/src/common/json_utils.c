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

#include "json_utils.h"
#include "cJSON.h"
#include "common_util.h"
#include "hc_error.h"
#include "hc_log.h"
#include "hc_types.h"

#define RECURSE_FLAG_TRUE 1

CJson *CreateJsonFromString(const char *jsonStr)
{
    if (jsonStr == NULL) {
        LOGE("Param is null.");
        return NULL;
    }
    return cJSON_Parse(jsonStr);
}

CJson *CreateJson(void)
{
    return cJSON_CreateObject();
}

CJson *CreateJsonArray(void)
{
    return cJSON_CreateArray();
}

CJson *DuplicateJson(const CJson *jsonObj)
{
    if (jsonObj == NULL) {
        LOGE("Param is null.");
        return NULL;
    }
    return cJSON_Duplicate(jsonObj, RECURSE_FLAG_TRUE);
}

void FreeJson(CJson *jsonObj)
{
    cJSON_Delete(jsonObj);
}

void DeleteItemFromJson(CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return;
    }
    cJSON_DeleteItemFromObjectCaseSensitive(jsonObj, key);
}

void DeleteAllItemExceptOne(CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return;
    }

    CJson *curItem = jsonObj->child;
    CJson *nextItem = NULL;
    while (curItem != NULL) {
        nextItem = curItem->next;
        if (strcmp(key, curItem->string) != 0) {
            cJSON_Delete(cJSON_DetachItemViaPointer(jsonObj, curItem));
        }
        curItem = nextItem;
    }
}

void DeleteAllItem(CJson *jsonObj)
{
    if (jsonObj == NULL) {
        LOGE("Param is null.");
        return;
    }

    CJson *curItem = jsonObj->child;
    CJson *nextItem = NULL;
    while (curItem != NULL) {
        nextItem = curItem->next;
        cJSON_Delete(cJSON_DetachItemViaPointer(jsonObj, curItem));
        curItem = nextItem;
    }
}

CJson *DetachItemFromJson(CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return NULL;
    }

    return cJSON_DetachItemFromObjectCaseSensitive(jsonObj, key);
}

char *PackJsonToString(const CJson *jsonObj)
{
    if (jsonObj == NULL) {
        LOGE("Param is null.");
        return NULL;
    }
    return cJSON_PrintUnformatted(jsonObj);
}

void FreeJsonString(char *jsonStr)
{
    cJSON_free(jsonStr);
}

int GetItemNum(const CJson *jsonObj)
{
    if (jsonObj == NULL) {
        LOGE("Param is null.");
        return 0;
    }
    return cJSON_GetArraySize(jsonObj);
}

const char *GetItemKey(const CJson *item)
{
    if (item == NULL) {
        LOGE("Param is null.");
        return NULL;
    }
    return item->string;
}

CJson *GetObjFromJson(const CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return NULL;
    }

    cJSON *objValue = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (objValue != NULL) {
        return objValue;
    }

    int len = cJSON_GetArraySize(jsonObj);
    for (int i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(jsonObj, i);
        if (cJSON_IsObject(item)) {
            cJSON *obj = GetObjFromJson(item, key);
            if (obj != NULL) {
                return obj;
            }
        }
    }

    return NULL;
}

CJson *GetItemFromArray(const CJson *jsonArr, int index)
{
    if (jsonArr == NULL) {
        LOGE("Param is null.");
        return NULL;
    }
    return cJSON_GetArrayItem(jsonArr, index);
}

const char *GetStringFromJson(const CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return NULL;
    }

    cJSON *jsonObjTmp = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (jsonObjTmp != NULL && cJSON_IsString(jsonObjTmp)) {
        return cJSON_GetStringValue(jsonObjTmp);
    }

    int len = cJSON_GetArraySize(jsonObj);
    for (int i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(jsonObj, i);
        if (cJSON_IsObject(item)) {
            const char *resValue = GetStringFromJson(item, key);
            if (resValue != NULL) {
                return resValue;
            }
        }
    }

    return NULL;
}

int32_t GetByteFromJson(const CJson *jsonObj, const char *key, uint8_t *byte, uint32_t len)
{
    if (jsonObj == NULL || key == NULL || byte == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    const char *valueStr = GetStringFromJson(jsonObj, key);
    if (valueStr == NULL) {
        return HAL_ERR_JSON_GET;
    }
    if (len < strlen(valueStr) / BYTE_TO_HEX_OPER_LENGTH) {
        LOGE("Invalid length.");
        return HAL_ERR_INVALID_LEN;
    }
    int32_t ret = HexStringToByte(valueStr, byte, len);
    if (ret != HAL_SUCCESS) {
        LOGE("Convert hex string to byte failed.");
        return ret;
    }

    return ret;
}

int32_t GetIntFromJson(const CJson *jsonObj, const char *key, int *value)
{
    if (jsonObj == NULL || key == NULL || value == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *jsonObjTmp = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (jsonObjTmp != NULL && cJSON_IsNumber(jsonObjTmp)) {
        *value = (int)cJSON_GetNumberValue(jsonObjTmp);
        return HAL_SUCCESS;
    }

    int len = cJSON_GetArraySize(jsonObj);
    for (int i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(jsonObj, i);
        if (cJSON_IsObject(item)) {
            int32_t ret = GetIntFromJson(item, key, value);
            if (ret == HAL_SUCCESS) {
                return ret;
            }
        }
    }

    return HAL_ERR_JSON_GET;
}

int32_t GetUnsignedIntFromJson(const CJson *jsonObj, const char *key, uint32_t *value)
{
    if (jsonObj == NULL || key == NULL || value == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *jsonObjTmp = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (jsonObjTmp != NULL && cJSON_IsNumber(jsonObjTmp)) {
        double realValue = cJSON_GetNumberValue(jsonObjTmp);
        if (realValue < 0) {
            int32_t tmpValue = (int32_t)realValue;
            *value = (uint32_t)tmpValue;
        } else {
            *value = (uint32_t)realValue;
        }
        return HAL_SUCCESS;
    }

    int len = cJSON_GetArraySize(jsonObj);
    for (int i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(jsonObj, i);
        if (cJSON_IsObject(item)) {
            int32_t ret = GetUnsignedIntFromJson(item, key, value);
            if (ret == HAL_SUCCESS) {
                return ret;
            }
        }
    }

    return HAL_ERR_JSON_GET;
}

int32_t GetInt64FromJson(const CJson *jsonObj, const char *key, int64_t *value)
{
    const char *str = GetStringFromJson(jsonObj, key);
    if (str == NULL) {
        LOGE("Failed to get int64_t from json!");
        return HAL_ERR_JSON_GET;
    }
    *value = StringToInt64(str);
    return HAL_SUCCESS;
}

int32_t GetBoolFromJson(const CJson *jsonObj, const char *key, bool *value)
{
    if (jsonObj == NULL || key == NULL || value == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *jsonObjTmp = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (jsonObjTmp != NULL && cJSON_IsBool(jsonObjTmp)) {
        *value = cJSON_IsTrue(jsonObjTmp) ? true : false;
        return HAL_SUCCESS;
    }

    int len = cJSON_GetArraySize(jsonObj);
    for (int i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(jsonObj, i);
        if (cJSON_IsObject(item)) {
            int32_t ret = GetBoolFromJson(item, key, value);
            if (ret == HAL_SUCCESS) {
                return ret;
            }
        }
    }

    return HAL_ERR_JSON_GET;
}

char *GetStringValue(const CJson *item)
{
    return cJSON_GetStringValue(item);
}

int32_t AddObjToJson(CJson *jsonObj, const char *key, const CJson *childObj)
{
    if (jsonObj == NULL || key == NULL || childObj == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *tmpObj = cJSON_Duplicate(childObj, RECURSE_FLAG_TRUE);
    if (tmpObj == NULL) {
        LOGE("Duplicate json object failed.");
        return HAL_ERR_JSON_DUPLICATE;
    }

    cJSON *objInJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (objInJson == NULL) {
        if (cJSON_AddItemToObject(jsonObj, key, tmpObj) == false) {
            LOGE("Add object to json failed.");
            cJSON_Delete(tmpObj);
            return HAL_ERR_JSON_ADD;
        }
    } else {
        if (cJSON_ReplaceItemInObjectCaseSensitive(jsonObj, key, tmpObj) == false) {
            LOGE("Replace object in json failed.");
            cJSON_Delete(tmpObj);
            return HAL_ERR_JSON_REPLACE;
        }
    }

    return HAL_SUCCESS;
}

int32_t AddObjToArray(CJson *jsonArr, CJson *item)
{
    if (jsonArr == NULL || item == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    if (cJSON_IsArray(jsonArr) == false) {
        LOGE("Object is not array.");
        return HAL_ERR_INVALID_PARAM;
    }

    bool ret = cJSON_AddItemToArray(jsonArr, item);
    if (ret == false) {
        LOGE("Add object to array failed.");
        return HAL_ERR_JSON_ADD;
    }

    return HAL_SUCCESS;
}

int32_t AddStringToArray(CJson *jsonArr, const char *string)
{
    if (jsonArr == NULL || string == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    if (cJSON_IsArray(jsonArr) == false) {
        LOGE("Object is not array.");
        return HAL_ERR_INVALID_PARAM;
    }

    cJSON *strObj = cJSON_CreateString(string);
    if (strObj == NULL) {
        LOGE("Create string json object failed.");
        return HAL_ERR_BAD_ALLOC;
    }
    bool ret = cJSON_AddItemToArray(jsonArr, strObj);
    if (ret == false) {
        LOGE("Add string to array failed.");
        cJSON_Delete(strObj);
        return HAL_ERR_JSON_ADD;
    }

    return HAL_SUCCESS;
}

int32_t AddStringToJson(CJson *jsonObj, const char *key, const char *value)
{
    if (jsonObj == NULL || key == NULL || value == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *objInJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (objInJson == NULL) {
        if (cJSON_AddStringToObject(jsonObj, key, value) == NULL) {
            LOGE("Add string to json failed.");
            return HAL_ERR_JSON_GET;
        }
    } else {
        cJSON *tmp = cJSON_CreateString(value);
        if (tmp == NULL) {
            LOGE("Create string json object failed.");
            return HAL_ERR_BAD_ALLOC;
        }
        if (cJSON_ReplaceItemInObjectCaseSensitive(jsonObj, key, tmp) == false) {
            LOGE("Replace string in json failed.");
            cJSON_Delete(tmp);
            return HAL_ERR_JSON_REPLACE;
        }
    }

    return HAL_SUCCESS;
}

int32_t AddByteToJson(CJson *jsonObj, const char *key, const uint8_t *byte, uint32_t len)
{
    if (jsonObj == NULL || key == NULL || byte == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    uint32_t hexLen = len * BYTE_TO_HEX_OPER_LENGTH + 1;
    char *hexStr = (char *)HcMalloc(hexLen, 0);
    if (hexStr == NULL) {
        LOGE("Malloc hex string failed.");
        return HAL_ERR_BAD_ALLOC;
    }
    int32_t ret = ByteToHexString(byte, len, hexStr, hexLen);
    if (ret != HAL_SUCCESS) {
        LOGE("Convert byte to hex string failed.");
        HcFree(hexStr);
        return ret;
    }

    ret = AddStringToJson(jsonObj, key, hexStr);
    if (ret != HAL_SUCCESS) {
        LOGE("Add string to json failed.");
        HcFree(hexStr);
        return ret;
    }

    HcFree(hexStr);
    return HAL_SUCCESS;
}

int32_t AddBoolToJson(CJson *jsonObj, const char *key, bool value)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *objInJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (objInJson == NULL) {
        if (cJSON_AddBoolToObject(jsonObj, key, value) == NULL) {
            LOGE("Add bool to json failed.");
            return HAL_ERR_JSON_GET;
        }
    } else {
        cJSON *tmp = cJSON_CreateBool(value);
        if (tmp == NULL) {
            LOGE("Create bool json object failed.");
            return HAL_ERR_BAD_ALLOC;
        }
        if (cJSON_ReplaceItemInObjectCaseSensitive(jsonObj, key, tmp) == false) {
            LOGE("Repalce bool in json failed.");
            cJSON_Delete(tmp);
            return HAL_ERR_JSON_REPLACE;
        }
    }

    return HAL_SUCCESS;
}

int32_t AddIntToJson(CJson *jsonObj, const char *key, int value)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *objInJson = cJSON_GetObjectItemCaseSensitive(jsonObj, key);
    if (objInJson == NULL) {
        if (cJSON_AddNumberToObject(jsonObj, key, value) == NULL) {
            LOGE("Add int to json failed.");
            return HAL_ERR_JSON_GET;
        }
    } else {
        cJSON *tmp = cJSON_CreateNumber(value);
        if (tmp == NULL) {
            LOGE("Create int json object failed.");
            return HAL_ERR_BAD_ALLOC;
        }
        if (cJSON_ReplaceItemInObjectCaseSensitive(jsonObj, key, tmp) == false) {
            LOGE("Replace int in json failed.");
            cJSON_Delete(tmp);
            return HAL_ERR_JSON_REPLACE;
        }
    }

    return HAL_SUCCESS;
}

int32_t AddInt64StringToJson(CJson *jsonObj, const char *key, int64_t value)
{
    char buffer[65] = { 0 };
    if (sprintf_s(buffer, sizeof(buffer), "%" PRId64, value) <= 0) {
        LOGE("Failed to convert int64_t to string!");
        return HAL_FAILED;
    }
    if (AddStringToJson(jsonObj, key, buffer) != HAL_SUCCESS) {
        LOGE("Failed to add int64_t string to json!");
        return HAL_ERR_JSON_ADD;
    }
    return HAL_SUCCESS;
}

int32_t AddStringArrayToJson(CJson *jsonObj, const char *key, const char * const *stringArray, uint32_t arrayLen)
{
    if (jsonObj == NULL || key == NULL || stringArray == NULL) {
        LOGE("Param is null.");
        return HAL_ERR_NULL_PTR;
    }

    cJSON *strArrayObj = cJSON_CreateStringArray(stringArray, arrayLen);
    if (strArrayObj == NULL) {
        LOGE("Create string array object failed.");
        return HAL_ERR_BAD_ALLOC;
    }
    if (cJSON_AddItemToObject(jsonObj, key, strArrayObj) == false) {
        LOGE("Add string array to json failed.");
        cJSON_Delete(strArrayObj);
        return HAL_ERR_JSON_ADD;
    }
    return HAL_SUCCESS;
}

void ClearSensitiveStringInJson(CJson *jsonObj, const char *key)
{
    if (jsonObj == NULL || key == NULL) {
        LOGE("Param is null.");
        return;
    }
    char *str = (char *)GetStringFromJson(jsonObj, key);
    if (str == NULL) {
        LOGD("Not find this value in json.");
        return;
    }
    (void)memset_s(str, strlen(str), 0, strlen(str));
}

void ClearAndFreeJsonString(char *jsonStr)
{
    if (jsonStr == NULL) {
        LOGE("Param is invalid.");
        return;
    }
    (void)memset_s(jsonStr, strlen(jsonStr), 0, strlen(jsonStr));
    FreeJsonString(jsonStr);
}
