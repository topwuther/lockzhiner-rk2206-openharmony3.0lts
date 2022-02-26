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

#include "peer_to_peer_group.h"
#include "alg_defs.h"
#include "callback_manager.h"
#include "channel_manager.h"
#include "common_util.h"
#include "database_manager.h"
#include "group_common.h"
#include "hc_dev_info.h"
#include "hc_log.h"
#include "session_manager.h"

static int32_t CheckGroupName(const char *appId, const CJson *jsonParams)
{
    const char *groupName = GetStringFromJson(jsonParams, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from jsonParams!");
        return HC_ERR_JSON_GET;
    }

    if (IsGroupExist(appId, groupName)) {
        LOGE("A group with the same group name has been created! [AppId]: %s, [GroupName]: %s", appId, groupName);
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

static int32_t GenerateGroupId(const CJson *jsonParams, char **returnGroupId)
{
    const char *groupName = GetStringFromJson(jsonParams, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    /* peer to peer group: groupId = sha256(groupName | appId) */
    uint8_t *hashMessage = NULL;
    uint32_t messageSize = 0;
    Uint8Buff groupNameBuff = {(uint8_t *)groupName, HcStrlen(groupName)};
    Uint8Buff appIdBuff = {(uint8_t *)appId, HcStrlen(appId)};
    int32_t result = GetHashMessage(&groupNameBuff, &appIdBuff, &hashMessage, &messageSize);
    if (result != HC_SUCCESS) {
        return result;
    }
    int hashStrLen = SHA256_LEN * BYTE_TO_HEX_OPER_LENGTH + 1;
    *returnGroupId = (char *)HcMalloc(hashStrLen, 0);
    result = GetHashResult(hashMessage, messageSize, *returnGroupId, hashStrLen);
    HcFree(hashMessage);
    if (result != HC_SUCCESS) {
        LOGE("Failed to get hash for groupId! [AppId]: %s, [GroupName]: %s", appId, groupName);
        HcFree(*returnGroupId);
        *returnGroupId = NULL;
        return HC_ERR_HASH_FAIL;
    }
    LOGI("Generate groupId successfully! [AppId]: %s, [GroupName]: %s", appId, groupName);
    return HC_SUCCESS;
}

static int32_t CheckCreateParams(const CJson *jsonParams)
{
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    int32_t result;
    if (((result = CheckGroupName(appId, jsonParams)) != HC_SUCCESS) ||
        ((result = CheckUserTypeIfExist(jsonParams)) != HC_SUCCESS) ||
        ((result = CheckGroupVisibilityIfExist(jsonParams)) != HC_SUCCESS) ||
        ((result = CheckExpireTimeIfExist(jsonParams)) != HC_SUCCESS) ||
        ((result = CheckGroupNumLimit(PEER_TO_PEER_GROUP, appId)) != HC_SUCCESS)) {
        return result;
    }
    LOGI("The group creation parameters transferred by the user are valid! [AppId]: %s", appId);
    return HC_SUCCESS;
}

static int32_t GenerateGroupParams(const CJson *jsonParams, const char *groupId, GroupInfo *groupParams)
{
    const char *groupName = GetStringFromJson(jsonParams, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    int32_t result;
    if (((result = AddGroupTypeToParams(PEER_TO_PEER_GROUP, groupParams)) != HC_SUCCESS) ||
        ((result = AddGroupNameToParams(groupName, groupParams)) != HC_SUCCESS) ||
        ((result = AddGroupIdToParams(groupId, groupParams)) != HC_SUCCESS) ||
        ((result = AddGroupOwnerToParams(appId, groupParams)) != HC_SUCCESS) ||
        ((result = AddGroupVisibilityOrDefault(groupParams)) != HC_SUCCESS) ||
        ((result = AddExpireTimeOrDefault(jsonParams, groupParams)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t GenerateDevParams(const CJson *jsonParams, const char *groupId, DeviceInfo *devParams)
{
    int32_t result;
    if (((result = AddUdidToParams(devParams)) != HC_SUCCESS) ||
        ((result = AddAuthIdToParamsOrDefault(jsonParams, devParams))) ||
        ((result = AddUserTypeToParamsOrDefault(jsonParams, devParams)) != HC_SUCCESS) ||
        ((result = AddGroupIdToDevParams(groupId, devParams)) != HC_SUCCESS) ||
        ((result = AddServiceTypeToParams(groupId, devParams)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t CreateGroupInner(const CJson *jsonParams, char **returnGroupId)
{
    char *groupId = NULL;
    int32_t result;
    if (((result = CheckCreateParams(jsonParams)) != HC_SUCCESS) ||
        ((result = GenerateGroupId(jsonParams, &groupId)) != HC_SUCCESS) ||
        ((result = ProcessKeyPair(CREATE_KEY_PAIR, jsonParams, groupId)) != HC_SUCCESS) ||
        ((result = AddGroupToDatabaseByJson(GenerateGroupParams, jsonParams, groupId)) != HC_SUCCESS) ||
        ((result = AddDeviceToDatabaseByJson(GenerateDevParams, jsonParams, groupId)) != HC_SUCCESS)) {
        HcFree(groupId);
        return result;
    }
    *returnGroupId = groupId;
    return HC_SUCCESS;
}

static int32_t GetPeerUserType(const char *groupId, const char *peerAuthId)
{
    int peerUserType = DEVICE_TYPE_ACCESSORY;
    DeviceInfo *devAuthParams = CreateDeviceInfoStruct();
    if (devAuthParams == NULL) {
        LOGE("Failed to allocate devEntry memory!");
        return peerUserType;
    }
    if (GetDeviceInfoByAuthId(peerAuthId, groupId, devAuthParams) != HC_SUCCESS) {
        LOGE("Failed to obtain the device information from the database!");
        DestroyDeviceInfoStruct(devAuthParams);
        return peerUserType;
    }
    peerUserType = devAuthParams->devType;
    DestroyDeviceInfoStruct(devAuthParams);
    return peerUserType;
}

static int32_t HandleLocalUnbind(int64_t requestId, const CJson *jsonParams, const DeviceAuthCallback *callback)
{
    const char *peerAuthId = GetStringFromJson(jsonParams, FIELD_DELETE_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    int32_t peerUserType = GetPeerUserType(groupId, peerAuthId);
    int32_t result = DelTrustedDeviceByAuthId(peerAuthId, groupId);
    if (result != HC_SUCCESS) {
        LOGE("Failed to delete trust device from database!");
        return result;
    }
    /*
     * If the trusted device has been deleted from the database but the peer key fails to be deleted,
     * the forcible unbinding is still considered successful. Only logs need to be printed.
     */
    result = DeletePeerKeyIfForceUnbind(groupId, peerAuthId, peerUserType);
    if (result != HC_SUCCESS) {
        LOGD("Failed to delete peer key!");
    }
    char *returnDataStr = NULL;
    result = GenerateUnbindSuccessData(peerAuthId, groupId, &returnDataStr);
    if (result != HC_SUCCESS) {
        return result;
    }
    ProcessFinishCallback(requestId, MEMBER_DELETE, returnDataStr, callback);
    FreeJsonString(returnDataStr);
    return HC_SUCCESS;
}

static int32_t AddAuthIdAndUserTypeToParams(const char *groupId, CJson *jsonParams)
{
    DeviceInfo *devAuthParams = CreateDeviceInfoStruct();
    if (devAuthParams == NULL) {
        LOGE("Failed to allocate devEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    if (GetDeviceInfoForDevAuth((const char *)udid, groupId, devAuthParams) != HC_SUCCESS) {
        LOGE("Failed to obtain the device information from the database!");
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_DB;
    }
    if (AddStringToJson(jsonParams, FIELD_DEVICE_ID, StringGet(&devAuthParams->authId)) != HC_SUCCESS) {
        LOGE("Failed to add authId to params!");
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(jsonParams, FIELD_USER_TYPE, devAuthParams->devType) != HC_SUCCESS) {
        LOGE("Failed to add userType to params!");
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_JSON_FAIL;
    }
    DestroyDeviceInfoStruct(devAuthParams);
    return HC_SUCCESS;
}

static int32_t AssertPeerToPeerGroupType(int32_t groupType)
{
    if (groupType != PEER_TO_PEER_GROUP) {
        LOGE("Invalid group type! [GroupType]: %d", groupType);
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

static int32_t CheckInputGroupTypeValid(const CJson *jsonParams)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetIntFromJson(jsonParams, FIELD_GROUP_TYPE, &groupType) != HC_SUCCESS) {
        LOGE("Failed to get groupType from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    return AssertPeerToPeerGroupType(groupType);
}

static int32_t IsPeerDeviceIdNotSelf(const char *peerUdid)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    if (strcmp(peerUdid, (char *)udid) == 0) {
        LOGE("You are not allowed to delete yourself!");
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

static int32_t CheckPeerDeviceStatus(const char *groupId, const CJson *jsonParams)
{
    const char *peerAuthId = GetStringFromJson(jsonParams, FIELD_DELETE_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerUdid from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    DeviceInfo *deviceInfo = CreateDeviceInfoStruct();
    if (deviceInfo == NULL) {
        LOGE("Failed to allocate deviceInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GetDeviceInfoByAuthId(peerAuthId, groupId, deviceInfo);
    if (result != HC_SUCCESS) {
        LOGE("Failed to obtain the peer device information from the database!");
        DestroyDeviceInfoStruct(deviceInfo);
        return result;
    }
    result = IsPeerDeviceIdNotSelf(StringGet(&deviceInfo->udid));
    DestroyDeviceInfoStruct(deviceInfo);
    return result;
}

static int32_t CheckInvitePeer(const CJson *jsonParams)
{
    const char *groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }

    int32_t groupType = PEER_TO_PEER_GROUP;
    int32_t result;
    if (((result = CheckGroupExist(groupId)) != HC_SUCCESS) ||
        ((result = GetGroupTypeFromDb(groupId, &groupType)) != HC_SUCCESS) ||
        ((result = AssertPeerToPeerGroupType(groupType)) != HC_SUCCESS) ||
        ((result = CheckPermForGroup(MEMBER_INVITE, appId, groupId)) != HC_SUCCESS) ||
        ((result = CheckDeviceNumLimit(groupId, NULL)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t CheckJoinPeer(const CJson *jsonParams)
{
    return CheckInputGroupTypeValid(jsonParams);
}

static int32_t CheckDeletePeer(const CJson *jsonParams)
{
    const char *groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }

    int32_t groupType = PEER_TO_PEER_GROUP;
    int32_t result;
    if (((result = CheckGroupExist(groupId)) != HC_SUCCESS) ||
        ((result = GetGroupTypeFromDb(groupId, &groupType)) != HC_SUCCESS) ||
        ((result = AssertPeerToPeerGroupType(groupType)) != HC_SUCCESS) ||
        ((result = CheckPermForGroup(MEMBER_DELETE, appId, groupId)) != HC_SUCCESS) ||
        ((result = CheckPeerDeviceStatus(groupId, jsonParams)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t CheckClientStatus(int operationCode, const CJson *jsonParams)
{
    switch (operationCode) {
        case MEMBER_INVITE:
            return CheckInvitePeer(jsonParams);
        case MEMBER_JOIN:
            return CheckJoinPeer(jsonParams);
        case MEMBER_DELETE:
            return CheckDeletePeer(jsonParams);
        default:
            LOGE("Enter the exception case!");
            return HC_ERR_CASE;
    }
}

static CJson *GenerateGroupErrorMsg(int32_t errorCode, int64_t requestId)
{
    CJson *errorData = CreateJson();
    if (errorData == NULL) {
        LOGE("Failed to allocate errorData memory!");
        return NULL;
    }
    if (AddIntToJson(errorData, FIELD_GROUP_ERROR_MSG, errorCode) != HC_SUCCESS) {
        LOGE("Failed to add errorCode to errorData!");
        FreeJson(errorData);
        return NULL;
    }
    if (AddInt64StringToJson(errorData, FIELD_REQUEST_ID, requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to errorData!");
        FreeJson(errorData);
        return NULL;
    }
    return errorData;
}

static void InformPeerProcessError(int64_t requestId, const CJson *jsonParams, const DeviceAuthCallback *callback,
    int32_t errorCode)
{
    ChannelType channelType = GetChannelType(callback, jsonParams);
    int64_t channelId = DEFAULT_CHANNEL_ID;
    if ((channelType == NO_CHANNEL) ||
        ((channelType == SOFT_BUS) &&
        (GetByteFromJson(jsonParams, FIELD_CHANNEL_ID, (uint8_t *)&channelId, sizeof(int64_t)) != HC_SUCCESS))) {
        LOGE("No available channels found!");
        return;
    }
    CJson *errorData = GenerateGroupErrorMsg(errorCode, requestId);
    if (errorData == NULL) {
        return;
    }
    char *errorDataStr = PackJsonToString(errorData);
    FreeJson(errorData);
    if (errorDataStr == NULL) {
        LOGE("An error occurred when converting json to string!");
        return;
    }
    (void)HcSendMsg(channelType, requestId, channelId, callback, errorDataStr);
    FreeJsonString(errorDataStr);
}

static int32_t CheckServerStatusIfNotInvite(int operationCode, const CJson *jsonParams)
{
    if (operationCode == MEMBER_INVITE) {
        return HC_SUCCESS;
    }
    const char *groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *peerUdid = GetStringFromJson(jsonParams, FIELD_CONN_DEVICE_ID);
    if (peerUdid == NULL) {
        LOGE("Failed to get peerUdid from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    int32_t result = CheckGroupExist(groupId);
    if (result != HC_SUCCESS) {
        return result;
    }
    if (operationCode == MEMBER_JOIN) {
        /* The client sends a join request, which is equivalent to the server performing an invitation operation. */
        result = CheckPermForGroup(MEMBER_INVITE, appId, groupId);
        if (result != HC_SUCCESS) {
            return result;
        }
        result = CheckDeviceNumLimit(groupId, peerUdid);
    } else if (operationCode == MEMBER_DELETE) {
        result = CheckPermForGroup(MEMBER_DELETE, appId, groupId);
        if (result != HC_SUCCESS) {
            return result;
        }
        if (!IsTrustedDeviceInGroup(groupId, peerUdid)) {
            result = HC_ERR_DEVICE_NOT_EXIST;
        }
    }
    return result;
}

static int32_t CreateGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    LOGI("[Start]: Start to create a peer to peer group! [RequestId]: %" PRId64, requestId);
    int32_t result;
    char *groupId = NULL;
    char *returnDataStr = NULL;
    do {
        result = CheckInputGroupTypeValid(jsonParams);
        if (result != HC_SUCCESS) {
            break;
        }
        result = CreateGroupInner(jsonParams, &groupId);
        if (result != HC_SUCCESS) {
            break;
        }
        result = ConvertGroupIdToJsonStr(groupId, &returnDataStr);
        HcFree(groupId);
    } while (0);
    if (result != HC_SUCCESS) {
        ProcessErrorCallback(requestId, GROUP_CREATE, result, NULL, callback);
        LOGE("[End]: Failed to create a peer to peer group! [RequestId]: %" PRId64, requestId);
        return result;
    }
    ProcessFinishCallback(requestId, GROUP_CREATE, returnDataStr, callback);
    FreeJsonString(returnDataStr);
    LOGI("[End]: Create a peer to peer group successfully! [RequestId]: %" PRId64, requestId);
    return HC_SUCCESS;
}

static int32_t DeleteGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    int32_t result;
    const char *groupId = NULL;
    const char *appId = NULL;
    char *returnDataStr = NULL;
    do {
        LOGI("[Start]: Start to delete a peer to peer group! [RequestId]: %" PRId64, requestId);
        groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
        if (groupId == NULL) {
            LOGE("Failed to get groupId from jsonParams!");
            result = HC_ERR_JSON_GET;
            break;
        }
        appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
        if (appId == NULL) {
            LOGE("Failed to get appId from jsonParams!");
            result = HC_ERR_JSON_GET;
            break;
        }
        int32_t groupType = PEER_TO_PEER_GROUP;
        if (((result = CheckGroupExist(groupId)) != HC_SUCCESS) ||
            ((result = GetGroupTypeFromDb(groupId, &groupType)) != HC_SUCCESS) ||
            ((result = AssertPeerToPeerGroupType(groupType)) != HC_SUCCESS) ||
            ((result = CheckPermForGroup(GROUP_DISBAND, appId, groupId)) != HC_SUCCESS) ||
            ((result = AddAuthIdAndUserTypeToParams(groupId, jsonParams)) != HC_SUCCESS) ||
            ((result = DelGroupFromDatabase(groupId)) != HC_SUCCESS) ||
            ((result = ConvertGroupIdToJsonStr(groupId, &returnDataStr)) != HC_SUCCESS)) {
            break;
        }
    } while (0);
    if (result != HC_SUCCESS) {
        ProcessErrorCallback(requestId, GROUP_DISBAND, result, NULL, callback);
        LOGE("[End]: Failed to delete a peer to peer group! [RequestId]: %" PRId64, requestId);
        return result;
    }
    /*
     * If the group has been disbanded from the database but the key pair fails to be deleted,
     * we still believe we succeeded in disbanding the group. Only logs need to be printed.
     */
    result = ProcessKeyPair(DELETE_KEY_PAIR, jsonParams, groupId);
    if (result != HC_SUCCESS) {
        LOGD("Failed to delete peer key!");
    }
    ProcessFinishCallback(requestId, GROUP_DISBAND, returnDataStr, callback);
    FreeJsonString(returnDataStr);
    LOGI("[End]: Delete a peer to peer group successfully! [RequestId]: %" PRId64, requestId);
    return HC_SUCCESS;
}

static int32_t AddMemberToGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    int32_t result;
    int operationCode = MEMBER_INVITE;
    do {
        LOGI("[Start]: Start to add member to a peer to peer group! [RequestId]: %" PRId64, requestId);
        (void)GetIntFromJson(jsonParams, FIELD_OPERATION_CODE, &operationCode);
        result = CheckClientStatus(operationCode, jsonParams);
        if (result != HC_SUCCESS) {
            break;
        }
        ChannelType channelType = GetChannelType(callback, jsonParams);
        if (channelType == NO_CHANNEL) {
            LOGE("No available channels found!");
            result = HC_ERR_CHANNEL_NOT_EXIST;
            break;
        }
        result = CreateSession(requestId, TYPE_CLIENT_BIND_SESSION, jsonParams, callback);
        if (result != HC_SUCCESS) {
            LOGE("Failed to create client bind session!");
            break;
        }
        /**
         * If service open the channel by itself,
         * a channel opened message needs to be triggered to unify the channel usage policy.
         */
        if (channelType == SERVICE_CHANNEL) {
            /* Release the memory in advance to reduce the memory usage. */
            DeleteAllItem(jsonParams);
            OnChannelOpened(requestId, DEFAULT_CHANNEL_ID);
        }
    } while (0);
    if (result != HC_SUCCESS) {
        ProcessErrorCallback(requestId, operationCode, result, NULL, callback);
        LOGE("[End]: Failed to add member to a peer to peer group! [RequestId]: %" PRId64, requestId);
    }
    return result;
}

static int32_t DeleteMemberFromGroupInner(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    int32_t result = CheckClientStatus(MEMBER_DELETE, jsonParams);
    if (result != HC_SUCCESS) {
        return result;
    }
    bool isForceDelete = false;
    bool isIgnoreChannel = false;
    (void)(GetBoolFromJson(jsonParams, FIELD_IS_FORCE_DELETE, &isForceDelete));
    (void)(GetBoolFromJson(jsonParams, FIELD_IS_IGNORE_CHANNEL, &isIgnoreChannel));
    /* Release the memory in advance to reduce the memory usage. */
    DeleteItemFromJson(jsonParams, FIELD_IS_IGNORE_CHANNEL);
    if (isForceDelete && isIgnoreChannel) {
        LOGI("The service requires that the device be forcibly unbound locally instead of being unbound online!");
        return HandleLocalUnbind(requestId, jsonParams, callback);
    }
    ChannelType channelType = GetChannelType(callback, jsonParams);
    if (channelType == NO_CHANNEL) {
        LOGI("No available channels found!");
        if (isForceDelete) {
            LOGI("The peer device is forcibly unbound because no available channel is found!");
            return HandleLocalUnbind(requestId, jsonParams, callback);
        } else {
            LOGE("The service cannot unbind the device because it is not forcibly unbind and no channel is available!");
            return HC_ERR_CHANNEL_NOT_EXIST;
        }
    }
    result = CreateSession(requestId, TYPE_CLIENT_BIND_SESSION, jsonParams, callback);
    if (result != HC_SUCCESS) {
        if (isForceDelete) {
            LOGE("Failed to create client bind session. Therefore, we forcibly unbind the peer device!");
            return HandleLocalUnbind(requestId, jsonParams, callback);
        }
        LOGE("Failed to create client bind session!");
        return result;
    }
    /*
     * If service open the channel by itself,
     * a channel opened message needs to be triggered to unify the channel usage policy.
     */
    if (channelType == SERVICE_CHANNEL) {
        /* Release the memory in advance to reduce the memory usage. */
        DeleteAllItem(jsonParams);
        OnChannelOpened(requestId, DEFAULT_CHANNEL_ID);
    }
    return HC_SUCCESS;
}

static int32_t DeleteMemberFromGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    LOGI("[Start]: Start to delete member from a peer to peer group! [RequestId]: %" PRId64, requestId);
    int32_t result = DeleteMemberFromGroupInner(requestId, jsonParams, callback);
    if (result != HC_SUCCESS) {
        ProcessErrorCallback(requestId, MEMBER_DELETE, result, NULL, callback);
        LOGE("[End]: Failed to delete member from a peer to peer group! [RequestId]: %" PRId64, requestId);
    }
    return result;
}

static int32_t ProcessData(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    int32_t result;
    int operationCode = MEMBER_INVITE;
    (void)(GetIntFromJson(jsonParams, FIELD_GROUP_OP, &operationCode));
    result = CheckServerStatusIfNotInvite(operationCode, jsonParams);
    if (result != HC_SUCCESS) {
        InformPeerProcessError(requestId, jsonParams, callback, result);
        ProcessErrorCallback(requestId, operationCode, result, NULL, callback);
        return result;
    }
    result = CreateSession(requestId, TYPE_SERVER_BIND_SESSION, jsonParams, callback);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create server bind session!");
    }
    return result;
}

static int32_t AddManager(const char *appId, const char *groupId, const char *managerAppId)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupOwner(groupId, appId)) {
        LOGE("You do not have the permission to add a manager to the group!");
        return HC_ERR_ACCESS_DENIED;
    }
    if (AddGroupManager(groupId, managerAppId) != HC_SUCCESS) {
        LOGE("Failed to add manager!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t AddFriend(const char *appId, const char *groupId, const char *friendAppId)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupEditAllowed(groupId, appId)) {
        LOGE("You do not have the permission to add a friend to the group!");
        return HC_ERR_ACCESS_DENIED;
    }
    if (CompareVisibility(groupId, GROUP_VISIBILITY_ALLOW_LIST) != HC_SUCCESS) {
        LOGE("The group dose not support the allow list protection!");
        return HC_ERR_NOT_SUPPORT;
    }
    if (AddGroupFriend(groupId, friendAppId) != HC_SUCCESS) {
        LOGE("Failed to add friend!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t DeleteManager(const char *appId, const char *groupId, const char *managerAppId)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupOwner(groupId, appId)) {
        LOGE("You do not have the permission to delete a manager from the group!");
        return HC_ERR_ACCESS_DENIED;
    }
    if (RemoveGroupManager(groupId, managerAppId) != HC_SUCCESS) {
        LOGE("Failed to delete manager!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t DeleteFriend(const char *appId, const char *groupId, const char *friendAppId)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupEditAllowed(groupId, appId)) {
        LOGE("You do not have the permission to add a friend to the group!");
        return HC_ERR_ACCESS_DENIED;
    }
    if (CompareVisibility(groupId, GROUP_VISIBILITY_ALLOW_LIST) != HC_SUCCESS) {
        LOGE("The group dose not support the allow list protection!");
        return HC_ERR_NOT_SUPPORT;
    }
    if (RemoveGroupFriend(groupId, friendAppId) != HC_SUCCESS) {
        LOGE("Failed to delete friend!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t GetManagers(const char *appId, const char *groupId, char **returnManagers, uint32_t *returnSize)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupOwner(groupId, appId)) {
        LOGE("You do not have the permission to query the group managers information!");
        return HC_ERR_ACCESS_DENIED;
    }
    CJson *managers = CreateJsonArray();
    if (managers == NULL) {
        LOGE("Failed to allocate managers memory!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = GetGroupManagers(groupId, managers);
    if (result != HC_SUCCESS) {
        LOGE("Failed to get managers!");
        FreeJson(managers);
        return result;
    }
    *returnManagers = PackJsonToString(managers);
    if (*returnManagers == NULL) {
        LOGE("Failed to convert json to string!");
        FreeJson(managers);
        return HC_ERR_JSON_FAIL;
    }
    *returnSize = GetItemNum(managers);
    FreeJson(managers);
    return HC_SUCCESS;
}

static int32_t GetFriends(const char *appId, const char *groupId, char **returnFriends, uint32_t *returnSize)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetGroupTypeFromDb(groupId, &groupType) != HC_SUCCESS) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (AssertPeerToPeerGroupType(groupType) != HC_SUCCESS) {
        return HC_ERR_NOT_SUPPORT;
    }
    if (!IsGroupEditAllowed(groupId, appId)) {
        LOGE("You do not have the permission to query the group friends information!");
        return HC_ERR_ACCESS_DENIED;
    }
    CJson *friends = CreateJsonArray();
    if (friends == NULL) {
        LOGE("Failed to allocate friends memory!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = GetGroupFriends(groupId, friends);
    if (result != HC_SUCCESS) {
        LOGE("Failed to get friends!");
        FreeJson(friends);
        return result;
    }
    *returnFriends = PackJsonToString(friends);
    if (*returnFriends == NULL) {
        LOGE("Failed to convert json to string!");
        FreeJson(friends);
        return HC_ERR_JSON_FAIL;
    }
    *returnSize = GetItemNum(friends);
    FreeJson(friends);
    return HC_SUCCESS;
}

static int32_t AddGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (isManager) {
        return AddManager(appId, groupId, roleAppId);
    }
    return AddFriend(appId, groupId, roleAppId);
}

static int32_t DeleteGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (isManager) {
        return DeleteManager(appId, groupId, roleAppId);
    }
    return DeleteFriend(appId, groupId, roleAppId);
}

static int32_t GetGroupRole(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
    uint32_t *returnSize)
{
    if (isManager) {
        return GetManagers(appId, groupId, returnJsonStr, returnSize);
    }
    return GetFriends(appId, groupId, returnJsonStr, returnSize);
}

static PeerToPeerGroup g_peerToPeerGroup = {
    .base.type = PEER_TO_PEER_GROUP,
    .createGroup = CreateGroup,
    .deleteGroup = DeleteGroup,
    .addMember = AddMemberToGroup,
    .deleteMember = DeleteMemberFromGroup,
    .processData = ProcessData,
    .addGroupRole = AddGroupRole,
    .deleteGroupRole = DeleteGroupRole,
    .getGroupRole = GetGroupRole
};

BaseGroup *GetPeerToPeerGroupInstance()
{
    return (BaseGroup *)&g_peerToPeerGroup;
}

bool IsPeerToPeerGroupSupported()
{
    return true;
}
