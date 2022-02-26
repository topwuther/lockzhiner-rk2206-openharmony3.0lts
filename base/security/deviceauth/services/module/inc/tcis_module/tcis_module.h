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

#ifndef TCIS_DEVICE_AUTH_TCIS_MODULE_H
#define TCIS_DEVICE_AUTH_TCIS_MODULE_H

#include "task_base.h"
#include "auth_common.h"
#include "tcis_auth_token_manager.h"

#define TASK_MULTI_AUTH_NUM_MAX 8
#define TASK_INFOS_LENGTH 5

typedef enum {
    CMD_ASY_AUTH_MAIN_ONE = 0x0030,
    RET_ASY_AUTH_FOLLOWER_ONE = 0x0031,
    CMD_ASY_AUTH_MAIN_TWO = 0x0032,
    RET_ASY_AUTH_FOLLOWER_TWO = 0x0033,
    CMD_ASY_AUTH_MAIN_FINAL = 0x0034,

    ASY_BIND_PAKE_REQUEST = 0x0040,
    ASY_BIND_PAKE_RESPONSE = 0x0041,
    ASY_BIND_PAKE_CLIENT_CONFIRM = 0x0042,
    ASY_BIND_PAKE_SERVER_CONFIRM = 0x0043,
    ASY_BIND_EXCHANGE_REQUEST = 0x0043,
    ASY_BIND_EXCHANGE_RESPONSE = 0x0044,

    CMD_SYM_AUTH_MAIN_ONE = 0x0050,
    RET_SYM_AUTH_FOLLOWER_ONE = 0x0051,
    CMD_SYM_AUTH_MAIN_TWO = 0x0052,
    RET_SYM_AUTH_FOLLOWER_TWO = 0x0053,
    ERR_MSG = 0x8080,
} TcisMessageType;

typedef struct {
    AuthModuleBase moduleBase;
    TcisAuthTokenManager *tokenManager;
    AlgLoader *algLoader;
} TcisAuthModule;

AuthModuleBase *CreateTcisModule(void);

bool IsTcisSupported(void);

#endif
