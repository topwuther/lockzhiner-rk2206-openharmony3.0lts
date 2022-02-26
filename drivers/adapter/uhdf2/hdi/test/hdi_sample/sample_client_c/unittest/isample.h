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

#ifndef HDI_SAMPLE_CLIENT_C_INF_H
#define HDI_SAMPLE_CLIENT_C_INF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct HdfRemoteService;

struct StructSample {
    int8_t first;
    int16_t second;
};

enum EnumSample {
    MEM_FIRST,
    MEM_SECOND,
    MEM_THIRD,
};

enum {
    CMD_BOOLEAN_TYPE_TEST,
    CMD_BYTE_TYPE_TEST,
    CMD_SHORT_TYPE_TEST,
    CMD_INT_TYPE_TEST,
    CMD_LONG_TYPE_TEST,
    CMD_FLOAT_TYPE_TEST,
    CMD_DOUBLE_TYPE_TEST,
    CMD_STRING_TYPE_TEST,
    CMD_UCHAR_TYPE_TEST,
    CMD_USHORT_TYPE_TEST,
    CMD_UINT_TYPE_TEST,
    CMD_ULONG_TYPE_TEST,
    CMD_LIST_TYPE_TEST,
    CMD_ARRAY_TYPE_TEST,
    CMD_STRUCT_TYPE_TEST,
    CMD_ENUM_TYPE_TEST,
};

struct ISample {
    struct HdfRemoteService *remote;

    int32_t (*BooleanTypeTest)(struct ISample *self, const bool input, bool *output);

    int32_t (*ByteTypeTest)(struct ISample *self, const int8_t input, int8_t *output);

    int32_t (*ShortTypeTest)(struct ISample *self, const int16_t input, int16_t *output);

    int32_t (*IntTypeTest)(struct ISample *self, const int32_t input, int32_t *output);

    int32_t (*LongTypeTest)(struct ISample *self, const int64_t input, int64_t *output);

    int32_t (*FloatTypeTest)(struct ISample *self, const float input, float *output);

    int32_t (*DoubleTypeTest)(struct ISample *self, const double input, double *output);

    int32_t (*StringTypeTest)(struct ISample *self, const char* input, char **output);

    int32_t (*UcharTypeTest)(struct ISample *self, const uint8_t input, uint8_t *output);

    int32_t (*UshortTypeTest)(struct ISample *self, const uint16_t input, uint16_t *output);

    int32_t (*UintTypeTest)(struct ISample *self, const uint32_t input, uint32_t *output);

    int32_t (*UlongTypeTest)(struct ISample *self, const uint64_t input, uint64_t *output);

    int32_t (*ListTypeTest)(struct ISample *self, const int8_t *input, const uint32_t inSize,
        int8_t **output, uint32_t *outSize);

    int32_t (*ArrayTypeTest)(struct ISample *self, const int8_t *input, const uint32_t inSize,
        int8_t **output, uint32_t *outSize);

    int32_t (*StructTypeTest)(struct ISample *self, const struct StructSample *input, struct StructSample *output);

    int32_t (*EnumTypeTest)(struct ISample *self, const enum EnumSample input, enum EnumSample *output);
};

struct ISample *HdiSampleGet(const char *serviceName);

void HdiSampleRelease(struct ISample *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // HDI_SAMPLE_CLIENT_C_INF_H