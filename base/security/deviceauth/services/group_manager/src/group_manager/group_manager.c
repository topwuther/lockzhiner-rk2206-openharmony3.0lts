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

#include "group_manager.h"
#include "account_related_group_manager.h"
#include "account_unrelated_group_manager.h"
#include "broadcast_manager.h"
#include "channel_manager.h"
#include "database_manager.h"
#include "hc_log.h"

DECLARE_HC_VECTOR(GmCallbackEntryVec, CallbackEntry)
IMPLEMENT_HC_VECTOR(GmCallbackEntryVec, CallbackEntry, 1)
static GmCallbackEntryVec g_callbackVec;

static bool IsGroupTypeSupported(int groupType)
{
    if (((groupType == PEER_TO_PEER_GROUP) && (IsAccountUnrelatedGroupTypeSupported(groupType))) ||
        ((groupType == IDENTICAL_ACCOUNT_GROUP) && (IsAccountRelatedGroupTypeSupported(groupType))) ||
        ((groupType == ACROSS_ACCOUNT_AUTHORIZE_GROUP) && (IsAccountRelatedGroupTypeSupported(groupType)))) {
        return true;
    }
    return false;
}

static int32_t UpdateCallbackIfExist(const char *appId, const DeviceAuthCallback *callback)
{
    uint32_t index;
    CallbackEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_callbackVec, index, entry) {
        if ((entry != NULL) && (strcmp(entry->appId, appId) == 0)) {
            if (memcpy_s(entry->callback, sizeof(DeviceAuthCallback),
                callback, sizeof(DeviceAuthCallback)) != HC_SUCCESS) {
                LOGE("Failed to copy service callback!");
                return HC_ERR_MEMORY_COPY;
            }
            return HC_SUCCESS;
        }
    }
    return HC_ERR_CALLBACK_NOT_FOUND;
}

static int32_t AddCallbackIfNotExist(const char *appId, const DeviceAuthCallback *callback)
{
    uint32_t appIdLen = HcStrlen(appId) + 1;
    char *copyAppId = (char *)HcMalloc(appIdLen, 0);
    if (copyAppId == NULL) {
        LOGE("Failed to allocate copyAppId memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (strcpy_s(copyAppId, appIdLen, appId) != HC_SUCCESS) {
        LOGE("Failed to copy appId!");
        HcFree(copyAppId);
        return HC_ERR_MEMORY_COPY;
    }
    DeviceAuthCallback *copyCallback = (DeviceAuthCallback *)HcMalloc(sizeof(DeviceAuthCallback), 0);
    if (copyCallback == NULL) {
        LOGE("Failed to allocate copyCallback memory!");
        HcFree(copyAppId);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(copyCallback, sizeof(DeviceAuthCallback),
        callback, sizeof(DeviceAuthCallback)) != HC_SUCCESS) {
        LOGE("Failed to copy service callback!");
        HcFree(copyAppId);
        HcFree(copyCallback);
        return HC_ERR_MEMORY_COPY;
    }
    CallbackEntry entry;
    entry.appId = copyAppId;
    entry.callback = copyCallback;
    g_callbackVec.pushBack(&g_callbackVec, &entry);
    LOGI("[End]: Service register GM callback successfully!");
    return HC_SUCCESS;
}

static int32_t AddGroupNameToReturn(const GroupInfo *groupEntry, CJson *json)
{
    const char *groupName = StringGet(&groupEntry->name);
    if (AddStringToJson(json, FIELD_GROUP_NAME, groupName) != HC_SUCCESS) {
        LOGE("Failed to add groupName to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupIdToReturn(const GroupInfo *groupEntry, CJson *json)
{
    const char *groupId = StringGet(&groupEntry->id);
    if (AddStringToJson(json, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupOwnerToReturn(const GroupInfo *groupEntry, CJson *json)
{
    const char *ownerName = StringGet(&groupEntry->ownerName);
    if (AddStringToJson(json, FIELD_GROUP_OWNER, ownerName) != HC_SUCCESS) {
        LOGE("Failed to add groupOwner to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupTypeToReturn(const GroupInfo *groupEntry, CJson *json)
{
    int32_t groupType = groupEntry->type;
    if (AddIntToJson(json, FIELD_GROUP_TYPE, groupType) != HC_SUCCESS) {
        LOGE("Failed to add groupType to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupVisibilityToReturn(const GroupInfo *groupEntry, CJson *json)
{
    int groupVisibility = groupEntry->visibility;
    if (AddIntToJson(json, FIELD_GROUP_VISIBILITY, groupVisibility) != HC_SUCCESS) {
        LOGE("Failed to add groupType to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddUdidToReturn(const DeviceInfo *devEntry, CJson *json)
{
    const char *udid = StringGet(&devEntry->udid);
    if (AddStringToJson(json, FIELD_PEER_UDID, udid) != HC_SUCCESS) {
        LOGE("Failed to add udid to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddAuthIdToReturn(const DeviceInfo *devEntry, CJson *json)
{
    const char *authId = StringGet(&devEntry->authId);
    if (AddStringToJson(json, FIELD_AUTH_ID, authId) != HC_SUCCESS) {
        LOGE("Failed to add authId to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddUserIdToReturn(const DeviceInfo *devEntry, CJson *json)
{
    int64_t userId = devEntry->userId;
    if (AddByteToJson(json, FIELD_USER_ID, (uint8_t*)&userId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to add userId to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddCredentialTypeToReturn(const DeviceInfo *devEntry, CJson *json)
{
    int credentialType = devEntry->credential;
    if (AddIntToJson(json, FIELD_CREDENTIAL_TYPE, credentialType) != HC_SUCCESS) {
        LOGE("Failed to add credentialType to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddUserTypeToReturn(const DeviceInfo *devEntry, CJson *json)
{
    int userType = devEntry->devType;
    if (AddIntToJson(json, FIELD_USER_TYPE, userType) != HC_SUCCESS) {
        LOGE("Failed to add userType to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GenerateReturnGroupInfo(const GroupInfo *groupEntry, CJson *returnJson)
{
    int32_t result;
    if (((result = AddGroupNameToReturn(groupEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddGroupIdToReturn(groupEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddGroupOwnerToReturn(groupEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddGroupTypeToReturn(groupEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddGroupVisibilityToReturn(groupEntry, returnJson)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t GenerateReturnDevInfo(const DeviceInfo *devEntry, CJson *returnJson)
{
    int32_t result;
    if (((result = AddUdidToReturn(devEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddAuthIdToReturn(devEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddUserIdToReturn(devEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddCredentialTypeToReturn(devEntry, returnJson)) != HC_SUCCESS) ||
        ((result = AddUserTypeToReturn(devEntry, returnJson)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static void RemoveNoPermissionGroup(GroupInfoVec *groupEntryVec, const char *appId)
{
    uint32_t index = 0;
    void **groupEntryPtr = NULL;
    while (index < groupEntryVec->size(groupEntryVec)) {
        groupEntryPtr = groupEntryVec->getp(groupEntryVec, index);
        if ((groupEntryPtr == NULL) || (*groupEntryPtr == NULL)) {
            index++;
            continue;
        }
        GroupInfo *groupEntry = (GroupInfo *)(*groupEntryPtr);
        if (IsGroupAccessible(StringGet(&groupEntry->id), appId)) {
            index++;
            continue;
        }
        void *tempEntry = NULL;
        HC_VECTOR_POPELEMENT(groupEntryVec, &tempEntry, index);
        LOGI("Pop a no permission group from vector!");
    }
}

static int32_t GenerateReturnEmptyArrayStr(char **returnVec)
{
    CJson *json = CreateJsonArray();
    if (json == NULL) {
        LOGE("Failed to allocate json memory!");
        return HC_ERR_JSON_FAIL;
    }
    *returnVec = PackJsonToString(json);
    FreeJson(json);
    if (*returnVec == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GenerateReturnGroupVec(GroupInfoVec *groupEntryVec, char **returnGroupVec, uint32_t *groupNum)
{
    if (HC_VECTOR_SIZE(groupEntryVec) == 0) {
        LOGI("No group is found based on the query parameters!");
        *groupNum = 0;
        return GenerateReturnEmptyArrayStr(returnGroupVec);
    }

    CJson *json = CreateJsonArray();
    if (json == NULL) {
        LOGE("Failed to allocate json memory!");
        return HC_ERR_JSON_FAIL;
    }
    uint32_t groupCount = 0;
    uint32_t index;
    void **groupEntryPtr = NULL;
    FOR_EACH_HC_VECTOR(*groupEntryVec, index, groupEntryPtr) {
        if ((groupEntryPtr != NULL) && ((*groupEntryPtr) != NULL)) {
            GroupInfo *groupEntry = (GroupInfo*)(*groupEntryPtr);
            CJson *groupInfoJson = CreateJson();
            if (groupInfoJson == NULL) {
                LOGE("Failed to allocate groupInfoJson memory!");
                FreeJson(json);
                return HC_ERR_ALLOC_MEMORY;
            }
            int32_t result = GenerateReturnGroupInfo(groupEntry, groupInfoJson);
            if (result != HC_SUCCESS) {
                FreeJson(groupInfoJson);
                FreeJson(json);
                return result;
            }
            if (AddObjToArray(json, groupInfoJson) != HC_SUCCESS) {
                LOGE("Failed to add groupInfoStr to returnGroupVec!");
                FreeJson(groupInfoJson);
                FreeJson(json);
                return HC_ERR_JSON_FAIL;
            }
            ++groupCount;
        }
    }
    *returnGroupVec = PackJsonToString(json);
    FreeJson(json);
    if ((*returnGroupVec) == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    *groupNum = groupCount;
    return HC_SUCCESS;
}

static int32_t GenerateReturnDeviceVec(DeviceInfoVec *devEntryVec, char **returnDevInfoVec, uint32_t *deviceNum)
{
    if (HC_VECTOR_SIZE(devEntryVec) == 0) {
        LOGI("No device is found based on the query parameters!");
        *deviceNum = 0;
        return GenerateReturnEmptyArrayStr(returnDevInfoVec);
    }

    CJson *json = CreateJsonArray();
    if (json == NULL) {
        LOGE("Failed to allocate json memory!");
        return HC_ERR_JSON_FAIL;
    }
    uint32_t devCount = 0;
    uint32_t index;
    void **devEntryPtr = NULL;
    FOR_EACH_HC_VECTOR(*devEntryVec, index, devEntryPtr) {
        if ((devEntryPtr != NULL) && ((*devEntryPtr) != NULL)) {
            DeviceInfo *devEntry = (DeviceInfo*)(*devEntryPtr);
            CJson *devInfoJson = CreateJson();
            if (devInfoJson == NULL) {
                LOGE("Failed to allocate devInfoJson memory!");
                FreeJson(json);
                return HC_ERR_ALLOC_MEMORY;
            }
            int32_t result = GenerateReturnDevInfo(devEntry, devInfoJson);
            if (result != HC_SUCCESS) {
                FreeJson(devInfoJson);
                FreeJson(json);
                return result;
            }
            if (AddObjToArray(json, devInfoJson) != HC_SUCCESS) {
                LOGE("Failed to add devInfoStr to returnGroupVec!");
                FreeJson(devInfoJson);
                FreeJson(json);
                return HC_ERR_JSON_FAIL;
            }
            ++devCount;
        }
    }
    *returnDevInfoVec = PackJsonToString(json);
    FreeJson(json);
    if ((*returnDevInfoVec) == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    *deviceNum = devCount;
    return HC_SUCCESS;
}

static bool IsQueryParamsValid(int groupType, const char *groupId, const char *groupName, const char *groupOwner)
{
    if ((groupType == ALL_GROUP) && (groupId == NULL) && (groupName == NULL) && (groupOwner == NULL)) {
        return false;
    } else {
        return true;
    }
}

static const DeviceAuthCallback *GetGmCallbackByAppId(const char *appId)
{
    uint32_t index;
    CallbackEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_callbackVec, index, entry) {
        if (strcmp(entry->appId, appId) == 0) {
            return entry->callback;
        }
    }
    return NULL;
}

static int32_t RegGroupManagerCallback(const char *appId, const DeviceAuthCallback *callback)
{
    if ((appId == NULL) || (callback == NULL)) {
        LOGE("The input appId or callback is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    int32_t res = InitChannelManager();
    if (res != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init channel manage module!");
        return res;
    }
    if (UpdateCallbackIfExist(appId, callback) == HC_SUCCESS) {
        LOGI("[End]: The callback associated with the appId already exists, so we update the callback!");
        return HC_SUCCESS;
    }
    return AddCallbackIfNotExist(appId, callback);
}

static int32_t UnRegGroupManagerCallback(const char *appId)
{
    if (appId == NULL) {
        LOGE("The input appId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t index;
    CallbackEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_callbackVec, index, entry) {
        if (strcmp(entry->appId, appId) == 0) {
            HcFree(entry->appId);
            HcFree(entry->callback);
            CallbackEntry tempEntry;
            HC_VECTOR_POPELEMENT(&g_callbackVec, &tempEntry, index);
            LOGI("[End]: Service deregister GM callback successfully!");
            return HC_SUCCESS;
        }
    }
    LOGI("[End]: Although the callback does not exist, we still believe it is correct to deregister the callback!");
    return HC_SUCCESS;
}

static void CreatePeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return;
    }
    instance->createPeerToPeerGroup(requestId, jsonParams, callback);
}

static void DeletePeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return;
    }
    instance->deletePeerToPeerGroup(requestId, jsonParams, callback);
}

static void AddMemberToPeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return;
    }
    instance->addMemberToPeerToPeerGroup(requestId, jsonParams, callback);
}

static void DeleteMemberFromPeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return;
    }
    instance->deleteMemberFromPeerToPeerGroup(requestId, jsonParams, callback);
}

static void ProcessBindData(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return;
    }
    instance->processBindData(requestId, jsonParams, callback);
}

static int32_t AddGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->addGroupRole(isManager, appId, groupId, roleAppId);
}

static int32_t DeleteGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->deleteGroupRole(isManager, appId, groupId, roleAppId);
}

static int32_t GetGroupRole(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
    uint32_t *returnSize)
{
    if (!IsAccountUnrelatedGroupManagerSupported()) {
        LOGE("All non-account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountUnrelatedGroupManager *instance = GetAccountUnrelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountUnrelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getGroupRole(isManager, appId, groupId, returnJsonStr, returnSize);
}

static int32_t SyncCreateIdenticalAccountGroup(const CJson *jsonParams, char **returnDataStr)
{
    if (!IsAccountRelatedGroupManagerSupported()) {
        LOGE("All account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountRelatedGroupManager *instance = GetAccountRelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountRelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->syncCreateIdenticalAccountGroup(jsonParams, returnDataStr);
}

static int32_t SyncDeleteIdenticalAccountGroup(const CJson *jsonParams)
{
    if (!IsAccountRelatedGroupManagerSupported()) {
        LOGE("All account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountRelatedGroupManager *instance = GetAccountRelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountRelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->syncDeleteIdenticalAccountGroup(jsonParams);
}

static int32_t GetRegisterInfo(char **returnRegisterInfo)
{
    if (!IsAccountRelatedGroupManagerSupported()) {
        LOGE("All account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountRelatedGroupManager *instance = GetAccountRelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountRelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getRegisterInfo(returnRegisterInfo);
}

static int32_t ProcessCredential(int operationCode, const char *reqJsonStr, char **returnJsonStr)
{
    if (!IsAccountRelatedGroupManagerSupported()) {
        LOGE("All account groups are not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    AccountRelatedGroupManager *instance = GetAccountRelatedGroupManager();
    if (instance == NULL) {
        LOGE("Failed to get AccountRelatedGroupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->processCredential(operationCode, reqJsonStr, returnJsonStr);
}

static int32_t RegListener(const char *appId, const DataChangeListener *listener)
{
    if ((appId == NULL) || (listener == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsBroadcastSupported()) {
        LOGE("Broadcast not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    return AddListener(appId, listener);
}

static int32_t UnRegListener(const char *appId)
{
    if (appId == NULL) {
        LOGE("The input appId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsBroadcastSupported()) {
        LOGE("Broadcast not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    return RemoveListener(appId);
}

static int32_t GetAccessibleGroupInfoById(const char *appId, const char *groupId, char **returnGroupInfo)
{
    if ((appId == NULL) || (groupId == NULL) || (returnGroupInfo == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsGroupExistByGroupId(groupId)) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (!IsGroupAccessible(groupId, appId)) {
        LOGE("You do not have the permission to query the group information!");
        return HC_ERR_ACCESS_DENIED;
    }
    GroupInfo *groupEntry = CreateGroupInfoStruct();
    if (groupEntry == NULL) {
        LOGE("Failed to allocate groupInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetGroupEntryByGroupId(groupId, groupEntry) != HC_SUCCESS) {
        LOGE("Failed to obtain the group information from the database!");
        DestroyGroupInfoStruct(groupEntry);
        return HC_ERR_DB;
    }
    CJson *groupInfoJson = CreateJson();
    if (groupInfoJson == NULL) {
        LOGE("Failed to allocate groupInfoJson memory!");
        DestroyGroupInfoStruct(groupEntry);
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = GenerateReturnGroupInfo(groupEntry, groupInfoJson);
    DestroyGroupInfoStruct(groupEntry);
    if (result != HC_SUCCESS) {
        FreeJson(groupInfoJson);
        return result;
    }
    *returnGroupInfo = PackJsonToString(groupInfoJson);
    FreeJson(groupInfoJson);
    if (*returnGroupInfo == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GetAccessibleGroupInfo(const char *appId, const char *queryParams, char **returnGroupVec,
    uint32_t *groupNum)
{
    if ((appId == NULL) || (queryParams == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    CJson *queryParamsJson = CreateJsonFromString(queryParams);
    if (queryParamsJson == NULL) {
        LOGE("Failed to create queryParamsJson from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t groupType = ALL_GROUP;
    (void)GetIntFromJson(queryParamsJson, FIELD_GROUP_TYPE, &groupType);
    if ((groupType != ALL_GROUP) && (!IsGroupTypeSupported(groupType))) {
        LOGE("Invalid group type!");
        FreeJson(queryParamsJson);
        return HC_ERR_INVALID_PARAMS;
    }
    const char *groupId = GetStringFromJson(queryParamsJson, FIELD_GROUP_ID);
    const char *groupName = GetStringFromJson(queryParamsJson, FIELD_GROUP_NAME);
    const char *groupOwner = GetStringFromJson(queryParamsJson, FIELD_GROUP_OWNER);
    if (!IsQueryParamsValid(groupType, groupId, groupName, groupOwner)) {
        LOGE("The query parameters cannot be all null!");
        FreeJson(queryParamsJson);
        return HC_ERR_INVALID_PARAMS;
    }
    GroupInfoVec groupInfoVec;
    CreateGroupInfoVecStruct(&groupInfoVec);
    int32_t result = GetGroupInfo(groupType, groupId, groupName, groupOwner, &groupInfoVec);
    FreeJson(queryParamsJson);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoVecStruct(&groupInfoVec);
        return result;
    }
    RemoveNoPermissionGroup(&groupInfoVec, appId);
    result = GenerateReturnGroupVec(&groupInfoVec, returnGroupVec, groupNum);
    DestroyGroupInfoVecStruct(&groupInfoVec);
    return result;
}

static int32_t GetAccessibleJoinedGroups(const char *appId, int groupType, char **returnGroupVec, uint32_t *groupNum)
{
    if ((appId == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsGroupTypeSupported(groupType)) {
        LOGE("Invalid group type!");
        return HC_ERR_INVALID_PARAMS;
    }
    GroupInfoVec groupInfoVec;
    CreateGroupInfoVecStruct(&groupInfoVec);
    int32_t result = GetJoinedGroups(groupType, &groupInfoVec);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoVecStruct(&groupInfoVec);
        return result;
    }
    RemoveNoPermissionGroup(&groupInfoVec, appId);
    result = GenerateReturnGroupVec(&groupInfoVec, returnGroupVec, groupNum);
    DestroyGroupInfoVecStruct(&groupInfoVec);
    return result;
}

static int32_t GetAccessibleRelatedGroups(const char *appId, const char *peerDeviceId,
    char **returnGroupVec, uint32_t *groupNum)
{
    if ((appId == NULL) || (peerDeviceId == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("Start to get related groups! [AppId]: %s", appId);
    GroupInfoVec groupInfoVec;
    CreateGroupInfoVecStruct(&groupInfoVec);
    int32_t result = GetRelatedGroups(peerDeviceId, &groupInfoVec);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoVecStruct(&groupInfoVec);
        return result;
    }
    RemoveNoPermissionGroup(&groupInfoVec, appId);
    result = GenerateReturnGroupVec(&groupInfoVec, returnGroupVec, groupNum);
    DestroyGroupInfoVecStruct(&groupInfoVec);
    return result;
}

static int32_t GetAccessibleDeviceInfoById(const char *appId, const char *deviceId,
    const char *groupId, char **returnDeviceInfo)
{
    if ((appId == NULL) || (deviceId == NULL) || (groupId == NULL) || (returnDeviceInfo == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsGroupExistByGroupId(groupId)) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (!IsGroupAccessible(groupId, appId)) {
        LOGE("You do not have the permission to query the group information!");
        return HC_ERR_ACCESS_DENIED;
    }
    DeviceInfo *devInfo = CreateDeviceInfoStruct();
    if (devInfo == NULL) {
        LOGE("Failed to allocate devInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetDeviceInfoByAuthId(deviceId, groupId, devInfo) != HC_SUCCESS) {
        LOGE("No device is found based on the query parameters!");
        DestroyDeviceInfoStruct(devInfo);
        return HC_ERR_DEVICE_NOT_EXIST;
    }
    CJson *devInfoJson = CreateJson();
    if (devInfoJson == NULL) {
        LOGE("Failed to allocate devInfoJson memory!");
        DestroyDeviceInfoStruct(devInfo);
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = GenerateReturnDevInfo(devInfo, devInfoJson);
    DestroyDeviceInfoStruct(devInfo);
    if (result != HC_SUCCESS) {
        FreeJson(devInfoJson);
        return result;
    }
    *returnDeviceInfo = PackJsonToString(devInfoJson);
    FreeJson(devInfoJson);
    if (*returnDeviceInfo == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GetAccessibleTrustedDevices(const char *appId, const char *groupId,
    char **returnDevInfoVec, uint32_t *deviceNum)
{
    if ((appId == NULL) || (groupId == NULL) || (returnDevInfoVec == NULL) || (deviceNum == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsGroupExistByGroupId(groupId)) {
        LOGE("No group is found based on the query parameters!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    if (!IsGroupAccessible(groupId, appId)) {
        LOGE("You do not have the permission to query the group information!");
        return HC_ERR_ACCESS_DENIED;
    }
    DeviceInfoVec deviceInfoVec;
    CreateDeviceInfoVecStruct(&deviceInfoVec);
    int32_t result = GetTrustedDevices(groupId, &deviceInfoVec);
    if (result != HC_SUCCESS) {
        DestroyDeviceInfoVecStruct(&deviceInfoVec);
        return result;
    }
    result = GenerateReturnDeviceVec(&deviceInfoVec, returnDevInfoVec, deviceNum);
    DestroyDeviceInfoVecStruct(&deviceInfoVec);
    return result;
}

static bool IsDeviceInAccessibleGroup(const char *appId, const char *groupId, const char *deviceId)
{
    if ((appId == NULL) || (groupId == NULL) || (deviceId == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return false;
    }
    if (!IsGroupExistByGroupId(groupId)) {
        LOGE("No group is found based on the query parameters!");
        return false;
    }
    if (!IsGroupAccessible(groupId, appId)) {
        LOGI("You do not have the permission to query the group information!");
        return false;
    }
    return IsTrustedDeviceInGroupByAuthId(groupId, deviceId);
}

static void DestroyInfo(char **returnInfo)
{
    FreeJsonString(*returnInfo);
    *returnInfo = NULL;
}

static void InitGroupManagerInstance(GroupManager *instance)
{
    instance->regGroupManagerCallback = RegGroupManagerCallback;
    instance->unRegGroupManagerCallback = UnRegGroupManagerCallback;
    instance->getGmCallbackByAppId = GetGmCallbackByAppId;
    instance->createPeerToPeerGroup = CreatePeerToPeerGroup;
    instance->deletePeerToPeerGroup = DeletePeerToPeerGroup;
    instance->addMemberToPeerToPeerGroup = AddMemberToPeerToPeerGroup;
    instance->deleteMemberFromPeerToPeerGroup = DeleteMemberFromPeerToPeerGroup;
    instance->processBindData = ProcessBindData;
    instance->addGroupRole = AddGroupRole;
    instance->deleteGroupRole = DeleteGroupRole;
    instance->getGroupRole = GetGroupRole;
    instance->regListener = RegListener;
    instance->unRegListener = UnRegListener;
    instance->syncCreateIdenticalAccountGroup = SyncCreateIdenticalAccountGroup;
    instance->syncDeleteIdenticalAccountGroup = SyncDeleteIdenticalAccountGroup;
    instance->getRegisterInfo = GetRegisterInfo;
    instance->processCredential = ProcessCredential;
    instance->getAccessibleGroupInfoById = GetAccessibleGroupInfoById;
    instance->getAccessibleGroupInfo = GetAccessibleGroupInfo;
    instance->getAccessibleJoinedGroups = GetAccessibleJoinedGroups;
    instance->getAccessibleRelatedGroups = GetAccessibleRelatedGroups;
    instance->getAccessibleDeviceInfoById = GetAccessibleDeviceInfoById;
    instance->getAccessibleTrustedDevices = GetAccessibleTrustedDevices;
    instance->isDeviceInAccessibleGroup = IsDeviceInAccessibleGroup;
    instance->destroyInfo = DestroyInfo;
}

static GroupManager *g_groupManager = NULL;

int32_t InitGroupManager()
{
    if (InitDatabase() != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init database module!");
        return HC_ERR_SERVICE_NEED_RESTART;
    }
    if (IsBroadcastSupported()) {
        if (InitBroadcastManager() != HC_SUCCESS) {
            LOGE("[End]: [Service]: Failed to init broadcast manage module!");
            DestroyDatabase();
            return HC_ERR_SERVICE_NEED_RESTART;
        }
        Broadcaster *broadcaster = GetBroadcaster();
        if (broadcaster == NULL) {
            LOGE("[End]: [Service]: Failed to get broadcast manager instance!");
            DestroyDatabase();
            DestroyBroadcastManager();
            return HC_ERR_NULL_PTR;
        }
        RegisterBroadcaster(broadcaster);
    }
    if (IsAccountRelatedGroupManagerSupported() && IsGroupTypeSupported(ACROSS_ACCOUNT_AUTHORIZE_GROUP)) {
        AccountRelatedGroupManager *instance = GetAccountRelatedGroupManager();
        if (instance == NULL) {
            LOGE("Failed to get AccountRelatedGroupManager instance!");
            DestroyDatabase();
            DestroyBroadcastManager();
            return HC_ERR_NULL_PTR;
        }
        RegGenerateGroupIdFunc(instance->generateAcrossAccountGroupId);
    }
    g_groupManager = (GroupManager *)HcMalloc(sizeof(GroupManager), 0);
    if (g_groupManager == NULL) {
        LOGE("Failed to allocate groupManager instance memory!");
        DestroyDatabase();
        DestroyBroadcastManager();
        return HC_ERR_ALLOC_MEMORY;
    }
    g_callbackVec = CREATE_HC_VECTOR(GmCallbackEntryVec)
    InitGroupManagerInstance(g_groupManager);
    return HC_SUCCESS;
}

void DestroyGroupManager()
{
    HcFree(g_groupManager);
    g_groupManager = NULL;
    DeregGenerateGroupIdFunc();
    DeregisterBroadcaster();
    DestroyBroadcastManager();
    DestroyDatabase();
    uint32_t index;
    CallbackEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_callbackVec, index, entry) {
        if (entry != NULL) {
            HcFree(entry->appId);
            HcFree(entry->callback);
        }
    }
    DESTROY_HC_VECTOR(GmCallbackEntryVec, &g_callbackVec)
}

GroupManager *GetGroupManagerInstance()
{
    return g_groupManager;
}

bool IsGroupManagerSupported()
{
    return true;
}