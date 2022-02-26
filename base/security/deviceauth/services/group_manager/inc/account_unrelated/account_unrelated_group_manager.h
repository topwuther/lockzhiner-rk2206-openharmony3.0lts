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

#ifndef ACCOUNT_UNRELATED_GROUP_MANAGER_H
#define ACCOUNT_UNRELATED_GROUP_MANAGER_H

#include "device_auth.h"
#include "json_utils.h"

typedef struct {
    void (*createPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*deletePeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*addMemberToPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*deleteMemberFromPeerToPeerGroup)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    void (*processBindData)(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback);
    int32_t (*addGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*deleteGroupRole)(bool isManager, const char *appId, const char *groupId, const char *roleAppId);
    int32_t (*getGroupRole)(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
        uint32_t *returnSize);
} AccountUnrelatedGroupManager;

AccountUnrelatedGroupManager *GetAccountUnrelatedGroupManager(void);
bool IsAccountUnrelatedGroupManagerSupported(void);
bool IsAccountUnrelatedGroupTypeSupported(int32_t groupType);

#endif
