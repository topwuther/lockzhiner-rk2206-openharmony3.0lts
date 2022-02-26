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

#include "account_unrelated_group_auth.h"
#include "auth_session_common.h"
#include "auth_session_common_util.h"
#include "common_defs.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "json_utils.h"

static void OnDasFinish(int64_t requestId, const CJson *authParam, const CJson *out,
    const DeviceAuthCallback *callback);
static int32_t FillNonAccountAuthInfo(const GroupInfo *entry, const DeviceInfo *localAuthInfo, CJson *paramsData);
static void OnDasError(int64_t requestId, const AuthSession *session, int errorCode);
static int32_t GetDasAuthParamForServer(CJson *dataFromClient, ParamsVec *authParamsVec);
static int32_t GetDasReqParams(const CJson *receiveData, CJson *reqParam);
static int32_t CombineDasServerConfirmParams(const CJson *confirmationJson, CJson *dataFromClient);

static NonAccountGroupAuth g_nonAccountGroupAuth = {
    .base.onFinish = OnDasFinish,
    .base.onError = OnDasError,
    .base.fillDeviceAuthInfo = FillNonAccountAuthInfo,
    .base.getAuthParamForServer = GetDasAuthParamForServer,
    .base.getReqParams = GetDasReqParams,
    .base.combineServerConfirmParams = CombineDasServerConfirmParams,
    .base.authType = ACCOUNT_UNRELATED_GROUP_AUTH_TYPE,
};

static int32_t AddGroupIdToSelfData(const CJson *authParam, CJson *returnToSelf)
{
    const char *groupId = GetStringFromJson(authParam, FIELD_SERVICE_TYPE);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from authParam!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(returnToSelf, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add group id!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPeerUdidToSelfData(const CJson *authParam, CJson *returnToSelf)
{
    const char *peerUdid = GetStringFromJson(authParam, FIELD_PEER_CONN_DEVICE_ID);
    if (peerUdid == NULL) {
        LOGE("Failed to get peerUdid from authParam!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(returnToSelf, FIELD_PEER_CONN_DEVICE_ID, peerUdid) != HC_SUCCESS) {
        LOGE("Failed to add peer udid!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddSelfTypeToSelfData(const CJson *authParam, CJson *returnToSelf)
{
    int32_t userType = 0;
    if (GetIntFromJson(authParam, FIELD_SELF_TYPE, &userType) != HC_SUCCESS) {
        LOGE("Failed to get self userType from authParam!");
        return HC_ERR_JSON_GET;
    }
    if (AddIntToJson(returnToSelf, FIELD_USER_TYPE, userType) != HC_SUCCESS) {
        LOGE("Failed to add peer self userType for onFinish!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPeerTypeToSelfData(const CJson *authParam, CJson *returnToSelf)
{
    int32_t peerUserType = 0;
    if (GetIntFromJson(authParam, FIELD_PEER_USER_TYPE, &peerUserType) != HC_SUCCESS) {
        LOGE("Failed to get peerUserType from authParam!");
        return HC_ERR_JSON_GET;
    }
    if (AddIntToJson(returnToSelf, FIELD_PEER_USER_TYPE, peerUserType) != HC_SUCCESS) {
        LOGE("Failed to add peer peerUserType for onFinish!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddSessionKeyToSelfData(const CJson *authParam, const CJson *sendToSelf, CJson *returnToSelf)
{
    int32_t keyLen = DEFAULT_RETURN_KEY_LENGTH;
    (void)GetIntFromJson(authParam, FIELD_KEY_LENGTH, &keyLen);
    uint8_t *sessionKey = (uint8_t *)HcMalloc(keyLen, 0);
    if (sessionKey == NULL) {
        LOGE("Failed to allocate memory for sessionKey!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t res = HC_SUCCESS;
    do {
        if (GetByteFromJson(sendToSelf, FIELD_SESSION_KEY, sessionKey, keyLen) != HC_SUCCESS) {
            LOGE("Failed to get sessionKey!");
            res = HC_ERR_JSON_GET;
            break;
        }
        if (AddByteToJson(returnToSelf, FIELD_SESSION_KEY, (const uint8_t *)sessionKey, keyLen) != HC_SUCCESS) {
            LOGE("Failed to add sessionKey for onFinish!");
            res = HC_ERR_JSON_FAIL;
            break;
        }
    } while (0);
    (void)memset_s(sessionKey, keyLen, 0, keyLen);
    HcFree(sessionKey);
    sessionKey = NULL;
    return res;
}

static int32_t PrepareDasReturnToSelfData(const CJson *authParam, const CJson *sendToSelf, CJson *returnToSelf)
{
    int32_t res = AddGroupIdToSelfData(authParam, returnToSelf);
    if (res != HC_SUCCESS) {
        return res;
    }
    res = AddPeerUdidToSelfData(authParam, returnToSelf);
    if (res != HC_SUCCESS) {
        return res;
    }
    res = AddSelfTypeToSelfData(authParam, returnToSelf);
    if (res != HC_SUCCESS) {
        return res;
    }
    res = AddPeerTypeToSelfData(authParam, returnToSelf);
    if (res != HC_SUCCESS) {
        return res;
    }
    res = AddSessionKeyToSelfData(authParam, sendToSelf, returnToSelf);
    if (res != HC_SUCCESS) {
        return res;
    }
    return HC_SUCCESS;
}

static int32_t DasOnFinishToPeer(int64_t requestId, const CJson *out, const DeviceAuthCallback *callback)
{
    int32_t res = HC_SUCCESS;
    const CJson *sendToPeer = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    if (sendToPeer == NULL) {
        LOGI("No need to transmit data to peer.");
        return res;
    }
    char *sendToPeerStr = PackJsonToString(sendToPeer);
    if (sendToPeerStr == NULL) {
        LOGE("Failed to pack sendToPeerStr for onTransmit!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if ((callback != NULL) && (callback->onTransmit != NULL)) {
        if (!callback->onTransmit(requestId, (uint8_t *)sendToPeerStr, (uint32_t)strlen(sendToPeerStr) + 1)) {
            LOGE("Failed to transmit data to peer!");
            res = HC_ERR_TRANSMIT_FAIL;
        }
    }
    FreeJsonString(sendToPeerStr);
    return res;
}

static int32_t DasOnFinishToSelf(int64_t requestId, const CJson *authParam, const CJson *out,
    const DeviceAuthCallback *callback)
{
    const CJson *sendToSelf = GetObjFromJson(out, FIELD_SEND_TO_SELF);
    if (sendToSelf == NULL) {
        LOGE("No data to send to self for onFinish.");
        return HC_ERR_LOST_DATA;
    }
    CJson *returnToSelf = CreateJson();
    if (returnToSelf == NULL) {
        LOGE("Create json failed!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t res = PrepareDasReturnToSelfData(authParam, sendToSelf, returnToSelf);
    if (res != HC_SUCCESS) {
        LOGE("Failed to add das returnToSelf data!");
        ClearSensitiveStringInJson(returnToSelf, FIELD_SESSION_KEY);
        FreeJson(returnToSelf);
        return res;
    }
    char *returnStr = PackJsonToString(returnToSelf);
    ClearSensitiveStringInJson(returnToSelf, FIELD_SESSION_KEY);
    FreeJson(returnToSelf);
    if (returnStr == NULL) {
        LOGE("Failed to pack returnToSelf for onFinish!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if ((callback != NULL) && (callback->onFinish != NULL)) {
        callback->onFinish(requestId, AUTH_FORM_ACCOUNT_UNRELATED, returnStr);
    }
    ClearAndFreeJsonString(returnStr);
    return res;
}

static int32_t AddNonAccountPkgName(const GroupInfo *entry, CJson *paramsData)
{
    int32_t res = HC_SUCCESS;
    int32_t groupType = entry->type;
    if (groupType == COMPATIBLE_GROUP) {
        if (AddStringToJson(paramsData, FIELD_SERVICE_PKG_NAME, StringGet(&entry->ownerName))
            != HC_SUCCESS) {
            LOGE("Failed to add ownerName to json!");
            res = HC_ERR_JSON_FAIL;
        }
    } else {
        if (AddStringToJson(paramsData, FIELD_SERVICE_PKG_NAME, GROUP_MANAGER_PACKAGE_NAME) != HC_SUCCESS) {
            LOGE("Failed to add group manager name to json!");
            res = HC_ERR_JSON_FAIL;
        }
    }
    return res;
}

static int32_t AddPeerUdidIfPossible(const char *peerUdid, const DeviceInfo *peerAuthInfo, CJson *paramsData)
{
    if (peerUdid != NULL) {
        return HC_SUCCESS;
    }
    const char *peerUdidFromDb = StringGet(&peerAuthInfo->udid);
    if (peerUdidFromDb == NULL) {
        LOGE("Peer device's udid in db is null!");
        return HC_ERR_DB;
    }
    if (AddStringToJson(paramsData, FIELD_PEER_CONN_DEVICE_ID, peerUdidFromDb) != HC_SUCCESS) {
        LOGE("Failed to add peer udid in db to paramsData!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddNonAccountAuthInfo(const DeviceInfo *localAuthInfo, const DeviceInfo *peerAuthInfo,
    CJson *paramsData)
{
    int32_t keyLen = DEFAULT_RETURN_KEY_LENGTH;
    (void)GetIntFromJson(paramsData, FIELD_KEY_LENGTH, &keyLen);
    if (AddIntToJson(paramsData, FIELD_KEY_LENGTH, keyLen) != HC_SUCCESS) {
        LOGE("Failed to add keyLen for client auth!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(paramsData, FIELD_SELF_AUTH_ID, StringGet(&localAuthInfo->authId))
        != HC_SUCCESS) {
        LOGE("Failed to add self authId to paramsData!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(paramsData, FIELD_SELF_TYPE, localAuthInfo->devType) != HC_SUCCESS) {
        LOGE("Failed to add self devType to paramsData!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(paramsData, FIELD_PEER_AUTH_ID, StringGet(&peerAuthInfo->authId))
        != HC_SUCCESS) {
        LOGE("Failed to add peer authId to paramsData!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(paramsData, FIELD_PEER_USER_TYPE, peerAuthInfo->devType) != HC_SUCCESS) {
        LOGE("Failed to add peer devType to paramsData!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static void OnDasError(int64_t requestId, const AuthSession *session, int errorCode)
{
    const DeviceAuthCallback *callback = session->base.callback;
    ParamsVec list = session->paramsList;
    CJson *authParam = list.get(&list, session->currentIndex);
    if (authParam == NULL) {
        LOGE("The json data in session is null!");
        return;
    }
    /* If there is alternative group, do not return error. */
    const char *altGroup = GetStringFromJson(authParam, FIELD_ALTERNATIVE);
    if ((session->currentIndex < (list.size(&list) - 1)) || (altGroup != NULL)) {
        return;
    }
    CJson *returnData = CreateJson();
    if (returnData == NULL) {
        LOGE("Failed to create json for returnData!");
        return;
    }
    int32_t res = AddGroupIdToSelfData(authParam, returnData);
    if (res != HC_SUCCESS) {
        FreeJson(returnData);
        return;
    }
    res = AddPeerUdidToSelfData(authParam, returnData);
    if (res != HC_SUCCESS) {
        FreeJson(returnData);
        return;
    }
    char *returnStr = PackJsonToString(returnData);
    FreeJson(returnData);
    if (returnStr == NULL) {
        LOGE("Failed to pack returnStr for onError!");
        return;
    }
    if ((callback != NULL) && (callback->onError != NULL)) {
        callback->onError(requestId, AUTH_FORM_ACCOUNT_UNRELATED, errorCode, returnStr);
    }
    FreeJsonString(returnStr);
}

static int32_t FillNonAccountAuthInfo(const GroupInfo *entry, const DeviceInfo *localAuthInfo, CJson *paramsData)
{
    int32_t res;
    const char *groupId = StringGet(&entry->id);
    DeviceInfo *peerAuthInfo = CreateDeviceInfoStruct();
    if (peerAuthInfo == NULL) {
        LOGE("Failed to allocate devEntry memory for peerAuthInfo!");
        return HC_ERR_ALLOC_MEMORY;
    }
    const char *peerUdid = GetStringFromJson(paramsData, FIELD_PEER_CONN_DEVICE_ID);
    const char *peeAuthId = GetStringFromJson(paramsData, FIELD_PEER_AUTH_ID);
    if (peerUdid != NULL) {
        res = GetDeviceInfoForDevAuth(peerUdid, groupId, peerAuthInfo);
    } else if (peeAuthId != NULL) {
        res = GetDeviceInfoByAuthId(peeAuthId, groupId, peerAuthInfo);
    } else {
        LOGE("Invalid input, both peer udid and peer authId are null!");
        res = HC_ERR_NULL_PTR;
    }
    do {
        if (res != HC_SUCCESS) {
            LOGE("Failed to get peer device info from database!");
            break;
        }
        res = AddNonAccountPkgName(entry, paramsData);
        if (res != HC_SUCCESS) {
            LOGE("Failed to add pkg name to paramsData!");
            break;
        }
        res = AddPeerUdidIfPossible(peerUdid, peerAuthInfo, paramsData);
        if (res != HC_SUCCESS) {
            LOGE("Failed to add peer udid!");
            break;
        }
        res = AddNonAccountAuthInfo(localAuthInfo, peerAuthInfo, paramsData);
        if (res != HC_SUCCESS) {
            LOGE("Failed to add device auth info for non-account group!");
            break;
        }
    } while (0);
    DestroyDeviceInfoStruct(peerAuthInfo);
    return res;
}

static int32_t GetDasReqParams(const CJson *receiveData, CJson *reqParam)
{
    int32_t res = GetGeneralReqParams(receiveData, reqParam);
    if (res != HC_SUCCESS) {
        LOGE("Failed to get general request params!");
        return res;
    }
    bool receiveDeviceLevel = false;
    (void)GetBoolFromJson(receiveData, FIELD_IS_DEVICE_LEVEL, &receiveDeviceLevel);
    if (receiveDeviceLevel) {
        const char *altGroup = GetStringFromJson(receiveData, FIELD_ALTERNATIVE);
        if (altGroup != NULL && AddStringToJson(reqParam, FIELD_ALTERNATIVE, altGroup) != HC_SUCCESS) {
            LOGE("Failed to add alternativeGroup to reqParam!");
            return HC_ERR_JSON_FAIL;
        }
    }
    if (AddBoolToJson(reqParam, FIELD_IS_DEVICE_LEVEL, receiveDeviceLevel) != HC_SUCCESS) {
        LOGE("Failed to add reqParam for onRequest!");
        return HC_ERR_JSON_FAIL;
    }
    return res;
}

static int32_t CombineDasServerConfirmParams(const CJson *confirmationJson, CJson *dataFromClient)
{
    bool isClient = false;
    if (AddBoolToJson(dataFromClient, FIELD_IS_CLIENT, isClient) != HC_SUCCESS) {
        LOGE("Failed to combine server param for isClient!");
        return HC_ERR_JSON_FAIL;
    }
    const char *pkgName = GetStringFromJson(confirmationJson, FIELD_SERVICE_PKG_NAME);
    if (pkgName != NULL) {
        if (AddStringToJson(dataFromClient, FIELD_SERVICE_PKG_NAME, pkgName) != HC_SUCCESS) {
            LOGE("Failed to combine server param for pkgName!");
            return HC_ERR_JSON_FAIL;
        }
    }

    const char *peerUdid = GetStringFromJson(confirmationJson, FIELD_PEER_CONN_DEVICE_ID);
    if (peerUdid != NULL) {
        if (AddStringToJson(dataFromClient, FIELD_PEER_CONN_DEVICE_ID, peerUdid) != HC_SUCCESS) {
            LOGE("Failed to combine server param for peerUdid!");
            return HC_ERR_JSON_FAIL;
        }
    }
    const char *peerAuthId = GetStringFromJson(confirmationJson, FIELD_PEER_AUTH_ID);
    if (peerAuthId != NULL) {
        if (AddStringToJson(dataFromClient, FIELD_PEER_AUTH_ID, peerAuthId) != HC_SUCCESS) {
            LOGE("Failed to combine server param for peerAuthId!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t GetDasAuthParamForServer(CJson *dataFromClient, ParamsVec *authParamsVec)
{
    LOGI("Begin get non-account auth params for server.");
    int32_t res = GetAuthParamsList(dataFromClient, authParamsVec);
    if (res != HC_SUCCESS) {
        LOGE("Failed to get non-account auth params!");
    }
    return res;
}

static void OnDasFinish(int64_t requestId, const CJson *authParam, const CJson *out,
    const DeviceAuthCallback *callback)
{
    LOGI("Begin call onFinish for non-account auth.");
    if (DasOnFinishToPeer(requestId, out, callback) != HC_SUCCESS) {
        LOGE("Failed to send data to peer when auth finished!");
        return;
    }
    if (ReturnSessionKey(requestId, authParam, out, callback) != HC_SUCCESS) {
        LOGE("Failed to return session key when auth finished!");
        return;
    }
    if (DasOnFinishToSelf(requestId, authParam, out, callback) != HC_SUCCESS) {
        LOGE("Failed to send data to self when auth finished!");
        return;
    }
    LOGI("Call onFinish for non-account auth successfully.");
}

BaseGroupAuth *GetNonAccountGroupAuth()
{
    return (BaseGroupAuth *)&g_nonAccountGroupAuth;
}
