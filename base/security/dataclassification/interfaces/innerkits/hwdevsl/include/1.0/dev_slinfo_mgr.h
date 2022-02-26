/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEV_SLINFO_MGR_H
#define DEV_SLINFO_MGR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *val;
    uint32_t len;
    uint32_t mSize;
} DEVSLData;

#define DEV_TYPE_PHONE 1 /* device type - PHONE */
#define DEV_TYPE_PAD 2 /* device type - PAD */
#define DEV_TYPE_TV 3 /* device type - TV */
#define DEV_TYPE_PC 4 /* device type - PC */
#define DEV_TYPE_WATCH 5 /* device type - WATCH */

/* caller queries data security level :
 * 1 - if with udid, devType should be 0
 * 2 - if devType > 0, only queried data security level with policy configuriation
 */
typedef struct {
    const uint8_t *udid; /* if devType is 0, it must */
    const uint8_t *sensitiveData; /* optional */
    uint32_t idLen; /* if udid is a string, the length should not with end tag 0 */
    uint32_t sensitiveDataLen; /* length of sensitiveData */
    uint32_t devType; /* only support PHONE, PAD, TV, PC, WATCH */
} DEVSLQueryParams;

/* tmpParams is pointer */
#define DEVSL_INIT_PARAMS(tmpParams) do { \
    (tmpParams)->udid = NULL; \
    (tmpParams)->sensitiveData = NULL; \
    (tmpParams)->idLen = 0; \
    (tmpParams)->sensitiveDataLen = 0; \
    (tmpParams)->devType = 0; \
} while (0)

enum {
    DEVSL_SUCCESS = 0,
    DEVSL_ERROR,
    DEVSL_ERR_UNINITIALIZED,
    DEVSL_ERR_INITIALIZED,
    DEVSL_ERR_INVALID_PARAMS,
    DEVSL_ERR_ALLOC_MEMORY,
    DEVSL_ENTRY_NUMBER_ERROR,
    DEVSL_INIT_MUTEX_FAILED,
    DEVSL_ASYNC_PROCESSING,
    DEVSL_RESOURCE_BUSY,
    DEVSL_ERR_IN_LOCKING,
    DEVSL_ERR_GET_LOCAL_SENSITIVE,
    DEVSL_ERR_ENTRY_FULL,
    DEVSL_ERR_MEM_CPY,
    DEVSL_ERR_CREATE_THREAD,
    DEVSL_ASYNC_QUERY,
    DEVSL_UNKNOWN_SEC_LEVEL,
    DEVSL_ERR_PARSE_CFG,
    DEVSL_ERR_GET_TIME,
    DEVSL_ERR_FORM_CERT,
    DEVSL_ERR_HKS_BLOB_BUFFER,
    DEVSL_ERR_HKS_ATTEST_KEY,
    DEVSL_ERR_HKS_CERT_CHAIN,
    DEVSL_ERR_CERT_CHAIN_BUFFER,
    DEVSL_ERR_GET_CERT_CHAIN,
    DEVSL_ERR_VALIDATE_ATTEST_CERT,
    DEVSL_ERR_ENTRY_NULL,
    DEVSL_ERR_QUERY_SEC_LEVEL,
    DEVSL_ERR_PROFILE_CONN,
    DEVSL_ERR_PROFILE_GET_DATA,
    DEVSL_ERR_PROFILE_PUT_DATA,
    DEVSL_ERR_PROFILE_PUT_SERVICE,
    DEVSL_ERR_PROFILE_PUT_DEVICE,
    DEVSL_ERR_PROFILE_PROC_HOST,
    DEVSL_ERR_PROFILE_DEV_DATA,
    DEVSL_ERR_PROFILE_UDID,
    DEVSL_ERR_PROFILE_DATA_CTX,
    DEVSL_ERR_CERT_DATA_LEN,
    DEVSL_ERR_PROFILE_CONN_IN_QUERY,
    DEVSL_LEVEL_ONLY_WITH_POLICY,
    DEVSL_ERR_WITHOUT_PERMISSION,
    DEVSL_ERR_UNKNOWN_DEV_TYPE,
    DEVSL_ERR_PROFILE_INIT
};

#define DATA_SEC_LEVEL0 0 /* s0 */
#define DATA_SEC_LEVEL1 1 /* s1 */
#define DATA_SEC_LEVEL2 2 /* s2 */
#define DATA_SEC_LEVEL3 3 /* s3 */
#define DATA_SEC_LEVEL4 4 /* s4 */

/*
 * note: 1 - if return error code, the out levelInfo is invalid
 *       2 - if @param queryParams's devType > 0, only do the compatible processing
 * @param queryParams - if caller set the devType valid, get data security level only with policy configuration
 * @param levelInfo - store the queried data level
 * if success, return DEVSL_SUCCESS, else return error code.
 */
int32_t DEVSL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo);
/* cert buffer length must be more than 13k, suggest 14k */
int32_t DEVSL_GetLocalCertData(uint8_t *buff, uint32_t bufSz, uint32_t *dataLen);
/* not support mutil-thread */
int32_t DEVSL_OnStart(int32_t maxDevNum);
/* not support mutil-thread */
void DEVSL_ToFinish(void);

#ifdef __cplusplus
}
#endif

#endif
