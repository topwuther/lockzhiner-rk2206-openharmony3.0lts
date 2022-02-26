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

#include "broadcast_manager.h"
#include "common_defs.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "hc_mutex.h"
#include "securec.h"

typedef struct {
    char *appId;
    DataChangeListener *listener;
} ListenerEntry;

DECLARE_HC_VECTOR(ListenerEntryVec, ListenerEntry)
IMPLEMENT_HC_VECTOR(ListenerEntryVec, ListenerEntry, 1)
static ListenerEntryVec g_listenerEntryVec;
static HcMutex *g_broadcastMutex = NULL;

static int32_t AddGroupId(const GroupInfo *groupInfo, CJson *message)
{
    if (AddStringToJson(message, FIELD_GROUP_ID, StringGet(&groupInfo->id)) != HC_SUCCESS) {
        LOGE("Failed to add groupId to message!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupType(const GroupInfo *groupInfo, CJson *message)
{
    if (AddIntToJson(message, FIELD_GROUP_TYPE, groupInfo->type) != HC_SUCCESS) {
        LOGE("Failed to add groupType to message!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupName(const GroupInfo *groupInfo, CJson *message)
{
    if (AddStringToJson(message, FIELD_GROUP_NAME, StringGet(&groupInfo->name)) != HC_SUCCESS) {
        LOGE("Failed to add groupName to message!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupOwner(const GroupInfo *groupInfo, CJson *message)
{
    if (AddStringToJson(message, FIELD_GROUP_OWNER, StringGet(&groupInfo->ownerName)) != HC_SUCCESS) {
        LOGE("Failed to add groupOwner to message!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddGroupVisibility(const GroupInfo *groupInfo, CJson *message)
{
    if (AddIntToJson(message, FIELD_GROUP_VISIBILITY, groupInfo->visibility) != HC_SUCCESS) {
        LOGE("Failed to add groupVisibility to message!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t GenerateMessage(const GroupInfo *groupInfo, char **returnGroupInfo)
{
    CJson *message = CreateJson();
    if (message == NULL) {
        LOGE("Failed to allocate message memory!");
        return HC_ERR_ALLOC_MEMORY;
    }
    int32_t result;
    if (((result = AddGroupId(groupInfo, message)) != HC_SUCCESS) ||
        ((result = AddGroupType(groupInfo, message)) != HC_SUCCESS) ||
        ((result = AddGroupName(groupInfo, message)) != HC_SUCCESS) ||
        ((result = AddGroupOwner(groupInfo, message)) != HC_SUCCESS) ||
        ((result = AddGroupVisibility(groupInfo, message)) != HC_SUCCESS)) {
        FreeJson(message);
        return result;
    }
    char *messageStr = PackJsonToString(message);
    FreeJson(message);
    if (messageStr == NULL) {
        LOGE("Failed to convert json to string!");
        return HC_ERR_JSON_FAIL;
    }
    *returnGroupInfo = messageStr;
    return HC_SUCCESS;
}

static void PostOnGroupCreated(const GroupInfo *groupInfo)
{
    if (groupInfo == NULL) {
        LOGE("The groupEntry is NULL!");
        return;
    }
    char *messageStr = NULL;
    if (GenerateMessage(groupInfo, &messageStr) != HC_SUCCESS) {
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onGroupCreated != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onGroupCreated(messageStr);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
    FreeJsonString(messageStr);
}

static void PostOnGroupDeleted(const GroupInfo *groupInfo)
{
    if (groupInfo == NULL) {
        LOGE("The groupEntry is NULL!");
        return;
    }
    char *messageStr = NULL;
    if (GenerateMessage(groupInfo, &messageStr) != HC_SUCCESS) {
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onGroupDeleted != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onGroupDeleted(messageStr);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
    FreeJsonString(messageStr);
}

static void PostOnDeviceBound(const char *peerUdid, const GroupInfo *groupInfo)
{
    if ((peerUdid == NULL) || (groupInfo == NULL)) {
        LOGE("The peerUdid or groupEntry is NULL!");
        return;
    }
    char *messageStr = NULL;
    if (GenerateMessage(groupInfo, &messageStr) != HC_SUCCESS) {
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onDeviceBound != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onDeviceBound(peerUdid, messageStr);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
    FreeJsonString(messageStr);
}

static void PostOnDeviceUnBound(const char *peerUdid, const GroupInfo *groupInfo)
{
    if ((peerUdid == NULL) || (groupInfo == NULL)) {
        LOGE("The peerUdid or groupEntry is NULL!");
        return;
    }
    char *messageStr = NULL;
    if (GenerateMessage(groupInfo, &messageStr) != HC_SUCCESS) {
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onDeviceUnBound != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onDeviceUnBound(peerUdid, messageStr);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
    FreeJsonString(messageStr);
}

static void PostOnDeviceNotTrusted(const char *peerUdid)
{
    if (peerUdid == NULL) {
        LOGE("The peerUdid is NULL!");
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onDeviceNotTrusted != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onDeviceNotTrusted(peerUdid);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
}

static void PostOnLastGroupDeleted(const char *peerUdid, int groupType)
{
    if (peerUdid == NULL) {
        LOGE("The peerUdid is NULL!");
        return;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onLastGroupDeleted != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s, [GroupType]: %d",
                entry->appId, groupType);
            entry->listener->onLastGroupDeleted(peerUdid, groupType);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
}

static void PostOnTrustedDeviceNumChanged(int curTrustedDeviceNum)
{
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (entry->listener != NULL) && (entry->listener->onTrustedDeviceNumChanged != NULL)) {
            LOGI("[Broadcaster]: Ready to broadcast the message to the listener! [AppId]: %s", entry->appId);
            entry->listener->onTrustedDeviceNumChanged(curTrustedDeviceNum);
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
}

static int32_t UpdateListenerIfExist(const char *appId, const DataChangeListener *listener)
{
    uint32_t index;
    ListenerEntry *entry = NULL;
    g_broadcastMutex->lock(g_broadcastMutex);
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if ((entry != NULL) && (strcmp(entry->appId, appId) == 0)) {
            if (memcpy_s(entry->listener, sizeof(DataChangeListener),
                listener, sizeof(DataChangeListener)) != HC_SUCCESS) {
                g_broadcastMutex->unlock(g_broadcastMutex);
                LOGE("Failed to copy listener!");
                return HC_ERR_MEMORY_COPY;
            }
            g_broadcastMutex->unlock(g_broadcastMutex);
            return HC_SUCCESS;
        }
    }
    g_broadcastMutex->unlock(g_broadcastMutex);
    return HC_ERR_LISTENER_NOT_EXIST;
}

static int32_t AddListenerIfNotExist(const char *appId, const DataChangeListener *listener)
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
    DataChangeListener *copyListener = (DataChangeListener *)HcMalloc(sizeof(DataChangeListener), 0);
    if (copyListener == NULL) {
        LOGE("Failed to allocate saveCallback memory!");
        HcFree(copyAppId);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(copyListener, sizeof(DataChangeListener),
        listener, sizeof(DataChangeListener)) != HC_SUCCESS) {
        LOGE("Failed to copy listener!");
        HcFree(copyAppId);
        HcFree(copyListener);
        return HC_ERR_MEMORY_COPY;
    }
    ListenerEntry entry;
    entry.appId = copyAppId;
    entry.listener = copyListener;
    g_broadcastMutex->lock(g_broadcastMutex);
    g_listenerEntryVec.pushBack(&g_listenerEntryVec, &entry);
    g_broadcastMutex->unlock(g_broadcastMutex);
    LOGI("[End]: Service register listener successfully!");
    return HC_SUCCESS;
}

static Broadcaster g_broadcaster = {
    .postOnGroupCreated = PostOnGroupCreated,
    .postOnGroupDeleted = PostOnGroupDeleted,
    .postOnDeviceBound = PostOnDeviceBound,
    .postOnDeviceUnBound = PostOnDeviceUnBound,
    .postOnDeviceNotTrusted = PostOnDeviceNotTrusted,
    .postOnLastGroupDeleted = PostOnLastGroupDeleted,
    .postOnTrustedDeviceNumChanged = PostOnTrustedDeviceNumChanged
};

bool IsBroadcastSupported()
{
    return true;
}

int32_t InitBroadcastManager()
{
    if (g_broadcastMutex == NULL) {
        g_broadcastMutex = (HcMutex *)HcMalloc(sizeof(HcMutex), 0);
        if (g_broadcastMutex == NULL) {
            LOGE("Failed to allocate broadcast mutex memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (InitHcMutex(g_broadcastMutex) != HC_SUCCESS) {
            LOGE("Init mutex failed");
            HcFree(g_broadcastMutex);
            g_broadcastMutex = NULL;
            return HC_ERROR;
        }
    }
    g_listenerEntryVec = CREATE_HC_VECTOR(ListenerEntryVec)
    LOGI("[Broadcaster]: Init broadcast manager module successfully!");
    return HC_SUCCESS;
}

void DestroyBroadcastManager()
{
    uint32_t index;
    ListenerEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if (entry != NULL) {
            HcFree(entry->appId);
            HcFree(entry->listener);
        }
    }
    DESTROY_HC_VECTOR(ListenerEntryVec, &g_listenerEntryVec)
    if (g_broadcastMutex != NULL) {
        DestroyHcMutex(g_broadcastMutex);
        HcFree(g_broadcastMutex);
        g_broadcastMutex = NULL;
    }
}

Broadcaster *GetBroadcaster()
{
    return &g_broadcaster;
}

int32_t AddListener(const char *appId, const DataChangeListener *listener)
{
    if ((appId == NULL) || (listener == NULL)) {
        LOGE("The input appId or listener is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    if (UpdateListenerIfExist(appId, listener) == HC_SUCCESS) {
        LOGI("The listener associated with the appId already exists, so we choose to update the listener!");
        return HC_SUCCESS;
    }
    return AddListenerIfNotExist(appId, listener);
}

int32_t RemoveListener(const char *appId)
{
    if (appId == NULL) {
        LOGE("The input appId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    uint32_t index;
    ListenerEntry *entry = NULL;
    FOR_EACH_HC_VECTOR(g_listenerEntryVec, index, entry) {
        if (strcmp(entry->appId, appId) == 0) {
            HcFree(entry->appId);
            HcFree(entry->listener);
            ListenerEntry tempEntry;
            HC_VECTOR_POPELEMENT(&g_listenerEntryVec, &tempEntry, index);
            LOGI("[End]: Service deregister listener successfully!");
            return HC_SUCCESS;
        }
    }
    LOGI("[End]: Although the listener does not exist, we still believe it is correct to deregister the listener!");
    return HC_SUCCESS;
}
