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

#include "common_util.h"
#include <stdlib.h>
#include <string.h>
#include "hc_error.h"
#include "hc_log.h"
#include "hc_types.h"

#define OUT_OF_HEX 16
#define NUMBER_9_IN_DECIMAL 9

static char HexToChar(uint8_t hex)
{
    return (hex > NUMBER_9_IN_DECIMAL) ? (hex + 0x37) : (hex + 0x30); /* Convert to the corresponding character */
}

int32_t ByteToHexString(const uint8_t *byte, uint32_t byteLen, char *hexStr, uint32_t hexLen)
{
    if (byte == NULL || hexStr == NULL) {
        LOGE("Param is null ptr.");
        return HAL_ERR_NULL_PTR;
    }
    /* The terminator('\0') needs 1 bit */
    if (hexLen < byteLen * BYTE_TO_HEX_OPER_LENGTH + 1) {
        LOGE("Invalid length");
        return HAL_ERR_INVALID_LEN;
    }

    for (uint32_t i = 0; i < byteLen; i++) {
        hexStr[i * BYTE_TO_HEX_OPER_LENGTH] = HexToChar((byte[i] & 0xF0) >> 4); /* 4: shift right for filling */
        hexStr[i * BYTE_TO_HEX_OPER_LENGTH + 1] = HexToChar(byte[i] & 0x0F); /* get low four bits */
    }
    hexStr[byteLen * BYTE_TO_HEX_OPER_LENGTH] = '\0';

    return HAL_SUCCESS;
}

static uint8_t CharToHex(char c)
{
    if ((c >= 'A') && (c <= 'F')) {
        return (c - 'A' + DEC);
    } else if ((c >= 'a') && (c <= 'f')) {
        return (c - 'a' + DEC);
    } else if ((c >= '0') && (c <= '9')) {
        return (c - '0');
    } else {
        return OUT_OF_HEX;
    }
}

int32_t HexStringToByte(const char *hexStr, uint8_t *byte, uint32_t byteLen)
{
    if (byte == NULL || hexStr == NULL) {
        LOGE("Param is null ptr.");
        return HAL_ERR_NULL_PTR;
    }
    uint32_t realHexLen = strlen(hexStr);
    /* even number or not */
    if (realHexLen % BYTE_TO_HEX_OPER_LENGTH != 0 || byteLen < realHexLen / BYTE_TO_HEX_OPER_LENGTH) {
        return HAL_ERR_INVALID_LEN;
    }

    for (uint32_t i = 0; i < realHexLen / BYTE_TO_HEX_OPER_LENGTH; i++) {
        uint8_t high = CharToHex(hexStr[i * BYTE_TO_HEX_OPER_LENGTH]);
        uint8_t low = CharToHex(hexStr[i * BYTE_TO_HEX_OPER_LENGTH + 1]);
        if (high == OUT_OF_HEX || low == OUT_OF_HEX) {
            return HAL_ERR_INVALID_PARAM;
        }
        byte[i] = high << 4; /* 4: Set the high nibble */
        byte[i] |= low; /* Set the low nibble */
    }
    return HAL_SUCCESS;
}

int64_t StringToInt64(const char *cp)
{
    if (cp == NULL) {
        return 0;
    }
    return strtoll(cp, NULL, DEC);
}

void ConvertToAnnoymousStr(const char *originalStr, char **anonymousStr)
{
    if ((originalStr == NULL) || (anonymousStr == NULL)) {
        return;
    }
    int desensitizationLen = 4;
    uint32_t len = HcStrlen(originalStr);
    if (len <= desensitizationLen) {
        LOGD("The input string length is too short!");
        return;
    }
    *anonymousStr = (char *)HcMalloc(len + 1, 0);
    if ((*anonymousStr) == NULL) {
        LOGD("Failed to allocate anonymousStr memory!");
        return;
    }
    if (memset_s(*anonymousStr, len + 1, '*', len) != EOK) {
        LOGD("Failed to memset string!");
        HcFree(*anonymousStr);
        *anonymousStr = NULL;
        return;
    }
    if (memcpy_s(*anonymousStr, len + 1, originalStr, len - desensitizationLen) != EOK) {
        LOGD("Failed to copy string!");
        HcFree(*anonymousStr);
        *anonymousStr = NULL;
        return;
    }
}