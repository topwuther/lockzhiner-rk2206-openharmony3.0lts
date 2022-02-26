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

#include "version_util.h"
#include "common_util.h"
#include "hc_log.h"
#include "hc_types.h"
#include "protocol_common.h"
#include "securec.h"

static const char *Split(char *str, char delim, int *next)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == delim) {
            *next = *next + i + 1;
            str[i] = '\0';
            return str;
        }
    }
    return str;
}

int32_t StringToVersion(const char* verStr, uint32_t len, VersionStruct* version)
{
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");
    CHECK_PTR_RETURN_ERROR_CODE(verStr, "verStr");
    if (*(verStr + len) != '\0') {
        return HC_ERR_INVALID_PARAMS;
    }

    const char *subVer = NULL;
    int next = 0;

    char *verStrTmp = (char *)HcMalloc(len + 1, 0);
    if (verStrTmp == NULL) {
        LOGE("Malloc verStrTmp failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(verStrTmp, len + 1, verStr, len) != EOK) {
        LOGE("Memcpy verStrTmp failed.");
        HcFree(verStrTmp);
        return HC_ERR_MEMORY_COPY;
    }

    subVer = Split(verStrTmp, '.', &next);
    if (subVer == NULL) {
        goto err;
    }
    version->first = (uint32_t)strtoul(subVer, NULL, DEC);

    subVer = Split(verStrTmp + next, '.', &next);
    if (subVer == NULL) {
        goto err;
    }
    version->second = (uint32_t)strtoul(subVer, NULL, DEC);

    subVer = Split(verStrTmp + next, '.', &next);
    if (subVer == NULL) {
        goto err;
    }
    version->third = (uint32_t)strtoul(subVer, NULL, DEC);

    HcFree(verStrTmp);
    return HC_SUCCESS;
err:
    LOGE("Split failed");
    HcFree(verStrTmp);
    return HC_ERR_NULL_PTR;
}

int32_t VersionToString(const VersionStruct *version, char *verStr, uint32_t len)
{
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");
    CHECK_PTR_RETURN_ERROR_CODE(verStr, "verStr");

    char tmpStr[TMP_VERSION_STR_LEN] = { 0 };
    if (sprintf_s(tmpStr, TMP_VERSION_STR_LEN, "%d.%d.%d", version->first, version->second, version->third) == -1) {
        return HC_ERROR;
    }

    if (memcpy_s(verStr, len, tmpStr, strlen(tmpStr) + 1) != 0) {
        return HC_ERR_MEMORY_COPY;
    }

    return HC_SUCCESS;
}

int32_t AddSingleVersionToJson(CJson *jsonObj, const VersionStruct *version)
{
    CHECK_PTR_RETURN_ERROR_CODE(jsonObj, "jsonObj");
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");

    char versionStr[TMP_VERSION_STR_LEN] = { 0 };
    int32_t ret = VersionToString(version, versionStr, TMP_VERSION_STR_LEN);
    if (ret != HC_SUCCESS) {
        return HC_ERROR;
    }

    CJson *sendToPeer = GetObjFromJson(jsonObj, FIELD_SEND_TO_PEER);
    if (sendToPeer == NULL) {
        LOGD("There is not sendToPeer in json.");
        return HC_SUCCESS;
    }
    if (AddStringToJson(sendToPeer, FIELD_GROUP_AND_MODULE_VERSION, versionStr) != HC_SUCCESS) {
        LOGE("Add group and module version to sendToPeer failed.");
        return HC_ERR_JSON_ADD;
    }
    return HC_SUCCESS;
}

int32_t GetSingleVersionFromJson(const CJson* jsonObj, VersionStruct *version)
{
    CHECK_PTR_RETURN_ERROR_CODE(jsonObj, "jsonObj");
    CHECK_PTR_RETURN_ERROR_CODE(version, "version");

    const char *versionStr = GetStringFromJson(jsonObj, FIELD_GROUP_AND_MODULE_VERSION);
    CHECK_PTR_RETURN_ERROR_CODE(versionStr, "versionStr");

    int32_t ret = StringToVersion(versionStr, strlen(versionStr), version);
    if (ret != HC_SUCCESS) {
        return HC_ERROR;
    }
    return HC_SUCCESS;
}

void InitGroupAndModuleVersion(VersionStruct *version)
{
    if (version == NULL) {
        LOGE("version is invalid.");
        return;
    }
    version->first = VERSION_FIRST_BIT;
    version->second = 0;
    version->third = 0;
}