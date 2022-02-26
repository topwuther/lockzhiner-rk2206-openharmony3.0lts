/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTEDSCHEDULE_TLVCOMMON_H
#define OHOS_DISTRIBUTEDSCHEDULE_TLVCOMMON_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define TLV_ONE_BYTE_LENGTH  127
#define TLV_MAX_LENGTH_BYTES 2
#define TLV_TYPE_LEN         1
#define MAX_DMS_MSG_LENGTH   1024

typedef struct TlvNode {
    uint8_t type;
    uint16_t length;
    const uint8_t *value;
    struct TlvNode *next;
} TlvNode;

typedef enum {
    DMS_TLV_SUCCESS = 0,
    DMS_TLV_ERR_NO_MEM = 1,
    DMS_TLV_ERR_PARAM = 2,
    DMS_TLV_ERR_LEN = 3,
    DMS_TLV_ERR_OUT_OF_ORDER = 4,
    DMS_TLV_ERR_BAD_NODE_NUM = 5,
    DMS_TLV_ERR_UNKNOWN_TYPE = 6,
    DMS_TLV_ERR_BAD_SOURCE = 7,
} TlvErrorCode;

typedef enum {
    COMMAND_ID = 1,
    DMS_VERSION = 2,
    CALLEE_BUNDLE_NAME = 3,
    CALLEE_ABILITY_NAME = 4,
    CALLER_SIGNATURE = 5,
    CALLER_PAYLOAD = 6,
    REPLY_ERR_CODE = 0xFF
} FieldType;

typedef struct {
    uint16_t payloadLength;
    const uint8_t *payload;
} CommuMessage;

enum DmsCommuMsgCmdType {
    DMS_MSG_CMD_START_FA = 0x01,
    DMS_MSG_CMD_REPLY = 0xFFFF
};

uint8_t UnMarshallUint8(const TlvNode *tlvHead, uint8_t nodeType);
uint16_t UnMarshallUint16(const TlvNode *tlvHead, uint8_t nodeType);
uint32_t UnMarshallUint32(const TlvNode *tlvHead, uint8_t nodeType);
uint64_t UnMarshallUint64(const TlvNode *tlvHead, uint8_t nodeType);
int8_t UnMarshallInt8(const TlvNode *tlvHead, uint8_t nodeType);
int16_t UnMarshallInt16(const TlvNode *tlvHead, uint8_t nodeType);
int32_t UnMarshallInt32(const TlvNode *tlvHead, uint8_t nodeType);
int64_t UnMarshallInt64(const TlvNode *tlvHead, uint8_t nodeType);
const char* UnMarshallString(const TlvNode *tlvHead, uint8_t nodeType);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* OHOS_DISTRIBUTEDSCHEDULE_TLVCOMMON_H */