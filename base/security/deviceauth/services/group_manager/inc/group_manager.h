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

#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include "device_auth.h"
#include "json_utils.h"

typedef struct {
    /* callback operation interface for device group management */
    int32_t (*regGroupManagerCallback)(const char *appId, const DeviceAuthCallback *callback);
    int32_t (*unRegGroupManagerCallback)(const char *appId);
    const DeviceAuthCallback *(*getGmCallbackByAppId)(const char *appId);
    /* Non-account groups operation interfaces */
    void (*createPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*deletePeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*addMemberToPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*deleteMemberFromPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*processBindData)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*addGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*deleteGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*getGroupRole)(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
        uint32_t *returnSize);
    /* listener related operation interfaces */
    int32_t (*regListener)(const char *appId, const DataChangeListener *listener);
    int32_t (*unRegListener)(const char *appId);
    /* Account groups operation interfaces */
    int32_t (*syncCreateIdenticalAccountGroup)(const CJson *jsonParams, char **returnDataStr);
    int32_t (*syncDeleteIdenticalAccountGroup)(const CJson *jsonParams);
    int32_t (*getRegisterInfo)(char **returnRegisterInfo);
    int32_t (*processCredential)(int operationCode, const char *reqJsonStr, char **returnJsonStr);
    /* external query interface */
    int32_t (*getAccessibleGroupInfoById)(const char *appId, const char *groupId, char **returnGroupInfo);
    int32_t (*getAccessibleGroupInfo)(const char *appId, const char *queryParams, char **returnGroupVec,
        uint32_t *groupNum);
    int32_t (*getAccessibleJoinedGroups)(const char *appId, int groupType, char **returnGroupVec, uint32_t *groupNum);
    int32_t (*getAccessibleRelatedGroups)(const char *appId, const char *peerDeviceId,
        char **returnGroupVec, uint32_t *groupNum);
    int32_t (*getAccessibleDeviceInfoById)(const char *appId, const char *deviceId,
        const char *groupId, char **returnDeviceInfo);
    int32_t (*getAccessibleTrustedDevices)(const char *appId, const char *groupId,
        char **returnDevInfoVec, uint32_t *deviceNum);
    bool (*isDeviceInAccessibleGroup)(const char *appId, const char *groupId, const char *deviceId);
    void (*destroyInfo)(char **returnInfo);
} GroupManager;

int32_t InitGroupManager(void);
void DestroyGroupManager(void);
GroupManager *GetGroupManagerInstance(void);
bool IsGroupManagerSupported(void);

#endif
