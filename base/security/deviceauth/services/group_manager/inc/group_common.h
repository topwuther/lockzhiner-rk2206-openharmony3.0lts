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

#ifndef GROUP_COMMON_H
#define GROUP_COMMON_H

#include "common_util.h"
#include "database.h"
#include "json_utils.h"

bool IsUserTypeValid(int userType);
bool IsExpireTimeValid(int expireTime);
bool IsGroupVisibilityValid(int groupVisibility);

int32_t ProcessKeyPair(int action, const CJson *jsonParams, const char *groupId);
int32_t DeletePeerKeyIfForceUnbind(const char *groupId, const char *peerAuthId, int32_t peerUserType);
int32_t GetHashMessage(const Uint8Buff *first, const Uint8Buff *second, uint8_t **hashMessage, uint32_t *messageSize);
int32_t GetHashResult(const uint8_t *info, uint32_t infoLen, char *hash, uint32_t hashLen);

int32_t CheckGroupExist(const char *groupId);
int32_t CheckGroupNumLimit(int32_t groupType, const char *appId);
int32_t CheckDeviceNumLimit(const char *groupId, const char *peerUdid);
int32_t CheckUserTypeIfExist(const CJson *jsonParams);
int32_t CheckGroupVisibilityIfExist(const CJson *jsonParams);
int32_t CheckExpireTimeIfExist(const CJson *jsonParams);
int32_t CheckPermForGroup(int actionType, const char *callerPkgName, const char *groupId);

int32_t AddGroupNameToParams(const char *groupName, GroupInfo *groupParams);
int32_t AddGroupIdToParams(const char *groupId, GroupInfo *groupParams);
int32_t AddGroupOwnerToParams(const char *owner, GroupInfo *groupParams);
int32_t AddGroupTypeToParams(int groupType, GroupInfo *groupParams);
int32_t AddGroupVisibilityOrDefault(GroupInfo *groupParams);
int32_t AddExpireTimeOrDefault(const CJson *jsonParams, GroupInfo *groupParams);
int32_t AddUdidToParams(DeviceInfo *devParams);
int32_t AddAuthIdToParamsOrDefault(const CJson *jsonParams, DeviceInfo *devParams);
int32_t AddUserTypeToParamsOrDefault(const CJson *jsonParams, DeviceInfo *devParams);
int32_t AddServiceTypeToParams(const char *groupId, DeviceInfo *devParams);
int32_t AddGroupIdToDevParams(const char *groupId, DeviceInfo *devParams);

int32_t AddGroupToDatabaseByJson(int32_t (*generateGroupParams)(const CJson*, const char *, GroupInfo*),
    const CJson *jsonParams, const char *groupId);
int32_t AddDeviceToDatabaseByJson(int32_t (*generateDevParams)(const CJson*, const char*, DeviceInfo*),
    const CJson *jsonParams, const char *groupId);
int32_t DelGroupFromDatabase(const char *groupId);

int32_t ConvertGroupIdToJsonStr(const char *groupId, char **returnJsonStr);
int32_t GenerateBindSuccessData(const char *peerAuthId, const char *groupId, char **returnDataStr);
int32_t GenerateUnbindSuccessData(const char *peerAuthId, const char *groupId, char **returnDataStr);

int32_t GetGroupTypeFromDb(const char *groupId, int *returnGroupType);

#endif
