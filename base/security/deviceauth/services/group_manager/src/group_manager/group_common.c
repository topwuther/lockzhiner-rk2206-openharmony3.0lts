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

#include "group_common.h"
#include "alg_loader.h"
#include "common_util.h"
#include "database_manager.h"
#include "dev_auth_module_manager.h"
#include "group_manager.h"
#include "hc_dev_info.h"
#include "hc_log.h"

int32_t GetHashMessage(const Uint8Buff *first, const Uint8Buff *second, uint8_t **hashMessage, uint32_t *messageSize)
{
    if ((first == NULL) || (second == NULL)) {
        LOGE("The input first message or second message is NULL!");
        return HC_ERR_NULL_PTR;
    }
    const char *separator = "|";
    uint32_t firstSize = first->length;
    uint32_t secondSize = second->length;
    uint32_t separatorSize = HcStrlen(separator);
    uint32_t totalSize = firstSize + secondSize + separatorSize;
    *hashMessage = (uint8_t *)HcMalloc(totalSize, 0);
    if (*hashMessage == NULL) {
        LOGE("Failed to allocate hashMessage memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = HC_SUCCESS;
    do {
        if (memcpy_s((*hashMessage), totalSize, first->val, firstSize) != HC_SUCCESS) {
            LOGE("Failed to copy first!");
            result = HC_ERR_MEMORY_COPY;
            break;
        }
        if (memcpy_s((*hashMessage) + firstSize, totalSize - firstSize, separator, separatorSize) != HC_SUCCESS) {
            LOGE("Failed to copy separator!");
            result = HC_ERR_MEMORY_COPY;
            break;
        }
        if (memcpy_s((*hashMessage) + firstSize + separatorSize, secondSize, second->val, secondSize) != HC_SUCCESS) {
            LOGE("Failed to copy second!");
            result = HC_ERR_MEMORY_COPY;
        }
    } while (0);
    if (result != HC_SUCCESS) {
        HcFree(*hashMessage);
        *hashMessage = NULL;
        return result;
    }
    *messageSize = totalSize;
    return HC_SUCCESS;
}

int32_t GetHashResult(const uint8_t *info, uint32_t infoLen, char *hash, uint32_t hashLen)
{
    if (info == NULL) {
        LOGE("The input info is NULL!");
        return HC_ERR_NULL_PTR;
    }
    Uint8Buff infoHash = { NULL, SHA256_LEN };
    Uint8Buff message = { NULL, infoLen };
    infoHash.val = (uint8_t *)HcMalloc(SHA256_LEN, 0);
    if (infoHash.val == NULL) {
        LOGE("Malloc infoHash.val failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    message.val = (uint8_t *)HcMalloc(infoLen, 0);
    if (message.val == NULL) {
        LOGE("Malloc message.val failed.");
        HcFree(infoHash.val);
        return HC_ERR_ALLOC_MEMORY;
    }
    (void)memcpy_s(message.val, infoLen, info, infoLen);
    const AlgLoader *loader = GetLoaderInstance();
    if (loader == NULL) {
        LOGE("Failed to get AlgLoader instance!");
        HcFree(infoHash.val);
        HcFree(message.val);
        return HC_ERR_NOT_SUPPORT;
    }
    int32_t result = loader->sha256(&message, &infoHash);
    HcFree(message.val);
    if (result == HC_SUCCESS) {
        if (ByteToHexString(infoHash.val, infoHash.length, hash, hashLen) != HC_SUCCESS) {
            LOGE("Failed to convert bytes to string!");
            result = HC_ERR_CONVERT_FAILED;
        }
    }
    HcFree(infoHash.val);
    return result;
}

int32_t CheckGroupNumLimit(int32_t groupType, const char *appId)
{
    if ((groupType == IDENTICAL_ACCOUNT_GROUP) && (IsIdenticalGroupExist())) {
        LOGE("The identical account group already exists!");
        return HC_ERR_BEYOND_LIMIT;
    }
    if (GetGroupNumberByOwner(appId) >= HC_TRUST_GROUP_ENTRY_MAX_NUM) {
        LOGE("The number of groups created by the service exceeds the maximum! [AppId]: %s", appId);
        return HC_ERR_BEYOND_LIMIT;
    }
    return HC_SUCCESS;
}

int32_t CheckDeviceNumLimit(const char *groupId, const char *peerUdid)
{
    /*
     * If the peer device does not exist in the group and needs to be added,
     * check whether the number of trusted devices exceeds the upper limit.
     */
    if ((peerUdid != NULL) && (IsTrustedDeviceInGroup(groupId, peerUdid))) {
        return HC_SUCCESS;
    }
    if (GetCurDeviceNumByGroupId(groupId) >= HC_TRUST_DEV_ENTRY_MAX_NUM) {
        LOGE("The number of devices in the group has reached the upper limit!");
        return HC_ERR_BEYOND_LIMIT;
    }
    return HC_SUCCESS;
}

bool IsUserTypeValid(int userType)
{
    if ((userType == DEVICE_TYPE_ACCESSORY) ||
        (userType == DEVICE_TYPE_CONTROLLER) ||
        (userType == DEVICE_TYPE_PROXY)) {
        return true;
    }
    return false;
}

bool IsExpireTimeValid(int expireTime)
{
    if ((expireTime < -1) || (expireTime == 0) || (expireTime > MAX_EXPIRE_TIME)) {
        return false;
    }
    return true;
}

bool IsGroupVisibilityValid(int groupVisibility)
{
    /* Currently, only the public group can be created. */
    if (groupVisibility == GROUP_VISIBILITY_PUBLIC) {
        return true;
    }
    return false;
}

int32_t CheckUserTypeIfExist(const CJson *jsonParams)
{
    int userType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(jsonParams, FIELD_USER_TYPE, &userType);
    if (!IsUserTypeValid(userType)) {
        LOGE("The input userType is invalid! [UserType]: %d", userType);
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

int32_t CheckGroupVisibilityIfExist(const CJson *jsonParams)
{
    int groupVisibility = GROUP_VISIBILITY_PUBLIC;
    (void)GetIntFromJson(jsonParams, FIELD_GROUP_VISIBILITY, &groupVisibility);
    if (!IsGroupVisibilityValid(groupVisibility)) {
        LOGE("The input groupVisibility is invalid! [GroupVisibility]: %d", groupVisibility);
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

int32_t CheckExpireTimeIfExist(const CJson *jsonParams)
{
    int expireTime = DEFAULT_EXPIRE_TIME;
    (void)GetIntFromJson(jsonParams, FIELD_EXPIRE_TIME, &expireTime);
    if (!IsExpireTimeValid(expireTime)) {
        LOGE("Invalid group expire time! [ExpireTime]: %d", expireTime);
        return HC_ERR_INVALID_PARAMS;
    }
    return HC_SUCCESS;
}

int32_t AddGroupNameToParams(const char *groupName, GroupInfo *groupParams)
{
    if (!StringSetPointer(&groupParams->name, groupName)) {
        LOGE("Failed to copy groupName!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddGroupIdToParams(const char *groupId, GroupInfo *groupParams)
{
    if (!StringSetPointer(&groupParams->id, groupId)) {
        LOGE("Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddGroupOwnerToParams(const char *owner, GroupInfo *groupParams)
{
    if (!StringSetPointer(&groupParams->ownerName, owner)) {
        LOGE("Failed to copy groupOwner!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddGroupTypeToParams(int groupType, GroupInfo *groupParams)
{
    groupParams->type = groupType;
    return HC_SUCCESS;
}

int32_t AddGroupVisibilityOrDefault(GroupInfo *groupParams)
{
    /* Currently, only the public group can be created. */
    groupParams->visibility = GROUP_VISIBILITY_PUBLIC;
    return HC_SUCCESS;
}

int32_t AddExpireTimeOrDefault(const CJson *jsonParams, GroupInfo *groupParams)
{
    int expireTime = DEFAULT_EXPIRE_TIME;
    (void)GetIntFromJson(jsonParams, FIELD_EXPIRE_TIME, &expireTime);
    groupParams->expireTime = expireTime;
    return HC_SUCCESS;
}

int32_t AddUdidToParams(DeviceInfo *devParams)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    if (!StringSetPointer(&devParams->udid, (char *)udid)) {
        LOGE("Failed to copy udid!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddAuthIdToParamsOrDefault(const CJson *jsonParams, DeviceInfo *devParams)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    const char *authId = GetStringFromJson(jsonParams, FIELD_DEVICE_ID);
    if (authId == NULL) {
        LOGD("No authId is found. The default value is udid!");
        if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
            LOGE("Get local udid failed");
            return HC_ERROR;
        }
        authId = (const char *)udid;
    }
    if (!StringSetPointer(&devParams->authId, authId)) {
        LOGE("Failed to copy authId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddUserTypeToParamsOrDefault(const CJson *jsonParams, DeviceInfo *devParams)
{
    int userType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(jsonParams, FIELD_USER_TYPE, &userType);
    devParams->devType = userType;
    return HC_SUCCESS;
}

int32_t AddServiceTypeToParams(const char *groupId, DeviceInfo *devParams)
{
    if (!StringSetPointer(&devParams->serviceType, groupId)) {
        LOGE("Failed to copy authId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t AddGroupIdToDevParams(const char *groupId, DeviceInfo *devParams)
{
    if (!StringSetPointer(&devParams->groupId, groupId)) {
        LOGE("Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t CheckGroupExist(const char *groupId)
{
    if (!IsGroupExistByGroupId(groupId)) {
        LOGE("The group does not exist!");
        return HC_ERR_GROUP_NOT_EXIST;
    }
    return HC_SUCCESS;
}

int32_t AddGroupToDatabaseByJson(int32_t (*generateGroupParams)(const CJson*, const char *, GroupInfo*),
    const CJson *jsonParams, const char *groupId)
{
    GroupInfo *groupParams = CreateGroupInfoStruct();
    if (groupParams == NULL) {
        LOGE("Failed to allocate groupParams memory!");
        return HC_ERR_ALLOC_MEMORY;
    }

    int32_t result = (*generateGroupParams)(jsonParams, groupId, groupParams);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupParams);
        return result;
    }

    result = AddGroup(groupParams);
    DestroyGroupInfoStruct(groupParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to add the group to the database!");
    }
    return result;
}

int32_t AddDeviceToDatabaseByJson(int32_t (*generateDevParams)(const CJson*, const char*, DeviceInfo*),
    const CJson *jsonParams, const char *groupId)
{
    DeviceInfo *devParams = CreateDeviceInfoStruct();
    if (devParams == NULL) {
        LOGE("Failed to allocate devParams memory!");
        return HC_ERR_ALLOC_MEMORY;
    }

    int32_t result = (*generateDevParams)(jsonParams, groupId, devParams);
    if (result != HC_SUCCESS) {
        DestroyDeviceInfoStruct(devParams);
        return result;
    }

    result = AddTrustedDevice(devParams, NULL);
    DestroyDeviceInfoStruct(devParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to add the trust device to the database!");
    }
    return result;
}

int32_t DelGroupFromDatabase(const char *groupId)
{
    int32_t result = DelGroupByGroupId(groupId);
    if (result != HC_SUCCESS) {
        LOGE("Failed to delete group from database!");
        return result;
    }
    return HC_SUCCESS;
}

int32_t ConvertGroupIdToJsonStr(const char *groupId, char **returnJsonStr)
{
    CJson *json = CreateJson();
    if (json == NULL) {
        LOGE("Failed to allocate json memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (AddStringToJson(json, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to json!");
        FreeJson(json);
        return HC_ERR_JSON_FAIL;
    }
    *returnJsonStr = PackJsonToString(json);
    FreeJson(json);
    if (*returnJsonStr == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

int32_t GenerateBindSuccessData(const char *peerAuthId, const char *groupId, char **returnDataStr)
{
    if ((peerAuthId == NULL) || (groupId == NULL) || (returnDataStr == NULL)) {
        LOGE("The input params contains NULL value!");
        return HC_ERR_NULL_PTR;
    }
    char *tempGroupId = NULL;
    char *tempAuthId = NULL;
    ConvertToAnnoymousStr(groupId, &tempGroupId);
    ConvertToAnnoymousStr(peerAuthId, &tempAuthId);
    LOGI("Bind successfully! [GroupId]: %s, [AddId]: %s",
        tempGroupId == NULL ? "NULL" : tempGroupId,
        tempAuthId == NULL ? "NULL" : tempAuthId);
    HcFree(tempGroupId);
    HcFree(tempAuthId);
    CJson *jsonData = CreateJson();
    if (jsonData == NULL) {
        LOGE("Failed to allocate jsonData memory!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonData, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to jsonData!");
        FreeJson(jsonData);
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonData, FIELD_ADD_ID, peerAuthId) != HC_SUCCESS) {
        LOGE("Failed to add addId to jsonData!");
        FreeJson(jsonData);
        return HC_ERR_JSON_FAIL;
    }
    char *jsonDataStr = PackJsonToString(jsonData);
    FreeJson(jsonData);
    if (jsonDataStr == NULL) {
        LOGE("An error occurred when converting JSON data to String data!");
        return HC_ERR_JSON_FAIL;
    }
    *returnDataStr = jsonDataStr;
    return HC_SUCCESS;
}

int32_t GenerateUnbindSuccessData(const char *peerAuthId, const char *groupId, char **returnDataStr)
{
    if ((peerAuthId == NULL) || (groupId == NULL) || (returnDataStr == NULL)) {
        LOGE("The input params contains NULL value!");
        return HC_ERR_NULL_PTR;
    }
    char *tempGroupId = NULL;
    char *tempAuthId = NULL;
    ConvertToAnnoymousStr(groupId, &tempGroupId);
    ConvertToAnnoymousStr(peerAuthId, &tempAuthId);
    LOGI("Unbind successfully! [GroupId]: %s, [DeleteId]: %s",
        tempGroupId == NULL ? "NULL" : tempGroupId,
        tempAuthId == NULL ? "NULL" : tempAuthId);
    HcFree(tempGroupId);
    HcFree(tempAuthId);
    CJson *jsonData = CreateJson();
    if (jsonData == NULL) {
        LOGE("Failed to allocate jsonData memory!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonData, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to jsonData!");
        FreeJson(jsonData);
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonData, FIELD_DELETE_ID, peerAuthId) != HC_SUCCESS) {
        LOGE("Failed to add deleteId to jsonData!");
        FreeJson(jsonData);
        return HC_ERR_JSON_FAIL;
    }
    char *jsonDataStr = PackJsonToString(jsonData);
    FreeJson(jsonData);
    if (jsonDataStr == NULL) {
        LOGE("An error occurred when converting JSON data to String data!");
        return HC_ERR_JSON_FAIL;
    }
    *returnDataStr = jsonDataStr;
    return HC_SUCCESS;
}

int32_t ProcessKeyPair(int action, const CJson *jsonParams, const char *groupId)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    /* Use the DeviceGroupManager package name. */
    const char *appId = GROUP_MANAGER_PACKAGE_NAME;
    const char *authId = GetStringFromJson(jsonParams, FIELD_DEVICE_ID);
    if (authId == NULL) {
        LOGD("No authId is found. The default value is udid!");
        if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
            LOGE("Get local udid failed");
            return HC_ERROR;
        }
        authId = (const char *)udid;
    }
    int userType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(jsonParams, FIELD_USER_TYPE, &userType);
    Uint8Buff authIdBuff = { 0, 0 };
    authIdBuff.length = HcStrlen(authId);
    authIdBuff.val = (uint8_t *)HcMalloc(authIdBuff.length, 0);
    if (authIdBuff.val == NULL) {
        LOGE("Failed to allocate authIdBuff memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(authIdBuff.val, authIdBuff.length, authId, authIdBuff.length) != HC_SUCCESS) {
        LOGE("Failed to copy authId!");
        HcFree(authIdBuff.val);
        return HC_ERR_MEMORY_COPY;
    }
    int32_t result;
    if (action == CREATE_KEY_PAIR) {
        LOGI("Start to register identity! [GroupId]: %s, [UserType]: %d", groupId, userType);
        result = RegisterLocalIdentity(appId, groupId, &authIdBuff, userType, DAS_MODULE);
    } else {
        LOGI("Start to deregister identity! [GroupId]: %s, [UserType]: %d", groupId, userType);
        result = UnregisterLocalIdentity(appId, groupId, &authIdBuff, userType, DAS_MODULE);
    }
    HcFree(authIdBuff.val);
    return result;
}

int32_t DeletePeerKeyIfForceUnbind(const char *groupId, const char *peerAuthId, int32_t peerUserType)
{
    /* Use the DeviceGroupManager package name. */
    const char *appId = GROUP_MANAGER_PACKAGE_NAME;
    Uint8Buff peerAuthIdBuff = {
        .val = (uint8_t *)peerAuthId,
        .length = HcStrlen(peerAuthId)
    };
    return DeletePeerAuthInfo(appId, groupId, &peerAuthIdBuff, peerUserType, DAS_MODULE);
}

int32_t GetGroupTypeFromDb(const char *groupId, int *returnGroupType)
{
    GroupInfo *groupEntry = CreateGroupInfoStruct();
    if (groupEntry == NULL) {
        LOGE("Failed to allocate groupEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetGroupEntryByGroupId(groupId, groupEntry) != HC_SUCCESS) {
        LOGE("Failed to get groupEntry from database!");
        DestroyGroupInfoStruct(groupEntry);
        return HC_ERR_DB;
    }
    *returnGroupType = groupEntry->type;
    DestroyGroupInfoStruct(groupEntry);
    return HC_SUCCESS;
}

int32_t CheckPermForGroup(int actionType, const char *callerPkgName, const char *groupId)
{
    if (((actionType == GROUP_DISBAND) && (IsGroupOwner(groupId, callerPkgName))) ||
        ((actionType == MEMBER_INVITE) && (IsGroupEditAllowed(groupId, callerPkgName))) ||
        ((actionType == MEMBER_DELETE) && (IsGroupEditAllowed(groupId, callerPkgName)))) {
        return HC_SUCCESS;
    }
    LOGE("You do not have the right to execute the command!");
    return HC_ERR_ACCESS_DENIED;
}