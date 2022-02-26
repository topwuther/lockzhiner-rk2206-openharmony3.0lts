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

#ifndef TCIS_DEVICE_AUTH_TCIS_AUTH_TOKEN_MANAGER_H
#define TCIS_DEVICE_AUTH_TCIS_AUTH_TOKEN_MANAGER_H

#ifndef ASY
#define ASY

#include <stdint.h>
#include "common_defs.h"
#include "common_util.h"
#include "auth_common.h"

#define FILE_VERSION 0x01
#define SERVER_PUBLIC_KEY_ALIAS "tcisServerPk"
#define ECC_KEY_ALIAS "tcisEccKeyPair"
#define AUTH_CODE_ALIAS "tcisAuthCode"

#define PRINT_CHAR_ARRAY(x) do { \
        for (int i = 0; i < sizeof(x); ++i) { \
            printf_s("%c", x[i]); \
        } \
        printf_s("\n"); \
    } while (0)

typedef enum {
    CMD_USER_ID = 0,
    CMD_DEVICE_ID,
} FileCmd;

typedef struct {
    uint64_t userId;
    uint8_t deviceType[DEV_TYPE_SIZE];
    uint8_t deviceId[DEV_ID_MAX_SIZE];
    uint8_t deviceModel[DEV_MODEL_MAX_SIZE];
    uint8_t publicKey[PUBLIC_KEY_HEX_LENGTH];
} PkInfo;

#ifdef ASY
typedef struct {
    uint32_t version;
    uint8_t userId[USER_ID_SIZE];
    uint8_t deviceId[DEV_ID_MAX_SIZE];
    uint8_t deviceModel[DEV_MODEL_MAX_SIZE];
    uint8_t deviceType[DEV_TYPE_SIZE];
    uint32_t algType;
    uint8_t pairingTcisId[TCIS_ID_SIZE];
    uint8_t publicKey[PUBLIC_KEY_HEX_LENGTH];
    uint8_t signature[SIGNATURE_SIZE];
} RegisterProof;

typedef struct {
    uint8_t pkInfoStr[PUBLIC_KEY_INFO_SIZE];
    PkInfo pkInfo;
    uint8_t pkInfoSignature[SIGNATURE_SIZE];
} TcisToken;
#endif

typedef struct {
    int (*setToken)(CJson *in, CJson *out);
    uint64_t (*getUserId)(void);
    int (*getDeviceId)(Uint8Buff *deviceId);
    int (*getToken)(TcisToken *token);
    int (*getServerPublicKey)(Uint8Buff *publicKey);
    int (*deleteToken)(void);
#ifdef ASY
    int (*setRegisterProof)(CJson *in);
    int (*getRegisterProof)(CJson *out);
#endif
} TcisAuthTokenManager;

TcisAuthTokenManager *GetTcisAuthTokenManager(void);

void InitTcisTokenManagerAlg(void);

#endif
#endif
