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

#include "auth_session_common.h"
#include "account_related_group_auth.h"
#include "account_unrelated_group_auth.h"
#include "alg_defs.h"
#include "auth_session_util.h"
#include "common_defs.h"
#include "common_util.h"
#include "dev_auth_module_manager.h"
#include "hc_log.h"
#include "json_utils.h"

IMPLEMENT_HC_VECTOR(ParamsVec, void *, 1)

static bool IsOldFormatParams(const CJson *param)
{
    int32_t authForm = AUTH_FORM_INVALID_TYPE;
    (void)GetIntFromJson(param, FIELD_AUTH_FORM, &authForm);
    int64_t uid = 0;
    (void)GetByteFromJson(param, FIELD_USER_ID, (uint8_t *)&uid, sizeof(int64_t));
    bool isOldFormat = ((GetStringFromJson(param, FIELD_SELF_AUTH_ID) != NULL) ||
        (uid != 0L) || (authForm == AUTH_FORM_IDENTICAL_ACCOUNT));
    return isOldFormat;
}

static int32_t UnifyOldFormatParams(const CJson *param, ParamsVec *paramsVec)
{
    LOGI("Unify old format parameters.");
    CJson *oldFormatParams = DuplicateJson(param);
    if (oldFormatParams == NULL) {
        LOGE("Failed to create json for old form params!");
        return HC_ERR_JSON_FAIL;
    }
    int64_t uid = 0;
    uint32_t groupIdLen = SHA256_LEN * BYTE_TO_HEX_OPER_LENGTH + 1;
    char groupId[SHA256_LEN * BYTE_TO_HEX_OPER_LENGTH + 1] = { 0 };
    int32_t res = HC_SUCCESS;
    do {
        if (GetByteFromJson(param, FIELD_USER_ID, (uint8_t *)&uid, sizeof(int64_t)) != HC_SUCCESS) {
            LOGI("No uid in auth param!");
        }
        if (uid == 0L) {
            break;
        }
        if (GetInfoHash((const uint8_t *)&uid, sizeof(int64_t), groupId, groupIdLen) != HC_SUCCESS) {
            LOGE("Failed to get hash for groupId!");
            res = HC_ERR_HASH_FAIL;
            break;
        }
        if (AddStringToJson(oldFormatParams, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
            LOGE("Failed to add groupId to json params!");
            res = HC_ERR_JSON_FAIL;
            break;
        }
    } while (0);
    if (res != HC_SUCCESS) {
        FreeJson(oldFormatParams);
        return res;
    }
    paramsVec->pushBack(paramsVec, (const void **)&oldFormatParams);
    return res;
}

static bool IsGroupAvailable(const char *groupId, const char *pkgName)
{
    if (IsGroupAccessible(groupId, pkgName)) {
        return true;
    }
    LOGI("%s don't have enough right for group: %s!", pkgName, groupId);
    return false;
}

static int32_t GroupTypeToAuthForm(int32_t groupType)
{
    int32_t authForm;
    switch (groupType) {
        case PEER_TO_PEER_GROUP:
        case COMPATIBLE_GROUP:
            authForm = AUTH_FORM_ACCOUNT_UNRELATED;
            break;
        case IDENTICAL_ACCOUNT_GROUP:
            authForm = AUTH_FORM_IDENTICAL_ACCOUNT;
            break;
        case ACROSS_ACCOUNT_AUTHORIZE_GROUP:
            authForm = AUTH_FORM_ACROSS_ACCOUNT;
            break;
        default:
            LOGE("Invalid group type!");
            authForm = AUTH_FORM_INVALID_TYPE;
            break;
    }
    return authForm;
}

static int32_t AddGeneralParams(const char *groupId, int32_t groupType, const DeviceInfo *localAuthInfo,
    CJson *paramsData)
{
    int32_t res = HC_SUCCESS;
    int32_t authForm = GroupTypeToAuthForm(groupType);
    const char *serviceType = StringGet(&(localAuthInfo->serviceType));
    do {
        if (AddStringToJson(paramsData, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
            LOGE("Failed to add groupId for client auth!");
            res = HC_ERR_JSON_FAIL;
            break;
        }
        if (AddIntToJson(paramsData, FIELD_AUTH_FORM, authForm) != HC_SUCCESS) {
            LOGE("Failed to add authFrom for client auth!");
            res = HC_ERR_JSON_FAIL;
            break;
        }
        if ((groupType == COMPATIBLE_GROUP) && (serviceType != NULL)) {
            if (AddStringToJson(paramsData, FIELD_SERVICE_TYPE, serviceType) != HC_SUCCESS) {
                LOGE("Failed to add serviceType for client compatible group auth!");
                res = HC_ERR_JSON_FAIL;
                break;
            }
        } else {
            if (AddStringToJson(paramsData, FIELD_SERVICE_TYPE, groupId) != HC_SUCCESS) {
                LOGE("Failed to add serviceType with groupId for client auth!");
                res = HC_ERR_JSON_FAIL;
                break;
            }
        }
    } while (0);
    return res;
}

static int32_t GetLocalDeviceInfoFromDatabase(const char *groupId, DeviceInfo *localAuthInfo)
{
    int32_t res;
    char *localUdidStr = NULL;
    if (GetLocalDevUdid(&localUdidStr) != HC_SUCCESS) {
        LOGE("Failed to get local udid!");
        return HC_ERROR;
    }
    res = GetDeviceInfoForDevAuth(localUdidStr, groupId, localAuthInfo);
    if (res != HC_SUCCESS) {
        LOGE("Failed to get local device info from database!");
    }
    DestroyUdid(&localUdidStr);
    return res;
}

static int32_t ExtractAndAddParams(const char *groupId, const GroupInfo *groupInfo, CJson *paramsData)
{
    int32_t res;
    DeviceInfo *localAuthInfo = CreateDeviceInfoStruct();
    if (localAuthInfo == NULL) {
        LOGE("Failed to allocate memory for peerAuthInfo!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t groupType = groupInfo->type;
    int32_t authForm = GroupTypeToAuthForm(groupType);
    do {
        res = GetLocalDeviceInfoFromDatabase(groupId, localAuthInfo);
        if (res != HC_SUCCESS) {
            break;
        }
        res = AddGeneralParams(groupId, groupType, localAuthInfo, paramsData);
        if (res != HC_SUCCESS) {
            LOGE("Failed to add general params!");
            break;
        }
        BaseGroupAuth *groupAuth = NULL;
        res = GetGroupAuth(GetGroupAuthType(authForm), &groupAuth);
        if (res != HC_SUCCESS) {
            LOGE("Failed to get group auth handle!");
            break;
        }
        res = groupAuth->fillDeviceAuthInfo(groupInfo, localAuthInfo, paramsData);
        if (res != HC_SUCCESS) {
            LOGE("Failed to fill device auth info!");
            break;
        }
    } while (0);
    DestroyDeviceInfoStruct(localAuthInfo);
    return res;
}

static int32_t FillAuthParams(const CJson *param, const GroupInfoVec *vec, ParamsVec *paramsVec)
{
    const char *pkgName = GetStringFromJson(param, FIELD_SERVICE_PKG_NAME);
    if (pkgName == NULL) {
        LOGE("Pkg name is null, can't extract params from db!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, ptr) {
        if ((ptr == NULL) || (*ptr == NULL)) {
            continue;
        }
        const GroupInfo *groupInfo = (GroupInfo *)(*ptr);
        const char *groupId = StringGet(&(groupInfo->id));
        if (groupId == NULL) {
            continue;
        }
        if (!IsGroupAvailable(groupId, pkgName)) {
            continue;
        }
        CJson *paramsData = DuplicateJson(param);
        if (paramsData == NULL) {
            LOGE("Failed to duplicate auth param data!");
            return HC_ERR_JSON_FAIL;
        }
        if (ExtractAndAddParams(groupId, groupInfo, paramsData) == HC_SUCCESS) {
            paramsVec->pushBack(paramsVec, (const void **)&paramsData);
        }
    }
    return HC_SUCCESS;
}

static void GetCandidateGroupByOrder(const CJson *param, GroupQueryParams *queryParams, GroupInfoVec *vec)
{
    BaseGroupAuth *groupAuth = NULL;
    if (GetGroupAuth(ACCOUNT_RELATED_GROUP_AUTH_TYPE, &groupAuth) == HC_SUCCESS) {
        AccountRelatedGroupAuth *realGroupAuth = (AccountRelatedGroupAuth *)groupAuth;
        realGroupAuth->getTcisCandidateGroup(param, queryParams, vec);
    }
    queryParams->type = PEER_TO_PEER_GROUP;
    (void)GetJoinedGroupInfoVecByDevId(queryParams, vec);
    queryParams->type = COMPATIBLE_GROUP;
    (void)GetJoinedGroupInfoVecByDevId(queryParams, vec);
}

static int32_t InitGroupQueryParams(const char *peerUdid, const char *peerAuthId, GroupQueryParams *queryParams)
{
    queryParams->type = ALL_GROUP_TYPE;
    queryParams->visibility = ANY_GROUP_VISIBILITY;
    if (peerUdid != NULL) {
        queryParams->udid = (char *)HcMalloc(HcStrlen(peerUdid) + 1, 0);
        if (queryParams->udid == NULL) {
            LOGE("Failed to allocate memory for queryParams of udid!");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (strcpy_s(queryParams->udid, HcStrlen(peerUdid) + 1, peerUdid) != HC_SUCCESS) {
            LOGE("Failed to copy udid for queryParams!");
            HcFree(queryParams->udid);
            queryParams->udid = NULL;
            return HC_ERR_MEMORY_COPY;
        }
    } else if (peerAuthId != NULL) {
        queryParams->authId = (char *)HcMalloc(HcStrlen(peerAuthId) + 1, 0);
        if (queryParams->authId == NULL) {
            LOGE("Failed to allocate memory for queryParams of authId!");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (strcpy_s(queryParams->authId, HcStrlen(peerAuthId) + 1, peerAuthId) != HC_SUCCESS) {
            LOGE("Failed to copy authId for queryParams!");
            HcFree(queryParams->authId);
            queryParams->authId = NULL;
            return HC_ERR_MEMORY_COPY;
        }
    }
    return HC_SUCCESS;
}

static void DestroyGroupQueryParams(const char *peerUdid, const char *peerAuthId, GroupQueryParams *queryParams)
{
    if (peerUdid != NULL) {
        HcFree(queryParams->udid);
        queryParams->udid = NULL;
    } else if (peerAuthId != NULL) {
        HcFree(queryParams->authId);
        queryParams->authId = NULL;
    }
}

static void GetCandidateGroupInfo(const CJson *param, const char *peerUdid,
    const char *peerAuthId, GroupInfoVec *vec)
{
    LOGI("No input of groupId, extract group info without groupId.");
    bool deviceLevelFlag = false;
    bool isClient = true;
    (void)GetBoolFromJson(param, FIELD_IS_DEVICE_LEVEL, &deviceLevelFlag);
    if (GetBoolFromJson(param, FIELD_IS_CLIENT, &isClient) != HC_SUCCESS) {
        LOGE("Failed to get the value: isClient!");
        return;
    }
    GroupQueryParams queryParams = { 0 };
    if (InitGroupQueryParams(peerUdid, peerAuthId, &queryParams) != HC_SUCCESS) {
        LOGE("Failed to init group query params!");
        return;
    }
    if (deviceLevelFlag && isClient) {
        GetCandidateGroupByOrder(param, &queryParams, vec);
    } else {
        queryParams.visibility = GROUP_VISIBILITY_PUBLIC;
        GetCandidateGroupByOrder(param, &queryParams, vec);
    }
    DestroyGroupQueryParams(peerUdid, peerAuthId, &queryParams);
}

static void GetGroupInfoByGroupId(const char *groupId, const char *peerUdid, const char *peerAuthId, GroupInfoVec *vec)
{
    GroupInfo *entry = CreateGroupInfoStruct();
    if (entry == NULL) {
        LOGE("Failed to create entry struct!");
        return;
    }
    int32_t res = HC_SUCCESS;
    if (peerUdid != NULL) {
        res = GetGroupEntry(groupId, peerUdid, entry);
    } else {
        if (IsTrustedDeviceInGroupByAuthId(groupId, peerAuthId)) {
            res = GetGroupEntryByGroupId(groupId, entry);
        }
    }
    if (res == HC_SUCCESS) {
        vec->pushBackT(vec, (void *)entry);
    } else {
        LOGE("Failed to get group entry!");
        DestroyGroupInfoStruct(entry);
    }
}

static int32_t GetCandidateAuthInfo(const char *groupId, const CJson *param, ParamsVec *authParamsVec)
{
    const char *peerUdid = GetStringFromJson(param, FIELD_PEER_CONN_DEVICE_ID);
    const char *peerAuthId = GetStringFromJson(param, FIELD_PEER_AUTH_ID);
    if ((peerUdid == NULL) && (peerAuthId == NULL)) {
        LOGE("Invalid input, peer udid and peer auth id are both null!");
        return HC_ERR_INVALID_PARAMS;
    }
    GroupInfoVec vec;
    CreateGroupInfoVecStruct(&vec);
    if (groupId == NULL) {
        GetCandidateGroupInfo(param, peerUdid, peerAuthId, &vec);
    } else {
        GetGroupInfoByGroupId(groupId, peerUdid, peerAuthId, &vec);
    }
    if (vec.size(&vec) == 0) {
        LOGE("No satisfied candidate group!");
        DestroyGroupInfoVecStruct(&vec);
        return HC_ERR_NO_CANDIDATE_GROUP;
    }
    int32_t ret = FillAuthParams(param, &vec, authParamsVec);
    DestroyGroupInfoVecStruct(&vec);
    return ret;
}

static int32_t AddGroupAuthTransmitData(const AuthSession *session, CJson *sendToPeer)
{
    ParamsVec list = session->paramsList;
    CJson *authParam = list.get(&list, session->currentIndex);
    if (authParam == NULL) {
        LOGE("The json data in session is null!");
        return HC_ERR_NULL_PTR;
    }
    bool isDeviceLevel = false;
    /* Disable device-level auth. */
    if (AddBoolToJson(sendToPeer, FIELD_IS_DEVICE_LEVEL, isDeviceLevel) != HC_SUCCESS) {
        LOGE("Failed to add device level!");
        return HC_ERR_JSON_FAIL;
    }
    bool isClient = true;
    if (GetBoolFromJson(authParam, FIELD_IS_CLIENT, &isClient)) {
        LOGE("Failed to get isClient!");
        return HC_ERR_JSON_GET;
    }
    if (isClient && (session->currentIndex < (list.size(&list) - 1))) {
        const char *altGroup = GetStringFromJson(list.get(&list, session->currentIndex + 1), FIELD_SERVICE_TYPE);
        if ((altGroup != NULL) && (AddStringToJson(sendToPeer, FIELD_ALTERNATIVE, altGroup) != HC_SUCCESS)) {
            LOGE("Failed to add alternative group!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t ReturnTransmitData(const AuthSession *session, CJson *out)
{
    CJson *sendToPeer = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    if (sendToPeer == NULL) {
        LOGI("The transmit data to peer is null!");
        return HC_ERR_JSON_GET;
    }
    CJson *authParam = (session->paramsList).get(&(session->paramsList), session->currentIndex);
    if (authParam == NULL) {
        LOGE("The json data in session is null!");
        return HC_ERR_NULL_PTR;
    }
    int64_t requestId = 0;
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to get request id!");
        return HC_ERR_JSON_GET;
    }

    int32_t ret = AddGroupAuthTransmitData(session, sendToPeer);
    if (ret != HC_SUCCESS) {
        LOGE("Failed to add extra data!");
        return ret;
    }
    char *outStr = PackJsonToString(sendToPeer);
    if (outStr == NULL) {
        LOGE("Failed to pack outStr for onTransmit!");
        return HC_ERR_ALLOC_MEMORY;
    }

    const DeviceAuthCallback *callback = session->base.callback;
    do {
        if ((callback == NULL) || (callback->onTransmit == NULL)) {
            LOGE("The callback for transmit is null!");
            ret = HC_ERR_TRANSMIT_FAIL;
            break;
        }
        if (!callback->onTransmit(requestId, (uint8_t *)outStr, (uint32_t)strlen(outStr) + 1)) {
            LOGE("Failed to transmit data to peer!");
            ret = HC_ERR_TRANSMIT_FAIL;
        }
    } while (0);
    FreeJsonString(outStr);
    return ret;
}

int32_t GetAuthParamsList(const CJson *param, ParamsVec *authParamsVec)
{
    int32_t ret;
    const char *groupId = GetStringFromJson(param, FIELD_GROUP_ID);
    if (groupId == NULL) {
        groupId = GetStringFromJson(param, FIELD_SERVICE_TYPE);
    }
    if (IsOldFormatParams(param)) {
        LOGI("The input params' type is in old format!");
        ret = UnifyOldFormatParams(param, authParamsVec);
    } else {
        ret = GetCandidateAuthInfo(groupId, param, authParamsVec);
    }
    return ret;
}

static void ReturnFinishData(const AuthSession *session, const CJson *out)
{
    if (out == NULL) {
        LOGE("The return data is null!");
        return;
    }
    ParamsVec list = session->paramsList;
    const CJson *authParam = list.get(&list, session->currentIndex);
    if (authParam == NULL) {
        LOGE("The json data in session is null!");
        return;
    }
    int64_t requestId = 0;
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to get request id!");
        return;
    }
    int32_t authForm = AUTH_FORM_INVALID_TYPE;
    if (GetIntFromJson(authParam, FIELD_AUTH_FORM, &authForm) != HC_SUCCESS) {
        LOGE("Failed to get auth type!");
        return;
    }
    BaseGroupAuth *groupAuth = NULL;
    if (GetGroupAuth(GetGroupAuthType(authForm), &groupAuth) == HC_SUCCESS) {
        groupAuth->onFinish(requestId, authParam, out, session->base.callback);
    }
}

static int32_t ReturnErrorToLocalBySession(const AuthSession *session, int errorCode)
{
    ParamsVec list = session->paramsList;
    CJson *authParam = list.get(&list, session->currentIndex);
    int64_t requestId = 0;
    int32_t authForm = 0;
    if (authParam == NULL) {
        LOGE("The json data in session is null!");
        return HC_ERR_NULL_PTR;
    }
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to add request id!");
        return HC_ERR_JSON_GET;
    }
    if (GetIntFromJson(authParam, FIELD_AUTH_FORM, &authForm) != HC_SUCCESS) {
        LOGE("Failed to add auth form!");
        return HC_ERR_JSON_GET;
    }

    BaseGroupAuth *groupAuth = NULL;
    int32_t res = GetGroupAuth(GetGroupAuthType(authForm), &groupAuth);
    if (res == HC_SUCCESS) {
        groupAuth->onError(requestId, session, errorCode);
    }
    return res;
}

static int32_t ReturnErrorToPeerBySession(const CJson *authParam, const DeviceAuthCallback *callback)
{
    int64_t requestId = 0;
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to get request ID!");
        return HC_ERR_JSON_GET;
    }
    CJson *errorToPeer = CreateJson();
    if (errorToPeer == NULL) {
        LOGE("Failed to allocate memory for errorToPeer!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (AddIntToJson(errorToPeer, FIELD_GROUP_ERROR_MSG, GROUP_ERR_MSG) != HC_SUCCESS) {
        LOGE("Failed to add err message to return data!");
        FreeJson(errorToPeer);
        return HC_ERR_JSON_FAIL;
    }
    char *errorToPeerStr = PackJsonToString(errorToPeer);
    FreeJson(errorToPeer);
    if (errorToPeerStr == NULL) {
        LOGE("Failed to pack errorToPeer to string!");
        return HC_ERR_ALLOC_MEMORY;
    }

    int32_t res = HC_SUCCESS;
    do {
        if ((callback == NULL) || (callback->onTransmit == NULL)) {
            LOGE("The callback of onTransmit is null!");
            res = HC_ERR_NULL_PTR;
            break;
        }
        if (!callback->onTransmit(requestId, (uint8_t *)errorToPeerStr, (uint32_t)strlen(errorToPeerStr) + 1)) {
            LOGE("Failed to invoke onTransmit!");
            res = HC_ERR_TRANSMIT_FAIL;
            break;
        }
    } while (0);
    FreeJsonString(errorToPeerStr);
    return res;
}

static int32_t ReturnErrorToPeerByTask(const CJson *sendToPeer, const CJson *authParam,
    const DeviceAuthCallback *callback)
{
    if (sendToPeer == NULL) {
        LOGD("NO need to send data to peer!");
        return HC_SUCCESS;
    }
    int64_t requestId = 0;
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to get request id!");
        return HC_ERR_JSON_FAIL;
    }
    char *sendToPeerStr = PackJsonToString(sendToPeer);
    if (sendToPeerStr == NULL) {
        LOGE("Failed to pack json to string!");
        return HC_ERR_ALLOC_MEMORY;
    }

    int32_t res = HC_SUCCESS;
    do {
        if ((callback == NULL) || (callback->onTransmit == NULL)) {
            LOGE("The callback of onTransmit is null!");
            res = HC_ERR_NULL_PTR;
            break;
        }
        if (!callback->onTransmit(requestId, (uint8_t *)sendToPeerStr, (uint32_t)strlen(sendToPeerStr) + 1)) {
            LOGE("Failed to invoke onTransmit!");
            res = HC_ERR_TRANSMIT_FAIL;
            break;
        }
    } while (0);
    FreeJsonString(sendToPeerStr);
    return res;
}

bool HasAlternativeAuthGroup(const AuthSession *session)
{
    if (session->currentIndex >= session->paramsList.size(&session->paramsList) - 1) {
        return false;
    }
    return true;
}

static int32_t ProcessNextGroupIfPossible(AuthSession *session)
{
    if (!HasAlternativeAuthGroup(session)) {
        LOGD("There is no alternative auth group.");
        return HC_ERR_NO_CANDIDATE_GROUP;
    }
    int32_t res;
    session->currentIndex++;
    CJson *paramInNextSession = (session->paramsList).get(&(session->paramsList), session->currentIndex);
    if (paramInNextSession == NULL) {
        LOGE("The json data in session is null!");
        return HC_ERR_NULL_PTR;
    }
    CJson *outNext = CreateJson();
    do {
        if (outNext == NULL) {
            LOGE("Failed to create json for outNext!");
            res = HC_ERR_ALLOC_MEMORY;
            break;
        }
        int32_t status = 0;
        res = CreateAndProcessTask(session, paramInNextSession, outNext, &status);
        if (res != HC_SUCCESS) {
            break;
        }
        res = ProcessTaskStatusForAuth(session, paramInNextSession, outNext, status);
    } while (0);
    if (res != HC_SUCCESS) {
        DestroyTask(session->curTaskId, GetAuthModuleType(paramInNextSession));
        res = InformAuthError(session, outNext, res);
    }
    FreeJson(outNext);
    return res;
}

int32_t ReturnSessionKey(int64_t requestId, const CJson *authParam,
    const CJson *out, const DeviceAuthCallback *callback)
{
    LOGI("Begin return session key.");
    int32_t keyLen = DEFAULT_RETURN_KEY_LENGTH;
    (void)GetIntFromJson(authParam, FIELD_KEY_LENGTH, &keyLen);
    uint8_t *sessionKey = (uint8_t *)HcMalloc(keyLen, 0);
    if (sessionKey == NULL) {
        LOGE("Failed to allocate memory for sessionKey!");
        return HC_ERR_ALLOC_MEMORY;
    }

    int32_t res = HC_SUCCESS;
    do {
        if (GetByteFromJson(out, FIELD_SESSION_KEY, sessionKey, keyLen) != HC_SUCCESS) {
            LOGE("Failed to get sessionKey!");
            res = HC_ERR_JSON_GET;
            break;
        }
        if ((callback == NULL) || (callback->onSessionKeyReturned == NULL)) {
            LOGE("The callback of onSessionKeyReturned is null!");
            res = HC_ERR_INVALID_PARAMS;
            break;
        }
        callback->onSessionKeyReturned(requestId, sessionKey, keyLen);
    } while (0);
    (void)memset_s(sessionKey, keyLen, 0, keyLen);
    HcFree(sessionKey);
    sessionKey = NULL;
    LOGI("End return session key, res = %d.", res);
    return res;
}

void DeleteCachedData(CJson *paramInSession)
{
    DeleteItemFromJson(paramInSession, FIELD_PAYLOAD);
    DeleteItemFromJson(paramInSession, FIELD_SELF_AUTH_ID);
    DeleteItemFromJson(paramInSession, FIELD_PEER_AUTH_ID);
    DeleteItemFromJson(paramInSession, FIELD_OPERATION_CODE);
}

int32_t GetGroupAuthType(int32_t authForm)
{
    switch (authForm) {
        case AUTH_FORM_ACCOUNT_UNRELATED:
            return ACCOUNT_UNRELATED_GROUP_AUTH_TYPE;
        case AUTH_FORM_IDENTICAL_ACCOUNT:
        case AUTH_FORM_ACROSS_ACCOUNT:
            return ACCOUNT_RELATED_GROUP_AUTH_TYPE;
        default:
            LOGE("Invalid authForm!");
            return INVALID_GROUP_AUTH_TYPE;
    }
}

int32_t InformAuthError(AuthSession *session, const CJson *out, int errorCode)
{
    ParamsVec list = session->paramsList;
    CJson *paramInSession = list.get(&list, session->currentIndex);
    if (paramInSession == NULL) {
        LOGE("The json data in session is null!");
        return HC_ERR_NULL_PTR;
    }
    int32_t res;
    if (out == NULL) {
        res = ReturnErrorToPeerBySession(paramInSession, session->base.callback);
        LOGI("Out data is null, so assemble error msg to peer by auth session.");
        return res;
    }
    const CJson *sendToPeer = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    res = ReturnErrorToPeerByTask(sendToPeer, paramInSession, session->base.callback);
    if (res != HC_SUCCESS) {
        LOGE("Failed to return task's error msg to peer!");
        return res;
    }

    res = ProcessNextGroupIfPossible(session);
    if (res == HC_SUCCESS) {
        return HC_SUCCESS;
    }

    const char *altGroup = GetStringFromJson(paramInSession, FIELD_ALTERNATIVE);
    if (altGroup == NULL) {
        res = ReturnErrorToLocalBySession(session, errorCode);
    }
    return res;
}

void ProcessDeviceLevel(const CJson *receiveData, CJson *authParam)
{
    bool receiveLevel = false;
    bool authLevel = false;
    (void)GetBoolFromJson(receiveData, FIELD_IS_DEVICE_LEVEL, &receiveLevel);
    (void)GetBoolFromJson(authParam, FIELD_IS_DEVICE_LEVEL, &authLevel);
    authLevel = receiveLevel && authLevel;
    if (AddBoolToJson(authParam, FIELD_IS_DEVICE_LEVEL, authLevel) != HC_SUCCESS) {
        LOGE("Failed to add device level to auth param!");
        return;
    }
}

int32_t ProcessTaskStatusForAuth(const AuthSession *session, const CJson *param, CJson *out, int32_t status)
{
    int32_t res = HC_SUCCESS;
    switch (status) {
        case IGNORE_MSG:
            LOGI("Ignore this msg.");
            break;
        case CONTINUE:
            res = ReturnTransmitData(session, out);
            if (res != HC_SUCCESS) {
                LOGE("Failed to transmit data to peer!");
                InformLocalAuthError(param, session->base.callback);
            }
            break;
        case FINISH:
            ReturnFinishData(session, out);
            ClearSensitiveStringInJson(out, FIELD_SESSION_KEY);
            res = FINISH;
            break;
        default:
            LOGE("Invalid status after process task!");
            res = HC_ERR_INVALID_PARAMS;
            break;
    }
    return res;
}

int32_t CreateAndProcessTask(AuthSession *session, CJson *paramInSession, CJson *out, int32_t *status)
{
    int32_t moduleType = GetAuthModuleType(paramInSession);
    const char *pkgName = GetStringFromJson(paramInSession, FIELD_SERVICE_PKG_NAME);
    if (AddStringToJson(paramInSession, FIELD_PKG_NAME, pkgName) != HC_SUCCESS) {
        LOGE("Failed to add pkg name to json!");
        return HC_ERR_JSON_FAIL;
    }
    session->curTaskId = 0;
    int32_t res = CreateTask(&(session->curTaskId), paramInSession, out, moduleType);
    if (res != HC_SUCCESS) {
        LOGE("Failed to create task for auth!");
        return res;
    }
    res = ProcessTask(session->curTaskId, paramInSession, out, status, moduleType);
    DeleteCachedData(paramInSession);
    if (res != HC_SUCCESS) {
        LOGE("Failed to process task for auth!");
        return res;
    }
    return HC_SUCCESS;
}

void InformLocalAuthError(const CJson *authParam, const DeviceAuthCallback *callback)
{
    bool isClient = false;
    (void)GetBoolFromJson(authParam, FIELD_IS_CLIENT, &isClient);
    const char *altGroup = GetStringFromJson(authParam, FIELD_ALTERNATIVE);
    if (!isClient && (altGroup != NULL)) { /* Server wait for next group auth. */
        LOGI("Server wait for next group auth.");
        return;
    }
    int64_t requestId = 0;
    if (GetByteFromJson(authParam, FIELD_REQUEST_ID, (uint8_t *)&requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to get request id!");
        return;
    }
    if ((callback != NULL) && (callback->onError != NULL)) {
        callback->onError(requestId, AUTH_FORM_INVALID_TYPE, HC_ERR_CREATE_SESSION_FAIL, NULL);
    }
}

void InformPeerAuthError(const CJson *authParam, const DeviceAuthCallback *callback)
{
    if (ReturnErrorToPeerBySession(authParam, callback) != HC_SUCCESS) {
        LOGE("Failed to return error to peer by session!");
    }
}

void DestroyAuthSession(Session *session)
{
    if (session == NULL) {
        return;
    }
    AuthSession *realSession = (AuthSession *)session;
    CJson *paramInSession = (realSession->paramsList).get(&(realSession->paramsList), realSession->currentIndex);
    if (paramInSession == NULL) {
        LOGE("The json data in session is null!");
        return;
    }
    DestroyTask(realSession->curTaskId, GetAuthModuleType(paramInSession));

    uint32_t index;
    void **paramsData = NULL;
    FOR_EACH_HC_VECTOR(realSession->paramsList, index, paramsData) {
        if (paramsData != NULL) {
            FreeJson((CJson *)*paramsData);
        }
    }
    DESTROY_HC_VECTOR(ParamsVec, &(realSession->paramsList))
    HcFree(realSession);
    realSession = NULL;
}

void CreateAuthParamsVec(ParamsVec *vec)
{
    *vec = CREATE_HC_VECTOR(ParamsVec)
}

void DestroyAuthParamsVec(ParamsVec *vec)
{
    DESTROY_HC_VECTOR(ParamsVec, vec)
}
