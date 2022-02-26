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

#include "dmslite_parser.h"

#include <stdlib.h>
#include <unistd.h>

#include "dmsfwk_interface.h"
#include "dmslite_log.h"
#include "dmslite_msg_handler.h"
#include "dmslite_tlv_common.h"
#include "securec.h"

#define MIN_VALID_NODES 2

#define HIGH_BIT_MASK 0xFF
#define LOW_BIT_MASK 0x7F
#define TLV_LENGTH_SHIFT_BITS 7

#define BREAK_IF_FAILURE(errCode);       \
    if ((errCode) != DMS_TLV_SUCCESS) {  \
        break;                           \
    }                                    \

static inline bool IsNextTlvLength(uint8_t num)
{
    /* 128-16383 : 0b1xxxxxxx 0b0xxxxxxx */
    return (((num) & HIGH_BIT_MASK) >> TLV_LENGTH_SHIFT_BITS) == 0x00;
}

static inline void TlvByteToLength(uint8_t byte, uint16_t *len)
{
    *len = ((*len) << TLV_LENGTH_SHIFT_BITS) | (byte & LOW_BIT_MASK);
}

static TlvErrorCode TlvBytesToLength(const uint8_t *bytesBuffer, uint16_t bufLength,
    uint16_t *length, uint8_t *bytesNumber)
{
    uint8_t bytesNum = 0;
    uint16_t len = 0;
    /* compute TLV's L, when value > 127, the L should be two bytes, else L is one byte long
       0-127      :  0b0xxxxxxx
       128-16383  :  0b1xxxxxxx 0b0xxxxxxx */
    for (uint8_t i = 0; i < bufLength; i++) {
        TlvByteToLength(bytesBuffer[i], &len);
        bytesNum++;
        if (IsNextTlvLength(bytesBuffer[i])) {
            HILOGD("[bytesNum = %d, byte = %x]", bytesNum, bytesBuffer[i]);
            break;
        }
        if (bytesNum >= TLV_MAX_LENGTH_BYTES) {
            return DMS_TLV_ERR_LEN;
        }
    }
    /* it is meaningless to have a node with length being zero */
    if (len == 0) {
        HILOGE("[Invalid zero-size length]");
        return DMS_TLV_ERR_LEN;
    }

    *length = len;
    *bytesNumber = bytesNum;

    return DMS_TLV_SUCCESS;
}

static TlvErrorCode TlvFillNode(const uint8_t *byteBuffer, uint16_t bufLength,
    TlvNode *node, uint16_t *actualHandledLen)
{
    if (bufLength <= TLV_TYPE_LEN) {
        HILOGE("[Bad bufLength %d]", bufLength);
        return DMS_TLV_ERR_LEN;
    }

    /* fill TLV's T(type) */
    node->type = *byteBuffer;
    uint16_t curTlvNodeLen = TLV_TYPE_LEN;

    /* fill TLV's L(length) */
    uint16_t length = 0;
    uint8_t bytesNum = 0;
    const uint8_t *lengthPartAddr = byteBuffer + curTlvNodeLen;
    TlvErrorCode errCode = TlvBytesToLength(lengthPartAddr, bufLength - curTlvNodeLen,
        &length, &bytesNum);
    if (errCode != DMS_TLV_SUCCESS) {
        return DMS_TLV_ERR_LEN;
    }
    node->length = length;
    curTlvNodeLen += bytesNum;

    /* fill TLV's V(value) */
    node->value = byteBuffer + curTlvNodeLen;
    curTlvNodeLen += length;
    if (curTlvNodeLen > bufLength) {
        return DMS_TLV_ERR_LEN;
    } else {
        *actualHandledLen = curTlvNodeLen;
    }

    return DMS_TLV_SUCCESS;
}

static void TlvFreeNodeList(TlvNode *node)
{
    TlvNode *next = NULL;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
}

static inline TlvNode* MallocTlvNode()
{
    TlvNode *node = (TlvNode *)malloc(sizeof(TlvNode));
    if (node == NULL) {
        HILOGE("[Out of memory]");
        return NULL;
    }
    /* won't fail */
    (void) memset_s(node, sizeof(TlvNode), 0x00, sizeof(TlvNode));

    return node;
}

static inline TlvErrorCode CheckNodeSequence(const TlvNode *lastNode, const TlvNode *curNode)
{
    if (lastNode == curNode) {
        return DMS_TLV_SUCCESS;
    }
    if (lastNode->type >= curNode->type) {
        HILOGE("[Bad node type sequence '%d' is expected but '%d' appears]",
            lastNode->type, curNode->type);
        return DMS_TLV_ERR_OUT_OF_ORDER;
    }

    return DMS_TLV_SUCCESS;
}

static inline TlvErrorCode MoveToNextTlvNode(TlvNode **curNode)
{
    TlvNode *next = MallocTlvNode();
    if (next == NULL) {
        return DMS_TLV_ERR_NO_MEM;
    }

    (*curNode)->next = next;
    *curNode = next;
    return DMS_TLV_SUCCESS;
}

TlvErrorCode TlvBytesToNode(const uint8_t *byteBuffer, uint16_t bufLength, TlvNode **tlv)
{
    if ((tlv == NULL) || (byteBuffer == NULL)) {
        HILOGE("[Bad parameter]");
        return DMS_TLV_ERR_PARAM;
    }
    /* bufLength is longer than TLV_TYPE_LEN + 1(means length should be at least 1 byte) */
    if (bufLength <= (TLV_TYPE_LEN + 1)) {
        HILOGE("[Bad Length %d]", bufLength);
        return DMS_TLV_ERR_LEN;
    }

    TlvNode *head = MallocTlvNode();
    if (head == NULL) {
        return DMS_TLV_ERR_NO_MEM;
    }
    TlvNode *curNode = head;
    TlvNode *lastNode = curNode;

    /* translate bytes to tlv node until the end of buffer */
    uint16_t remainingLen = bufLength;
    uint8_t handledNodeNum = 0;
    const uint8_t *nodeStartAddr = byteBuffer;
    TlvErrorCode errCode = DMS_TLV_SUCCESS;
    while (true) {
        uint16_t curTlvNodeLen = 0;
        errCode = TlvFillNode(nodeStartAddr, remainingLen, curNode, &curTlvNodeLen);
        BREAK_IF_FAILURE(errCode);

        /* check whether there is need to continue processing the remaining nodes */
        handledNodeNum++;

        /* check node type sequence: the type of node must appear in strictly increasing order */
        errCode = CheckNodeSequence(lastNode, curNode);
        BREAK_IF_FAILURE(errCode);

        remainingLen -= curTlvNodeLen;
        if (remainingLen == 0) {
            break;
        }

        HILOGD("[curNode length:%d type:%d handledNodeNum:%d]", curNode->length,
            curNode->type, handledNodeNum);
        /* if all is ok, then move to the T part of the next tlv node */
        nodeStartAddr += curTlvNodeLen;
        lastNode = curNode;
        errCode = MoveToNextTlvNode(&curNode);
        BREAK_IF_FAILURE(errCode);
    }

    if (errCode == DMS_TLV_SUCCESS && handledNodeNum < MIN_VALID_NODES) {
        HILOGE("[Parse done, but node num is invalid]");
        errCode = DMS_TLV_ERR_BAD_NODE_NUM;
    }

    if (errCode != DMS_TLV_SUCCESS) {
        TlvFreeNodeList(head);
        head = NULL;
    }
    *tlv = head;

    return errCode;
}

static int32_t Parse(const uint8_t *payload, uint16_t length, TlvNode **head)
{
    if (length > MAX_DMS_MSG_LENGTH) {
        HILOGE("[Bad parameters][length = %d]", length);
        return DMS_TLV_ERR_PARAM;
    }

    TlvNode *tlvHead = NULL;
    TlvErrorCode errCode = TlvBytesToNode(payload, length, &tlvHead);
    *head = tlvHead;
    HILOGI("[errCode = %d]", errCode);
    return errCode;
}

static bool CanCall()
{
#ifndef WEARABLE_PRODUCT
    uid_t callerUid = getuid();
    /* only foundation and xts (shell-enabled mode only) is reasonable to call ProcessCommuMsg directly */
    if (callerUid != FOUNDATION_UID && callerUid != SHELL_UID) {
        HILOGD("[Caller uid is not allowed, uid = %d]", callerUid);
        return false;
    }
#endif
    return true;
}

int32_t ProcessCommuMsg(const CommuMessage *commuMessage, const IDmsFeatureCallback *dmsFeatureCallback)
{
    if (!CanCall()) {
        return DMS_EC_FAILURE;
    }

    if (commuMessage == NULL || commuMessage->payload == NULL || dmsFeatureCallback == NULL) {
        return DMS_EC_FAILURE;
    }

    TlvNode *tlvHead = NULL;
    int32_t errCode = Parse(commuMessage->payload, commuMessage->payloadLength, &tlvHead);
    /* mainly for xts testsuit convenient, in non-test mode the onTlvParseDone should be set NULL */
    if (dmsFeatureCallback->onTlvParseDone != NULL) {
        dmsFeatureCallback->onTlvParseDone(errCode, tlvHead);
    }
    if (errCode != DMS_TLV_SUCCESS) {
        return DMS_EC_PARSE_TLV_FAILURE;
    }

    uint16_t commandId = UnMarshallUint16(tlvHead, COMMAND_ID);
    HILOGI("[ProcessCommuMsg commandId %d]", commandId);
    switch (commandId) {
        case DMS_MSG_CMD_START_FA: {
            errCode = StartAbilityFromRemoteHandler(tlvHead, dmsFeatureCallback->onStartAbilityDone);
            break;
        }
        case DMS_MSG_CMD_REPLY: {
            errCode = ReplyMsgHandler(tlvHead);
            break;
        }
        default: {
            HILOGW("[Unkonwn command id %d]", commandId);
            errCode = DMS_EC_UNKNOWN_COMMAND_ID;
            break;
        }
    }
    TlvFreeNodeList(tlvHead);
    return errCode;
}