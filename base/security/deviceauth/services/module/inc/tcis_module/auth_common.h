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

#ifndef AUTH_COMMON_H
#define AUTH_COMMON_H

#include "stdint.h"

#define AUTH_STEP                    "authStep"
#define DEV_ID                       "devId"
#define SALT                         "salt"
#define USER_ID                      "userId"
#define SEED                         "seed"
#define AUTH_FORM                    "authForm"
#define AUTH_KEY_ALG_ENCODE          "authKeyAlgEncode"
#define TOKEN                        "token"
#define SESSION_KEY                  "sessionKey"
#define AUTH_RESULT                  "authResult"
#define AUTH_RESULT_MAC              "authResultMac"
#define IS_CLIENT                    "isClient"
#define HI_CHAIN_RETURN_KEY          "hichain_return_key"
#define SEND_TO_SELF                 "sendToSelf"
#define SEND_TO_PEER                 "sendToPeer"

#define SEED_SIZE                    32
#define AUTH_ID_SIZE                 8
#define USER_ID_SIZE                 20
#define TCIS_ID_SIZE                 10
#define DEV_ID_SIZE                  40
#define DEV_ID_MAX_SIZE              40
#define DEV_TYPE_SIZE                3
#define DEV_MODEL_MAX_SIZE           40
#define UDID_SIZE                    10
#define RANDOM_SIZE                  16
#define TOKEN_SIZE                   32
#define PSK_SIZE                     32
#define AUTH_RESULT_MAC_SIZE         32
#define AUTH_CODE_SIZE               32
#define PUBLIC_KEY_LENGTH            64
#define PUBLIC_KEY_HEX_LENGTH        128
#define AUTH_PK_SIG_SIZE             64
#define AUTH_RESULT_SIGN_SIZE        64
#define SIGNATURE_SIZE               128
#define SERVER_PK_SIZE               64
#define HI_CHAIN_STRING_SIZE         18
#define PUBLIC_KEY_INFO_SIZE         300
#ifdef ASY
#define PROOF_LEN 146
#define TOKEN_LEN 218
#endif
#ifdef SYM
#define TOKEN_LEN 48
#endif

typedef enum {
    SYMMETRIC_AUTH = 0,
    ASYMMETRIC_AUTH,
    POINT_TO_POINT,
    SAME_ACCOUNT,
    ACROSS_ACCOUNT,
} AuthForm;

typedef enum {
    AUTH_ERR_AUTH_STEP,
    AUTH_ERR_INVALID_PARAM,
    AUTH_ERR_TOKEN_NOT_MATCH,
    AUTH_ERR_ALLOC_MEMORY,
    AUTH_ERR_MAC_NOT_EQUAL,
    AUTH_ERR_PROCESS_TASK,
    AUTH_ERR_MESSAGE_PEER,
    AUTH_ERR_STATUS,
    AUTH_ERR_INTERNAL,
    AUTH_ERR_GET_HMAC,
    AUTH_ERR_JSON,
    AUTH_ERR_GET_ITEM,
    AUTH_ERR_COMPARE,
    ERR_MULTI_AUTH_TASK_CREATE_FAIL,
    AUTH_ERR_SUCCESS = 0,
} AuthResult;

typedef enum {
    ALG_ECC = 0x0000,
    ALG_RSA = 0x0001,
    ALG_HKDF = 0x0002,
    ALG_HMAC = 0x0003,
    ALG_AES = 0x0004,
    ALG_PBKDF2 = 0x0005,
    ALG_ECDH = 0x0006,
    ALG_X25519 = 0x0007,
    ALG_ED25519 = 0x0008,
} KeyAlg;
#endif
