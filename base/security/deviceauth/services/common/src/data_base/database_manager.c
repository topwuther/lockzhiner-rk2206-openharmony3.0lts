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

#include "alg_defs.h"
#include "database.h"
#include "database_manager.h"
#include "device_auth.h"
#include "hc_dev_info.h"
#include "hc_file.h"
#include "hc_log.h"
#include "hc_mutex.h"
#include "securec.h"

#define MAX_STRING_LEN 256

DEFINE_TLV_FIX_LENGTH_TYPE(TlvDevAuthFixedLenInfo, NO_REVERT)

BEGIN_TLV_STRUCT_DEFINE(TlvGroupElement, 0x0001)
    TLV_MEMBER(TlvString, name, 0x4001)
    TLV_MEMBER(TlvString, id, 0x4002)
    TLV_MEMBER(TlvUint32, type, 0x4003)
    TLV_MEMBER(TlvInt32, visibility, 0x4004)
    TLV_MEMBER(TlvInt32, expireTime, 0x4005)
    TLV_MEMBER(TlvInt64, userId, 0x4006)
    TLV_MEMBER(TlvBuffer, sharedUserIdVec, 0x4007)
    TLV_MEMBER(TlvBuffer, managers, 0x4008)
    TLV_MEMBER(TlvBuffer, friends, 0x4009)
END_TLV_STRUCT_DEFINE()
IMPLEMENT_TLV_VECTOR(TlvGroupVec, TlvGroupElement, 1)

BEGIN_TLV_STRUCT_DEFINE(TlvDevAuthElement, 0x0002)
    TLV_MEMBER(TlvString, groupId, 0x4101)
    TLV_MEMBER(TlvString, udid, 0x4102)
    TLV_MEMBER(TlvString, authId, 0x4103)
    TLV_MEMBER(TlvString, serviceType, 0x4104)
    TLV_MEMBER(TlvBuffer, ext, 0x4105)
    TLV_MEMBER(TlvDevAuthFixedLenInfo, info, 0x4106)
END_TLV_STRUCT_DEFINE()
IMPLEMENT_TLV_VECTOR(TlvDevAuthVec, TlvDevAuthElement, 2)

BEGIN_TLV_STRUCT_DEFINE(HCDataBaseV1, 0x0001)
    TLV_MEMBER(TlvInt32, version, 0x6001)
    TLV_MEMBER(TlvGroupVec, groups, 0x6002)
    TLV_MEMBER(TlvDevAuthVec, deviceAuthInfos, 0x6003)
END_TLV_STRUCT_DEFINE()

IMPLEMENT_HC_VECTOR(TrustedGroupTable, TrustedGroupEntry *, 1)
IMPLEMENT_HC_VECTOR(TrustedDeviceTable, TrustedDeviceEntry, 2)
IMPLEMENT_HC_VECTOR(StringVector, HcString, 1)
IMPLEMENT_HC_VECTOR(Int64Vector, int64_t, 1)
IMPLEMENT_HC_VECTOR(GroupInfoVec, void *, 1)
IMPLEMENT_HC_VECTOR(DeviceInfoVec, void *, 2)

static TrustedGroupTable g_trustedGroupTable;
static TrustedDeviceTable g_trustedDeviceTable;

/* cache across account groupId func */
static int32_t (*g_generateIdFunc)(int64_t userId, int64_t sharedUserId, char **returnGroupId) = NULL;

/* cache broadcaster instance */
static const Broadcaster *g_broadcaster = NULL;

static HcMutex *g_databaseMutex = NULL;

static void DestroyStrVector(StringVector *vec)
{
    uint32_t index;
    HcString *strItemPtr = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, strItemPtr) {
        DeleteString(strItemPtr);
    }
    DESTROY_HC_VECTOR(StringVector, vec)
}

static TrustedGroupEntry *CreateGroupEntryStruct(void)
{
    TrustedGroupEntry *ptr = (TrustedGroupEntry *)HcMalloc(sizeof(TrustedGroupEntry), 0);
    if (ptr == NULL) {
        return NULL;
    }
    ptr->name = CreateString();
    ptr->id = CreateString();
    ptr->sharedUserIdVec = CREATE_HC_VECTOR(Int64Vector)
    ptr->managers = CREATE_HC_VECTOR(StringVector)
    ptr->friends = CREATE_HC_VECTOR(StringVector)
    return ptr;
}

static void DestroyGroupEntryStruct(TrustedGroupEntry *groupEntry)
{
    DeleteString(&groupEntry->name);
    DeleteString(&groupEntry->id);
    DestroyStrVector(&groupEntry->managers);
    DestroyStrVector(&groupEntry->friends);
    DESTROY_HC_VECTOR(Int64Vector, &groupEntry->sharedUserIdVec)
}

static void DestroyGroupTable(void)
{
    uint32_t groupIndex;
    TrustedGroupEntry **entry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        DestroyGroupEntryStruct(*entry);
        HcFree(*entry);
    }
    DESTROY_HC_VECTOR(TrustedGroupTable, &g_trustedGroupTable)
}

static void DestroyDeviceEntryStruct(TrustedDeviceEntry *deviceEntry)
{
    DeleteString(&deviceEntry->udid);
    DeleteString(&deviceEntry->authId);
    DeleteString(&deviceEntry->serviceType);
    DeleteParcel(&deviceEntry->ext);
}

static void DestroyTrustDevTable(void)
{
    uint32_t devIndex;
    TrustedDeviceEntry *deviceEntry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, devIndex, deviceEntry) {
        DestroyDeviceEntryStruct(deviceEntry);
    }
    DESTROY_HC_VECTOR(TrustedDeviceTable, &g_trustedDeviceTable)
}

static int32_t GetSharedUserIdFromVecByGroupId(const TrustedGroupEntry *groupEntry, const char *groupId,
    int64_t *userId)
{
    if ((groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) || (g_generateIdFunc == NULL)) {
        return HC_ERR_NOT_SUPPORT;
    }
    /* The groupId of the across account group needs to be generated temporarily due to device resource problems. */
    uint32_t index;
    int64_t *sharedUserId = NULL;
    FOR_EACH_HC_VECTOR(groupEntry->sharedUserIdVec, index, sharedUserId) {
        char *tempGroupId = NULL;
        int32_t result = g_generateIdFunc(groupEntry->userId, *sharedUserId, &tempGroupId);
        if (result != HC_SUCCESS) {
            LOGE("[DB]: Failed to generate temp groupId!");
            return result;
        }
        bool isEquals = (strcmp(tempGroupId, groupId) == 0) ? true : false;
        HcFree(tempGroupId);
        if (isEquals) {
            *userId = *sharedUserId;
            return HC_SUCCESS;
        }
    }
    return HC_ERR_GROUP_NOT_EXIST;
}

static bool IsGroupNameEquals(const TrustedGroupEntry *groupEntry, const char *groupName)
{
    if ((strcmp(StringGet(&groupEntry->name), groupName) == 0)) {
        return true;
    }
    int64_t sharedUserId = DEFAULT_USER_ID;
    if (GetSharedUserIdFromVecByGroupId(groupEntry, groupName, &sharedUserId) == HC_SUCCESS) {
        return true;
    }
    return false;
}

static bool IsGroupIdEquals(const TrustedGroupEntry *groupEntry, const char *groupId)
{
    if ((strcmp(StringGet(&groupEntry->id), groupId) == 0)) {
        return true;
    }
    int64_t sharedUserId = DEFAULT_USER_ID;
    if (GetSharedUserIdFromVecByGroupId(groupEntry, groupId, &sharedUserId) == HC_SUCCESS) {
        return true;
    }
    return false;
}

static bool CompareGroupIdInDeviceEntryOrNull(const TrustedDeviceEntry *deviceEntry, const char *groupId)
{
    if (groupId == NULL) {
        return true;
    } else {
        const char *tmpGroupId = (deviceEntry->groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) ?
            (StringGet(&deviceEntry->groupEntry->id)) : (StringGet(&deviceEntry->serviceType));
        return (strcmp(tmpGroupId, groupId) == 0);
    }
}

static bool CompareUdidInDeviceEntryOrNull(const TrustedDeviceEntry *deviceEntry, const char *udid)
{
    if (udid == NULL) {
        return true;
    } else {
        return (strcmp(StringGet(&deviceEntry->udid), udid) == 0);
    }
}

static bool CompareAuthIdInDeviceEntryOrNull(const TrustedDeviceEntry *deviceEntry, const char *authId)
{
    if (authId == NULL) {
        return true;
    } else {
        return (strcmp(StringGet(&deviceEntry->authId), authId) == 0);
    }
}

static TrustedDeviceEntry *GetTrustedDeviceEntry(const char *udid, const char *groupId)
{
    uint32_t index;
    TrustedDeviceEntry *deviceEntry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, deviceEntry) {
        if (CompareUdidInDeviceEntryOrNull(deviceEntry, udid)) {
            if (CompareGroupIdInDeviceEntryOrNull(deviceEntry, groupId)) {
                return deviceEntry;
            }
        }
    }
    return NULL;
}

static TrustedDeviceEntry *GetTrustedDeviceEntryByAuthId(const char *authId, const char *groupId)
{
    uint32_t index;
    TrustedDeviceEntry *deviceEntry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, deviceEntry) {
        if (CompareAuthIdInDeviceEntryOrNull(deviceEntry, authId)) {
            if (CompareGroupIdInDeviceEntryOrNull(deviceEntry, groupId)) {
                return deviceEntry;
            }
        }
    }
    return NULL;
}

/*
 * Currently, this interface does not return the actual number of trusted devices.
 * If at least one trusted device exists, return 1. Otherwise, return 0.
 */
static int GetTrustedDeviceNum(void)
{
    return (g_trustedDeviceTable.size(&g_trustedDeviceTable) > 0) ? 1 : 0;
}

static int32_t GenerateCommonGroupInfoByEntry(const TrustedGroupEntry *groupEntry, GroupInfo *returnGroupInfo)
{
    if (HC_VECTOR_SIZE(&(groupEntry->managers)) == 0) {
        LOGE("[DB]: The group owner is lost!");
        return HC_ERR_LOST_DATA;
    }
    HcString entryOwner = HC_VECTOR_GET(&groupEntry->managers, 0);
    if (!StringSet(&(returnGroupInfo->ownerName), entryOwner)) {
        LOGE("[DB]: Failed to copy groupOwner!");
        return HC_ERR_MEMORY_COPY;
    }
    returnGroupInfo->type = groupEntry->type;
    returnGroupInfo->visibility = groupEntry->visibility;
    returnGroupInfo->expireTime = groupEntry->expireTime;
    returnGroupInfo->userId = groupEntry->userId;
    return HC_SUCCESS;
}

static int32_t GenerateGroupInfoIdAndName(const char *groupId, const char *groupName, GroupInfo *returnGroupInfo)
{
    if (!StringSetPointer(&(returnGroupInfo->id), groupId)) {
        LOGE("[DB]: Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSetPointer(&(returnGroupInfo->name), groupName)) {
        LOGE("[DB]: Failed to copy groupName!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static void GenerateGroupInfoSharedUserId(int64_t sharedUserId, GroupInfo *returnGroupInfo)
{
    returnGroupInfo->sharedUserId = sharedUserId;
}

static int32_t GenerateGroupInfoByEntry(const TrustedGroupEntry *groupEntry, int64_t userId, GroupInfo *returnGroupInfo)
{
    int32_t result;
    if ((groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) || (g_generateIdFunc == NULL)) {
        result = GenerateCommonGroupInfoByEntry(groupEntry, returnGroupInfo);
        if (result != HC_SUCCESS) {
            return result;
        }
        return GenerateGroupInfoIdAndName(StringGet(&groupEntry->id), StringGet(&groupEntry->name), returnGroupInfo);
    }
    char *tempGroupId = NULL;
    result = g_generateIdFunc(groupEntry->userId, userId, &tempGroupId);
    if (result != HC_SUCCESS) {
        LOGE("[DB]: Failed to generate temp groupId!");
        return result;
    }
    result = GenerateGroupInfoIdAndName(tempGroupId, tempGroupId, returnGroupInfo);
    HcFree(tempGroupId);
    if (result != HC_SUCCESS) {
        return result;
    }
    GenerateGroupInfoSharedUserId(userId, returnGroupInfo);
    return GenerateCommonGroupInfoByEntry(groupEntry, returnGroupInfo);
}

static int32_t GenerateCommonDeviceInfoByEntry(const TrustedDeviceEntry *entry, DeviceInfo *returnDeviceInfo)
{
    if (!StringSet(&returnDeviceInfo->udid, entry->udid)) {
        LOGE("[DB]: Failed to copy udid!");
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&returnDeviceInfo->authId, entry->authId)) {
        LOGE("[DB]: Failed to copy authId!");
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&returnDeviceInfo->serviceType, entry->serviceType)) {
        LOGE("[DB]: Failed to copy authId!");
        return HC_ERR_MEMORY_COPY;
    }
    returnDeviceInfo->credential = entry->credential;
    returnDeviceInfo->devType = entry->devType;
    returnDeviceInfo->userId = entry->userId;
    return HC_SUCCESS;
}

static int32_t GenerateDeviceInfoId(const char *groupId, DeviceInfo *returnDeviceInfo)
{
    if (!StringSetPointer(&(returnDeviceInfo->groupId), groupId)) {
        LOGE("[DB]: Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int32_t GenerateDeviceInfoByEntry(const TrustedDeviceEntry *deviceEntry, DeviceInfo *returnDeviceInfo)
{
    int32_t result = GenerateCommonDeviceInfoByEntry(deviceEntry, returnDeviceInfo);
    if (result != HC_SUCCESS) {
        return result;
    }
    if (deviceEntry->groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        return GenerateDeviceInfoId(StringGet(&deviceEntry->groupEntry->id), returnDeviceInfo);
    }
    return GenerateDeviceInfoId(StringGet(&deviceEntry->serviceType), returnDeviceInfo);
}

static void NotifyGroupCreated(const TrustedGroupEntry *groupEntry, int64_t userId)
{
    if ((g_broadcaster == NULL) || (g_broadcaster->postOnGroupCreated == NULL)) {
        return;
    }
    GroupInfo *groupInfo = CreateGroupInfoStruct();
    if (groupInfo == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return;
    }
    if (GenerateGroupInfoByEntry(groupEntry, userId, groupInfo) != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return;
    }
    g_broadcaster->postOnGroupCreated(groupInfo);
    DestroyGroupInfoStruct(groupInfo);
}

static void NotifyGroupDeleted(const TrustedGroupEntry *groupEntry, int64_t userId)
{
    if ((g_broadcaster == NULL) || (g_broadcaster->postOnGroupDeleted == NULL)) {
        return;
    }
    GroupInfo *groupInfo = CreateGroupInfoStruct();
    if (groupInfo == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return;
    }
    if (GenerateGroupInfoByEntry(groupEntry, userId, groupInfo) != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return;
    }
    g_broadcaster->postOnGroupDeleted(groupInfo);
    DestroyGroupInfoStruct(groupInfo);
}

static void NotifyDeviceBound(const TrustedGroupEntry *groupEntry, const char *udid, int64_t sharedUserId)
{
    if ((g_broadcaster == NULL) || (g_broadcaster->postOnDeviceBound == NULL)) {
        return;
    }
    GroupInfo *groupInfo = CreateGroupInfoStruct();
    if (groupInfo == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return;
    }
    if (GenerateGroupInfoByEntry(groupEntry, sharedUserId, groupInfo) != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return;
    }
    g_broadcaster->postOnDeviceBound(udid, groupInfo);
    DestroyGroupInfoStruct(groupInfo);
}

static void NotifyDeviceUnBound(const TrustedGroupEntry *groupEntry, const char *udid, int64_t sharedUserId)
{
    if ((g_broadcaster == NULL) || (g_broadcaster->postOnDeviceUnBound == NULL)) {
        return;
    }
    GroupInfo *groupInfo = CreateGroupInfoStruct();
    if (groupInfo == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return;
    }
    if (GenerateGroupInfoByEntry(groupEntry, sharedUserId, groupInfo) != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return;
    }
    g_broadcaster->postOnDeviceUnBound(udid, groupInfo);
    DestroyGroupInfoStruct(groupInfo);
}

static void NotifyDeviceNotTrusted(const char *peerUdid)
{
    if (g_broadcaster != NULL && g_broadcaster->postOnDeviceNotTrusted != NULL) {
        g_broadcaster->postOnDeviceNotTrusted(peerUdid);
    }
}

static void NotifyLastGroupDeleted(const char *peerUdid, int groupType)
{
    if (g_broadcaster != NULL && g_broadcaster->postOnLastGroupDeleted != NULL) {
        g_broadcaster->postOnLastGroupDeleted(peerUdid, groupType);
    }
}

static void NotifyTrustedDeviceNumChanged(void)
{
    int trustedDeviceNum = GetTrustedDeviceNum();
    if (g_broadcaster != NULL && g_broadcaster->postOnTrustedDeviceNumChanged != NULL) {
        g_broadcaster->postOnTrustedDeviceNumChanged(trustedDeviceNum);
    }
}

static void DeleteExpiredSharedUserId(const Int64Vector *sharedUserIdList, TrustedGroupEntry *entry)
{
    int64_t sharedUserId;
    uint32_t sharedUserIdIndex = 0;
    while (sharedUserIdIndex < entry->sharedUserIdVec.size(&entry->sharedUserIdVec)) {
        sharedUserId = sharedUserIdList->get(sharedUserIdList, sharedUserIdIndex);
        bool isNeedRemove = true;
        uint32_t tmpIndex;
        int64_t *tmpSharedUserId = NULL;
        FOR_EACH_HC_VECTOR(*sharedUserIdList, tmpIndex, tmpSharedUserId) {
            if (sharedUserId == *tmpSharedUserId) {
                isNeedRemove = false;
                break;
            }
        }
        if (!isNeedRemove) {
            sharedUserIdIndex++;
            continue;
        }
        HC_VECTOR_POPELEMENT(&entry->sharedUserIdVec, tmpSharedUserId, sharedUserIdIndex);
        NotifyGroupDeleted(entry, sharedUserId);
    }
}

static void AddNewSharedUserId(Int64Vector *sharedUserIdList, TrustedGroupEntry *entry)
{
    int64_t sharedUserId;
    uint32_t sharedUserIdIndex = 0;
    while (sharedUserIdIndex < sharedUserIdList->size(sharedUserIdList)) {
        sharedUserId = sharedUserIdList->get(sharedUserIdList, sharedUserIdIndex);
        bool isNeedAdd = true;
        uint32_t tmpIndex;
        int64_t *tmpSharedUserId = NULL;
        FOR_EACH_HC_VECTOR(entry->sharedUserIdVec, tmpIndex, tmpSharedUserId) {
            if (sharedUserId == *tmpSharedUserId) {
                isNeedAdd = false;
                break;
            }
        }
        /* Release the memory in advance to reduce the memory usage. */
        HC_VECTOR_POPELEMENT(sharedUserIdList, &sharedUserId, sharedUserIdIndex);
        if (!isNeedAdd) {
            continue;
        }
        HC_VECTOR_PUSHBACK(&entry->sharedUserIdVec, &sharedUserId);
        NotifyGroupCreated(entry, sharedUserId);
    }
}

static int GenerateGroupEntryByInfo(const GroupInfo *groupInfo, TrustedGroupEntry *entry)
{
    if (!StringSet(&entry->name, groupInfo->name)) {
        LOGE("[DB]: Failed to copy groupName!");
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&entry->id, groupInfo->id)) {
        LOGE("[DB]: Failed to copy groupId!");
        return HC_ERR_MEMORY_COPY;
    }
    entry->type = groupInfo->type;
    entry->visibility = groupInfo->visibility;
    entry->expireTime = groupInfo->expireTime;
    entry->userId = groupInfo->userId;
    HcString ownerName = CreateString();
    if (!StringSet(&ownerName, groupInfo->ownerName)) {
        LOGE("[DB]: Failed to copy groupOwner!");
        DeleteString(&ownerName);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (entry->managers.pushBack(&entry->managers, &ownerName) == NULL) {
        LOGE("[DB]: Failed to push groupOwner to managers!");
        DeleteString(&ownerName);
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static TrustedGroupEntry *GetGroupEntryByGroupIdInner(const char *groupId)
{
    uint32_t groupIndex;
    TrustedGroupEntry **entry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            return *entry;
        }
    }
    return NULL;
}

static bool InitAuthInfo(const DeviceInfo *deviceInfo, const Uint8Buff *ext, TrustedDeviceEntry *deviceEntry)
{
    deviceEntry->udid = CreateString();
    deviceEntry->authId = CreateString();
    deviceEntry->serviceType = CreateString();
    /* reserved field */
    deviceEntry->ext = CreateParcel(0, 0);
    deviceEntry->groupEntry = GetGroupEntryByGroupIdInner(StringGet(&deviceInfo->groupId));
    if (deviceEntry->groupEntry == NULL) {
        LOGE("[DB]: The group corresponding to groupId cannot be found!");
        return false;
    }
    if (!StringSet(&deviceEntry->udid, deviceInfo->udid)) {
        return false;
    }
    if (!StringSet(&deviceEntry->authId, deviceInfo->authId)) {
        return false;
    }
    if (!StringSet(&deviceEntry->serviceType, deviceInfo->serviceType)) {
        return false;
    }
    deviceEntry->credential = deviceInfo->credential;
    deviceEntry->devType = deviceInfo->devType;
    deviceEntry->userId = deviceInfo->userId;
    deviceEntry->lastTm = 0;
    if (ext != NULL && ext->val != NULL) {
        if (!ParcelWrite(&deviceEntry->ext, ext->val, ext->length)) {
            LOGE("[DB]: Failed to copy extern data!");
            return false;
        }
    }
    return true;
}

static bool IsDeleteLastSuchTypeGroup(const char *udid, int groupType)
{
    uint32_t devIndex;
    TrustedDeviceEntry *deviceEntry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, devIndex, deviceEntry) {
        if ((deviceEntry != NULL) && (deviceEntry->groupEntry != NULL)) {
            const char *devUdid = StringGet(&deviceEntry->udid);
            if ((strcmp(devUdid, udid) == 0) && (deviceEntry->groupEntry->type == groupType)) {
                return false;
            }
        }
    }
    return true;
}

static void CheckAndNotifyAfterDelDevice(const TrustedDeviceEntry *deviceEntry)
{
    const char *udid = StringGet(&deviceEntry->udid);
    int64_t sharedUserId = DEFAULT_USER_ID;
    if (deviceEntry->groupEntry->type == ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        (void)GetSharedUserIdFromVecByGroupId(deviceEntry->groupEntry, StringGet(&deviceEntry->serviceType),
            &sharedUserId);
    }
    NotifyDeviceUnBound(deviceEntry->groupEntry, udid, sharedUserId);
    if (IsDeleteLastSuchTypeGroup(udid, deviceEntry->groupEntry->type)) {
        NotifyLastGroupDeleted(udid, deviceEntry->groupEntry->type);
    }
    if (GetTrustedDeviceEntry(udid, NULL) == NULL) {
        NotifyDeviceNotTrusted(udid);
        NotifyTrustedDeviceNumChanged();
    }
}

static bool SatisfyType(int32_t type, int32_t standardType)
{
    return ((standardType == ALL_GROUP) || (type == standardType));
}

static bool SatisfyVisibility(int32_t visibility, int32_t standardVisibility)
{
    return ((standardVisibility == ALL_GROUP_VISIBILITY) || (visibility == standardVisibility));
}

static bool SatisfyUdid(const char *udid, const char *standardUdid)
{
    return (strcmp(udid, standardUdid) == 0);
}

static bool SatisfyAuthId(const char *authId, const char *standardAuthId)
{
    return (strcmp(authId, standardAuthId) == 0);
}

static bool IsSatisfyGroup(const TrustedGroupEntry *entry, const GroupQueryParams *params)
{
    return (SatisfyType(entry->type, params->type) && SatisfyVisibility(entry->visibility, params->visibility));
}

static bool IsGroupManager(const char *appId, const TrustedGroupEntry *entry)
{
    uint32_t index;
    HcString *manager = NULL;
    FOR_EACH_HC_VECTOR(entry->managers, index, manager) {
        if (strcmp(StringGet(manager), appId) == 0) {
            return true;
        }
    }
    return false;
}

static bool IsGroupFriend(const char *appId, const TrustedGroupEntry *entry)
{
    uint32_t index;
    HcString *trustedFriend = NULL;
    FOR_EACH_HC_VECTOR(entry->friends, index, trustedFriend) {
        if (strcmp(StringGet(trustedFriend), appId) == 0) {
            return true;
        }
    }
    return false;
}

static bool CompareSearchParams(int groupType, const char *groupId, const char *groupName, const char *groupOwner,
    const TrustedGroupEntry *entry)
{
    if ((groupType != ALL_GROUP) && (entry->type != groupType)) {
        return false;
    }
    if ((groupId != NULL) && (!IsGroupIdEquals(entry, groupId))) {
        return false;
    }
    if ((groupName != NULL) && (!IsGroupNameEquals(entry, groupName))) {
        return false;
    }
    if (HC_VECTOR_SIZE(&(entry->managers)) == 0) {
        LOGE("[DB]: The group owner is lost!");
        return false;
    }
    HcString entryOwner = HC_VECTOR_GET(&(entry->managers), 0);
    if ((groupOwner != NULL) && (strcmp(StringGet(&entryOwner), groupOwner) != 0)) {
        return false;
    }
    return true;
}

static int32_t PushGroupInfoToVec(const TrustedGroupEntry *entry, GroupInfoVec *groupInfoVec)
{
    int32_t result;
    if (entry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        GroupInfo *groupInfo = CreateGroupInfoStruct();
        if (groupInfo == NULL) {
            LOGE("[DB]: Failed to allocate groupInfo memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
        result = GenerateGroupInfoByEntry(entry, DEFAULT_USER_ID, groupInfo);
        if (result != HC_SUCCESS) {
            DestroyGroupInfoStruct(groupInfo);
            return result;
        }
        if (groupInfoVec->pushBackT(groupInfoVec, groupInfo) == NULL) {
            LOGE("[DB]: Failed to push groupInfo to groupInfoVec!");
            DestroyGroupInfoStruct(groupInfo);
            return HC_ERR_MEMORY_COPY;
        }
        return HC_SUCCESS;
    }
    uint32_t index;
    int64_t *sharedUserId = NULL;
    FOR_EACH_HC_VECTOR(entry->sharedUserIdVec, index, sharedUserId) {
        GroupInfo *groupInfo = CreateGroupInfoStruct();
        if (groupInfo == NULL) {
            LOGE("[DB]: Failed to allocate groupInfo memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
        result = GenerateGroupInfoByEntry(entry, *sharedUserId, groupInfo);
        if (result != HC_SUCCESS) {
            DestroyGroupInfoStruct(groupInfo);
            return result;
        }
        if (groupInfoVec->pushBackT(groupInfoVec, groupInfo) == NULL) {
            LOGE("[DB]: Failed to push groupInfo to groupInfoVec!");
            DestroyGroupInfoStruct(groupInfo);
            return HC_ERR_MEMORY_COPY;
        }
    }
    return HC_SUCCESS;
}

static int32_t PushDevInfoToVec(const TrustedDeviceEntry *entry, DeviceInfoVec *deviceInfoVec)
{
    DeviceInfo *deviceInfo = CreateDeviceInfoStruct();
    if (deviceInfo == NULL) {
        LOGE("[DB]: Failed to allocate deviceInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GenerateDeviceInfoByEntry(entry, deviceInfo);
    if (result != HC_SUCCESS) {
        DestroyDeviceInfoStruct(deviceInfo);
        return result;
    }
    if (deviceInfoVec->pushBackT(deviceInfoVec, deviceInfo) == NULL) {
        LOGE("[DB]: Failed to push deviceInfo to deviceInfoVec!");
        DestroyDeviceInfoStruct(deviceInfo);
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static bool IsSatisfyUdidAndAuthId(const TrustedDeviceEntry *deviceEntry, const GroupQueryParams *params)
{
    if ((params->udid != NULL) && (!SatisfyUdid(StringGet(&deviceEntry->udid), params->udid))) {
        return false;
    }
    if ((params->authId != NULL) && (!SatisfyAuthId(StringGet(&deviceEntry->authId), params->authId))) {
        return false;
    }
    return true;
}

static int32_t GetGroupEntryInner(const char *groupId, const char *udid, GroupInfo *returnGroupInfo)
{
    int32_t result;
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if (((*entry)->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) && (strcmp(StringGet(&(*entry)->id), groupId) == 0)) {
            if (udid != NULL) {
                if (GetTrustedDeviceEntry(udid, groupId) == NULL) {
                    LOGE("[DB]: The trusted device does not exist in the group!");
                    return HC_ERR_DEVICE_NOT_EXIST;
                }
            }
            return GenerateGroupInfoByEntry(*entry, DEFAULT_USER_ID, returnGroupInfo);
        }
        int64_t sharedUserId = DEFAULT_USER_ID;
        result = GetSharedUserIdFromVecByGroupId(*entry, groupId, &sharedUserId);
        if (result == HC_SUCCESS) {
            if (udid != NULL) {
                if (GetTrustedDeviceEntry(udid, groupId) == NULL) {
                    LOGE("[DB]: The trusted device does not exist in the group!");
                    return HC_ERR_DEVICE_NOT_EXIST;
                }
            }
            return GenerateGroupInfoByEntry(*entry, sharedUserId, returnGroupInfo);
        }
    }
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

static bool EndWithZero(HcParcel *parcel)
{
    const char *p = GetParcelLastChar(parcel);
    if (p == NULL) {
        return false;
    }
    if (*p == '\0') {
        return true;
    }
    return false;
}

static bool LoadInt64VectorFromParcel(Int64Vector* vec, HcParcel* parcel)
{
    do {
        int64_t value;
        if (!ParcelReadInt64(parcel, &value)) {
            return true;
        }

        if (vec->pushBack(vec, &value) == NULL) {
            return false;
        }
    } while (1);
}

static bool SaveInt64VectorToParcel(const Int64Vector* vec, HcParcel* parcel)
{
    uint32_t index;
    int64_t *value = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, value) {
        if (!ParcelWriteInt64(parcel, *value)) {
            return false;
        }
    }
    return true;
}

static bool LoadStringVectorFromParcel(StringVector *vec, HcParcel *parcel)
{
    uint32_t strLen;
    do {
        if (!ParcelReadUint32(parcel, &strLen)) {
            return true;
        }
        if (strLen > MAX_STRING_LEN) {
            return false;
        }
        HcString str = CreateString();
        ClearParcel(&str.parcel);
        if (!ParcelReadParcel(parcel, &str.parcel, strLen, false) ||
            !EndWithZero(&str.parcel)) {
            DeleteString(&str);
            return false;
        } else {
            if (vec->pushBack(vec, &str) == NULL) {
                DeleteString(&str);
                return false;
            }
        }
    } while (1);
}

static bool SaveStringVectorToParcel(const StringVector *vec, HcParcel *parcel)
{
    uint32_t index;
    HcString *str = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, str) {
        uint32_t len = StringLength(str) + sizeof(char);
        if (!ParcelWriteUint32(parcel, len)) {
            return false;
        }
        if (!ParcelWrite(parcel, GetParcelData(&str->parcel), GetParcelDataSize(&str->parcel))) {
            return false;
        }
    }
    return true;
}

static bool SetGroupFromTlv(TrustedGroupEntry *entry, TlvGroupElement *group)
{
    entry->name = CreateString();
    entry->id = CreateString();
    entry->managers = CREATE_HC_VECTOR(StringVector)
    entry->friends = CREATE_HC_VECTOR(StringVector)
    entry->sharedUserIdVec = CREATE_HC_VECTOR(Int64Vector)
    if (!LoadStringVectorFromParcel(&entry->managers, &group->managers.data)) {
        return false;
    }
    if (!LoadStringVectorFromParcel(&entry->friends, &group->friends.data)) {
        return false;
    }
    if (!LoadInt64VectorFromParcel(&entry->sharedUserIdVec, &group->sharedUserIdVec.data)) {
        return false;
    }
    if (!StringSet(&entry->name, group->name.data)) {
        return false;
    }
    if (!StringSet(&entry->id, group->id.data)) {
        return false;
    }
    entry->type = group->type.data;
    entry->visibility = group->visibility.data;
    entry->expireTime = group->expireTime.data;
    entry->userId = group->userId.data;
    return true;
}

static bool LoadGroupDb(HCDataBaseV1 *db)
{
    uint32_t index;
    TlvGroupElement *group = NULL;
    FOR_EACH_HC_VECTOR(db->groups.data, index, group) {
        TrustedGroupEntry *entry = HcMalloc(sizeof(TrustedGroupEntry), 0);
        if (entry == NULL) {
            return false;
        }
        if (!SetGroupFromTlv(entry, group)) {
            DestroyGroupEntryStruct(entry);
            HcFree(entry);
            return false;
        }
        if (g_trustedGroupTable.pushBack(&g_trustedGroupTable, (const TrustedGroupEntry **)&entry) == NULL) {
            DestroyGroupEntryStruct(entry);
            HcFree(entry);
            return false;
        }
    }
    return true;
}

static bool SetDevAuthFromTlv(TrustedDeviceEntry *authInfo, TlvDevAuthElement *devAuth)
{
    authInfo->udid = CreateString();
    authInfo->authId = CreateString();
    authInfo->serviceType = CreateString();
    authInfo->ext = CreateParcel(0, 0);
    authInfo->groupEntry = GetGroupEntryByGroupIdInner(StringGet(&devAuth->groupId.data));
    if (authInfo->groupEntry != NULL) {
        if (!StringSet(&authInfo->udid, devAuth->udid.data)) {
            return false;
        }
        if (!StringSet(&authInfo->authId, devAuth->authId.data)) {
            return false;
        }
        if (!StringSet(&authInfo->serviceType, devAuth->serviceType.data)) {
            return false;
        }
        if (!ParcelCopy(&devAuth->ext.data, &authInfo->ext)) {
            return false;
        }
        authInfo->credential = devAuth->info.data.credential;
        authInfo->devType = devAuth->info.data.devType;
        authInfo->userId = devAuth->info.data.userId;
        authInfo->lastTm = devAuth->info.data.lastTm;
        return true;
    } else {
        return false;
    }
}

static bool LoadDevAuthDb(HCDataBaseV1 *db)
{
    uint32_t index;
    TlvDevAuthElement *devAuth = NULL;
    FOR_EACH_HC_VECTOR(db->deviceAuthInfos.data, index, devAuth) {
        TrustedDeviceEntry authInfo;
        if (!SetDevAuthFromTlv(&authInfo, devAuth)) {
            DestroyDeviceEntryStruct(&authInfo);
            return false;
        }
        if (g_trustedDeviceTable.pushBack(&g_trustedDeviceTable, &authInfo) == NULL) {
            DestroyDeviceEntryStruct(&authInfo);
            return false;
        }
    }
    return true;
}

static bool LoadDBFromParcel(HcParcel *parcelIn)
{
    bool ret = false;
    HCDataBaseV1 dbv1;
    TLV_INIT(HCDataBaseV1, &dbv1);
    if (DecodeTlvMessage((TlvBase *)&dbv1, parcelIn, false)) {
        if (!LoadGroupDb(&dbv1)) {
            TLV_DEINIT(dbv1);
            return false;
        }
        if (!LoadDevAuthDb(&dbv1)) {
            TLV_DEINIT(dbv1);
            return false;
        }
        ret = true;
    } else {
        LOGE("[DB]: Decode Tlv Message Failed!");
    }
    TLV_DEINIT(dbv1);
    return ret;
}

static bool LoadDB(void)
{
    FileHandle file;
    int ret = HcFileOpen(FILE_ID_GROUP, MODE_FILE_READ, &file);
    if (ret != 0) {
        return false;
    }
    int fileSize = HcFileSize(file);
    if (fileSize <= 0) {
        HcFileClose(file);
        return false;
    }
    char *fileData = (char *)HcMalloc(fileSize, 0);
    if (fileData == NULL) {
        HcFileClose(file);
        return false;
    }
    if (HcFileRead(file, fileData, fileSize) != fileSize) {
        HcFileClose(file);
        HcFree(fileData);
        return false;
    }
    HcFileClose(file);
    HcParcel parcel = CreateParcel(0, 0);
    if (!ParcelWrite(&parcel, fileData, fileSize)) {
        HcFree(fileData);
        DeleteParcel(&parcel);
        return false;
    }
    ret = LoadDBFromParcel(&parcel);
    HcFree(fileData);
    DeleteParcel(&parcel);
    return ret;
}

static bool SaveGroupDb(HCDataBaseV1 *db)
{
    uint32_t index;
    TrustedGroupEntry **entry;
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        TlvGroupElement tmp;
        TlvGroupElement *element = db->groups.data.pushBack(&db->groups.data, &tmp);
        if (element != NULL) {
            TLV_INIT(TlvGroupElement, element);
            if (!StringSet(&element->name.data, (*entry)->name)) {
                return false;
            }
            if (!StringSet(&element->id.data, (*entry)->id)) {
                return false;
            }
            element->type.data = (*entry)->type;
            element->visibility.data = (*entry)->visibility;
            element->expireTime.data = (*entry)->expireTime;
            element->userId.data = (*entry)->userId;
            if (!SaveStringVectorToParcel(&(*entry)->managers, &element->managers.data)) {
                return false;
            }
            if (!SaveStringVectorToParcel(&(*entry)->friends, &element->friends.data)) {
                return false;
            }
            if (!SaveInt64VectorToParcel(&(*entry)->sharedUserIdVec, &element->sharedUserIdVec.data)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

static bool SaveDevAuthDb(HCDataBaseV1 *db)
{
    uint32_t index;
    TrustedDeviceEntry *authInfo = NULL;
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, authInfo) {
        TlvDevAuthElement tmp;
        TlvDevAuthElement *element = db->deviceAuthInfos.data.pushBack(&db->deviceAuthInfos.data, &tmp);
        if (element != NULL) {
            TLV_INIT(TlvDevAuthElement, element);
            if (!StringSet(&element->groupId.data, authInfo->groupEntry->id)) {
                return false;
            }
            if (!StringSet(&element->udid.data, authInfo->udid)) {
                return false;
            }
            if (!StringSet(&element->authId.data, authInfo->authId)) {
                return false;
            }
            if (!StringSet(&element->serviceType.data, authInfo->serviceType)) {
                return false;
            }
            if (!ParcelCopy(&element->ext.data, &authInfo->ext)) {
                return false;
            }
            element->info.data.credential = authInfo->credential;
            element->info.data.devType = authInfo->devType;
            element->info.data.userId = authInfo->userId;
            element->info.data.lastTm = authInfo->lastTm;
        } else {
            return false;
        }
    }
    return true;
}

static bool SaveDBToParcel(HcParcel *parcelOut)
{
    int ret;
    HCDataBaseV1 dbv1;
    TLV_INIT(HCDataBaseV1, &dbv1);
    dbv1.version.data = 1;
    do {
        if (!SaveGroupDb(&dbv1)) {
            ret = false;
            break;
        }
        if (!SaveDevAuthDb(&dbv1)) {
            ret = false;
            break;
        }
        if (!EncodeTlvMessage((TlvBase *)&dbv1, parcelOut)) {
            LOGE("[DB]: Encode Tlv Message failed!");
            ret = false;
            break;
        }
        ret = true;
    } while (0);
    TLV_DEINIT(dbv1);
    return ret;
}

static bool SaveDB(void)
{
    HcParcel parcel = CreateParcel(0, 0);
    if (!SaveDBToParcel(&parcel)) {
        DeleteParcel(&parcel);
        return false;
    }
    FileHandle file;
    int ret = HcFileOpen(FILE_ID_GROUP, MODE_FILE_WRITE, &file);
    if (ret != 0) {
        DeleteParcel(&parcel);
        return false;
    }
    int fileSize = (int)GetParcelDataSize(&parcel);
    const char *fileData = GetParcelData(&parcel);
    if (HcFileWrite(file, fileData, fileSize) == fileSize) {
        ret = true;
    } else {
        ret = false;
    }
    DeleteParcel(&parcel);
    HcFileClose(file);
    return ret;
}

int32_t AddGroup(const GroupInfo *groupInfo)
{
    LOGI("[DB]: Start to add a group to database!");
    if (groupInfo == NULL) {
        LOGE("[DB]: The input groupInfo is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (IsGroupExistByGroupId(StringGet(&groupInfo->id))) {
        LOGE("[DB]: The group corresponding to the groupId already exists and cannot be created again!");
        return HC_ERR_GROUP_DUPLICATE;
    }
    TrustedGroupEntry *entry = CreateGroupEntryStruct();
    if (entry == NULL) {
        LOGE("[DB]: Failed to allocate groupEntry memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result = GenerateGroupEntryByInfo(groupInfo, entry);
    if (result != HC_SUCCESS) {
        DestroyGroupEntryStruct(entry);
        HcFree(entry);
        return result;
    }
    g_databaseMutex->lock(g_databaseMutex);
    if (g_trustedGroupTable.pushBack(&g_trustedGroupTable, (const TrustedGroupEntry **)&entry) == NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: Failed to push groupEntry to groupTable!");
        DestroyGroupEntryStruct(entry);
        HcFree(entry);
        return HC_ERR_MEMORY_COPY;
    }
    if (!SaveDB()) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: Failed to save database!");
        return HC_ERR_SAVE_DB_FAILED;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    if (entry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
        NotifyGroupCreated(entry, DEFAULT_USER_ID);
    }
    LOGI("[DB]: Add a group to database successfully! [GroupType]: %d", groupInfo->type);
    return HC_SUCCESS;
}

int32_t AddGroupManager(const char *groupId, const char *managerAppId)
{
    LOGI("[DB]: Start to add a manager to the group! [Manager]: %s", managerAppId);
    if ((groupId == NULL) || (managerAppId == NULL)) {
        LOGE("[DB]: The input groupId or manager is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString managerStr = CreateString();
            if (!StringSetPointer(&managerStr, managerAppId)) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to copy manager!");
                DeleteString(&managerStr);
                return HC_ERR_MEMORY_COPY;
            }
            if ((*entry)->managers.pushBackT(&(*entry)->managers, managerStr) == NULL) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to push manager to managerVec!");
                DeleteString(&managerStr);
                return HC_ERR_MEMORY_COPY;
            }
            if (!SaveDB()) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to save database!");
                return HC_ERR_SAVE_DB_FAILED;
            }
            g_databaseMutex->unlock(g_databaseMutex);
            LOGI("[DB]: Add a manager to the group successfully! [Manager]: %s", managerAppId);
            return HC_SUCCESS;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t AddGroupFriend(const char *groupId, const char *friendAppId)
{
    LOGI("[DB]: Start to add a friend to the group! [Friend]: %s", friendAppId);
    if ((groupId == NULL) || (friendAppId == NULL)) {
        LOGE("[DB]: The input groupId or friend is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString friendStr = CreateString();
            if (!StringSetPointer(&friendStr, friendAppId)) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to copy friend!");
                DeleteString(&friendStr);
                return HC_ERR_MEMORY_COPY;
            }
            if ((*entry)->friends.pushBackT(&(*entry)->friends, friendStr) == NULL) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to push friend to friendVec!");
                return HC_ERR_MEMORY_COPY;
            }
            if (!SaveDB()) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to save database!");
                return HC_ERR_SAVE_DB_FAILED;
            }
            g_databaseMutex->unlock(g_databaseMutex);
            LOGI("[DB]: Add a friend to the group successfully! [Friend]: %s", friendAppId);
            return HC_SUCCESS;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_SUCCESS;
}

int32_t RemoveGroupManager(const char *groupId, const char *managerAppId)
{
    LOGI("[DB]: Start to delete a manager from the group! [Manager]: %s", managerAppId);
    if ((groupId == NULL) || (managerAppId == NULL)) {
        LOGE("[DB]: The input groupId or manager is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString *managerEntry = NULL;
            uint32_t managerIndex;
            FOR_EACH_HC_VECTOR((*entry)->managers, managerIndex, managerEntry) {
                if ((managerIndex > 0) && (managerEntry != NULL) &&
                    (strcmp(StringGet(managerEntry), managerAppId) == 0)) {
                    HcString tmpManager;
                    HC_VECTOR_POPELEMENT(&((*entry)->managers), &tmpManager, managerIndex);
                    DeleteString(&tmpManager);
                    if (!SaveDB()) {
                        LOGE("[DB]: Failed to save database!");
                        g_databaseMutex->unlock(g_databaseMutex);
                        return HC_ERR_SAVE_DB_FAILED;
                    }
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGI("[DB]: Delete a manager from the group successfully! [Manager]: %s", managerAppId);
                    return HC_SUCCESS;
                }
            }
            LOGE("[DB]: The manager does not exist in the group!");
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_ERR_MANAGER_NOT_EXIST;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t RemoveGroupFriend(const char *groupId, const char *friendAppId)
{
    LOGI("[DB]: Start to delete a friend from the group! [Friend]: %s", friendAppId);
    if ((groupId == NULL) || (friendAppId == NULL)) {
        LOGE("[DB]: The input groupId or friend is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString *friendEntry = NULL;
            uint32_t friendIndex;
            FOR_EACH_HC_VECTOR((*entry)->friends, friendIndex, friendEntry) {
                if ((friendEntry != NULL) && (strcmp(StringGet(friendEntry), friendAppId) == 0)) {
                    HcString tmpFriend;
                    HC_VECTOR_POPELEMENT(&((*entry)->friends), &tmpFriend, friendIndex);
                    DeleteString(&tmpFriend);
                    if (!SaveDB()) {
                        LOGE("[DB]: Failed to save database!");
                        g_databaseMutex->unlock(g_databaseMutex);
                        return HC_ERR_SAVE_DB_FAILED;
                    }
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGI("[DB]: Delete a friend from the group successfully! [Friend]: %s", friendAppId);
                    return HC_SUCCESS;
                }
            }
            LOGE("[DB]: The friend does not exist in the group!");
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_ERR_FRIEND_NOT_EXIST;
        }
    }
    LOGE("[DB]: The group does not exist!");
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t GetGroupManagers(const char *groupId, CJson *returnManagers)
{
    if ((groupId == NULL) || (returnManagers == NULL)) {
        LOGE("[DB]: The input groupId or returnManagers is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString *managerEntry = NULL;
            uint32_t managerIndex;
            FOR_EACH_HC_VECTOR((*entry)->managers, managerIndex, managerEntry) {
                if ((managerEntry != NULL) &&
                    (AddStringToArray(returnManagers, StringGet(managerEntry)) != HC_SUCCESS)) {
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGE("[DB]: Failed to add manager to returnManagers!");
                    return HC_ERR_JSON_FAIL;
                }
            }
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_SUCCESS;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t GetGroupFriends(const char *groupId, CJson *returnFriends)
{
    if ((groupId == NULL) || (returnFriends == NULL)) {
        LOGE("[DB]: The input groupId or returnFriends is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            HcString *friendEntry = NULL;
            uint32_t friendIndex;
            FOR_EACH_HC_VECTOR((*entry)->friends, friendIndex, friendEntry) {
                if ((friendEntry != NULL) &&
                    (AddStringToArray(returnFriends, StringGet(friendEntry)) != HC_SUCCESS)) {
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGE("[DB]: Failed to add friend to returnFriends!");
                    return HC_ERR_JSON_FAIL;
                }
            }
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_SUCCESS;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t CompareVisibility(const char *groupId, int groupVisibility)
{
    if (groupId == NULL) {
        LOGE("[DB]: The input groupId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    TrustedGroupEntry **entry = NULL;
    uint32_t groupIndex;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, groupIndex, entry) {
        if ((entry != NULL) && (*entry != NULL) && (IsGroupIdEquals(*entry, groupId))) {
            if (((uint32_t)((*entry)->visibility) & (uint32_t)groupVisibility) == 0) {
                g_databaseMutex->unlock(g_databaseMutex);
                return HC_ERROR;
            } else {
                g_databaseMutex->unlock(g_databaseMutex);
                return HC_SUCCESS;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

bool IsTrustedDeviceExist(const char *udid)
{
    g_databaseMutex->lock(g_databaseMutex);
    if (GetTrustedDeviceEntry(udid, NULL) != NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    } else {
        g_databaseMutex->unlock(g_databaseMutex);
        return false;
    }
}

int32_t GetTrustedDevNumber(void)
{
    g_databaseMutex->lock(g_databaseMutex);
    int num = GetTrustedDeviceNum();
    g_databaseMutex->unlock(g_databaseMutex);
    return num;
}

int32_t AddTrustedDevice(const DeviceInfo *deviceInfo, const Uint8Buff *ext)
{
    LOGI("[DB]: Start to add a trusted device to database!");
    if (deviceInfo == NULL) {
        LOGE("[DB]: The input deviceInfo is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    const char *udid = StringGet(&(deviceInfo->udid));
    bool isTrustedDeviceNumChanged = false;
    g_databaseMutex->lock(g_databaseMutex);
    if (GetTrustedDeviceEntry(udid, NULL) == NULL) {
        isTrustedDeviceNumChanged = true;
    }
    if (GetTrustedDeviceEntry(udid, StringGet(&deviceInfo->groupId)) == NULL) {
        TrustedDeviceEntry deviceEntry;
        if (!InitAuthInfo(deviceInfo, ext, &deviceEntry)) {
            g_databaseMutex->unlock(g_databaseMutex);
            DestroyDeviceEntryStruct(&deviceEntry);
            return HC_ERR_MEMORY_COPY;
        }
        if (g_trustedDeviceTable.pushBack(&g_trustedDeviceTable, &deviceEntry) == NULL) {
            g_databaseMutex->unlock(g_databaseMutex);
            LOGE("[DB]: Failed to push deviceEntry to deviceTable!");
            DestroyDeviceEntryStruct(&deviceEntry);
            return HC_ERR_MEMORY_COPY;
        }
        if (!SaveDB()) {
            g_databaseMutex->unlock(g_databaseMutex);
            LOGE("[DB]: Failed to save database!");
            return HC_ERR_SAVE_DB_FAILED;
        }
        if (isTrustedDeviceNumChanged) {
            NotifyTrustedDeviceNumChanged();
        }
        NotifyDeviceBound(deviceEntry.groupEntry, udid, DEFAULT_USER_ID);
        g_databaseMutex->unlock(g_databaseMutex);
        LOGI("[DB]: Add a trusted device to database successfully!");
        return HC_SUCCESS;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The device already exists in the group!");
    return HC_ERR_DEVICE_DUPLICATE;
}

int32_t DelTrustedDevice(const char *udid, const char *groupId)
{
    LOGI("[DB]: Start to delete a trusted device from database!");
    if ((udid == NULL) || (groupId == NULL)) {
        LOGE("[DB]: The input udid or groupId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t devIndex;
    TrustedDeviceEntry *deviceEntry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, devIndex, deviceEntry) {
        if ((deviceEntry != NULL) && (deviceEntry->groupEntry != NULL)) {
            if (((strcmp(StringGet(&deviceEntry->udid), udid) == 0)) &&
                (CompareGroupIdInDeviceEntryOrNull(deviceEntry, groupId))) {
                TrustedDeviceEntry tmpDeviceEntry;
                HC_VECTOR_POPELEMENT(&g_trustedDeviceTable, &tmpDeviceEntry, devIndex);
                if (!SaveDB()) {
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGE("[DB]: Failed to save database!");
                    return HC_ERR_SAVE_DB_FAILED;
                }
                CheckAndNotifyAfterDelDevice(&tmpDeviceEntry);
                DestroyDeviceEntryStruct(&tmpDeviceEntry);
                g_databaseMutex->unlock(g_databaseMutex);
                LOGI("[DB]: Delete a trusted device from database successfully!");
                return HC_SUCCESS;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The trusted device is not found!");
    return HC_ERR_DEVICE_NOT_EXIST;
}

int32_t DelTrustedDeviceByAuthId(const char *authId, const char *groupId)
{
    LOGI("[DB]: Start to delete a trusted device from database!");
    if ((authId == NULL) || (groupId == NULL)) {
        LOGE("[DB]: The input authId or groupId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t devIndex;
    TrustedDeviceEntry *deviceEntry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, devIndex, deviceEntry) {
        if ((deviceEntry != NULL) && (deviceEntry->groupEntry != NULL)) {
            if (((strcmp(StringGet(&deviceEntry->authId), authId) == 0)) &&
                (IsGroupIdEquals(deviceEntry->groupEntry, groupId))) {
                TrustedDeviceEntry tmpDeviceEntry;
                HC_VECTOR_POPELEMENT(&g_trustedDeviceTable, &tmpDeviceEntry, devIndex);
                if (!SaveDB()) {
                    g_databaseMutex->unlock(g_databaseMutex);
                    LOGE("[DB]: Failed to save database!");
                    return HC_ERR_SAVE_DB_FAILED;
                }
                CheckAndNotifyAfterDelDevice(&tmpDeviceEntry);
                DestroyDeviceEntryStruct(&tmpDeviceEntry);
                g_databaseMutex->unlock(g_databaseMutex);
                LOGI("[DB]: Delete a trusted device from database successfully!");
                return HC_SUCCESS;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The trusted device is not found!");
    return HC_ERR_DEVICE_NOT_EXIST;
}

static void DeleteUserIdExpiredDeviceEntry(int64_t curUserId)
{
    uint32_t devIndex = 0;
    TrustedDeviceEntry *deviceEntry = NULL;
    while (devIndex < g_trustedDeviceTable.size(&g_trustedDeviceTable)) {
        deviceEntry = g_trustedDeviceTable.getp(&g_trustedDeviceTable, devIndex);
        if (deviceEntry == NULL) {
            devIndex++;
            continue;
        }
        if (deviceEntry->groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
            devIndex++;
            continue;
        }
        if (deviceEntry->groupEntry->userId == curUserId) {
            devIndex++;
            continue;
        }
        TrustedDeviceEntry tmpDeviceEntry;
        HC_VECTOR_POPELEMENT(&g_trustedDeviceTable, &tmpDeviceEntry, devIndex);
        CheckAndNotifyAfterDelDevice(&tmpDeviceEntry);
        DestroyDeviceEntryStruct(&tmpDeviceEntry);
    }
}

static void DeleteUserIdExpiredGroupEntry(int64_t curUserId)
{
    TrustedGroupEntry **groupEntry = NULL;
    uint32_t groupIndex = 0;
    while (groupIndex < g_trustedGroupTable.size(&g_trustedGroupTable)) {
        groupEntry = g_trustedGroupTable.getp(&g_trustedGroupTable, groupIndex);
        if ((groupEntry == NULL) || (*groupEntry == NULL)) {
            groupIndex++;
            continue;
        }
        if ((*groupEntry)->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
            groupIndex++;
            continue;
        }
        if ((*groupEntry)->userId == curUserId) {
            groupIndex++;
            continue;
        }
        TrustedGroupEntry *tmpEntry = NULL;
        HC_VECTOR_POPELEMENT(&g_trustedGroupTable, &tmpEntry, groupIndex);
        uint32_t tmpIndex;
        int64_t *sharedUserId = NULL;
        FOR_EACH_HC_VECTOR(tmpEntry->sharedUserIdVec, tmpIndex, sharedUserId) {
            NotifyGroupDeleted(tmpEntry, *sharedUserId);
        }
        DestroyGroupEntryStruct(tmpEntry);
        HcFree(tmpEntry);
    }
}

static void DeleteAccountDeviceEntry(void)
{
    uint32_t devIndex = 0;
    TrustedDeviceEntry *deviceEntry = NULL;
    while (devIndex < g_trustedDeviceTable.size(&g_trustedDeviceTable)) {
        deviceEntry = g_trustedDeviceTable.getp(&g_trustedDeviceTable, devIndex);
        if (deviceEntry == NULL) {
            devIndex++;
            continue;
        }
        if ((deviceEntry->groupEntry->type != IDENTICAL_ACCOUNT_GROUP) &&
            (deviceEntry->groupEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP)) {
            devIndex++;
            continue;
        }
        TrustedDeviceEntry tmpDeviceEntry;
        HC_VECTOR_POPELEMENT(&g_trustedDeviceTable, &tmpDeviceEntry, devIndex);
        CheckAndNotifyAfterDelDevice(&tmpDeviceEntry);
        DestroyDeviceEntryStruct(&tmpDeviceEntry);
    }
}

static void DeleteAccountGroupEntry(void)
{
    TrustedGroupEntry **groupEntry = NULL;
    uint32_t groupIndex = 0;
    while (groupIndex < g_trustedGroupTable.size(&g_trustedGroupTable)) {
        groupEntry = g_trustedGroupTable.getp(&g_trustedGroupTable, groupIndex);
        if ((groupEntry == NULL) || (*groupEntry == NULL)) {
            groupIndex++;
            continue;
        }
        if (((*groupEntry)->type != IDENTICAL_ACCOUNT_GROUP) &&
            ((*groupEntry)->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP)) {
            groupIndex++;
            continue;
        }
        TrustedGroupEntry *tmpEntry = NULL;
        HC_VECTOR_POPELEMENT(&g_trustedGroupTable, &tmpEntry, groupIndex);
        if (tmpEntry->type == IDENTICAL_ACCOUNT_GROUP) {
            NotifyGroupDeleted(tmpEntry, DEFAULT_USER_ID);
        } else {
            uint32_t tmpIndex;
            int64_t *sharedUserId = NULL;
            FOR_EACH_HC_VECTOR(tmpEntry->sharedUserIdVec, tmpIndex, sharedUserId) {
                NotifyGroupDeleted(tmpEntry, *sharedUserId);
            }
        }
        DestroyGroupEntryStruct(tmpEntry);
        HcFree(tmpEntry);
    }
}

static void DelDeviceEntryByGroupId(const char *groupId)
{
    uint32_t devIndex = 0;
    TrustedDeviceEntry *deviceEntry = NULL;
    while (devIndex < g_trustedDeviceTable.size(&g_trustedDeviceTable)) {
        deviceEntry = g_trustedDeviceTable.getp(&g_trustedDeviceTable, devIndex);
        if (deviceEntry == NULL) {
            devIndex++;
            continue;
        }
        if (strcmp(StringGet(&deviceEntry->groupEntry->id), groupId) != 0) {
            devIndex++;
            continue;
        }
        TrustedDeviceEntry tmpDeviceEntry;
        HC_VECTOR_POPELEMENT(&g_trustedDeviceTable, &tmpDeviceEntry, devIndex);
        CheckAndNotifyAfterDelDevice(&tmpDeviceEntry);
        DestroyDeviceEntryStruct(&tmpDeviceEntry);
    }
}

static void DelGroupEntryByGroupId(const char *groupId)
{
    TrustedGroupEntry **groupEntry = NULL;
    uint32_t groupIndex = 0;
    while (groupIndex < g_trustedGroupTable.size(&g_trustedGroupTable)) {
        groupEntry = g_trustedGroupTable.getp(&g_trustedGroupTable, groupIndex);
        if ((groupEntry == NULL) || (*groupEntry == NULL) || (!IsGroupIdEquals(*groupEntry, groupId))) {
            groupIndex++;
            continue;
        }
        TrustedGroupEntry *tmpEntry = NULL;
        HC_VECTOR_POPELEMENT(&g_trustedGroupTable, &tmpEntry, groupIndex);
        if (tmpEntry->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
            NotifyGroupDeleted(tmpEntry, DEFAULT_USER_ID);
        } else {
            uint32_t tmpIndex;
            int64_t *sharedUserId = NULL;
            FOR_EACH_HC_VECTOR(tmpEntry->sharedUserIdVec, tmpIndex, sharedUserId) {
                NotifyGroupDeleted(tmpEntry, *sharedUserId);
            }
        }
        DestroyGroupEntryStruct(tmpEntry);
        HcFree(tmpEntry);
        LOGI("[DB]: Delete a group from database successfully!");
        break;
    }
    LOGI("[DB]: The group does not exist!");
}

int32_t DeleteUserIdExpiredGroups(int64_t curUserId)
{
    LOGI("[DB]: Start to delete all across account groups with expired userId!");
    g_databaseMutex->lock(g_databaseMutex);
    DeleteUserIdExpiredDeviceEntry(curUserId);
    DeleteUserIdExpiredGroupEntry(curUserId);
    if (!SaveDB()) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: Failed to save database!");
        return HC_ERR_SAVE_DB_FAILED;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGI("[DB]: Delete all across account groups with expired userId successfully!");
    return HC_SUCCESS;
}

int32_t DeleteAllAccountGroup(void)
{
    LOGI("[DB]: Start to delete all account-related groups!");
    g_databaseMutex->lock(g_databaseMutex);
    DeleteAccountDeviceEntry();
    DeleteAccountGroupEntry();
    if (!SaveDB()) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: Failed to save database!");
        return HC_ERR_SAVE_DB_FAILED;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGI("[DB]: Delete all account-related groups successfully!");
    return HC_SUCCESS;
}

int32_t ChangeSharedUserIdVec(Int64Vector *sharedUserIdList)
{
    LOGI("[DB]: Start to change shared userId list!");
    TrustedGroupEntry **entry = NULL;
    uint32_t index;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((entry != NULL) && (*entry != NULL) && ((*entry)->type == ACROSS_ACCOUNT_AUTHORIZE_GROUP)) {
            DeleteExpiredSharedUserId(sharedUserIdList, *entry);
            LOGI("[DB]: Delete expired local userIds successfully!");
            AddNewSharedUserId(sharedUserIdList, *entry);
            LOGI("[DB]: Add new userIds successfully!");
            if (!SaveDB()) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to save database!");
                return HC_ERR_SAVE_DB_FAILED;
            }
            g_databaseMutex->unlock(g_databaseMutex);
            LOGI("[DB]: Change shared userId list successfully!");
            return HC_SUCCESS;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The across account group does not exist!");
    return HC_ERR_GROUP_NOT_EXIST;
}

int32_t DelGroupByGroupId(const char *groupId)
{
    LOGI("[DB]: Start to delete a group from database!");
    if (groupId == NULL) {
        LOGE("[DB]: The input groupId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    g_databaseMutex->lock(g_databaseMutex);
    DelDeviceEntryByGroupId(groupId);
    DelGroupEntryByGroupId(groupId);
    if (!SaveDB()) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: Failed to save database!");
        return HC_ERR_SAVE_DB_FAILED;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

bool IsTrustedDeviceInGroup(const char *groupId, const char *udid)
{
    if ((groupId == NULL) || (udid == NULL)) {
        LOGE("[DB]: The input groupId or udid is NULL!");
        return false;
    }
    g_databaseMutex->lock(g_databaseMutex);
    if (GetTrustedDeviceEntry(udid, groupId) != NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    } else {
        g_databaseMutex->unlock(g_databaseMutex);
        return false;
    }
}

bool IsTrustedDeviceInGroupByAuthId(const char *groupId, const char *authId)
{
    if ((groupId == NULL) || (authId == NULL)) {
        LOGE("[DB]: The input groupId or authId is NULL!");
        return false;
    }
    g_databaseMutex->lock(g_databaseMutex);
    if (GetTrustedDeviceEntryByAuthId(authId, groupId) != NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    } else {
        g_databaseMutex->unlock(g_databaseMutex);
        return false;
    }
}

bool IsGroupExist(const char *ownerName, const char *groupName)
{
    if ((ownerName == NULL) || (groupName == NULL)) {
        LOGE("[DB]: The input ownerName or groupName is NULL!");
        return false;
    }
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if (strcmp(StringGet(&(*entry)->name), groupName) == 0) {
            if (HC_VECTOR_SIZE(&(*entry)->managers) != 0) {
                HcString entryOwner = HC_VECTOR_GET(&(*entry)->managers, 0);
                if (strcmp(StringGet(&entryOwner), ownerName) == 0) {
                    g_databaseMutex->unlock(g_databaseMutex);
                    return true;
                }
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

bool IsIdenticalGroupExist(void)
{
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((*entry)->type == IDENTICAL_ACCOUNT_GROUP) {
            g_databaseMutex->unlock(g_databaseMutex);
            return true;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

bool IsAcrossAccountGroupExist(void)
{
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((*entry)->type == ACROSS_ACCOUNT_AUTHORIZE_GROUP) {
            g_databaseMutex->unlock(g_databaseMutex);
            return true;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

bool IsGroupExistByGroupId(const char *groupId)
{
    if (groupId == NULL) {
        LOGE("[DB]: The input groupId is NULL!");
        return false;
    }
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if (IsGroupIdEquals(*entry, groupId)) {
            g_databaseMutex->unlock(g_databaseMutex);
            return true;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

int32_t GetDeviceInfoForDevAuth(const char *udid, const char *groupId, DeviceInfo *deviceInfo)
{
    if ((udid == NULL) || (groupId == NULL) || (deviceInfo == NULL)) {
        LOGE("[DB]: The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    g_databaseMutex->lock(g_databaseMutex);
    TrustedDeviceEntry *deviceEntry = GetTrustedDeviceEntry(udid, groupId);
    if (deviceEntry == NULL) {
        LOGE("[DB]: The trusted device is not found!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_DEVICE_NOT_EXIST;
    }
    if (!StringSet(&deviceInfo->authId, deviceEntry->authId)) {
        LOGE("[DB]: Failed to copy authId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&deviceInfo->udid, deviceEntry->udid)) {
        LOGE("[DB]: Failed to copy authId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&(deviceInfo->groupId), deviceEntry->groupEntry->id)) {
        LOGE("[DB]: Failed to copy groupId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&(deviceInfo->serviceType), deviceEntry->serviceType)) {
        LOGE("[DB]: Failed to copy serviceType!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    deviceInfo->credential = deviceEntry->credential;
    deviceInfo->devType = deviceEntry->devType;
    deviceInfo->userId = deviceEntry->userId;
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t GetDeviceInfoByAuthId(const char *authId, const char *groupId, DeviceInfo *deviceInfo)
{
    if ((authId == NULL) || (groupId == NULL) || (deviceInfo == NULL)) {
        LOGE("[DB]: The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    g_databaseMutex->lock(g_databaseMutex);
    TrustedDeviceEntry *deviceEntry = GetTrustedDeviceEntryByAuthId(authId, groupId);
    if (deviceEntry == NULL) {
        LOGE("[DB]: The trusted device is not found!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_DEVICE_NOT_EXIST;
    }
    if (!StringSet(&deviceInfo->authId, deviceEntry->authId)) {
        LOGE("[DB]: Failed to copy authId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&deviceInfo->udid, deviceEntry->udid)) {
        LOGE("[DB]: Failed to copy authId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&(deviceInfo->groupId), deviceEntry->groupEntry->id)) {
        LOGE("[DB]: Failed to copy groupId!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    if (!StringSet(&(deviceInfo->serviceType), deviceEntry->serviceType)) {
        LOGE("[DB]: Failed to copy serviceType!");
        g_databaseMutex->unlock(g_databaseMutex);
        return HC_ERR_MEMORY_COPY;
    }
    deviceInfo->credential = deviceEntry->credential;
    deviceInfo->devType = deviceEntry->devType;
    deviceInfo->userId = deviceEntry->userId;
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t GetGroupNumberByOwner(const char *ownerName)
{
    if (ownerName == NULL) {
        LOGE("[DB]: The input ownerName is NULL!");
        return 0;
    }
    int count = 0;
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if (HC_VECTOR_SIZE(&(*entry)->managers) > 0) {
            HcString entryOwner = HC_VECTOR_GET(&(*entry)->managers, 0);
            if (strcmp(StringGet(&entryOwner), ownerName) == 0) {
                count++;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return count;
}

int32_t GetCurDeviceNumByGroupId(const char *groupId)
{
    if (groupId == NULL) {
        LOGE("[DB]: The input groupId is NULL!");
        return 0;
    }
    int count = 0;
    uint32_t index;
    TrustedDeviceEntry *deviceEntry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, deviceEntry) {
        if (strcmp(StringGet(&deviceEntry->groupEntry->id), groupId) == 0) {
            ++count;
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return count;
}

int32_t GetGroupEntry(const char *groupId, const char *udid, GroupInfo *returnGroupInfo)
{
    if ((groupId == NULL) || (udid == NULL) || (returnGroupInfo == NULL)) {
        LOGE("[DB]: The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    g_databaseMutex->lock(g_databaseMutex);
    int32_t res = GetGroupEntryInner(groupId, udid, returnGroupInfo);
    g_databaseMutex->unlock(g_databaseMutex);
    return res;
}

int32_t GetGroupEntryByGroupId(const char *groupId, GroupInfo *returnGroupInfo)
{
    if ((groupId == NULL) || (returnGroupInfo == NULL)) {
        LOGE("[DB]: The input groupId or returnGroupInfo is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    g_databaseMutex->lock(g_databaseMutex);
    int32_t res = GetGroupEntryInner(groupId, NULL, returnGroupInfo);
    g_databaseMutex->unlock(g_databaseMutex);
    return res;
}

GroupInfo *CreateGroupInfoStruct(void)
{
    GroupInfo *ptr = (GroupInfo *)HcMalloc(sizeof(GroupInfo), 0);
    if (ptr == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return NULL;
    }
    ptr->name = CreateString();
    ptr->id = CreateString();
    ptr->ownerName = CreateString();
    return ptr;
}

void DestroyGroupInfoStruct(GroupInfo *groupInfo)
{
    if (groupInfo == NULL) {
        return;
    }
    DeleteString(&(groupInfo->name));
    DeleteString(&(groupInfo->id));
    DeleteString(&(groupInfo->ownerName));
    HcFree(groupInfo);
    groupInfo = NULL;
}

DeviceInfo *CreateDeviceInfoStruct(void)
{
    DeviceInfo *deviceInfo = (DeviceInfo *)HcMalloc(sizeof(DeviceInfo), 0);
    if (deviceInfo == NULL) {
        LOGE("[DB]: Failed to allocate deviceInfo memory!");
        return NULL;
    }
    deviceInfo->authId = CreateString();
    deviceInfo->udid = CreateString();
    deviceInfo->serviceType = CreateString();
    deviceInfo->groupId = CreateString();
    return deviceInfo;
}

void DestroyDeviceInfoStruct(DeviceInfo *deviceInfo)
{
    if (deviceInfo == NULL) {
        return;
    }
    DeleteString(&(deviceInfo->authId));
    DeleteString(&(deviceInfo->udid));
    DeleteString(&(deviceInfo->serviceType));
    DeleteString(&(deviceInfo->groupId));
    HcFree(deviceInfo);
    deviceInfo = NULL;
}

void CreateGroupInfoVecStruct(GroupInfoVec *vec)
{
    if (vec == NULL) {
        return;
    }
    *vec = CREATE_HC_VECTOR(GroupInfoVec)
}

void DestroyGroupInfoVecStruct(GroupInfoVec *vec)
{
    uint32_t index;
    void **entry = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, entry) {
        if ((entry != NULL) && (*entry != NULL)) {
            DestroyGroupInfoStruct(*entry);
        }
    }
    DESTROY_HC_VECTOR(GroupInfoVec, vec)
}

void CreateDeviceInfoVecStruct(DeviceInfoVec *vec)
{
    if (vec == NULL) {
        return;
    }
    *vec = CREATE_HC_VECTOR(DeviceInfoVec)
}

void DestroyDeviceInfoVecStruct(DeviceInfoVec *vec)
{
    uint32_t index;
    void **entry = NULL;
    FOR_EACH_HC_VECTOR(*vec, index, entry) {
        if ((entry != NULL) && (*entry != NULL)) {
            DestroyDeviceInfoStruct(*entry);
        }
    }
    DESTROY_HC_VECTOR(DeviceInfoVec, vec)
}

int32_t GetJoinedGroupInfoVecByDevId(const GroupQueryParams *params, GroupInfoVec *vec)
{
    if ((params == NULL) || (vec == NULL)) {
        LOGE("[DB]: The input params or vec is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((entry == NULL) || (*entry == NULL)) {
            continue;
        }
        TrustedDeviceEntry *deviceEntry = NULL;
        if (params->udid != NULL) {
            deviceEntry = GetTrustedDeviceEntry(params->udid, StringGet(&(*entry)->id));
        } else if (params->authId != NULL) {
            deviceEntry = GetTrustedDeviceEntryByAuthId(params->authId, StringGet(&(*entry)->id));
        } else {
            LOGE("[DB]: The values of udid and authId are null!");
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_ERR_INVALID_PARAMS;
        }
        if ((deviceEntry != NULL) &&
            (IsSatisfyGroup(*entry, params)) &&
            (IsSatisfyUdidAndAuthId(deviceEntry, params))) {
            int32_t result = PushGroupInfoToVec(*entry, vec);
            if (result != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                return result;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

bool IsGroupOwner(const char *groupId, const char *appId)
{
    if ((groupId == NULL) || (appId == NULL)) {
        LOGE("[DB]: The input parameter contains NULL value!");
        return false;
    }
    g_databaseMutex->lock(g_databaseMutex);
    TrustedGroupEntry *entry = GetGroupEntryByGroupIdInner(groupId);
    if (entry == NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: The group cannot be found!");
        return false;
    }
    if (HC_VECTOR_SIZE(&(entry->managers)) == 0) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: The group does not have manager and owner!");
        return false;
    }
    HcString entryOwner = HC_VECTOR_GET(&(entry->managers), 0);
    if (strcmp(StringGet(&entryOwner), appId) == 0) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    LOGE("[DB]: The visitor is not the group owner!");
    return false;
}

bool IsGroupAccessible(const char *groupId, const char *appId)
{
    if ((groupId == NULL) || (appId == NULL)) {
        LOGE("[DB]: The input groupId or appId is NULL!");
        return false;
    }
    g_databaseMutex->lock(g_databaseMutex);
    TrustedGroupEntry *entry = GetGroupEntryByGroupIdInner(groupId);
    if (entry == NULL) {
        LOGE("[DB]: The group cannot be found!");
        g_databaseMutex->unlock(g_databaseMutex);
        return false;
    }
    if ((entry->visibility == GROUP_VISIBILITY_PUBLIC) ||
        (IsGroupManager(appId, entry)) ||
        (IsGroupFriend(appId, entry))) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

bool IsGroupEditAllowed(const char *groupId, const char *appId)
{
    if ((groupId == NULL) || (appId == NULL)) {
        LOGE("[DB]: The input groupId or appId is NULL!");
        return false;
    }
    g_databaseMutex->lock(g_databaseMutex);
    TrustedGroupEntry *entry = GetGroupEntryByGroupIdInner(groupId);
    if (entry == NULL) {
        g_databaseMutex->unlock(g_databaseMutex);
        LOGE("[DB]: The group cannot be found!");
        return false;
    }
    if (IsGroupManager(appId, entry)) {
        g_databaseMutex->unlock(g_databaseMutex);
        return true;
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return false;
}

int32_t GetLocalDevUdid(char **udid)
{
    uint8_t udidLocal[INPUT_UDID_LEN] = { 0 };
    int res = HcGetUdid(udidLocal, INPUT_UDID_LEN);
    if (res != 0) {
        return res;
    }
    uint32_t index;
    TrustedDeviceEntry *entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, entry) {
        if ((entry != NULL) && (entry->groupEntry != NULL) && (CompareUdidInDeviceEntryOrNull(entry,
            (char *)udidLocal))) {
            const char *localUdid = StringGet(&entry->udid);
            *udid = (char *)HcMalloc((uint32_t)(strlen(localUdid) + 1), 0);
            if ((*udid) == NULL) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to allocate udid memory!");
                return HC_ERR_ALLOC_MEMORY;
            }
            if (strcpy_s(*udid, strlen(localUdid) + 1, localUdid) != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                LOGE("[DB]: Failed to copy localUdid!");
                HcFree(*udid);
                *udid = NULL;
                return HC_ERR_MEMORY_COPY;
            }
            g_databaseMutex->unlock(g_databaseMutex);
            return HC_SUCCESS;
        }
    }
    *udid = NULL;
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

void DestroyUdid(char **udid)
{
    HcFree(*udid);
    *udid = NULL;
}

static int32_t PushSearchGroupInfoToVec(const char *groupId, const char *groupName, TrustedGroupEntry *entry,
    GroupInfoVec *groupInfoVec)
{
    GroupInfo *groupInfo = CreateGroupInfoStruct();
    if (groupInfo == NULL) {
        LOGE("[DB]: Failed to allocate groupInfo memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    const char *tmpGroupId = (groupId != NULL) ? groupId : groupName;
    int64_t sharedUserId = DEFAULT_USER_ID;
    int32_t result = GetSharedUserIdFromVecByGroupId(entry, tmpGroupId, &sharedUserId);
    if (result != HC_SUCCESS) {
        LOGE("Failed to find sharedUserId by groupId!");
        DestroyGroupInfoStruct(groupInfo);
        return result;
    }
    result = GenerateGroupInfoIdAndName(tmpGroupId, tmpGroupId, groupInfo);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return result;
    }
    GenerateGroupInfoSharedUserId(sharedUserId, groupInfo);
    result = GenerateCommonGroupInfoByEntry(entry, groupInfo);
    if (result != HC_SUCCESS) {
        DestroyGroupInfoStruct(groupInfo);
        return result;
    }
    if (groupInfoVec->pushBackT(groupInfoVec, groupInfo) == NULL) {
        LOGE("[DB]: Failed to push groupInfo to groupInfoVec!");
        DestroyGroupInfoStruct(groupInfo);
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

int32_t GetGroupInfo(int groupType, const char *groupId, const char *groupName, const char *groupOwner,
    GroupInfoVec *groupInfoVec)
{
    if (groupInfoVec == NULL) {
        LOGE("[DB]: The input groupInfoVec is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    int32_t result;
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((entry != NULL) && (*entry != NULL) && (CompareSearchParams(groupType, groupId, groupName,
            groupOwner, *entry))) {
            if (((*entry)->type != ACROSS_ACCOUNT_AUTHORIZE_GROUP) || ((groupId == NULL) && (groupName == NULL))) {
                result = PushGroupInfoToVec(*entry, groupInfoVec);
            } else {
                result = PushSearchGroupInfoToVec(groupId, groupName, *entry, groupInfoVec);
            }
            if (result != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                return result;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t GetJoinedGroups(int groupType, GroupInfoVec *groupInfoVec)
{
    int32_t result;
    uint32_t index;
    TrustedGroupEntry **entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedGroupTable, index, entry) {
        if ((entry != NULL) && (*entry != NULL) && ((*entry)->type == groupType)) {
            result = PushGroupInfoToVec(*entry, groupInfoVec);
            if (result != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                return result;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t GetRelatedGroups(const char *peerAuthId, GroupInfoVec *groupInfoVec)
{
    int32_t result;
    uint32_t index;
    TrustedDeviceEntry *entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, entry) {
        if ((entry != NULL) && (strcmp(StringGet(&entry->authId), peerAuthId) == 0)) {
            result = PushGroupInfoToVec(entry->groupEntry, groupInfoVec);
            if (result != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                return result;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t GetTrustedDevices(const char *groupId, DeviceInfoVec *deviceInfoVec)
{
    int32_t result;
    uint32_t index;
    TrustedDeviceEntry *entry = NULL;
    g_databaseMutex->lock(g_databaseMutex);
    FOR_EACH_HC_VECTOR(g_trustedDeviceTable, index, entry) {
        if ((entry != NULL) && (entry->groupEntry != NULL) && (CompareGroupIdInDeviceEntryOrNull(entry, groupId))) {
            result = PushDevInfoToVec(entry, deviceInfoVec);
            if (result != HC_SUCCESS) {
                g_databaseMutex->unlock(g_databaseMutex);
                return result;
            }
        }
    }
    g_databaseMutex->unlock(g_databaseMutex);
    return HC_SUCCESS;
}

int32_t InitDatabase(void)
{
    g_trustedGroupTable = CREATE_HC_VECTOR(TrustedGroupTable)
    g_trustedDeviceTable = CREATE_HC_VECTOR(TrustedDeviceTable)
    if (g_databaseMutex == NULL) {
        g_databaseMutex = (HcMutex *)HcMalloc(sizeof(HcMutex), 0);
        if (g_databaseMutex == NULL) {
            LOGE("[DB]: Alloc databaseMutex failed");
            DESTROY_HC_VECTOR(TrustedDeviceTable, &g_trustedDeviceTable)
            DESTROY_HC_VECTOR(TrustedGroupTable, &g_trustedGroupTable)
            return HC_ERR_ALLOC_MEMORY;
        }
        if (InitHcMutex(g_databaseMutex) != HC_SUCCESS) {
            LOGE("[DB]: Init mutex failed");
            DESTROY_HC_VECTOR(TrustedDeviceTable, &g_trustedDeviceTable)
            DESTROY_HC_VECTOR(TrustedGroupTable, &g_trustedGroupTable)
            HcFree(g_databaseMutex);
            g_databaseMutex = NULL;
            return HC_ERROR;
        }
    }
    SetFilePath(FILE_ID_GROUP, GetStoragePath());
    if (!LoadDB()) {
        LOGI("[DB]: Failed to load database, it may be the first time the database is read!");
    } else {
        LOGI("[DB]: Load database successfully!");
    }
    return HC_SUCCESS;
}

void DestroyDatabase(void)
{
    DestroyTrustDevTable();
    DestroyGroupTable();
    if (g_databaseMutex != NULL) {
        DestroyHcMutex(g_databaseMutex);
        HcFree(g_databaseMutex);
        g_databaseMutex = NULL;
    }
}

void RegGenerateGroupIdFunc(int32_t (*generateGroupId)(int64_t userId, int64_t sharedUserId, char **returnGroupId))
{
    if (generateGroupId == NULL) {
        LOGE("[DB]: The input generate across account group groupId function is null ptr!");
        return;
    }
    g_generateIdFunc = generateGroupId;
}

void DeregGenerateGroupIdFunc(void)
{
    g_generateIdFunc = NULL;
}

void RegisterBroadcaster(const Broadcaster *broadcaster)
{
    if (broadcaster == NULL) {
        LOGE("[DB]: The registered broadcaster is NULL!");
        return;
    }
    g_broadcaster = broadcaster;
}

void DeregisterBroadcaster(void)
{
    g_broadcaster = NULL;
}