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

#include "account_unrelated_group_manager.h"
#include "common_defs.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "peer_to_peer_group.h"

static void CreatePeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return;
    }
    (void)instance->createGroup(requestId, jsonParams, callback);
}

static void DeletePeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return;
    }
    (void)instance->deleteGroup(requestId, jsonParams, callback);
}

static void AddMemberToPeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return;
    }
    (void)instance->addMember(requestId, jsonParams, callback);
}

static void DeleteMemberFromPeerToPeerGroup(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return;
    }
    (void)instance->deleteMember(requestId, jsonParams, callback);
}

static void ProcessBindData(int64_t requestId, CJson *jsonParams, const DeviceAuthCallback *callback)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return;
    }
    (void)instance->processData(requestId, jsonParams, callback);
}

static int32_t AddGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->addGroupRole(isManager, appId, groupId, roleAppId);
}

static int32_t DeleteGroupRole(bool isManager, const char *appId, const char *groupId, const char *roleAppId)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->deleteGroupRole(isManager, appId, groupId, roleAppId);
}

static int32_t GetGroupRole(bool isManager, const char *appId, const char *groupId, char **returnJsonStr,
    uint32_t *returnSize)
{
    if (!IsPeerToPeerGroupSupported()) {
        LOGE("Peer to peer group is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    PeerToPeerGroup *instance = (PeerToPeerGroup *)GetPeerToPeerGroupInstance();
    if (instance == NULL) {
        LOGE("Failed to get peerToPeerGroup instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getGroupRole(isManager, appId, groupId, returnJsonStr, returnSize);
}

static AccountUnrelatedGroupManager g_nonAccountGroupManager = {
    .createPeerToPeerGroup = CreatePeerToPeerGroup,
    .deletePeerToPeerGroup = DeletePeerToPeerGroup,
    .addMemberToPeerToPeerGroup = AddMemberToPeerToPeerGroup,
    .deleteMemberFromPeerToPeerGroup = DeleteMemberFromPeerToPeerGroup,
    .processBindData = ProcessBindData,
    .addGroupRole = AddGroupRole,
    .deleteGroupRole = DeleteGroupRole,
    .getGroupRole = GetGroupRole
};

AccountUnrelatedGroupManager *GetAccountUnrelatedGroupManager()
{
    return &g_nonAccountGroupManager;
}

bool IsAccountUnrelatedGroupManagerSupported()
{
    return true;
}

bool IsAccountUnrelatedGroupTypeSupported(int32_t groupType)
{
    if ((groupType == PEER_TO_PEER_GROUP) && (IsPeerToPeerGroupSupported())) {
        return true;
    }
    return false;
}