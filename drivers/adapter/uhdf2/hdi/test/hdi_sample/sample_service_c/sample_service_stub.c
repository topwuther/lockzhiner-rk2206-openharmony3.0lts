/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <hdf_base.h>
#include <hdf_device_desc.h>
#include <hdf_log.h>
#include <hdf_sbuf.h>
#include <osal_mem.h>

#include "isample.h"


static int32_t SerStubBooleanTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    bool input = false;
    bool output = false;
    if (!HdfSbufReadUint32(data, (uint32_t *)&input)) {
        HDF_LOGE("%{public}s: read bool data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->BooleanTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call BooleanTypeTest fuc failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint32(reply, (uint32_t)&output)) {
        HDF_LOGE("%{public}s: write bool output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return ec;
}

static int32_t SerStubByteTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int8_t input;
    int8_t output;
    if (!HdfSbufReadInt8(data, &input)) {
        HDF_LOGE("%{public}s: read int8_t data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->ByteTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call ByteTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteInt8(reply, output)) {
        HDF_LOGE("%{public}s: write int8_t output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubShortTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int16_t input;
    int16_t output;
    if (!HdfSbufReadInt16(data, &input)) {
        HDF_LOGE("%{public}s: read int16_t data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->ShortTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call ShortTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteInt16(reply, output)) {
        HDF_LOGE("%{public}s: write int16_t output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubIntTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t input;
    int32_t output;
    if (!HdfSbufReadInt32(data, &input)) {
        HDF_LOGE("%{public}s: read int32_t data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->IntTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call IntTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteInt32(reply, output)) {
        HDF_LOGE("%{public}s: write int32_t output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubLongTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int64_t input;
    int64_t output;
    if (!HdfSbufReadInt64(data, &input)) {
        HDF_LOGE("%{public}s: read int64_t data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->LongTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call LongTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteInt64(reply, output)) {
        HDF_LOGE("%{public}s: write int64_t output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return ec;
}

static int32_t SerStubFloatTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    float input;
    float output;
    if (!HdfSbufReadFloat(data, &input)) {
        HDF_LOGE("%{public}s: read float data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->FloatTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call FloatTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteFloat(reply, output)) {
        HDF_LOGE("%{public}s: write float output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubDoubleTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    double input;
    double output;
    if (!HdfSbufReadDouble(data, &input)) {
        HDF_LOGE("%{public}s: read double data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->DoubleTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call DoubleTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteDouble(reply, output)) {
        HDF_LOGE("%{public}s: write double output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubStringTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    const char *input = HdfSbufReadString(data);
    if (input == NULL) {
        HDF_LOGE("%{public}s: read string data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    char *output = NULL;

    int32_t ec = HdiSampleImplInstance()->StringTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call StringTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteString(reply, output)) {
        HDF_LOGE("%{public}s: write string output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubUcharTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint8_t input;
    uint8_t output;
    if (!HdfSbufReadUint8(data, &input)) {
        HDF_LOGE("%{public}s: read data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->UcharTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call UcharTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint8(reply, output)) {
        HDF_LOGE("%{public}s: write output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubUshortTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint16_t input;
    uint16_t output;
    if (!HdfSbufReadUint16(data, &input)) {
        HDF_LOGE("%{public}s: read data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->UshortTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call UshortTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint16(reply, output)) {
        HDF_LOGE("%{public}s: write output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubUintTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint32_t input;
    uint32_t output;
    if (!HdfSbufReadUint32(data, &input)) {
        HDF_LOGE("%{public}s: read data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->UintTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call UintTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint32(reply, output)) {
        HDF_LOGE("%{public}s: write output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubUlongTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint64_t input;
    uint64_t output;
    if (!HdfSbufReadUint64(data, &input)) {
        HDF_LOGE("%{public}s: read data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->UlongTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call UlongTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint64(reply, output)) {
        HDF_LOGE("%{public}s: write output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

static int32_t SerStubListTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ec = HDF_FAILURE;
    uint32_t inSize = 0;
    int8_t *input = NULL;
    uint32_t outSize = 0;
    int8_t *output = NULL;

    if (!HdfSbufReadUint32(data, &inSize)) {
        HDF_LOGE("%{public}s: read data size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        return ec;
    }

    input = (int8_t *)OsalMemAlloc(sizeof(int8_t) * inSize);
    for (uint32_t i = 0; i < inSize; i++) {
        if (!HdfSbufReadInt8(data, (input + i))) {
            HDF_LOGE("%{public}s: read data size failed!", __func__);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

    ec = HdiSampleImplInstance()->ArrayTypeTest(client->device, input, inSize, &output, &outSize);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed!", __func__);
        goto finished;
    }

    if (!HdfSbufWriteUint32(reply, outSize)) {
        HDF_LOGE("%{public}s: write output size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    for (uint8_t i = 0; i < outSize; i++) {
        if (!HdfSbufWriteInt8(reply, output[i])) {
            HDF_LOGE("%{public}s: write output failed!", __func__);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

finished:
    if (input != NULL) {
        (void)OsalMemFree(input);
    }
    if (output != NULL) {
        (void)OsalMemFree(output);
    }
    return ec;
}

static int32_t SerStubArrayTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ec = HDF_FAILURE;
    uint32_t inSize = 0;
    int8_t *input = NULL;
    uint32_t outSize = 0;
    int8_t *output = NULL;

    if (!HdfSbufReadUint32(data, &inSize)) {
        HDF_LOGE("%{public}s: read data size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        return ec;
    }

    input = (int8_t *)OsalMemAlloc(sizeof(int8_t) * inSize);
    for (uint32_t i = 0; i < inSize; i++) {
        if (!HdfSbufReadInt8(data, (input + i))) {
            HDF_LOGE("%{public}s: read data size failed!", __func__);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

    ec = HdiSampleImplInstance()->ArrayTypeTest(client->device, input, inSize, &output, &outSize);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed!", __func__);
        goto finished;
    }

    if (!HdfSbufWriteUint32(reply, outSize)) {
        HDF_LOGE("%{public}s: write output size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    for (uint8_t i = 0; i < outSize; i++) {
        if (!HdfSbufWriteInt8(reply, output[i])) {
            HDF_LOGE("%{public}s: write output failed!", __func__);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

finished:
    if (input != NULL) {
        (void)OsalMemFree(input);
    }
    if (output != NULL) {
        (void)OsalMemFree(output);
    }
    return ec;
}

static int32_t SerStubStructTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ec = HDF_FAILURE;
    uint32_t dataSize = 0;
    struct StructSample *input = NULL;
    struct StructSample output;

    if (!HdfSbufReadBuffer(data, (const void **)&input, &dataSize)) {
        HDF_LOGE("%{public}s: read struct data failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = HdiSampleImplInstance()->StructTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call StructTypeTest failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufWriteBuffer(reply, (const void *)&output, sizeof(output))) {
        HDF_LOGE("%{public}s: struct result write failed", __func__);
        goto finished;
    }
finished:
    return ec;
}

static int32_t SerStubEnumTypeTest(struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    enum EnumSample input;
    enum EnumSample output;

    if (!HdfSbufReadUint32(data, (uint32_t *)&input)) {
        HDF_LOGE("%{public}s: read EnumSample data failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = HdiSampleImplInstance()->EnumTypeTest(client->device, input, &output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call EnumTypeTest failed!", __func__);
        return ec;
    }

    if (!HdfSbufWriteUint32(reply, (uint32_t)output)) {
        HDF_LOGE("%{public}s: write EnumSample output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return ec;
}

int32_t SampleServiceOnRemoteRequest(struct HdfDeviceIoClient *client, int cmdId,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    switch (cmdId) {
        case CMD_BOOLEAN_TYPE_TEST:
            return SerStubBooleanTypeTest(client, data, reply);
        case CMD_BYTE_TYPE_TEST:
            return SerStubByteTypeTest(client, data, reply);
        case CMD_SHORT_TYPE_TEST:
            return SerStubShortTypeTest(client, data, reply);
        case CMD_INT_TYPE_TEST:
            return SerStubIntTypeTest(client, data, reply);
        case CMD_LONG_TYPE_TEST:
            return SerStubLongTypeTest(client, data, reply);
        case CMD_FLOAT_TYPE_TEST:
            return SerStubFloatTypeTest(client, data, reply);
        case CMD_DOUBLE_TYPE_TEST:
            return SerStubDoubleTypeTest(client, data, reply);
        case CMD_STRING_TYPE_TEST:
            return SerStubStringTypeTest(client, data, reply);
        case CMD_UCHAR_TYPE_TEST:
            return SerStubUcharTypeTest(client, data, reply);
        case CMD_USHORT_TYPE_TEST:
            return SerStubUshortTypeTest(client, data, reply);
        case CMD_UINT_TYPE_TEST:
            return SerStubUintTypeTest(client, data, reply);
        case CMD_ULONG_TYPE_TEST:
            return SerStubUlongTypeTest(client, data, reply);
        case CMD_LIST_TYPE_TEST:
            return SerStubListTypeTest(client, data, reply);
        case CMD_ARRAY_TYPE_TEST:
            return SerStubArrayTypeTest(client, data, reply);
        case CMD_STRUCT_TYPE_TEST:
            return SerStubStructTypeTest(client, data, reply);
        case CMD_ENUM_TYPE_TEST:
            return SerStubEnumTypeTest(client, data, reply);
        default: {
            HDF_LOGE("%{public}s: not support cmd %{public}d", __func__, cmdId);
            return HDF_ERR_INVALID_PARAM;
        }
    }
}
