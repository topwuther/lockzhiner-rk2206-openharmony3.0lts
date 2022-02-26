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

#ifndef TCIS_DEVICE_AUTH_TCIS_MODULE_DEFINES_H
#define TCIS_DEVICE_AUTH_TCIS_MODULE_DEFINES_H

#include "alg_defs.h"
#include "auth_common.h"
#include "common_util.h"
#include "pake_protocol_common.h"

typedef enum {
    TASK_TYPE_DEFAULT = 0,
    TASK_TYPE_ASY_SERVER_BIND = 1,
    TASK_TYPE_ASY_CLIENT_AUTH = 2,
    TASK_TYPE_ASY_SERVER_AUTH = 3,
    TASK_TYPE_SYM_SERVER_AUTH,
    TASK_TYPE_SYM_CLIENT_AUTH,
} TcisTaskType;

typedef struct {
    int authVersion;
    bool isClient;
    int authForm;
    uint8_t seed[SEED_SIZE];
    uint8_t userIdSelf[USER_ID_SIZE];
    uint8_t userIdPeer[USER_ID_SIZE];
    uint8_t devIdSelf[DEV_ID_MAX_SIZE];
    uint8_t devIdPeer[DEV_ID_MAX_SIZE];
    uint8_t saltSelf[RANDOM_SIZE];
    uint8_t saltPeer[RANDOM_SIZE];
    uint8_t token[TOKEN_SIZE];
    uint8_t *sessionKey;
    int keyLength;
    KeyAlg authKeyAlgEncode;
    uint8_t psk[PSK_SIZE];
    AlgLoader *loader;
} SymAuthParams;

typedef struct BindParams {
    PakeBaseParams baseParams;
    uint8_t nonce[16];
    uint8_t dataEncKey[32];
} BindParams;
#endif