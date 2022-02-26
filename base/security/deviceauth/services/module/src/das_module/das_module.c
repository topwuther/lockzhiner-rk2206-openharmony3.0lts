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

#include "das_module.h"
#include "common_defs.h"
#include "hc_log.h"
#include "hc_types.h"
#include "hc_vector.h"
#include "task_main.h"

DECLARE_HC_VECTOR(TaskInModuleVec, void *)
IMPLEMENT_HC_VECTOR(TaskInModuleVec, void *, 1)

TaskInModuleVec g_taskInModuleVec;
DasAuthModule g_dasModule = {0};

static int32_t RegisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    return RegisterLocalIdentityInTask(pkgName, serviceType, authId, userType);
}

static int32_t UnregisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    return UnregisterLocalIdentityInTask(pkgName, serviceType, authId, userType);
}

static int32_t DeletePeerAuthInfo(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType)
{
    return DeletePeerAuthInfoInTask(pkgName, serviceType, authId, userType);
}

static int CreateDasTask(int *taskId, const CJson *in, CJson *out)
{
    if (taskId == NULL || in == NULL || out == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    Task *task = CreateTaskT(taskId, in, out);
    if (task == NULL) {
        LOGE("CreateTaskT failed");
        return HC_ERR_ALLOC_MEMORY;
    }

    g_taskInModuleVec.pushBackT(&g_taskInModuleVec, (void *)task);
    return HC_SUCCESS;
}

static void DestroyDasModule(AuthModuleBase *module)
{
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_taskInModuleVec, index, ptr) {
        if ((ptr != NULL) && (*ptr != NULL)) {
            ((Task *)(*ptr))->destroyTask((Task *)(*ptr));
        }
    }
    DESTROY_HC_VECTOR(TaskInModuleVec, &g_taskInModuleVec)
    DestroyDasProtocolType();
    if (module != NULL) {
        (void)memset_s(module, sizeof(AuthModuleBase), 0, sizeof(AuthModuleBase));
    }
}

static int ProcessDasTask(int taskId, const CJson* in, CJson* out, int *status)
{
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_taskInModuleVec, index, ptr) {
        if ((ptr != NULL) && (*ptr != NULL)) {
            Task *temp = (Task *)*ptr;
            if (taskId == temp->taskId) {
                return temp->processTask(temp, in, out, status);
            }
        }
    }
    return HC_ERR_TASK_IS_NULL;
}

static void DestroyDasTask(int taskId)
{
    uint32_t index;
    void **ptr = NULL;
    FOR_EACH_HC_VECTOR(g_taskInModuleVec, index, ptr) {
        if ((ptr != NULL) && (*ptr != NULL)) {
            Task *temp = (Task *)(*ptr);
            if (taskId == temp->taskId) {
                temp->destroyTask(temp);
                void *tempPtr = NULL;
                HC_VECTOR_POPELEMENT(&g_taskInModuleVec, &tempPtr, index);
                return;
            }
        }
    }
}

bool IsDasSupported()
{
    return true;
}

AuthModuleBase *CreateDasModule()
{
    g_dasModule.moduleBase.moduleType = DAS_MODULE;
    g_dasModule.moduleBase.createTask = CreateDasTask;
    g_dasModule.moduleBase.processTask = ProcessDasTask;
    g_dasModule.moduleBase.destroyTask = DestroyDasTask;
    g_dasModule.moduleBase.destroyModule = DestroyDasModule;
    g_dasModule.registerLocalIdentity = RegisterLocalIdentity;
    g_dasModule.unregisterLocalIdentity = UnregisterLocalIdentity;
    g_dasModule.deletePeerAuthInfo = DeletePeerAuthInfo;
    g_taskInModuleVec = CREATE_HC_VECTOR(TaskInModuleVec)
    if (InitDasProtocolType() != HC_SUCCESS) {
        LOGE("InitDasProtocolType failed.");
        DestroyDasModule((AuthModuleBase *)&g_dasModule);
        return NULL;
    }
    return (AuthModuleBase *)&g_dasModule;
}
