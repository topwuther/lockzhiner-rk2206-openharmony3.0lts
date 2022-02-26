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

#include <string.h>

#include <hdf_base.h>
#include <hdf_log.h>
#include <hdf_sbuf.h>
#include <osal_mem.h>
#include <servmgr_hdi.h>

#include "isample.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int32_t SampleProxyCall(struct ISample *self,
    int32_t id, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    if (self->remote == NULL || self->remote->dispatcher == NULL ||
        self->remote->dispatcher->Dispatch == NULL) {
            HDF_LOGE("%{public}s: obj is null", __func__);
            return HDF_ERR_INVALID_OBJECT;
    }
    return self->remote->dispatcher->Dispatch(self->remote, id, data, reply);
}

static int32_t SampleProxyBooleanTypeTest(struct ISample *self, const bool input, bool *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        HDF_LOGE("%{public}s: HdfSubf malloc failed!", __func__);
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }
    
    if (!HdfSbufWriteUint32(data, (uint32_t)input)) {
        HDF_LOGE("%{public}s: write bool input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_BOOLEAN_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint32(reply, (uint32_t *)output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyByteTypeTest(struct ISample *self, const int8_t input, int8_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteInt8(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_BYTE_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadInt8(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyShortTypeTest(struct ISample *self, const int16_t input, int16_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteInt16(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_SHORT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadInt16(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyIntTypeTest(struct ISample *self, const int32_t input, int32_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteInt32(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_INT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadInt32(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyLongTypeTest(struct ISample *self, const int64_t input, int64_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteInt64(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_LONG_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadInt64(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyFloatTypeTest(struct ISample *self, const float input, float *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteFloat(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_FLOAT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadFloat(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }

    return ec;
}

static int32_t SampleProxyDoubleTypeTest(struct ISample *self, const double input, double *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteDouble(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_DOUBLE_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadDouble(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }

    return ec;
}

static int32_t SampleProxyStringTypeTest(struct ISample *self, const char* input, char **output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL || input == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteString(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_STRING_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    const char *result = HdfSbufReadString(reply);
    if (result == NULL) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }
    *output = strdup(result);

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyUcharTypeTest(struct ISample *self, const uint8_t input, uint8_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint8(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_UCHAR_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint8(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyUshortTypeTest(struct ISample *self, const uint16_t input, uint16_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint16(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_USHORT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint16(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyUintTypeTest(struct ISample *self, const uint32_t input, uint32_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint32(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_UINT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint32(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyUlongTypeTest(struct ISample *self, const uint64_t input, uint64_t *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint64(data, input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_ULONG_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint64(reply, output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyListTypeTest(struct ISample *self, const int8_t *input, const uint32_t inSize,
    int8_t **output, uint32_t *outSize)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL || input == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint32(data, inSize)) {
        HDF_LOGE("%{public}s: write input size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    for (uint32_t i = 0; i < inSize; i++) {
        if (!HdfSbufWriteInt8(data, input[i])) {
            HDF_LOGE("%{public}s: write input[%{public}d] failed!", __func__, i);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

    ec = SampleProxyCall(self, CMD_LIST_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint32(reply, outSize)) {
        HDF_LOGE("%{public}s: read output size failed!", __func__);
        goto finished;
    }

    if (*outSize == 0) {
        HDF_LOGE("%{public}s: read outSize = %{public}d", __func__, *outSize);
        goto finished;
    }

    int8_t *result = (int8_t *)OsalMemAlloc(sizeof(int8_t) * (*outSize));
    if (result == NULL) {
        HDF_LOGE("%{public}s: malloc output failed!", __func__);
        goto finished;
    }

    for (uint32_t i = 0; i < *outSize; i++) {
        if (!HdfSbufReadInt8(reply, &(result[i]))) {
            HDF_LOGE("%{public}s: read output[%{public}d] failed!", __func__, i);
            OsalMemFree(result);
            goto finished;
        }
    }
    *output = result;

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyArrayTypeTest(struct ISample *self, const int8_t *input, const uint32_t inSize,
    int8_t **output, uint32_t *outSize)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL || input == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint32(data, inSize)) {
        HDF_LOGE("%{public}s: write input size failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    for (uint32_t i = 0; i < inSize; i++) {
        if (!HdfSbufWriteInt8(data, input[i])) {
            HDF_LOGE("%{public}s: write input[%{public}d] failed!", __func__, i);
            ec = HDF_ERR_INVALID_PARAM;
            goto finished;
        }
    }

    ec = SampleProxyCall(self, CMD_ARRAY_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint32(reply, outSize)) {
        HDF_LOGE("%{public}s: read output size failed!", __func__);
        goto finished;
    }

    if (*outSize == 0) {
        HDF_LOGE("%{public}s: read outSize = %{public}d", __func__, *outSize);
        goto finished;
    }

    int8_t *result = (int8_t *)OsalMemAlloc(sizeof(int8_t) * (*outSize));
    if (result == NULL) {
        HDF_LOGE("%{public}s: malloc output failed!", __func__);
        goto finished;
    }

    for (uint32_t i = 0; i < *outSize; i++) {
        if (!HdfSbufReadInt8(reply, &(result[i]))) {
            HDF_LOGE("%{public}s: read output[%{public}d] failed!", __func__, i);
            OsalMemFree(result);
            goto finished;
        }
    }
    *output = result;

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyStructTypeTest(struct ISample *self, const struct StructSample *input,
    struct StructSample *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteBuffer(data, (const void *)input, sizeof(struct StructSample))) {
        HDF_LOGE("%{public}s: write struct StructSample input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_STRUCT_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    struct StructSample *result = NULL;
    uint32_t outSize = 0;
    if (!HdfSbufReadBuffer(reply, (const void **)&result, &outSize)) {
        HDF_LOGE("%{public}s: read struct StructSample result failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    if (result == NULL || outSize != sizeof(struct StructSample)) {
        HDF_LOGE("%{public}s: result is error", __func__);
        ec = HDF_ERR_INVALID_PARAM;
    }
    output->first = result->first;
    output->second = result->second;

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static int32_t SampleProxyEnumTypeTest(struct ISample *self, const enum EnumSample input, enum EnumSample *output)
{
    int32_t ec = HDF_FAILURE;
    if (self == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);

    if (data == NULL || reply == NULL) {
        ec = HDF_ERR_MALLOC_FAIL;
        goto finished;
    }

    if (!HdfSbufWriteUint32(data, (uint32_t)input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        ec = HDF_ERR_INVALID_PARAM;
        goto finished;
    }

    ec = SampleProxyCall(self, CMD_ENUM_TYPE_TEST, data, reply);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed! error code is %{public}d", __func__, ec);
        goto finished;
    }

    if (!HdfSbufReadUint32(reply, (uint32_t *)output)) {
        HDF_LOGE("%{public}s: read result failed!", __func__);
        goto finished;
    }

finished:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ec;
}

static void SampleConstruct(struct ISample *inst)
{
    inst->BooleanTypeTest = SampleProxyBooleanTypeTest;
    inst->ByteTypeTest = SampleProxyByteTypeTest;
    inst->ShortTypeTest = SampleProxyShortTypeTest;
    inst->IntTypeTest = SampleProxyIntTypeTest;
    inst->LongTypeTest = SampleProxyLongTypeTest;
    inst->FloatTypeTest = SampleProxyFloatTypeTest;
    inst->DoubleTypeTest = SampleProxyDoubleTypeTest;
    inst->StringTypeTest = SampleProxyStringTypeTest;
    inst->UcharTypeTest = SampleProxyUcharTypeTest;
    inst->UshortTypeTest = SampleProxyUshortTypeTest;
    inst->UintTypeTest = SampleProxyUintTypeTest;
    inst->UlongTypeTest = SampleProxyUlongTypeTest;
    inst->ListTypeTest = SampleProxyListTypeTest;
    inst->ArrayTypeTest = SampleProxyArrayTypeTest;
    inst->StructTypeTest = SampleProxyStructTypeTest;
    inst->EnumTypeTest = SampleProxyEnumTypeTest;
}

struct ISample *HdiSampleGet(const char *serviceName)
{
    struct HDIServiceManager *serviceMgr = HDIServiceManagerGet();
    if (serviceMgr == NULL) {
        HDF_LOGE("%{public}s: HDIServiceManager not found!", __func__);
        return NULL;
    }

    struct HdfRemoteService *remote = serviceMgr->GetService(serviceMgr, serviceName);
    if (remote == NULL) {
        HDF_LOGE("%{public}s: HdfRemoteService not found!", __func__);
        return NULL;
    }

    struct ISample *sampleClient = (struct ISample *)OsalMemAlloc(sizeof(struct ISample));
    if (sampleClient == NULL) {
        HDF_LOGE("%{public}s: malloc sample instance failed!", __func__);
        HdfRemoteServiceRecycle(remote);
        return NULL;
    }

    sampleClient->remote = remote;
    SampleConstruct(sampleClient);
    return sampleClient;
}

void HdiSampleRelease(struct ISample *instance)
{
    if (instance == NULL) {
        return;
    }

    HdfRemoteServiceRecycle(instance->remote);
    OsalMemFree(instance);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */