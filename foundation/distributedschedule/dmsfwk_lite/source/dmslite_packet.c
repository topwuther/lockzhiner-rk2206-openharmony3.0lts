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

#include "dmslite_packet.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "dmsfwk_interface.h"
#include "dmslite_inner_common.h"
#include "dmslite_log.h"
#include "dmslite_utils.h"
#include "securec.h"

#define PACKET_DATA_SIZE 1024
#define TLV_LENGTH_SHIFT_BITS 7
#define LOW_BIT_MASK   0x7F
#define HIGH_BIT_MASK   0x80
#define BYTE_MASK   0xFF
#define METADATA_SIZE 16

#define ONE_BYTE_BITS_NUM   8
#define ONE_BYTE_LENGTH   1
#define TYPE_FILED_LENGTH  1

#define MIN_BYTE_NUM   1
#define MAX_BYTE_NUM   2

static char g_buffer[PACKET_DATA_SIZE] = { 0 };
static uint16_t g_counter = 0;

static bool StringToHex(const char *stringValue);
static void EncodeLengthOfTlv(uint16_t length);
static uint64_t ConvertIntLittle2Big(const uint8_t *dataIn, uint8_t typeSize);
static bool MarshallInt(uint64_t field, FieldType fieldType, uint8_t fieldSize);
static void IntToHex(uint64_t value, uint8_t typeSize);

bool PreprareBuild()
{
    g_counter = 0;
    if (memset_s(g_buffer, PACKET_DATA_SIZE, 0x00, PACKET_DATA_SIZE) != EOK) {
        HILOGW("Packet buffer is not cleared");
        return false;
    }
    return true;
}

void CleanBuild()
{
    g_counter = 0;
    if (memset_s(g_buffer, PACKET_DATA_SIZE, 0x00, PACKET_DATA_SIZE) != EOK) {
        HILOGW("Packet buffer is not cleared");
    }
}

bool MarshallUint8(uint8_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(uint8_t));
}

bool MarshallUint16(uint16_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(uint16_t));
}

bool MarshallUint32(uint32_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(uint32_t));
}

bool MarshallUint64(uint64_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(uint16_t));
}

bool MarshallInt8(int8_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(int8_t));
}

bool MarshallInt16(int16_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(int16_t));
}

bool MarshallInt32(int32_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(int32_t));
}

bool MarshallInt64(int64_t field, FieldType fieldType)
{
    return MarshallInt(field, fieldType, sizeof(int64_t));
}

uint16_t GetPacketSize()
{
    return g_counter;
}

const char* GetPacketBufPtr()
{
    return g_buffer;
}

bool MarshallString(const char *field, uint8_t type)
{
    if (field == NULL) {
        return false;
    }

    // one more byte for '\0'
    size_t sz = strlen(field) + 1;
    if (g_counter + (TYPE_FILED_LENGTH + MAX_BYTE_NUM + sz) > PACKET_DATA_SIZE) {
        HILOGE("MarshallString field is too big to fit");
        return false;
    }

    IntToHex(type, sizeof(uint8_t));
    EncodeLengthOfTlv(sz);
    StringToHex(field);
    return true;
}

bool MarshallRawData(const void *field, uint8_t type, uint16_t length)
{
    if (field == NULL) {
        return false;
    }

    if (g_counter + (TYPE_FILED_LENGTH + MAX_BYTE_NUM + length) > PACKET_DATA_SIZE) {
        HILOGE("MarshallString field is too big to fit");
        return false;
    }

    IntToHex(type, sizeof(uint8_t));
    EncodeLengthOfTlv(length);
    for (uint32_t i = 0; i < length; i++) {
        char ch = ((const char *)field)[i];
        g_buffer[g_counter++] = ch;
    }
    return true;
}

static bool StringToHex(const char *stringValue)
{
    if (stringValue == NULL) {
        return false;
    }

    uint32_t len = strlen(stringValue);
    for (uint32_t i = 0; i < len; i++) {
        char ch = stringValue[i];
        g_buffer[g_counter++] = ch;
    }
    g_buffer[g_counter++] = '\0';

    return true;
}

static void EncodeLengthOfTlv(uint16_t length)
{
    g_buffer[g_counter] = ((length >> TLV_LENGTH_SHIFT_BITS) & LOW_BIT_MASK);
    if (g_buffer[g_counter]) {
        char highByte = (char)((uint8_t)g_buffer[g_counter] | HIGH_BIT_MASK);
        g_buffer[g_counter++] = highByte;
    }
    g_buffer[g_counter++] = (length & LOW_BIT_MASK);
}

static void IntToHex(uint64_t value, uint8_t typeSize)
{
    uint8_t *bytes = (uint8_t*)&value;
    /* put byte one by one from low address to high address */
    for (int8_t i = 0; i < typeSize; i++) {
        uint8_t val = bytes[i];
        g_buffer[g_counter++] = val;
    }
}

static uint64_t ConvertIntLittle2Big(const uint8_t *dataIn, uint8_t typeSize)
{
    uint64_t dataOut = 0;
    switch (typeSize) {
        case INT_16:
            Convert16DataLittle2Big(dataIn, (uint8_t*)&dataOut);
            break;
        case INT_32:
            Convert32DataLittle2Big(dataIn, (uint8_t*)&dataOut);
            break;
        case INT_64:
            Convert64DataLittle2Big(dataIn, (uint8_t*)&dataOut);
            break;
        default:
            break;
    }
    return dataOut;
}

static bool MarshallInt(uint64_t field, FieldType fieldType, uint8_t fieldSize)
{
    if (g_buffer == NULL) {
        return false;
    }

    if (g_counter + (TYPE_FILED_LENGTH + MAX_BYTE_NUM + fieldSize) > PACKET_DATA_SIZE) {
        HILOGE("MarshallInt field is too big to fit");
        return false;
    }

    IntToHex(fieldType, sizeof(uint8_t));
    EncodeLengthOfTlv(fieldSize);
    IntToHex(IsBigEndian() ? field : ConvertIntLittle2Big((uint8_t*)&field, fieldSize), fieldSize);
    return true;
}
