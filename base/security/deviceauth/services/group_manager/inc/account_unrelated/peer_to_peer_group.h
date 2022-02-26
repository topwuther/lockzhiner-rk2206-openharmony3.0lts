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

#ifndef PEER_TO_PEER_GROUP_H
#define PEER_TO_PEER_GROUP_H

#include "base_group.h"

typedef struct {
    BaseGroup base;
    int32_t (*createGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*deleteGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*addMember)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*deleteMember)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*processData)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*addGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*deleteGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*getGroupRole)(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
        uint32_t *returnSize);
} PeerToPeerGroup;

BaseGroup *GetPeerToPeerGroupInstance(void);
bool IsPeerToPeerGroupSupported(void);

#endif
