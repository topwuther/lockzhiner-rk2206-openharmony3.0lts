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

#ifndef BIND_SESSION_COMMON_LITE_H
#define BIND_SESSION_COMMON_LITE_H

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
} LiteBindSession;

void InitLiteBindSession(int bindType, int operationCode, int64_t requestId, LiteBindSession *session,
    const DeviceAuthCallback *callback);
void DestroyLiteBindSession(Session *session);
void LiteInformPeerErrorIfNeed(bool isNeedInform, int32_t errorCode, LiteBindSession *session);
int32_t LiteSaveReceivedData(LiteBindSession *session, const CJson *jsonParams);
int32_t LitePrepareAndSendData(LiteBindSession *session, bool *isNeedInform);
#endif
