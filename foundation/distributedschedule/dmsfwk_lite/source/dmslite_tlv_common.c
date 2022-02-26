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

#include "dmslite_tlv_common.h"

#include "dmslite_inner_common.h"
#include "dmslite_utils.h"

#include <stdbool.h>
#include <string.h>

#include "securec.h"

#include "dmslite_log.h"

TlvNode* GetNodeByType(uint8_t nodeType, const TlvNode *tlvHead)
{
    TlvNode* tlvNode = (TlvNode *)tlvHead;
    while (tlvNode != NULL) {
        if (tlvNode->type == nodeType) {
            return tlvNode;
        }
        tlvNode = tlvNode->next;
    }
    return NULL;
}

static uint64_t ConvertIntDataBig2Little(const uint8_t *dataIn, uint8_t typeSize)
{
    uint64_t dataOut = 0;
    switch (typeSize) {
        case INT_16:
            Convert16DataBig2Little(dataIn, (uint16_t*)&dataOut);
            break;
        case INT_32:
            Convert32DataBig2Little(dataIn, (uint32_t*)&dataOut);
            break;
        case INT_64:
            Convert64DataBig2Little(dataIn, (uint64_t*)&dataOut);
            break;
        default:
            break;
    }
    return dataOut;
}

static uint64_t ConvertIntByDefault(const uint8_t *dataIn, uint8_t typeSize)
{
    switch (typeSize) {
        case INT_8:
            return *((int8_t*)dataIn);
        case INT_16:
            return *((int16_t*)dataIn);
        case INT_32:
            return *((int32_t*)dataIn);
        case INT_64:
            return *((int64_t*)dataIn);
        default:
            return 0;
    }
}

static uint64_t UnMarshallInt(const TlvNode *tlvHead, uint8_t nodeType, uint8_t fieldSize)
{
    if (tlvHead == NULL) {
        return 0;
    }
    TlvNode* tlvNode = GetNodeByType(nodeType, tlvHead);
    if (tlvNode == NULL || tlvNode->value == NULL) {
        HILOGE("[Bad node type %d]", nodeType);
        return 0;
    }
    if (fieldSize != tlvNode->length) {
        HILOGE("[Mismatched fieldSize=%d while nodeLength=%d]", fieldSize, tlvNode->length);
        return 0;
    }
    return IsBigEndian() ? ConvertIntByDefault(tlvNode->value, fieldSize)
        : ConvertIntDataBig2Little(tlvNode->value, fieldSize);
}

uint8_t UnMarshallUint8(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(uint8_t));
}

uint16_t UnMarshallUint16(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(uint16_t));
}

uint32_t UnMarshallUint32(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(uint32_t));
}

uint64_t UnMarshallUint64(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(uint64_t));
}

int8_t UnMarshallInt8(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(int8_t));
}

int16_t UnMarshallInt16(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(int16_t));
}

int32_t UnMarshallInt32(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(int32_t));
}

int64_t UnMarshallInt64(const TlvNode *tlvHead, uint8_t nodeType)
{
    return UnMarshallInt(tlvHead, nodeType, sizeof(int64_t));
}

const char* UnMarshallString(const TlvNode *tlvHead, uint8_t nodeType)
{
    HILOGI("[Get string value for node %d]", nodeType);
    if (tlvHead == NULL) {
        return "";
    }
    TlvNode* tlvNode = GetNodeByType(nodeType, tlvHead);
    if (tlvNode == NULL || tlvNode->value == NULL) {
        HILOGE("[Bad node type %d]", nodeType);
        return "";
    }
    const char* value = (const char*)tlvNode->value;
    if (value[tlvNode->length - 1] != '\0') {
        HILOGE("[Non-zero ending string, length:%d, ending:%d]", tlvNode->length, value[tlvNode->length - 1]);
        return "";
    } else {
        return value;
    }
}