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

#include "task_main.h"
#include "alg_loader.h"
#include "base_sub_task.h"
#include "das_asy_token_manager.h"
#include "das_common.h"
#include "hc_log.h"
#include "iso_task_main.h"
#include "new_pake_protocol_dl.h"
#include "new_pake_protocol_ec.h"
#include "new_pake_task_main.h"
#include "pake_protocol_dl.h"
#include "pake_protocol_ec.h"
#include "pake_task_main.h"
#include "protocol_common.h"

typedef struct DasProtocolTypeT {
    ProtocolType type;
    uint32_t algInProtocol;
    const TokenManager *tokenManagerInstance;
    SubTaskBase *(*createSubTask)(const CJson *, CJson *);
} DasProtocolType;
IMPLEMENT_HC_VECTOR(SubTaskVec, void *, 1)
DECLARE_HC_VECTOR(DasProtocolTypeVec, void *)
IMPLEMENT_HC_VECTOR(DasProtocolTypeVec, void *, 1)

DasProtocolTypeVec g_protocolTypeVec;
ProtocolType g_subTaskTypeToAlgType[] = {
    ISO, // TASK_TYPE_ISO_PROTOCOL = 0,
    PAKE, // TASK_TYPE_PAKE_PROTOCOL = 1,
    NEW_PAKE // TASK_TYPE_NEW_PAKE_PROTOCOL = 2,
};

static void GetMinVersion(VersionStruct *version)
{
    version->first = 1;
    version->second = 0;
    version->third = 0;
}

static void GetMaxVersion(VersionStruct *version)
{
    version->first = VERSION_FIRST_BIT;
    version->second = 0;
    version->third = 0;

    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && *ptr != NULL) {
            DasProtocolType *temp = (DasProtocolType *)(*ptr);
            version->third = (version->third) | temp->algInProtocol;
        }
    }
}

static void InitVersionInfo(VersionInfo *versionInfo)
{
    GetMinVersion(&(versionInfo->minVersion));
    GetMaxVersion(&(versionInfo->curVersion));
    versionInfo->versionStatus = INITIAL;
}

static int AddVersionToOut(const VersionInfo *versionInfo, CJson *out)
{
    CJson *payload = GetObjFromJson(out, FIELD_PAYLOAD);
    if (payload == NULL) {
        LOGD("not find payload");
        return HC_SUCCESS;
    }
    return AddVersionToJson(payload, &(versionInfo->minVersion), &(versionInfo->curVersion));
}

static int CombineJson(CJson *desObj, const CJson *srcObj)
{
    CHECK_PTR_RETURN_ERROR_CODE(desObj, "desObj");
    CHECK_PTR_RETURN_ERROR_CODE(srcObj, "srcObj");
    int res;
    int len = GetItemNum(srcObj);
    for (int i = 0; i < len; i++) {
        CJson *item = GetItemFromArray(srcObj, i);
        const char *key = GetItemKey(item);
        CJson *payload = GetObjFromJson(desObj, FIELD_PAYLOAD);
        if (strcmp(key, FIELD_PAYLOAD) == 0 && payload != NULL) {
            res = CombineJson(payload, item);
            if (res != HC_SUCCESS) {
                LOGE("combine payload failed");
                return res;
            }
        } else {
            res = AddObjToJson(desObj, key, item);
            if (res != HC_SUCCESS) {
                LOGE("AddObjToJson failed");
                return res;
            }
        }
    }
    return res;
}

static int AddErrToOut(CJson *out, const CJson *err)
{
    CJson *sendToPeer = GetObjFromJson(err, FIELD_SEND_TO_PEER);
    if (sendToPeer != NULL) {
        if (AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer) != HC_SUCCESS) {
            LOGE("Add sendToPeer to json failed.");
            return HC_ERR_JSON_ADD;
        }
    }
    CJson *sendToSelf = GetObjFromJson(err, FIELD_SEND_TO_PEER);
    if (sendToSelf != NULL) {
        if (AddObjToJson(out, FIELD_SEND_TO_SELF, sendToSelf) != HC_SUCCESS) {
            LOGE("Add sendToSelf to json failed.");
            return HC_ERR_JSON_ADD;
        }
    }
    return HC_SUCCESS;
}

static void DestroyTask(Task *task)
{
    if (task == NULL) {
        return;
    }
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(task->vec, index, ptr) {
        if (ptr != NULL && *ptr != NULL) {
            ((SubTaskBase *)(*ptr))->destroyTask((SubTaskBase *)(*ptr));
        }
    }
    DESTROY_HC_VECTOR(SubTaskVec, &(task->vec))
    HcFree(task);
}

static int ProcessMultiTask(Task *task, const CJson *in, CJson *out, int *status)
{
    int res;
    uint32_t index;
    void **ptr = NULL;
    CJson *tmpOut = NULL;
    CJson *combinedSendToPeer = CreateJson();
    FOR_EACH_HC_VECTOR(task->vec, index, ptr) {
        if ((ptr == NULL) || (*ptr == NULL)) {
            LOGD("Null ptr in subTask vector.");
            continue;
        }
        tmpOut = CreateJson();
        res = ((SubTaskBase *)(*ptr))->process((*ptr), in, tmpOut, status);
        if (res != HC_SUCCESS) {
            LOGE("process SubTaskBase failed, index: %u", index);
            res = AddErrToOut(out, tmpOut);
            if (res != HC_SUCCESS) {
                LOGE("AddErrToOut failed");
                goto err;
            }
            goto err;
        }

        CJson *tmpSendToPeer = GetObjFromJson(tmpOut, FIELD_SEND_TO_PEER);
        res = CombineJson(combinedSendToPeer, tmpSendToPeer);
        if (res != HC_SUCCESS) {
            LOGE("CombineJson failed");
            goto err;
        }
        FreeJson(tmpOut);
        tmpOut = NULL;
    }
    res = AddObjToJson(out, FIELD_SEND_TO_PEER, combinedSendToPeer);
    if (res != HC_SUCCESS) {
        LOGE("AddObjToJson failed");
        goto err;
    }
err:
    FreeJson(combinedSendToPeer);
    FreeJson(tmpOut);
    return res;
}

static int NegotiateAndProcessTask(Task *task, const CJson *in, CJson *out, int *status)
{
    VersionStruct curVersionPeer = { 0, 0, 0 };
    VersionStruct minVersionPeer = { 0, 0, 0 };
    int res = GetVersionFromJson(in, &minVersionPeer, &curVersionPeer);
    if (res != HC_SUCCESS) {
        LOGE("Get peer version info failed");
        return res;
    }
    res = NegotiateVersion(&minVersionPeer, &curVersionPeer, &(task->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("NegotiateVersion failed");
        return res;
    }
    if (!IsVersionEqual(&(task->versionInfo.curVersion), &curVersionPeer)) {
        LOGE("Negotiated version not equal");
        return HC_ERR_UNSUPPORTED_VERSION;
    }
    ProtocolType protocolType = GetPrototolType(&(task->versionInfo.curVersion), task->versionInfo.opCode);
    LOGI("Client select protocolType:%d", protocolType);

    SubTaskBase *subTask = NULL;
    uint32_t index = 0;
    void **ptr = task->vec.getp(&(task->vec), 0);
    while (index < task->vec.size(&(task->vec)) && ptr != NULL) {
        SubTaskBase *temp = (SubTaskBase *)(*ptr);
        if (g_subTaskTypeToAlgType[temp->getTaskType(temp)] == protocolType) {
            subTask = temp;
            index++;
        } else {
            temp->destroyTask(temp);
            task->vec.eraseElement(&(task->vec), ptr, index);
        }
        ptr = task->vec.getp(&(task->vec), index);
    }
    if (subTask == NULL) {
        LOGE("Can't find subTask.");
        return HC_ERR_NOT_SUPPORT;
    }
    subTask->curVersion = task->versionInfo.curVersion;
    res = subTask->process(subTask, in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("Process subTask failed");
        return res;
    }
    return res;
}

static bool IsPeerErrMessage(const CJson *in, CJson *out, int32_t *res)
{
    int message = 0;
    if (GetIntFromJson(in, FIELD_MESSAGE, &message) != HC_SUCCESS) {
        LOGD("Get message code failed.");
        return false;
    }
    if (message != ERR_MESSAGE) {
        return false;
    }

    LOGE("Receive error message from peer.");
    if (GetIntFromJson(in, FIELD_ERROR_CODE, res) != HC_SUCCESS) {
        LOGE("Get error code failed.");
    }
    SendErrMsgToSelf(in, out, *res); /* when receive peer ERR_MESSAGE, only need to inform self */
    return true;
}

static int ProcessTask(Task *task, const CJson *in, CJson *out, int *status)
{
    if (task == NULL || in == NULL || out == NULL || status == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (task->vec.size(&(task->vec)) == 0) {
        LOGE("not has subTask");
        return HC_ERROR;
    }
    int res = HC_ERROR;
    if (IsPeerErrMessage(in, out, &res)) {
        LOGE("Peer message is error message.");
        return res;
    }

    if (task->versionInfo.versionStatus == INITIAL) {
        res = ProcessMultiTask(task, in, out, status);
        if (res != HC_SUCCESS) {
            LOGE("ProcessMultiTask failed");
            return res;
        }
        task->versionInfo.versionStatus = VERSION_CONFIRM;
    } else if (task->versionInfo.versionStatus == VERSION_CONFIRM) {
        res = NegotiateAndProcessTask(task, in, out, status);
        if (res != HC_SUCCESS) {
            LOGE("NegotiateAndProcessTask failed.");
            return res;
        }
        task->versionInfo.versionStatus = VERSION_DECIDED;
    } else {
        SubTaskBase *subTask = HC_VECTOR_GET(&(task->vec), 0);
        res = subTask->process(subTask, in, out, status);
        if (res != HC_SUCCESS) {
            LOGE("Process subTask failed");
            return res;
        }
    }

    res = AddVersionToOut(&(task->versionInfo), out);
    if (res != HC_SUCCESS) {
        LOGE("AddVersionToOut failed");
    }
    return res;
}

static int CreateMultiSubTask(Task *task, const CJson *in, CJson *out)
{
    InitVersionInfo(&(task->versionInfo));
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && (*ptr) != NULL) {
            DasProtocolType *temp = (DasProtocolType *)(*ptr);
            SubTaskBase *subTask = temp->createSubTask(in, out);
            if (subTask == NULL) {
                LOGE("Create subTask failed");
                return HC_ERR_ALLOC_MEMORY;
            }
            subTask->curVersion = task->versionInfo.curVersion;
            task->vec.pushBackT(&(task->vec), (void *)subTask);
        }
    }
    return HC_SUCCESS;
}

static int CreateSingleSubTask(Task *task, const CJson *in, CJson *out)
{
    VersionStruct curVersionPeer = { 0, 0, 0 };
    VersionStruct minVersionPeer = { 0, 0, 0 };
    int res = GetVersionFromJson(in, &minVersionPeer, &curVersionPeer);
    if (res != HC_SUCCESS) {
        LOGE("Get peer version info failed");
        return res;
    }
    InitVersionInfo(&(task->versionInfo));
    res = NegotiateVersion(&minVersionPeer, &curVersionPeer, &(task->versionInfo.curVersion));
    if (res != HC_SUCCESS) {
        LOGE("NegotiateVersion failed");
        return res;
    }
    task->versionInfo.versionStatus = VERSION_DECIDED;

    ProtocolType protocolType = GetPrototolType(&(task->versionInfo.curVersion), task->versionInfo.opCode);
    LOGI("Server create protocolType:%d", protocolType);

    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if ((ptr == NULL) || ((*ptr) == NULL)) {
            continue;
        }
        DasProtocolType *temp = (DasProtocolType *)(*ptr);
        if (temp->type == protocolType) {
            SubTaskBase *subTask = temp->createSubTask(in, out);
            if (subTask == NULL) {
                LOGE("Create subTask failed");
                return HC_ERR_ALLOC_MEMORY;
            }
            subTask->curVersion = task->versionInfo.curVersion;
            task->vec.pushBackT(&(task->vec), (void *)subTask);
            return HC_SUCCESS;
        }
    }

    LOGE("Can't find protocolType.");
    return HC_ERR_NOT_SUPPORT;
}

Task *CreateTaskT(int *taskId, const CJson *in, CJson *out)
{
    if (taskId == NULL || in == NULL || out == NULL) {
        return NULL;
    }
    Task *task = (Task *)HcMalloc(sizeof(Task), 0);
    if (task == NULL) {
        return NULL;
    }
    task->vec = CREATE_HC_VECTOR(SubTaskVec)
    task->destroyTask = DestroyTask;
    task->processTask = ProcessTask;
    const AlgLoader *loader = GetLoaderInstance();
    if (loader == NULL) {
        DestroyTask(task);
        return NULL;
    }
    Uint8Buff taskIdBuf = { (uint8_t *)taskId, sizeof(int) };
    int res = loader->generateRandom(&taskIdBuf);
    if (res != 0) {
        DestroyTask(task);
        return NULL;
    }
    task->taskId = *taskId;
    if (GetIntFromJson(in, FIELD_OPERATION_CODE, &(task->versionInfo.opCode)) != HC_SUCCESS) {
        LOGE("Get opcode failed.");
        DestroyTask(task);
        return NULL;
    }
    bool isClient = true;
    if (GetBoolFromJson(in, FIELD_IS_CLIENT, &isClient) != HC_SUCCESS) {
        LOGE("Get isClient failed.");
        DestroyTask(task);
        return NULL;
    }
    if (isClient) {
        res = CreateMultiSubTask(task, in, out);
    } else {
        res = CreateSingleSubTask(task, in, out);
    }
    if (res != 0) {
        DestroyTask(task);
        return NULL;
    }
    return task;
}

int32_t RegisterLocalIdentityInTask(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    int32_t res = HC_SUCCESS;
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && (*ptr) != NULL) {
            DasProtocolType *temp = (DasProtocolType *)(*ptr);
            if ((temp->tokenManagerInstance == NULL) || (temp->tokenManagerInstance->registerLocalIdentity == NULL)) {
                LOGD("Protocol type: %d, unsupported method!", temp->type);
                continue;
            }
            res = temp->tokenManagerInstance->registerLocalIdentity(pkgName, serviceType, authId, userType);
            if (res != HC_SUCCESS) {
                LOGE("Protocol type: %d, registerLocalIdentity failed, res: %d!", temp->type, res);
                return HC_ERR_GENERATE_KEY_FAILED;
            }
        }
    }
    return res;
}

int32_t UnregisterLocalIdentityInTask(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    int32_t res = HC_SUCCESS;
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && (*ptr) != NULL) {
            DasProtocolType *temp = (DasProtocolType *)(*ptr);
            if ((temp->tokenManagerInstance == NULL) || (temp->tokenManagerInstance->unregisterLocalIdentity == NULL)) {
                LOGD("Protocol type: %d, unsupported method!", temp->type);
                continue;
            }
            res = temp->tokenManagerInstance->unregisterLocalIdentity(pkgName, serviceType, authId, userType);
            if (res != HC_SUCCESS) {
                LOGE("Protocol type: %d, unregisterLocalIdentity failed, res: %d!", temp->type, res);
                return res;
            }
        }
    }
    return res;
}

int32_t DeletePeerAuthInfoInTask(const char *pkgName, const char *serviceType, Uint8Buff *authIdPeer, int userTypePeer)
{
    int32_t res = HC_SUCCESS;
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && (*ptr) != NULL) {
            DasProtocolType *temp = (DasProtocolType *)(*ptr);
            if ((temp->tokenManagerInstance == NULL) || (temp->tokenManagerInstance->deletePeerAuthInfo == NULL)) {
                LOGD("Protocol type: %d, unsupported method!", temp->type);
                continue;
            }
            res = temp->tokenManagerInstance->deletePeerAuthInfo(pkgName, serviceType, authIdPeer, userTypePeer);
            if (res != HC_SUCCESS) {
                LOGE("Protocol type: %d, deletePeerAuthInfo failed, res: %d!", temp->type, res);
                return res;
            }
        }
    }
    return res;
}

int32_t InitDasProtocolType(void)
{
    g_protocolTypeVec = CREATE_HC_VECTOR(DasProtocolTypeVec)
    DasProtocolType *protocol = NULL;
    if (IsIsoSupported()) {
        protocol = (DasProtocolType *)HcMalloc(sizeof(DasProtocolType), 0);
        if (protocol == NULL) {
            return HC_ERR_ALLOC_MEMORY;
        }
        protocol->type = ISO;
        protocol->algInProtocol = ISO_ALG;
        protocol->createSubTask = CreateIsoSubTask;
        protocol->tokenManagerInstance = GetSymTokenManagerInstance();
        g_protocolTypeVec.pushBackT(&g_protocolTypeVec, (void *)protocol);
    }

    if (IsSupportPake()) {
        protocol = (DasProtocolType *)HcMalloc(sizeof(DasProtocolType), 0);
        if (protocol == NULL) {
            return HC_ERR_ALLOC_MEMORY;
        }
        protocol->type = PAKE;
        protocol->algInProtocol = GetPakeEcAlg() | GetPakeDlAlg() | PSK_SPEKE;
        protocol->createSubTask = CreatePakeSubTask;
        protocol->tokenManagerInstance = GetAsyTokenManagerInstance();
        g_protocolTypeVec.pushBackT(&g_protocolTypeVec, (void *)protocol);
    }

    if (IsSupportNewPake()) {
        protocol = (DasProtocolType *)HcMalloc(sizeof(DasProtocolType), 0);
        if (protocol == NULL) {
            return HC_ERR_ALLOC_MEMORY;
        }
        protocol->type = NEW_PAKE;
        protocol->algInProtocol = GetPakeNewEcAlg() | GetPakeNewDlAlg() | PSK_SPEKE;
        protocol->createSubTask = CreateNewPakeSubTask;
        protocol->tokenManagerInstance = GetAsyTokenManagerInstance();
        g_protocolTypeVec.pushBackT(&g_protocolTypeVec, (void *)protocol);
    }

    return HC_SUCCESS;
}

void DestroyDasProtocolType(void)
{
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_protocolTypeVec, index, ptr) {
        if (ptr != NULL && *ptr != NULL) {
            HcFree((DasProtocolType *)(*ptr));
            *ptr = NULL;
        }
    }
    DESTROY_HC_VECTOR(DasProtocolTypeVec, &g_protocolTypeVec)
}
