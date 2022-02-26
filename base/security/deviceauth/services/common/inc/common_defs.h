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

#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "device_auth.h"
#include "device_auth_defines.h"
#include "json_utils.h"

#define FIELD_ADD_TYPE "addType"
#define FIELD_ADD_ID "addId"
#define FIELD_ADD_KEY "addKey"
#define FIELD_AUTH_DATA "authData"
#define FIELD_AUTH_RETURN "authReturn"
#define FIELD_AUTH_ID "authId"
#define FIELD_AUTH_PK "authPk"
#define FIELD_ADD_AUTH_INFO "addAuthInfo"
#define FIELD_ADD_RETURN "addReturn"
#define FIELD_APP_ID "appId"
#define FIELD_BIND_SESSION_TYPE "bindSessionType"
#define FIELD_CHALLENGE "challenge"
#define FIELD_CHANNEL_ID "channelId"
#define FIELD_CONN_DEVICE_ID "connDeviceId"
#define FIELD_CONNECT_PARAMS "connectParams"
#define FIELD_CURRENT_VERSION "currentVersion"
#define FIELD_DELETE_ID "deleteId"
#define FIELD_ENC_AUTH_TOKEN "encAuthToken"
#define FIELD_ENC_RESULT "encResult"
#define FIELD_ENC_DATA "encData"
#define FIELD_EPK "epk"
#define FIELD_EPK_LEN "epkLen"
#define FIELD_EX_AUTH_INFO "exAuthInfo"
#define FIELD_EXPIRE_TIME "expireTime"
#define FIELD_ERROR_CODE "errorCode"
#define FIELD_GROUP_INFO "groupInfo"
#define FIELD_GROUP_MANAGERS "groupManagers"
#define FIELD_GROUP_NAME "groupName"
#define FIELD_GROUP_OP "groupOp"
#define FIELD_GROUP_TYPE "groupType"
#define FIELD_GROUP_VISIBILITY "groupVisibility"
#define FIELD_IS_ADMIN "isAdmin"
#define FIELD_IS_ACCOUNT_BIND "isAccountBind"
#define FIELD_IS_FORCE_DELETE "isForceDelete"
#define FIELD_IS_CREDENTIAL_EXISTS "isCredentialExists"
#define FIELD_KCF_DATA "kcfData"
#define FIELD_KEY_TYPE "keyType"
#define FIELD_MESSAGE "message"
#define FIELD_GROUP_ERROR_MSG "groupErrorMsg"
#define FIELD_MIN_VERSION "minVersion"
#define FIELD_GROUP_AND_MODULE_VERSION "groupAndModuleVersion"
#define FIELD_NONCE "nonce"
#define FIELD_OPERATION_CODE "operationCode"
#define FIELD_OPERATION_PARAMS "operationParams"
#define FIELD_OWNER_ID "ownerId"
#define FIELD_OWNER_NAME "ownerName"
#define FIELD_PERMISSION "Permission"
#define FIELD_PAYLOAD "payload"
#define FIELD_PEER_DEVICE_ID "peerDeviceId"
#define FIELD_PIN_CODE "pinCode"
#define FIELD_PUBLIC_KEY "publicKey"
#define FIELD_PK_INFO "pkInfo"
#define FIELD_PKG_NAME "pkgName"
#define FIELD_SELF_AUTH_ID "selfAuthId"
#define FIELD_SELF_DEVICE_ID "selfDeviceId"
#define FIELD_REQUEST_ID "requestId"
#define FIELD_RECEIVED_DATA "receivedData"
#define FIELD_RETURN_CODE "returnCode"
#define FIELD_RETURN_DATA "returnData"
#define FIELD_RETURN_CODE_MAC "returnCodeMac"
#define FIELD_RMV_TYPE "rmvType"
#define FIELD_RMV_ID "rmvId"
#define FIELD_RMV_AUTH_INFO "rmvAuthInfo"
#define FIELD_RMV_RETURN "rmvReturn"
#define FIELD_SALT "salt"
#define FIELD_ISO_SALT "isoSalt"
#define FIELD_SEED "seed"
#define FIELD_SELF_TYPE "selfType"
#define FIELD_SEND_TO_PEER "sendToPeer"
#define FIELD_SEND_TO_SELF "sendToSelf"
#define FIELD_SHARED_USER_ID_LIST "sharedUserIdList"
#define FIELD_SUPPORT_256_MOD "support256mod"
#define FIELD_TOKEN "token"
#define FIELD_IS_UUID "uuIdAsDeviceId"
#define FIELD_VERSION "version"
#define FIELD_MK_AGREE_MSG "mkaMsg"

#define AUTH_FORM_INVALID_TYPE -1
#define AUTH_FORM_ACCOUNT_UNRELATED 0
#define AUTH_FORM_IDENTICAL_ACCOUNT 1
#define AUTH_FORM_ACROSS_ACCOUNT 2
#define INVALID_MODULE_TYPE -1
#define GROUP_ERR_MSG 0x8080

/* GroupVisibility */
#define ALL_GROUP_VISIBILITY -2
#define GROUP_VISIBILITY_PRIVATE 0
#define GROUP_VISIBILITY_ALLOW_LIST 1
#define GROUP_VISIBILITY_SIGNATURE 2
#define GROUP_VISIBILITY_SYSTEM 4
#define GROUP_VISIBILITY_PRIVILEGE 8
#define GROUP_VISIBILITY_PUBLIC -1

#define CLIENT 1
#define SERVER 0

#define CREATE_KEY_PAIR 1
#define DELETE_KEY_PAIR 2

/* DefaultValue */
#define DEFAULT_MEMORY_SIZE 64
#define DEFAULT_USER_ID 0
#define DEFAULT_REQUEST_ID 0
#define DEFAULT_CHANNEL_ID -1
#define DEFAULT_EXPIRE_TIME 90
#define GROUP_MANAGER_PACKAGE_NAME "com.huawei.devicegroupmanage"
#define DEFAULT_RETURN_KEY_LENGTH 32
#define ERR_AUTH_FORM 0
#define MAX_BUFFER_LEN 1024
#define MAX_DATA_BUFFER_SIZE 4096
#define MAX_AUTH_ID_LEN 256

typedef enum {
    ALL_GROUP = 0,
    IDENTICAL_ACCOUNT_GROUP = 1,
    PEER_TO_PEER_GROUP = 256,
    COMPATIBLE_GROUP = 512,
    ACROSS_ACCOUNT_AUTHORIZE_GROUP = 1282
} GroupType;

typedef enum {
    NO_CHANNEL = 1,
    SERVICE_CHANNEL = 2,
    SOFT_BUS = 3,
} ChannelType;

typedef enum {
    SYMMETRIC = 0,
    ASYMMETRIC = 1,
} CredentialType;

#define MAX_IN_PARAM_LEN 4096

#define CHECK_PTR_RETURN_NULL(ptr, paramTag) \
    do { \
        if ((ptr) == NULL) { \
            LOGE("%s is null ptr", (paramTag)); \
            return NULL; \
        } \
    } while (0)

#define CHECK_PTR_RETURN_ERROR_CODE(ptr, paramTag) \
    do { \
        if ((ptr) == NULL) { \
            LOGE("%s is null ptr", (paramTag)); \
            return HC_ERR_NULL_PTR; \
        } \
    } while (0)

#define CHECK_PTR_RETURN_NONE(ptr, paramTag) \
        do { \
            if ((ptr) == NULL) { \
                LOGE("%s is null ptr", (paramTag)); \
                return; \
            } \
        } while (0)

#define GOTO_IF_ERR(x) do { \
        int32_t res = x; \
        if ((res) != HC_SUCCESS) { \
            goto err; \
        } \
    } while (0)

#define GOTO_ERR_AND_SET_RET(x, res) do { \
        res = x; \
        if ((res) != HC_SUCCESS) { \
            goto err; \
        } \
    } while (0)

#define RETURN_IF_ERR(x) do { \
        int32_t res = x; \
        if ((res) != HC_SUCCESS) { \
            return res; \
        } \
    } while (0)

typedef struct AuthModuleBaseT {
    int moduleType;
    int (*createTask)(int *, const CJson *in, CJson *out);
    int (*processTask)(int, const CJson *in, CJson *out, int *status);
    void (*destroyTask)(int);
    void (*destroyModule)(struct AuthModuleBaseT *module);
} AuthModuleBase;

typedef enum {
    DAS_MODULE = 0x0001,
    TCIS_MODULE = 0x0010,
} EnumModuleType;

typedef enum {
    CONTINUE = 0,
    IGNORE_MSG = 1,
    FINISH,
} TaskStatus;

typedef enum {
    CODE_NULL = 0,
    OP_BIND = 1,
    AUTHENTICATE = 2,
    ADD_AUTH_INFO = 3,
    REMOVE_AUTH_INFO = 4,
    OP_UNBIND = 5,
    AUTH_KEY_AGREEMENT = 6,
    REGISTER = 7,
    SECURE_CLONE = 8,
} OperationCode;

typedef struct {
    int64_t requestId;
    int32_t type;
    int64_t sessionId;
} RequestInfo;

typedef struct {
    char *appId;
    DeviceAuthCallback *callback;
} CallbackEntry;

#endif
