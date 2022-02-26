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

#include "iso_task_main.h"
#include "das_asy_token_manager.h"
#include "das_common.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_base_cur_task.h"
#include "iso_client_task.h"
#include "iso_server_task.h"

static int32_t UnregisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    (void)userType;
    const AlgLoader *loader = GetLoaderInstance();
    Uint8Buff pkgNameBuff = { (uint8_t *)pkgName, HcStrlen(pkgName) };
    Uint8Buff serviceTypeBuff = { (uint8_t *)serviceType, HcStrlen(serviceType) };

    uint8_t isoKeyAliasVal[ISO_KEY_ALIAS_LEN] = { 0 };
    Uint8Buff isoKeyAliasBuff = { isoKeyAliasVal, ISO_KEY_ALIAS_LEN };
    int32_t res = GenerateKeyAlias(&pkgNameBuff, &serviceTypeBuff, KEY_ALIAS_AUTH_TOKEN, authId, &isoKeyAliasBuff);
    if (res != HC_SUCCESS) {
        LOGE("Failed to generate authtoken alias!");
        return res;
    }
    res = loader->deleteKey(&isoKeyAliasBuff);
    if (res != HC_SUCCESS) {
        LOGE("Failed to delete authtoken!");
        return res;
    }
    LOGI("Authtoken deleted successfully!");

    return HC_SUCCESS;
}

static int32_t DeletePeerAuthInfo(const char *pkgName, const char *serviceType, Uint8Buff *authIdPeer, int userTypePeer)
{
    (void)userTypePeer;
    const AlgLoader *loader = GetLoaderInstance();
    Uint8Buff pkgNameBuff = { (uint8_t *)pkgName, HcStrlen(pkgName)};
    Uint8Buff serviceTypeBuff = { (uint8_t *)serviceType, HcStrlen(serviceType) };

    uint8_t isoKeyAliasVal[ISO_KEY_ALIAS_LEN] = { 0 };
    Uint8Buff isoKeyAliasBuff = { isoKeyAliasVal, ISO_KEY_ALIAS_LEN };
    int32_t res = GenerateKeyAlias(&pkgNameBuff, &serviceTypeBuff, KEY_ALIAS_AUTH_TOKEN, authIdPeer, &isoKeyAliasBuff);
    if (res != HC_SUCCESS) {
        LOGE("Failed to generate authtoken alias!");
        return res;
    }
    res = loader->deleteKey(&isoKeyAliasBuff);
    if (res != HC_SUCCESS) {
        LOGE("Failed to delete authtoken!");
        return res;
    }
    LOGI("Authtoken deleted successfully!");

    return HC_SUCCESS;
}

TokenManager g_symTokenManagerInstance = {
    NULL,
    UnregisterLocalIdentity,
    DeletePeerAuthInfo,
    NULL
};

bool IsIsoSupported(void)
{
    return true;
}

SubTaskBase *CreateIsoSubTask(const CJson *in, CJson *out)
{
    if (in == NULL || out == NULL) {
        return NULL;
    }
    bool isClient = true;
    if (GetBoolFromJson(in, FIELD_IS_CLIENT, &isClient) != HC_SUCCESS) {
        LOGE("Get isClient failed.");
        return NULL;
    }
    if (isClient) {
        return CreateIsoClientTask(in);
    } else {
        return CreateIsoServerTask(in, out);
    }
}

const TokenManager *GetSymTokenManagerInstance(void)
{
    return &g_symTokenManagerInstance;
}