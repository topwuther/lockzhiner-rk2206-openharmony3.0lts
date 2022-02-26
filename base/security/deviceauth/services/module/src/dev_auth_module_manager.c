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

#include "dev_auth_module_manager.h"
#include "das_module.h"
#include "hc_log.h"
#include "hc_vector.h"
#include "tcis_module.h"
#include "version_util.h"

#define CLIENT_FIRST_MESSAGE 0x0001

DECLARE_HC_VECTOR(AuthModuleVec, void *)
IMPLEMENT_HC_VECTOR(AuthModuleVec, void *, 1)

static AuthModuleVec g_authModuleVec;
static VersionStruct g_version;

static AuthModuleBase *GetModule(int moduleType)
{
    uint32_t index;
    void **module = NULL;
    FOR_EACH_HC_VECTOR(g_authModuleVec, index, module) {
        if ((module != NULL) && (*module != NULL)) {
            if (moduleType == ((AuthModuleBase *)(*module))->moduleType) {
                return *module;
            }
        }
    }
    return NULL;
}

static bool IsDasMsgRepeated(const CJson *in)
{
    uint32_t message = 0;
    int res = GetIntFromJson(in, FIELD_MESSAGE, (int *)&message);
    if (res != HC_SUCCESS) {
        return false;
    }
    if ((message & 0xF00F) == CLIENT_FIRST_MESSAGE) {
        return false;
    }

    LOGI("The message is repeated, ignore it message :%u.", message);
    return true;
}

int32_t CheckMsgRepeatability(const CJson *in, int moduleType)
{
    if (in == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    switch (moduleType) {
        case DAS_MODULE:
            if (IsDasMsgRepeated(in)) {
                return HC_ERR_IGNORE_MSG;
            }
            return HC_SUCCESS;
        case TCIS_MODULE:
            return HC_ERR_UNSUPPORTED_METHOD;
        default:
            LOGE("Invalid module type.");
            return HC_ERR_MODULE_NOT_FOUNT;
    }
}

int32_t RegisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType)
{
    if (pkgName == NULL || serviceType == NULL || authId == NULL || authId->val == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    if (moduleType != DAS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    DasAuthModule *dasModule = (DasAuthModule *)module;
    int32_t res = dasModule->registerLocalIdentity(pkgName, serviceType, authId, userType);
    if (res != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task!");
        return res;
    }
    return HC_SUCCESS;
}

int32_t UnregisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType)
{
    if (pkgName == NULL || serviceType == NULL || authId == NULL || authId->val == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    if (moduleType != DAS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    DasAuthModule *dasModule = (DasAuthModule *)module;
    int32_t res = dasModule->unregisterLocalIdentity(pkgName, serviceType, authId, userType);
    if (res != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task!");
        return res;
    }
    return HC_SUCCESS;
}

int32_t DeletePeerAuthInfo(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType)
{
    if (pkgName == NULL || serviceType == NULL || authId == NULL || authId->val == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    if (moduleType != DAS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    DasAuthModule *dasModule = (DasAuthModule *)module;
    int32_t res = dasModule->deletePeerAuthInfo(pkgName, serviceType, authId, userType);
    if (res != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task!");
        return res;
    }
    return HC_SUCCESS;
}

int32_t SetToken(CJson *in, CJson *out, int moduleType)
{
    if (in == NULL || out == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    if (moduleType != TCIS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }

    TcisAuthModule *realModule = (TcisAuthModule *)module;
    if (realModule->tokenManager == NULL) {
        LOGE("Tcis tokenManager is null");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    realModule->tokenManager->setToken(in, out);

    return HC_SUCCESS;
}

int32_t DeleteToken(int moduleType)
{
    if (moduleType != TCIS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }

    TcisAuthModule *realModule = (TcisAuthModule *)module;
    if (realModule->tokenManager == NULL) {
        LOGE("Tcis tokenManager is null");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    realModule->tokenManager->deleteToken();

    return HC_SUCCESS;
}

int32_t GetRegisterProof(CJson *out, int moduleType)
{
    if (out == NULL) {
        LOGE("Params is null.");
        return HC_ERR_NULL_PTR;
    }
    if (moduleType != TCIS_MODULE) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }

    TcisAuthModule *realModule = (TcisAuthModule *)module;
    if (realModule->tokenManager == NULL) {
        LOGE("Tcis tokenManager is null");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    realModule->tokenManager->getRegisterProof(out);

    return HC_SUCCESS;
}

int32_t ProcessTask(int taskId, const CJson *in, CJson *out, int *status, int moduleType)
{
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    if (module->processTask == NULL) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    int32_t res = module->processTask(taskId, in, out, status);
    if (res != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task!");
        return res;
    }
    res = AddSingleVersionToJson(out, &g_version);
    if (res != HC_SUCCESS) {
        LOGE("AddSingleVersionToJson failed, res:%d", res);
    } else {
        LOGI("Process task success.");
    }
    return res;
}

int32_t CreateTask(int *taskId, const CJson *in, CJson *out, int moduleType)
{
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        LOGE("Failed to get module!");
        return HC_ERR_MODULE_NOT_FOUNT;
    }
    if (module->createTask == NULL) {
        LOGE("Unsupported method.");
        return HC_ERR_UNSUPPORTED_METHOD;
    }
    int32_t res = module->createTask(taskId, in, out);
    if (res != HC_SUCCESS) {
        LOGE("An error occurs when the module create task!");
        return res;
    }
    LOGI("Create task success.");
    return HC_SUCCESS;
}

void DestroyTask(int taskId, int moduleType)
{
    AuthModuleBase *module = GetModule(moduleType);
    if (module == NULL) {
        return;
    }
    if (module->destroyTask == NULL) {
        LOGE("Unsupported method.");
        return;
    }
    module->destroyTask(taskId);
}

static AuthModuleBase *CreateDasModuleStatic(void)
{
    return CreateDasModule();
}

static uint32_t InitDasModule(void)
{
    AuthModuleBase *das = CreateDasModuleStatic();
    if (das == NULL) {
        LOGE("CreateDasModuleStatic failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    g_authModuleVec.pushBackT(&g_authModuleVec, (void *)das);
    return HC_SUCCESS;
}

static uint32_t InitTcisModule(void)
{
    AuthModuleBase *tcis = CreateTcisModule();
    if (tcis == NULL) {
        LOGE("Create tcis module failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    g_authModuleVec.pushBackT(&g_authModuleVec, (void *)tcis);
    return HC_SUCCESS;
}

int32_t InitModules(void)
{
    g_authModuleVec = CREATE_HC_VECTOR(AuthModuleVec)
    InitGroupAndModuleVersion(&g_version);
    int res = HC_SUCCESS;
    if (IsDasSupported()) {
        res = InitDasModule();
        if (res != HC_SUCCESS) {
            LOGE("InitDasModule failed.");
            DESTROY_HC_VECTOR(AuthModuleVec, &g_authModuleVec)
        }
        g_version.third |= DAS_MODULE;
    }
    if (IsTcisSupported()) {
        res = InitTcisModule();
        if (res != HC_SUCCESS) {
            LOGE("InitTcisModule failed.");
            DESTROY_HC_VECTOR(AuthModuleVec, &g_authModuleVec)
        }
        g_version.third |= TCIS_MODULE;
    }
    return res;
}

void DestroyModules(void)
{
    uint32_t index;
    void **module = NULL;
    FOR_EACH_HC_VECTOR(g_authModuleVec, index, module) {
        if ((module != NULL) && (*module != NULL)) {
            ((AuthModuleBase *)(*module))->destroyModule((AuthModuleBase *)*module);
        }
    }
    DESTROY_HC_VECTOR(AuthModuleVec, &g_authModuleVec)
    (void)memset_s(&g_version, sizeof(VersionStruct), 0, sizeof(VersionStruct));
}
