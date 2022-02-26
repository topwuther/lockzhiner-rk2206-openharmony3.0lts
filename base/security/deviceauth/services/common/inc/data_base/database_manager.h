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

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "broadcast_manager.h"
#include "common_defs.h"
#include "common_util.h"
#include "device_auth_defines.h"

#define HC_TRUST_DEV_ENTRY_MAX_NUM 101
#define HC_TRUST_GROUP_ENTRY_MAX_NUM 100

typedef struct {
    int32_t type; /* group type */
    int32_t visibility; /* group visibility */
    char *udid; /* unique device id */
    char *authId; /* id by service defined for authentication */
} GroupQueryParams;

#ifdef __cplusplus
extern "C" {
#endif

int32_t InitDatabase(void);
void DestroyDatabase(void);

Int64Vector CreateInt64Vector(void);
void DestroyInt64Vector(Int64Vector *obj);

int32_t AddGroup(const GroupInfo *addParams);
int32_t DelGroupByGroupId(const char *groupId);
int32_t AddTrustedDevice(const DeviceInfo *deviceInfo, const Uint8Buff *ext);
int32_t DelTrustedDevice(const char *udid, const char *groupId);
int32_t DelTrustedDeviceByAuthId(const char *authId, const char *groupId);
int32_t DeleteUserIdExpiredGroups(int64_t curUserId);
int32_t DeleteAllAccountGroup(void);
int32_t ChangeSharedUserIdVec(Int64Vector *sharedUserIdList);

int32_t GetLocalDevUdid(char **udid);
void DestroyUdid(char **udid);

void RegisterBroadcaster(const Broadcaster *broadcaster);
void DeregisterBroadcaster(void);
void RegGenerateGroupIdFunc(int32_t (*generateGroupId)(int64_t userId, int64_t sharedUserId, char **returnGroupId));
void DeregGenerateGroupIdFunc(void);

int32_t GetTrustedDevNumber(void);
int32_t GetGroupEntryByGroupId(const char *groupId, GroupInfo *returnGroupInfo);
int32_t GetGroupEntry(const char *groupId, const char *udid, GroupInfo *returnGroupInfo);
int32_t GetDeviceInfoForDevAuth(const char *udid, const char *groupId, DeviceInfo *deviceInfo);
int32_t GetDeviceInfoByAuthId(const char *authId, const char *groupId, DeviceInfo *deviceInfo);
int32_t GetJoinedGroupInfoVecByDevId(const GroupQueryParams *params, GroupInfoVec *vec);
int32_t GetGroupNumberByOwner(const char *ownerName);
int32_t GetCurDeviceNumByGroupId(const char *groupId);
int32_t CompareVisibility(const char *groupId, int groupVisibility);
bool IsGroupOwner(const char *groupId, const char *appId);
bool IsGroupAccessible(const char *groupId, const char *appId);
bool IsGroupEditAllowed(const char *groupId, const char *appId);
bool IsGroupExist(const char *ownerName, const char *groupName);
bool IsIdenticalGroupExist(void);
bool IsAcrossAccountGroupExist(void);
bool IsGroupExistByGroupId(const char *groupId);
bool IsTrustedDeviceExist(const char *udid);
bool IsTrustedDeviceInGroup(const char *groupId, const char *udid);
bool IsTrustedDeviceInGroupByAuthId(const char *groupId, const char *authId);
int32_t GetJoinedGroups(int groupType, GroupInfoVec *groupInfoVec);
int32_t GetGroupInfo(int groupType, const char *groupId, const char *groupName, const char *groupOwner,
    GroupInfoVec *groupInfoVec);
int32_t GetRelatedGroups(const char *peerAuthId, GroupInfoVec *groupInfoVec);
int32_t GetTrustedDevices(const char *peerAuthId, DeviceInfoVec *deviceInfoVec);

int32_t AddGroupManager(const char *groupId, const char *managerAppId);
int32_t RemoveGroupManager(const char *groupId, const char *managerAppId);
int32_t GetGroupManagers(const char *groupId, CJson *returnManagers);
int32_t AddGroupFriend(const char *groupId, const char *friendAppId);
int32_t RemoveGroupFriend(const char *groupId, const char *friendAppId);
int32_t GetGroupFriends(const char *groupId, CJson *returnFriends);

GroupInfo *CreateGroupInfoStruct(void);
DeviceInfo *CreateDeviceInfoStruct(void);
void DestroyGroupInfoStruct(GroupInfo *groupInfo);
void DestroyDeviceInfoStruct(DeviceInfo *deviceInfo);
void CreateGroupInfoVecStruct(GroupInfoVec *vec);
void DestroyGroupInfoVecStruct(GroupInfoVec *vec);
void CreateDeviceInfoVecStruct(DeviceInfoVec *vec);
void DestroyDeviceInfoVecStruct(DeviceInfoVec *vec);

#ifdef __cplusplus
}
#endif
#endif