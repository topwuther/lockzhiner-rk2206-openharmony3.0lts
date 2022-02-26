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

#ifndef DEV_AUTH_MODULE_MANAGER_H
#define DEV_AUTH_MODULE_MANAGER_H

#include "common_util.h"
#include "json_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t InitModules(void);
void DestroyModules(void);

int32_t CreateTask(int *taskId, const CJson *in, CJson *out, int moduleType);
int32_t ProcessTask(int taskId, const CJson *in, CJson *out, int *status, int moduleType);
void DestroyTask(int taskId, int moduleType);
int32_t CheckMsgRepeatability(const CJson *in, int moduleType);

// for DAS
int32_t RegisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType);
int32_t UnregisterLocalIdentity(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType);
int32_t DeletePeerAuthInfo(const char *pkgName, const char *serviceType, Uint8Buff *authId, int userType,
    int moduleType);

// for TCIS
int32_t SetToken(CJson *in, CJson *out, int moduleType);
int32_t DeleteToken(int moduleType);
int32_t GetRegisterProof(CJson *out, int moduleType);

#ifdef __cplusplus
}
#endif
#endif
