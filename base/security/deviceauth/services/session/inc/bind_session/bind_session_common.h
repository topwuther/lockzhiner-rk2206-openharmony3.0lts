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

#ifndef BIND_SESSION_COMMON_H
#define BIND_SESSION_COMMON_H

#include "base_session.h"
#include "common_defs.h"
#include "hc_types.h"

typedef struct {
    Session base;
    void (*onChannelOpened)(Session *, int64_t channelId, int64_t requestId);
    void (*onConfirmationReceived)(Session *, CJson *returnData);
    int curTaskId;
    int operationCode;
    ChannelType channelType;
    bool isWaiting;
    int64_t requestId;
    int64_t channelId;
    CJson *params;
} BindSession;

void DestroyBindSession(Session *session);

/* session interfaces */
int32_t SendBindSessionData(const BindSession *session, const CJson *out);
void InformPeerProcessErrorIfNeed(bool isModuleError, int32_t errorCode, const BindSession *session);
void InformPeerModuleErrorIfNeed(CJson *out, const BindSession *session);
void InitBindSession(int bindType, int operationCode, int64_t requestId, const DeviceAuthCallback *callback,
    BindSession *session);
int32_t CreateAndProcessModule(BindSession *session, const CJson *in, CJson *out);
int32_t ProcessModule(const BindSession *session, const CJson *in, CJson *out, int *status);
int32_t AddInfoToSendData(bool isNeedCompatibleInfo, const BindSession *session, CJson *data);
int32_t GenerateBasicModuleParams(bool isClient, BindSession *session, CJson *moduleParams);
int32_t GenerateBindParams(int isClient, const CJson *jsonParams, BindSession *session);
bool NeedCreateGroup(int isClient, int operationCode);
bool NeedForceDelete(const BindSession *session);
int32_t ForceUnbindDevice(const BindSession *session);
#endif
