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
#include <hdf_log.h>
#include <osal_mem.h>
#include <securec.h>

#include "isample.h"

static int32_t HdiBooleanTypeTestImpl(struct HdfDeviceObject *device, const bool input, bool *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiByteTypeTestImpl(struct HdfDeviceObject *device, const int8_t input, int8_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiShortTypeTestImpl(struct HdfDeviceObject *device, const int16_t input, int16_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiIntTypeTestImpl(struct HdfDeviceObject *device, const int32_t input, int32_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiLongTypeTestImpl(struct HdfDeviceObject *device, const int64_t input, int64_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiFloatTypeTestImpl(struct HdfDeviceObject *device, const float input, float *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiDoubleTypeTestImpl(struct HdfDeviceObject *device, const double input, double *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiStringTypeTestImpl(struct HdfDeviceObject *device, const char *input, char **output)
{
    *output = strdup(input);
    return HDF_SUCCESS;
}

static int32_t HdiUcharTypeTestImpl(struct HdfDeviceObject *device, const uint8_t input, uint8_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiUshortTypeTestImpl(struct HdfDeviceObject *device, const uint16_t input, uint16_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiUintTypeTestImpl(struct HdfDeviceObject *device, const uint32_t input, uint32_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiUlongTypeTestImpl(struct HdfDeviceObject *device, const uint64_t input, uint64_t *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static int32_t HdiListTypeTestImpl(struct HdfDeviceObject *device, const int8_t *input, const uint32_t inSize,
    int8_t **output, uint32_t *outSize)
{
    int8_t *result = (int8_t*)OsalMemAlloc(sizeof(int8_t) * inSize);
    if (result == NULL) {
        HDF_LOGE("%{public}s: result OsalMemAlloc failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    for (uint32_t i = 0; i < inSize; i++) {
        result[i] = input[i];
    }

    *output = result;
    *outSize = inSize;
    return HDF_SUCCESS;
}

static int32_t HdiArrayTypeTestImpl(struct HdfDeviceObject *device, const int8_t *input, const uint32_t inSize,
    int8_t **output, uint32_t *outSize)
{
    int8_t *result = (int8_t*)OsalMemAlloc(sizeof(int8_t) * inSize);
    if (result == NULL) {
        HDF_LOGE("%{public}s: result OsalMemAlloc failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    for (uint32_t i = 0; i < inSize; i++) {
        result[i] = input[i];
    }

    *output = result;
    *outSize = inSize;
    return HDF_SUCCESS;
}

static int32_t HdiStructTypeTestImpl(struct HdfDeviceObject *device, const struct StructSample *input,
    struct StructSample *output)
{
    output->first = input->first;
    output->second = input->second;
    return HDF_SUCCESS;
}

static int32_t HdiEnumTypeTestImpl(struct HdfDeviceObject *device, const enum EnumSample input, enum EnumSample *output)
{
    *output = input;
    return HDF_SUCCESS;
}

static const struct ISample g_hdiSampleImpl = {
    .BooleanTypeTest = HdiBooleanTypeTestImpl,
    .ByteTypeTest = HdiByteTypeTestImpl,
    .ShortTypeTest = HdiShortTypeTestImpl,
    .IntTypeTest = HdiIntTypeTestImpl,
    .LongTypeTest = HdiLongTypeTestImpl,
    .FloatTypeTest = HdiFloatTypeTestImpl,
    .DoubleTypeTest = HdiDoubleTypeTestImpl,
    .StringTypeTest = HdiStringTypeTestImpl,
    .UcharTypeTest = HdiUcharTypeTestImpl,
    .UshortTypeTest = HdiUshortTypeTestImpl,
    .UintTypeTest = HdiUintTypeTestImpl,
    .UlongTypeTest = HdiUlongTypeTestImpl,
    .ListTypeTest = HdiListTypeTestImpl,
    .ArrayTypeTest = HdiArrayTypeTestImpl,
    .StructTypeTest = HdiStructTypeTestImpl,
    .EnumTypeTest = HdiEnumTypeTestImpl,
};

const struct ISample *HdiSampleImplInstance()
{
    return &g_hdiSampleImpl;
}
