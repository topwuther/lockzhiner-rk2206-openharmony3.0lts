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

#include "bind_session_common.h"
#include "callback_manager.h"
#include "channel_manager.h"
#include "database_manager.h"
#include "dev_auth_module_manager.h"
#include "group_common.h"
#include "hc_dev_info.h"
#include "hc_log.h"
#include "session_common.h"

static int32_t AddGroupInfoToSendData(const BindSession *session, CJson *data)
{
    const char *groupId = GetStringFromJson(session->params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from params!");
        return HC_ERR_JSON_GET;
    }
    const char *groupName = GetStringFromJson(session->params, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(data, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(data, FIELD_GROUP_NAME, groupName) != HC_SUCCESS) {
        LOGE("Failed to add groupName to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(data, FIELD_GROUP_OP, session->operationCode) != HC_SUCCESS) {
        LOGE("Failed to add groupOp to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(data, FIELD_GROUP_TYPE, PEER_TO_PEER_GROUP) != HC_SUCCESS) {
        LOGE("Failed to add groupType to data!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddDevInfoToSendData(const BindSession *session, CJson *data)
{
    const char *authId = GetStringFromJson(session->params, FIELD_AUTH_ID);
    if (authId == NULL) {
        LOGE("Failed to get authId from params!");
        return HC_ERR_JSON_GET;
    }
    const char *udid = GetStringFromJson(session->params, FIELD_CONN_DEVICE_ID);
    if (udid == NULL) {
        LOGE("Failed to get udid from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(data, FIELD_PEER_DEVICE_ID, authId) != HC_SUCCESS) {
        LOGE("Failed to add peerDeviceId to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(data, FIELD_CONN_DEVICE_ID, udid) != HC_SUCCESS) {
        LOGE("Failed to add connDeviceId to data!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddRequestInfoToSendData(const BindSession *session, CJson *data)
{
    const char *appId = GetStringFromJson(session->params, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(data, FIELD_APP_ID, appId) != HC_SUCCESS) {
        LOGE("Failed to add appId to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddInt64StringToJson(data, FIELD_REQUEST_ID, session->requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to data!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(data, FIELD_OWNER_NAME, "") != HC_SUCCESS) {
        LOGE("Failed to add ownerName to data!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GenerateCompatibleInfo(CJson *groupInfo)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    if (AddStringToJson(groupInfo, FIELD_DEVICE_ID, (const char *)udid) != HC_SUCCESS) {
        LOGE("Failed to add deviceId to groupInfo!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddBoolToJson(groupInfo, FIELD_IS_UUID, true) != HC_SUCCESS) {
        LOGE("Failed to add uuIdAsDeviceId to groupInfo!");
        return HC_ERR_JSON_FAIL;
    }
    /* To be compatible with packets of earlier versions. */
    CJson *managers = CreateJsonArray();
    if (managers == NULL) {
        LOGE("Failed to allocate managers memory!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddObjToJson(groupInfo, FIELD_GROUP_MANAGERS, managers) != HC_SUCCESS) {
        LOGE("Failed to add groupManagers to groupInfo!");
        FreeJson(managers);
        return HC_ERR_JSON_FAIL;
    }
    FreeJson(managers);
    /* Currently, only the public group can be created. */
    if (AddIntToJson(groupInfo, FIELD_GROUP_VISIBILITY, GROUP_VISIBILITY_PUBLIC) != HC_SUCCESS) {
        LOGE("Failed to add groupVisibility to groupInfo!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddCompatibleInfoToSendData(bool isNeedCompatibleInfo, CJson *data)
{
    if (!isNeedCompatibleInfo) {
        return HC_SUCCESS;
    }
    CJson *groupInfo = CreateJson();
    if (groupInfo == NULL) {
        LOGE("Failed to allocate groupInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GenerateCompatibleInfo(groupInfo) != HC_SUCCESS) {
        FreeJson(groupInfo);
        return HC_ERR_JSON_FAIL;
    }
    if (AddObjToJson(data, FIELD_GROUP_INFO, groupInfo) != HC_SUCCESS) {
        LOGE("Failed to add groupInfo to sendData!");
        FreeJson(groupInfo);
        return HC_ERR_JSON_FAIL;
    }
    FreeJson(groupInfo);
    return HC_SUCCESS;
}

static int32_t AddGroupAndDevInfoToParams(const BindSession *session, CJson *moduleParams)
{
    const char *groupId = GetStringFromJson(session->params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from params!");
        return HC_ERR_JSON_GET;
    }
    const char *authId = GetStringFromJson(session->params, FIELD_AUTH_ID);
    if (authId == NULL) {
        LOGE("Failed to get authId from params!");
        return HC_ERR_JSON_GET;
    }
    int userType = DEVICE_TYPE_ACCESSORY;
    if (GetIntFromJson(session->params, FIELD_USER_TYPE, &userType) != HC_SUCCESS) {
        LOGE("Failed to get userType from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(moduleParams, FIELD_SERVICE_TYPE, groupId) != HC_SUCCESS) {
        LOGE("Failed to add serviceType to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(moduleParams, FIELD_SELF_AUTH_ID, authId) != HC_SUCCESS) {
        LOGE("Failed to add serviceType to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(moduleParams, FIELD_SELF_TYPE, userType) != HC_SUCCESS) {
        LOGE("Failed to add userType to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddRequestInfoToParams(bool isClient, const BindSession *session, CJson *moduleParams)
{
    if (AddByteToJson(moduleParams, FIELD_REQUEST_ID,
        (const uint8_t *)&session->requestId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to add requestId to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(moduleParams, FIELD_KEY_LENGTH, DEFAULT_RETURN_KEY_LENGTH) != HC_SUCCESS) {
        LOGE("Failed to add sessionKeyLength to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddBoolToJson(moduleParams, FIELD_IS_CLIENT, isClient) != HC_SUCCESS) {
        LOGE("Failed to add isClient to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    /* Use the GroupManager package name. */
    if (AddStringToJson(moduleParams, FIELD_PKG_NAME, GROUP_MANAGER_PACKAGE_NAME) != HC_SUCCESS) {
        LOGE("Failed to add pkgName to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPinCodeToParamsIfNeed(BindSession *session, CJson *moduleParams)
{
    if (session->operationCode == MEMBER_DELETE) {
        return HC_SUCCESS;
    }
    const char *pinCode = GetStringFromJson(session->params, FIELD_PIN_CODE);
    if (pinCode == NULL) {
        LOGE("Failed to get pinCode from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(moduleParams, FIELD_PIN_CODE, pinCode) != HC_SUCCESS) {
        LOGE("Failed to add pinCode to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    /* Release the memory in advance to reduce the memory usage. */
    (void)DeleteItemFromJson(session->params, FIELD_PIN_CODE);
    return HC_SUCCESS;
}

static int32_t AddPeerAuthIdToParams(BindSession *session, CJson *moduleParams)
{
    const char *peerAuthId = GetStringFromJson(session->params, FIELD_PEER_AUTH_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(moduleParams, FIELD_PEER_AUTH_ID, peerAuthId) != HC_SUCCESS) {
        LOGE("Failed to add peerAuthId to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPeerUserTypeToParams(BindSession *session, CJson *moduleParams)
{
    int peerUserType = DEVICE_TYPE_ACCESSORY;
    if (GetIntFromJson(session->params, FIELD_PEER_USER_TYPE, &peerUserType) != HC_SUCCESS) {
        LOGE("Failed to get peerUserType from params!");
        return HC_ERR_JSON_GET;
    }
    if (AddIntToJson(moduleParams, FIELD_PEER_USER_TYPE, peerUserType) != HC_SUCCESS) {
        LOGE("Failed to add peerUserType to moduleParams!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPeerAuthIdAndUserTypeToParamsIfNeed(BindSession *session, CJson *moduleParams)
{
    if (session->operationCode != MEMBER_DELETE) {
        return HC_SUCCESS;
    }
    int32_t result = AddPeerAuthIdToParams(session, moduleParams);
    if (result != HC_SUCCESS) {
        return result;
    }
    return AddPeerUserTypeToParams(session, moduleParams);
}

static int32_t TryAddPeerUserTypeToParams(const CJson *jsonParams, BindSession *session)
{
    int peerUserType = DEVICE_TYPE_ACCESSORY;
    if (GetIntFromJson(jsonParams, FIELD_PEER_USER_TYPE, &peerUserType) == HC_SUCCESS) {
        if (AddIntToJson(session->params, FIELD_PEER_USER_TYPE, peerUserType) != HC_SUCCESS) {
            LOGE("Failed to add peerUserType to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t AddGroupInfoToParams(const GroupInfo *entry, CJson *params)
{
    if (AddStringToJson(params, FIELD_GROUP_ID, StringGet(&entry->id)) != HC_SUCCESS) {
        LOGE("Failed to add groupId to json!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(params, FIELD_GROUP_TYPE, entry->type) != HC_SUCCESS) {
        LOGE("Failed to add groupType to json!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(params, FIELD_GROUP_NAME, StringGet(&entry->name)) != HC_SUCCESS) {
        LOGE("Failed to add groupName to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddDevInfoToParams(const DeviceInfo *devAuthParams, CJson *params)
{
    if (AddStringToJson(params, FIELD_AUTH_ID, StringGet(&devAuthParams->authId)) != HC_SUCCESS) {
        LOGE("Failed to add authId to params!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(params, FIELD_CONN_DEVICE_ID, StringGet(&devAuthParams->udid)) != HC_SUCCESS) {
        LOGE("Failed to add udid to params!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddIntToJson(params, FIELD_USER_TYPE, devAuthParams->devType) != HC_SUCCESS) {
        LOGE("Failed to add userType to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupInfoByDatabase(const char *groupId, CJson *params)
{
    GroupInfo *entry = CreateGroupInfoStruct();
    if (entry == NULL) {
        LOGE("Failed to allocate groupEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetGroupEntryByGroupId(groupId, entry) != HC_SUCCESS) {
        LOGE("Failed to obtain the group information from the database!");
        DestroyGroupInfoStruct(entry);
        return HC_ERR_DB;
    }
    if (AddGroupInfoToParams(entry, params) != HC_SUCCESS) {
        DestroyGroupInfoStruct(entry);
        return HC_ERR_JSON_FAIL;
    }
    DestroyGroupInfoStruct(entry);
    return HC_SUCCESS;
}

static int32_t AddDevInfoByDatabase(const char *groupId, CJson *params)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    DeviceInfo *devAuthParams = CreateDeviceInfoStruct();
    if (devAuthParams == NULL) {
        LOGE("Failed to allocate devEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetDeviceInfoForDevAuth((const char *)udid, groupId, devAuthParams) != HC_SUCCESS) {
        LOGE("Failed to obtain the device information from the database!");
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_DB;
    }
    if (AddDevInfoToParams(devAuthParams, params) != HC_SUCCESS) {
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_JSON_FAIL;
    }
    DestroyDeviceInfoStruct(devAuthParams);
    return HC_SUCCESS;
}

static int32_t AddGroupId(const char *groupId, CJson *params)
{
    if (AddStringToJson(params, FIELD_GROUP_ID, groupId) != HC_SUCCESS) {
        LOGE("Failed to add groupId to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupName(const CJson *jsonParams, CJson *params)
{
    const char *groupName = GetStringFromJson(jsonParams, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(params, FIELD_GROUP_NAME, groupName) != HC_SUCCESS) {
        LOGE("Failed to add groupName to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupOwnerIfExist(const CJson *jsonParams, CJson *params)
{
    const char *groupOwner = GetStringFromJson(jsonParams, FIELD_GROUP_OWNER);
    if (groupOwner != NULL) {
        if (AddStringToJson(params, FIELD_GROUP_OWNER, groupOwner) != HC_SUCCESS) {
            LOGE("Failed to add groupOwner to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t AddGroupTypeIfValid(const CJson *jsonParams, CJson *params)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetIntFromJson(jsonParams, FIELD_GROUP_TYPE, &groupType) != HC_SUCCESS) {
        LOGE("Failed to get groupType from in!");
        return HC_ERR_JSON_GET;
    }
    if (groupType != PEER_TO_PEER_GROUP) {
        LOGE("The input groupType is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (AddIntToJson(params, FIELD_GROUP_TYPE, groupType) != HC_SUCCESS) {
        LOGE("Failed to add groupType to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupVisibilityIfValidOrDefault(const CJson *jsonParams, CJson *params)
{
    int groupVisibility = GROUP_VISIBILITY_PUBLIC;
    (void)GetIntFromJson(jsonParams, FIELD_GROUP_VISIBILITY, &groupVisibility);
    if (!IsGroupVisibilityValid(groupVisibility)) {
        LOGE("The input groupVisibility is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (AddIntToJson(params, FIELD_GROUP_VISIBILITY, groupVisibility) != HC_SUCCESS) {
        LOGE("Failed to add groupVisibility to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddExpireTimeIfValidOrDefault(const CJson *jsonParams, CJson *params)
{
    int expireTime = DEFAULT_EXPIRE_TIME;
    (void)GetIntFromJson(jsonParams, FIELD_EXPIRE_TIME, &expireTime);
    if (!IsExpireTimeValid(expireTime)) {
        LOGE("The input expireTime is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (AddIntToJson(params, FIELD_EXPIRE_TIME, expireTime) != HC_SUCCESS) {
        LOGE("Failed to add expireTime to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t CheckAuthIdAndUserTypeValid(int userType, const char *groupId, const char *authId)
{
    if (!IsGroupExistByGroupId(groupId)) {
        return HC_SUCCESS;
    }
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    DeviceInfo *deviceInfo = CreateDeviceInfoStruct();
    if (deviceInfo == NULL) {
        LOGE("Failed to allocate deviceInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GetDeviceInfoForDevAuth((char *)udid, groupId, deviceInfo);
    if (result != HC_SUCCESS) {
        LOGE("Failed to obtain the local device information from the database!");
        DestroyDeviceInfoStruct(deviceInfo);
        return result;
    }
    if ((deviceInfo->devType != userType) || (strcmp(StringGet(&deviceInfo->authId), authId) != 0)) {
        LOGE("Once a group is created, the service cannot change the local authId and userType used in the group!");
        DestroyDeviceInfoStruct(deviceInfo);
        return HC_ERR_INVALID_PARAMS;
    }
    DestroyDeviceInfoStruct(deviceInfo);
    return HC_SUCCESS;
}

static int32_t AddAuthIdAndUserTypeIfValidOrDefault(const char *groupId, const CJson *jsonParams, CJson *params)
{
    int userType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(jsonParams, FIELD_USER_TYPE, &userType);
    if (!IsUserTypeValid(userType)) {
        LOGE("The input userType is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
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
    int32_t result = CheckAuthIdAndUserTypeValid(userType, groupId, authId);
    if (result != HC_SUCCESS) {
        return result;
    }
    if (AddIntToJson(params, FIELD_USER_TYPE, userType) != HC_SUCCESS) {
        LOGE("Failed to add userType to params!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(params, FIELD_AUTH_ID, authId) != HC_SUCCESS) {
        LOGE("Failed to add authId to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddUdid(CJson *params)
{
    uint8_t udid[INPUT_UDID_LEN] = { 0 };
    if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
        LOGE("Get local udid failed");
        return HC_ERROR;
    }
    if (AddStringToJson(params, FIELD_CONN_DEVICE_ID, (const char *)udid) != HC_SUCCESS) {
        LOGE("Failed to add udid to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddUserTypeIfValidOrDefault(const CJson *jsonParams, CJson *params)
{
    int userType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(jsonParams, FIELD_USER_TYPE, &userType);
    if (!IsUserTypeValid(userType)) {
        LOGE("The input userType is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (AddIntToJson(params, FIELD_USER_TYPE, userType) != HC_SUCCESS) {
        LOGE("Failed to add userType to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupInfoToSessionParams(const char *groupId, const CJson *jsonParams, CJson *params)
{
    int32_t result;
    if (((result = AddGroupId(groupId, params)) != HC_SUCCESS) ||
        ((result = AddGroupName(jsonParams, params)) != HC_SUCCESS) ||
        ((result = AddGroupOwnerIfExist(jsonParams, params)) != HC_SUCCESS) ||
        ((result = AddGroupTypeIfValid(jsonParams, params)) != HC_SUCCESS) ||
        ((result = AddGroupVisibilityIfValidOrDefault(jsonParams, params)) != HC_SUCCESS) ||
        ((result = AddExpireTimeIfValidOrDefault(jsonParams, params)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t AddDevInfoToSessionParams(const char *groupId, const CJson *jsonParams, CJson *params)
{
    int32_t result;
    if (((result = AddAuthIdAndUserTypeIfValidOrDefault(groupId, jsonParams, params)) != HC_SUCCESS) ||
        ((result = AddUdid(params)) != HC_SUCCESS) ||
        ((result = AddUserTypeIfValidOrDefault(jsonParams, params)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t GenerateParamsByInput(const char *groupId, const CJson *jsonParams, CJson *params)
{
    int32_t result = AddGroupInfoToSessionParams(groupId, jsonParams, params);
    if (result != HC_SUCCESS) {
        return result;
    }
    return AddDevInfoToSessionParams(groupId, jsonParams, params);
}

static int32_t GenerateParamsByDatabase(const char *groupId, CJson *params)
{
    int32_t result = AddGroupInfoByDatabase(groupId, params);
    if (result != HC_SUCCESS) {
        return result;
    }
    return AddDevInfoByDatabase(groupId, params);
}

static int32_t AddIsForceDeleteIfNeed(int isClient, const CJson *jsonParams, BindSession *session)
{
    if ((isClient == CLIENT) && (session->operationCode == MEMBER_DELETE)) {
        bool isForceDelete = false;
        if (GetBoolFromJson(jsonParams, FIELD_IS_FORCE_DELETE, &isForceDelete) != HC_SUCCESS) {
            LOGE("Failed to get isForceDelete from jsonParams!");
            return HC_ERR_JSON_GET;
        }
        if (AddBoolToJson(session->params, FIELD_IS_FORCE_DELETE, isForceDelete) != HC_SUCCESS) {
            LOGE("Failed to add isForceDelete to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t AddChannelIdIfNeed(int isClient, const CJson *jsonParams, BindSession *session)
{
    if ((isClient == SERVER) && (session->channelType == SOFT_BUS)) {
        int64_t channelId = DEFAULT_CHANNEL_ID;
        if (GetByteFromJson(jsonParams, FIELD_CHANNEL_ID, (uint8_t *)&channelId, sizeof(int64_t)) != HC_SUCCESS) {
            LOGE("Failed to get channelId from jsonParams!");
            return HC_ERR_JSON_GET;
        }
        session->channelId = channelId;
    }
    return HC_SUCCESS;
}

static int32_t AddPinCodeIfNeed(const CJson *jsonParams, BindSession *session)
{
    if (session->operationCode != MEMBER_DELETE) {
        const char *pinCode = GetStringFromJson(jsonParams, FIELD_PIN_CODE);
        if (pinCode == NULL) {
            LOGE("Failed to get pinCode from jsonParams!");
            return HC_ERR_JSON_GET;
        }
        if (AddStringToJson(session->params, FIELD_PIN_CODE, pinCode) != HC_SUCCESS) {
            LOGE("Failed to add pinCode to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t AddPeerAuthIdAndUdidIfExist(const CJson *jsonParams, BindSession *session)
{
    const char *peerAuthId = GetStringFromJson(jsonParams, FIELD_PEER_DEVICE_ID);
    const char *peerUdid = GetStringFromJson(jsonParams, FIELD_PEER_UDID);
    if (peerAuthId != NULL) {
        if (AddStringToJson(session->params, FIELD_PEER_AUTH_ID, peerAuthId) != HC_SUCCESS) {
            LOGE("Failed to add peerAuthId to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    if (peerUdid != NULL) {
        if (AddStringToJson(session->params, FIELD_PEER_UDID, peerUdid) != HC_SUCCESS) {
            LOGE("Failed to add peerUdid to params!");
            return HC_ERR_JSON_FAIL;
        }
    }
    return HC_SUCCESS;
}

static int32_t AddPeerAuthIdIfDelete(bool isClient, const CJson *jsonParams, BindSession *session)
{
    const char *peerAuthId = NULL;
    if (isClient) {
        peerAuthId = GetStringFromJson(jsonParams, FIELD_DELETE_ID);
    } else {
        peerAuthId = GetStringFromJson(jsonParams, FIELD_PEER_DEVICE_ID);
    }
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(session->params, FIELD_PEER_AUTH_ID, peerAuthId) != HC_SUCCESS) {
        LOGE("Failed to add peerAuthId to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddPeerUserTypeIfDelete(BindSession *session)
{
    const char *peerAuthId = GetStringFromJson(session->params, FIELD_PEER_AUTH_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from params!");
        return HC_ERR_JSON_GET;
    }
    const char *groupId = GetStringFromJson(session->params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from params!");
        return HC_ERR_JSON_GET;
    }
    DeviceInfo *devAuthParams = CreateDeviceInfoStruct();
    if (devAuthParams == NULL) {
        LOGE("Failed to allocate devEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (GetDeviceInfoByAuthId(peerAuthId, groupId, devAuthParams) != HC_SUCCESS) {
        LOGE("Failed to obtain the device information from the database!");
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_DB;
    }
    if (AddIntToJson(session->params, FIELD_PEER_USER_TYPE, devAuthParams->devType) != HC_SUCCESS) {
        DestroyDeviceInfoStruct(devAuthParams);
        return HC_ERR_JSON_FAIL;
    }
    DestroyDeviceInfoStruct(devAuthParams);
    return HC_SUCCESS;
}

static int32_t AddPeerDevInfoIfNeed(bool isClient, const CJson *jsonParams, BindSession *session)
{
    if (session->operationCode == MEMBER_DELETE) {
        int32_t result = AddPeerAuthIdIfDelete(isClient, jsonParams, session);
        if (result != HC_SUCCESS) {
            return result;
        }
        return AddPeerUserTypeIfDelete(session);
    }
    return AddPeerAuthIdAndUdidIfExist(jsonParams, session);
}

static int32_t AddGroupAndDevInfo(int isClient, const CJson *jsonParams, BindSession *session)
{
    const char *groupId = GetStringFromJson(jsonParams, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    if (NeedCreateGroup(isClient, session->operationCode)) {
        return GenerateParamsByInput(groupId, jsonParams, session->params);
    } else {
        return GenerateParamsByDatabase(groupId, session->params);
    }
}

static int32_t AddAppId(const CJson *jsonParams, BindSession *session)
{
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from in!");
        return HC_ERR_JSON_GET;
    }
    if (AddStringToJson(session->params, FIELD_APP_ID, appId) != HC_SUCCESS) {
        LOGE("Failed to add appId to params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t InteractWithPeer(const BindSession *session, CJson *sendData)
{
    int32_t result = AddInfoToSendData(false, session, sendData);
    if (result != HC_SUCCESS) {
        LOGE("Failed to generate sendData!");
        return result;
    }
    return SendBindSessionData(session, sendData);
}

static int32_t InformSelfBindSuccess(const char *peerAuthId, const char *groupId, const BindSession *session,
    CJson *out)
{
    uint8_t sessionKey[DEFAULT_RETURN_KEY_LENGTH] = { 0 };
    if (GetByteFromJson(out, FIELD_SESSION_KEY, sessionKey, DEFAULT_RETURN_KEY_LENGTH) == HC_SUCCESS) {
        ProcessSessionKeyCallback(session->requestId, sessionKey, DEFAULT_RETURN_KEY_LENGTH, session->base.callback);
        (void)memset_s(sessionKey, DEFAULT_RETURN_KEY_LENGTH, 0, DEFAULT_RETURN_KEY_LENGTH);
        ClearSensitiveStringInJson(out, FIELD_SESSION_KEY);
    }

    char *jsonDataStr = NULL;
    int32_t result = GenerateBindSuccessData(peerAuthId, groupId, &jsonDataStr);
    if (result != HC_SUCCESS) {
        LOGE("Failed to generate the data to be sent to the service!");
        return result;
    }
    ProcessFinishCallback(session->requestId, session->operationCode, jsonDataStr, session->base.callback);
    FreeJsonString(jsonDataStr);
    return HC_SUCCESS;
}

static int32_t InformSelfUnbindSuccess(const char *peerAuthId, const char *groupId, const BindSession *session)
{
    char *jsonDataStr = NULL;
    int32_t result = GenerateUnbindSuccessData(peerAuthId, groupId, &jsonDataStr);
    if (result != HC_SUCCESS) {
        LOGE("Failed to generate the data to be sent to the service!");
        return result;
    }
    ProcessFinishCallback(session->requestId, session->operationCode, jsonDataStr, session->base.callback);
    FreeJsonString(jsonDataStr);
    return HC_SUCCESS;
}

static int32_t SetGroupId(const CJson *params, GroupInfo *groupParams)
{
    const char *groupId = GetStringFromJson(params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from params!");
        return HC_ERR_JSON_GET;
    }
    if (!StringSetPointer(&groupParams->id, groupId)) {
        LOGE("Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t SetGroupName(const CJson *params, GroupInfo *groupParams)
{
    const char *groupName = GetStringFromJson(params, FIELD_GROUP_NAME);
    if (groupName == NULL) {
        LOGE("Failed to get groupName from params!");
        return HC_ERR_JSON_GET;
    }
    if (!StringSetPointer(&groupParams->name, groupName)) {
        LOGE("Failed to copy groupName!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t SetGroupOwner(const CJson *params, GroupInfo *groupParams)
{
    const char *groupOwner = GetStringFromJson(params, FIELD_APP_ID);
    if (groupOwner == NULL) {
        LOGE("Failed to get appId from params!");
        return HC_ERR_JSON_GET;
    }
    if (!StringSetPointer(&groupParams->ownerName, groupOwner)) {
        LOGE("Failed to copy groupOwner!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t SetGroupType(const CJson *params, GroupInfo *groupParams)
{
    int32_t groupType = PEER_TO_PEER_GROUP;
    if (GetIntFromJson(params, FIELD_GROUP_TYPE, &groupType) != HC_SUCCESS) {
        LOGE("Failed to get groupType from params!");
        return HC_ERR_JSON_GET;
    }
    groupParams->type = groupType;
    return HC_SUCCESS;
}

static int32_t SetGroupVisibility(const CJson *params, GroupInfo *groupParams)
{
    int groupVisibility = GROUP_VISIBILITY_PUBLIC;
    (void)GetIntFromJson(params, FIELD_GROUP_VISIBILITY, &groupVisibility);
    groupParams->visibility = groupVisibility;
    return HC_SUCCESS;
}

static int32_t SetGroupExpireTime(const CJson *params, GroupInfo *groupParams)
{
    int expireTime = DEFAULT_EXPIRE_TIME;
    (void)GetIntFromJson(params, FIELD_EXPIRE_TIME, &expireTime);
    groupParams->expireTime = expireTime;
    return HC_SUCCESS;
}

static int32_t ForceDeletePeerKey(CJson *params)
{
    /* Use the DeviceGroupManager package name. */
    const char *appId = GROUP_MANAGER_PACKAGE_NAME;
    const char *peerAuthId = GetStringFromJson(params, FIELD_PEER_AUTH_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from params!");
        return HC_ERR_JSON_GET;
    }
    const char *groupId = GetStringFromJson(params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from params!");
        return HC_ERR_JSON_GET;
    }
    int peerUserType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(params, FIELD_PEER_USER_TYPE, &peerUserType);
    Uint8Buff peerAuthIdBuff = {
        .val = (uint8_t *)peerAuthId,
        .length = HcStrlen(peerAuthId)
    };
    return DeletePeerAuthInfo(appId, groupId, &peerAuthIdBuff, peerUserType, DAS_MODULE);
}

static int32_t GenerateGroupParams(const CJson *params, GroupInfo *groupParams)
{
    int32_t result;
    if (((result = SetGroupId(params, groupParams)) != HC_SUCCESS) ||
        ((result = SetGroupName(params, groupParams)) != HC_SUCCESS) ||
        ((result = SetGroupOwner(params, groupParams)) != HC_SUCCESS) ||
        ((result = SetGroupType(params, groupParams)) != HC_SUCCESS) ||
        ((result = SetGroupVisibility(params, groupParams)) != HC_SUCCESS) ||
        ((result = SetGroupExpireTime(params, groupParams)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupToDatabase(const CJson *params)
{
    GroupInfo *groupParams = CreateGroupInfoStruct();
    if (groupParams == NULL) {
        LOGE("Failed to allocate groupParams memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GenerateGroupParams(params, groupParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to generate groupParams!");
        DestroyGroupInfoStruct(groupParams);
        return HC_ERR_DB;
    }
    result = AddGroup(groupParams);
    DestroyGroupInfoStruct(groupParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to add the group to the database!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t GenerateDevAuthParams(const char *authId, const char *udid, const char *groupId,
    int userType, DeviceInfo *devAuthParams)
{
    devAuthParams->devType = userType;
    StringSetPointer(&(devAuthParams->authId), authId);
    StringSetPointer(&(devAuthParams->udid), udid);
    StringSetPointer(&(devAuthParams->groupId), groupId);
    StringSetPointer(&(devAuthParams->serviceType), groupId);
    return HC_SUCCESS;
}

static int32_t AddTrustDevToDatabase(const char *authId, const char *udid, const char *groupId, int userType)
{
    DeviceInfo *devAuthParams = CreateDeviceInfoStruct();
    if (devAuthParams == NULL) {
        LOGE("Failed to allocate devAuthParams memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GenerateDevAuthParams(authId, udid, groupId, userType, devAuthParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to generate devAuthParams!");
        DestroyDeviceInfoStruct(devAuthParams);
        return result;
    }
    result = AddTrustedDevice(devAuthParams, NULL);
    DestroyDeviceInfoStruct(devAuthParams);
    if (result != HC_SUCCESS) {
        LOGE("Failed to add the trusted devices to the database!");
        return HC_ERR_DB;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupAndLocalDevIfNotExist(const char *groupId, const BindSession *session)
{
    int32_t result = HC_SUCCESS;
    if (!IsGroupExistByGroupId(groupId)) {
        result = AddGroupToDatabase(session->params);
        if (result != HC_SUCCESS) {
            return result;
        }
        uint8_t udid[INPUT_UDID_LEN] = { 0 };
        if (HcGetUdid(udid, INPUT_UDID_LEN) != HC_SUCCESS) {
            LOGE("Get local udid failed");
            return HC_ERROR;
        }
        const char *authId = GetStringFromJson(session->params, FIELD_AUTH_ID);
        if (authId == NULL) {
            LOGI("No authId is found. The default value is udid!");
            authId = (const char *)udid;
        }
        int userType = DEVICE_TYPE_ACCESSORY;
        (void)GetIntFromJson(session->params, FIELD_USER_TYPE, &userType);
        return AddTrustDevToDatabase(authId, (char *)udid, groupId, userType);
    }
    LOGI("The group corresponding to the groupId already exists. "
         "Therefore, the group will not be created again, and some data transferred by the service may be ignored!");
    return result;
}

static int32_t AddPeerDevToGroup(const char *peerAuthId, const char *peerUdid,
    const char *groupId, const BindSession *session)
{
    if (IsTrustedDeviceInGroupByAuthId(groupId, peerAuthId)) {
        LOGI("The peer device already exists in the group! RequestId: %" PRId64, session->requestId);
        if (DelTrustedDeviceByAuthId(peerAuthId, groupId) != HC_SUCCESS) {
            LOGE("Failed to delete the original data! RequestId: %" PRId64, session->requestId);
            return HC_ERR_DB;
        }
        LOGI("Delete the original data successfully! RequestId: %" PRId64, session->requestId);
    }
    int peerUserType = DEVICE_TYPE_ACCESSORY;
    (void)GetIntFromJson(session->params, FIELD_PEER_USER_TYPE, &peerUserType);
    int32_t result = AddTrustDevToDatabase(peerAuthId, peerUdid, groupId, peerUserType);
    if (result != HC_SUCCESS) {
        LOGE("Failed to update the peer trusted device information! RequestId: %" PRId64, session->requestId);
        return result;
    }
    LOGI("The peer trusted device is added to the database successfully! RequestId: %" PRId64, session->requestId);
    return HC_SUCCESS;
}

static int32_t AddGroupAndDev(const char *peerAuthId, const char *peerUdid,
    const char *groupId, const BindSession *session)
{
    int32_t result = AddGroupAndLocalDevIfNotExist(groupId, session);
    if (result != HC_SUCCESS) {
        return result;
    }
    return AddPeerDevToGroup(peerAuthId, peerUdid, groupId, session);
}

static int32_t HandleBindSuccess(const char *peerAuthId, const char *peerUdid,
    const char *groupId, const BindSession *session, CJson *out)
{
    int32_t result = AddGroupAndDev(peerAuthId, peerUdid, groupId, session);
    if (result != HC_SUCCESS) {
        return result;
    }
    return InformSelfBindSuccess(peerAuthId, groupId, session, out);
}

static int32_t HandleUnbindSuccess(const char *peerAuthId, const char *groupId, const BindSession *session)
{
    if (IsGroupExistByGroupId(groupId)) {
        if (DelTrustedDeviceByAuthId(peerAuthId, groupId) != HC_SUCCESS) {
            LOGE("Failed to unbind device from database!");
            return HC_ERR_DB;
        }
        LOGI("The device is successfully unbound from the database!");
    }
    return InformSelfUnbindSuccess(peerAuthId, groupId, session);
}

static int32_t OnBindOrUnbindFinish(const BindSession *session, const CJson *jsonParams, CJson *out)
{
    const char *peerAuthId = GetStringFromJson(jsonParams, FIELD_PEER_DEVICE_ID);
    if (peerAuthId == NULL) {
        peerAuthId = GetStringFromJson(session->params, FIELD_PEER_AUTH_ID);
    }
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from jsonParams and params!");
        return HC_ERR_JSON_GET;
    }
    const char *peerUdid = GetStringFromJson(jsonParams, FIELD_CONN_DEVICE_ID);
    if (peerUdid == NULL) {
        peerUdid = GetStringFromJson(session->params, FIELD_PEER_UDID);
    }
    if (peerUdid == NULL) {
        LOGE("Failed to get peerUdid from jsonParams and params!");
        return HC_ERR_JSON_GET;
    }
    const char *groupId = GetStringFromJson(session->params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from session params!");
        return HC_ERR_JSON_GET;
    }
    if (session->operationCode == MEMBER_DELETE) {
        return HandleUnbindSuccess(peerAuthId, groupId, session);
    } else {
        return HandleBindSuccess(peerAuthId, peerUdid, groupId, session, out);
    }
}

static int32_t OnSessionFinish(const BindSession *session, CJson *jsonParams, CJson *out)
{
    int32_t result;
    CJson *sendData = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    /* The last packet may need to be sent. */
    if (sendData != NULL) {
        result = InteractWithPeer(session, sendData);
        if (result != HC_SUCCESS) {
            return result;
        }
    }
    result = OnBindOrUnbindFinish(session, jsonParams, out);
    if (result != HC_SUCCESS) {
        LOGE("An error occurred when processing different end operations!");
        return result;
    }
    LOGI("The device is successfully %s!", (session->operationCode == MEMBER_DELETE) ? "unbound" : "bound");
    SetAuthResult(session->channelType, session->channelId);
    CloseChannel(session->channelType, session->channelId);
    return HC_SUCCESS;
}

static int32_t CheckPeerStatus(const CJson *jsonParams, bool *isNeedInform)
{
    int32_t errorCode = 0;
    if (GetIntFromJson(jsonParams, FIELD_GROUP_ERROR_MSG, &errorCode) == HC_SUCCESS) {
        LOGE("An error occurs in the peer service! [ErrorCode]: %d", errorCode);
        *isNeedInform = false;
        return errorCode;
    }
    return HC_SUCCESS;
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

static int32_t ProcessBindSessionInner(BindSession *session, CJson *jsonParams, int32_t *status, bool *isNeedInform)
{
    int32_t result;
    if (((result = CheckPeerStatus(jsonParams, isNeedInform)) != HC_SUCCESS) ||
        ((result = TryAddPeerUserTypeToParams(jsonParams, session))) != HC_SUCCESS) {
        return result;
    }

    CJson *out = CreateJson();
    if (out == NULL) {
        LOGE("Failed to allocate out memory!");
        return HC_ERR_JSON_FAIL;
    }
    result = ProcessModule(session, jsonParams, out, status);
    if (result != HC_SUCCESS) {
        *isNeedInform = false;
        InformPeerModuleErrorIfNeed(out, session);
        FreeJson(out);
        return result;
    }
    if (*status == IGNORE_MSG) {
        FreeJson(out);
        return HC_SUCCESS;
    } else if (*status == CONTINUE) {
        DeleteAllItem(jsonParams);
        CJson *sendData = DetachItemFromJson(out, FIELD_SEND_TO_PEER);
        FreeJson(out);
        if (sendData == NULL) {
            LOGE("Failed to get sendToPeer from out!");
            return HC_ERR_JSON_GET;
        }
        result = InteractWithPeer(session, sendData);
        FreeJson(sendData);
        return result;
    }
    result = OnSessionFinish(session, jsonParams, out);
    FreeJson(out);
    return result;
}

static int32_t ProcessBindSession(Session *session, CJson *jsonParams)
{
    if ((session == NULL) || (jsonParams == NULL)) {
        LOGE("The input session or jsonParams is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    BindSession *realSession = (BindSession *)session;
    LOGI("Start to process bind session! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        realSession->requestId, realSession->operationCode);

    bool isNeedInform = true;
    int32_t status = CONTINUE;
    int32_t result = ProcessBindSessionInner(realSession, jsonParams, &status, &isNeedInform);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs during processing bind session. We need to notify the service!");
        InformPeerProcessErrorIfNeed(isNeedInform, result, realSession);
        if ((!NeedForceDelete(realSession)) || (ForceUnbindDevice(realSession) != HC_SUCCESS)) {
            ProcessErrorCallback(realSession->requestId, realSession->operationCode, result, NULL,
                realSession->base.callback);
        }
        CloseChannel(realSession->channelType, realSession->channelId);
        return result;
    }
    LOGI("Process bind session successfully! [RequestId]: %" PRId64 ", [OperationCode]: %d",
         realSession->requestId, realSession->operationCode);
    if (status == FINISH) {
        return status;
    }
    return result;
}

bool NeedCreateGroup(int isClient, int operationCode)
{
    if (((isClient == CLIENT) && (operationCode == MEMBER_JOIN)) ||
        ((isClient == SERVER) && (operationCode == MEMBER_INVITE))) {
        return true;
    } else {
        return false;
    }
}

bool NeedForceDelete(const BindSession *session)
{
    bool needForceDelete = false;
    (void)GetBoolFromJson(session->params, FIELD_IS_FORCE_DELETE, &needForceDelete);
    return needForceDelete;
}

int32_t ForceUnbindDevice(const BindSession *session)
{
    const char *peerAuthId = GetStringFromJson(session->params, FIELD_PEER_AUTH_ID);
    if (peerAuthId == NULL) {
        LOGE("Failed to get peerAuthId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    const char *groupId = GetStringFromJson(session->params, FIELD_GROUP_ID);
    if (groupId == NULL) {
        LOGE("Failed to get groupId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    int32_t result = DelTrustedDeviceByAuthId(peerAuthId, groupId);
    if (result != HC_SUCCESS) {
        LOGE("Failed to delete trust device from database!");
        return result;
    }
    /*
     * If the trusted device has been deleted from the database but the peer key fails to be deleted,
     * the forcible unbinding is still considered successful. Only logs need to be printed.
     */
    result = ForceDeletePeerKey(session->params);
    if (result != HC_SUCCESS) {
        LOGE("Failed to delete peer key!");
    }
    LOGI("An error occurs during the online unbinding. Therefore, we forcibly unbind the peer device!");
    char *returnDataStr = NULL;
    result = GenerateUnbindSuccessData(peerAuthId, groupId, &returnDataStr);
    if (result != HC_SUCCESS) {
        return result;
    }
    ProcessFinishCallback(session->requestId, MEMBER_DELETE, returnDataStr, session->base.callback);
    FreeJsonString(returnDataStr);
    return HC_SUCCESS;
}

int32_t GenerateBindParams(int isClient, const CJson *jsonParams, BindSession *session)
{
    if (session->params == NULL) {
        session->params = CreateJson();
        if (session->params == NULL) {
            LOGE("Failed to allocate session params memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
    }

    int32_t result;
    if (((result = AddAppId(jsonParams, session)) != HC_SUCCESS) ||
        ((result = AddIsForceDeleteIfNeed(isClient, jsonParams, session)) != HC_SUCCESS) ||
        ((result = AddChannelIdIfNeed(isClient, jsonParams, session)) != HC_SUCCESS) ||
        ((result = AddPinCodeIfNeed(jsonParams, session)) != HC_SUCCESS) ||
        ((result = AddGroupAndDevInfo(isClient, jsonParams, session)) != HC_SUCCESS) ||
        ((result = AddPeerDevInfoIfNeed(isClient, jsonParams, session)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

int32_t SendBindSessionData(const BindSession *session, const CJson *sendData)
{
    char *sendDataStr = PackJsonToString(sendData);
    if (sendDataStr == NULL) {
        LOGE("An error occurred when converting json to string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = HcSendMsg(session->channelType, session->requestId,
        session->channelId, session->base.callback, sendDataStr);
    FreeJsonString(sendDataStr);
    return result;
}

void InformPeerProcessErrorIfNeed(bool isNeedInform, int32_t errorCode, const BindSession *session)
{
    if (!isNeedInform) {
        return;
    }
    CJson *errorData = GenerateGroupErrorMsg(errorCode, session->requestId);
    if (errorData == NULL) {
        return;
    }
    int32_t result = SendBindSessionData(session, errorData);
    FreeJson(errorData);
    if (result != HC_SUCCESS) {
        LOGE("An error occurred when notifying the peer service!");
        return;
    }
    LOGI("Succeeded in notifying the peer device that an error occurred at the local end!");
}

void InformPeerModuleErrorIfNeed(CJson *out, const BindSession *session)
{
    CJson *errorData = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    if (errorData == NULL) {
        return;
    }
    if (AddInt64StringToJson(errorData, FIELD_REQUEST_ID, session->requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to errorData!");
        return;
    }
    int32_t result = SendBindSessionData(session, errorData);
    if (result != HC_SUCCESS) {
        LOGE("An error occurred when notifying the peer service!");
        return;
    }
    LOGI("Succeeded in notifying the peer device that an error occurred at the local end!");
}

void DestroyBindSession(Session *session)
{
    if (session == NULL) {
        return;
    }
    BindSession *realSession = (BindSession *)session;
    DestroyTask(realSession->curTaskId, DAS_MODULE);
    FreeJson(realSession->params);
    realSession->params = NULL;
    HcFree(realSession);
    realSession = NULL;
}

int32_t ProcessModule(const BindSession *session, const CJson *in, CJson *out, int *status)
{
    LOGI("Start to process DAS module task!");
    int32_t result = ProcessTask(session->curTaskId, in, out, status, DAS_MODULE);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task! [ErrorCode]: %d", result);
        return result;
    }
    LOGI("Process DAS module task successfully!");
    return HC_SUCCESS;
}

int32_t CreateAndProcessModule(BindSession *session, const CJson *in, CJson *out)
{
    int status = 0;
    LOGI("Start to create and process DAS module task!");
    int32_t result = CreateTask(&(session->curTaskId), in, out, DAS_MODULE);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when creating a module task! [ErrorCode]: %d", result);
        return result;
    }
    result = ProcessTask(session->curTaskId, in, out, &status, DAS_MODULE);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task! [ErrorCode]: %d", result);
        return result;
    }
    LOGI("Create and process DAS module task successfully!");
    return HC_SUCCESS;
}

int32_t AddInfoToSendData(bool isNeedCompatibleInfo, const BindSession *session, CJson *data)
{
    int32_t result;
    if (((result = AddGroupInfoToSendData(session, data)) != HC_SUCCESS) ||
        ((result = AddDevInfoToSendData(session, data)) != HC_SUCCESS) ||
        ((result = AddRequestInfoToSendData(session, data)) != HC_SUCCESS) ||
        ((result = AddCompatibleInfoToSendData(isNeedCompatibleInfo, data)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

int32_t GenerateBasicModuleParams(bool isClient, BindSession *session, CJson *moduleParams)
{
    int32_t result;
    if (((result = AddGroupAndDevInfoToParams(session, moduleParams)) != HC_SUCCESS) ||
        ((result = AddRequestInfoToParams(isClient, session, moduleParams)) != HC_SUCCESS) ||
        ((result = AddPinCodeToParamsIfNeed(session, moduleParams)) != HC_SUCCESS) ||
        ((result = AddPeerAuthIdAndUserTypeToParamsIfNeed(session, moduleParams)) != HC_SUCCESS)) {
        return result;
    }
    return HC_SUCCESS;
}

void InitBindSession(int bindType, int operationCode, int64_t requestId, const DeviceAuthCallback *callback,
    BindSession *session)
{
    session->base.process = ProcessBindSession;
    session->base.destroy = DestroyBindSession;
    session->curTaskId = 0;
    session->base.callback = callback;
    int res = GenerateSessionOrTaskId(&session->base.sessionId);
    if (res != 0) {
        return;
    }
    session->base.type = bindType;
    session->channelType = NO_CHANNEL;
    session->operationCode = operationCode;
    session->requestId = requestId;
    session->channelId = DEFAULT_CHANNEL_ID;
    session->isWaiting = HC_FALSE;
    session->params = NULL;
    session->onChannelOpened = NULL;
    session->onConfirmationReceived = NULL;
}