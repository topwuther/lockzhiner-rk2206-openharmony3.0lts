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

#include "auth_session_util.h"
#include "account_related_group_auth.h"
#include "account_unrelated_group_auth.h"
#include "alg_loader.h"
#include "auth_session_common_defines.h"
#include "common_defs.h"
#include "common_util.h"
#include "device_auth_defines.h"
#include "hc_log.h"

static int32_t AuthFormToModuleType(int32_t authForm)
{
    int moduleType = INVALID_MODULE_TYPE;
    if (authForm == AUTH_FORM_ACCOUNT_UNRELATED) {
        moduleType = DAS_MODULE;
    } else if ((authForm == AUTH_FORM_IDENTICAL_ACCOUNT) || (authForm == AUTH_FORM_ACROSS_ACCOUNT)) {
        moduleType = TCIS_MODULE;
    } else {
        LOGE("Invalid auth form!");
    }
    return moduleType;
}

int32_t GetGroupAuth(int32_t groupAuthType, BaseGroupAuth **groupAuthHandle)
{
    switch (groupAuthType) {
        case ACCOUNT_UNRELATED_GROUP_AUTH_TYPE:
            LOGI("Non-account auth type.");
            *groupAuthHandle = GetNonAccountGroupAuth();
            break;
        case ACCOUNT_RELATED_GROUP_AUTH_TYPE:
            LOGI("Account-related auth type.");
            *groupAuthHandle = GetAccountRelatedGroupAuth();
            break;
        default:
            LOGE("Invalid auth type!");
            break;
    }
    if ((*groupAuthHandle) == NULL) {
        LOGE("Device auth do not support groupAuthType: %d, !", groupAuthType);
        return HC_ERR_NULL_PTR;
    }
    return HC_SUCCESS;
}

int32_t GetAuthModuleType(const CJson *in)
{
    int32_t authForm = AUTH_FORM_INVALID_TYPE;
    if (GetIntFromJson(in, FIELD_AUTH_FORM, &authForm) != HC_SUCCESS) {
        LOGE("Failed to get auth form!");
        return INVALID_MODULE_TYPE;
    }
    return AuthFormToModuleType(authForm);
}

int32_t GetInfoHash(const uint8_t *info, uint32_t infoLen, char *str, uint32_t strLen)
{
    Uint8Buff infoHash = {NULL, SHA256_LEN};
    Uint8Buff message = {NULL, infoLen};
    infoHash.val = (uint8_t *)HcMalloc(SHA256_LEN, 0);
    if (infoHash.val == NULL) {
        LOGE("Failed to malloc for info hash!");
        return HC_ERR_ALLOC_MEMORY;
    }
    message.val = (uint8_t *)HcMalloc(infoLen, 0);
    if (message.val == NULL) {
        LOGE("Failed to malloc for message hash!");
        HcFree(infoHash.val);
        infoHash.val = NULL;
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t res;
    do {
        if (memcpy_s(message.val, message.length, info, infoLen) != HC_SUCCESS) {
            LOGE("Failed to copy data!");
            res = HC_ERR_ALLOC_MEMORY;
            break;
        }
        res = GetLoaderInstance()->sha256(&message, &infoHash);
        if (res != HC_SUCCESS) {
            LOGE("Failed to sha256 message!");
            res = HC_ERROR;
            break;
        }
        if (ByteToHexString(infoHash.val, infoHash.length, str, strLen) != HC_SUCCESS) {
            LOGE("Failed to convert byte to string!");
            res = HC_ERROR;
            break;
        }
    } while (0);
    HcFree(message.val);
    message.val = NULL;
    HcFree(infoHash.val);
    infoHash.val = NULL;
    return res;
}
