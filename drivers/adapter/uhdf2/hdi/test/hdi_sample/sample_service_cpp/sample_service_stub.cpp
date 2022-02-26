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

#include "sample_service_stub.h"
#include <hdf_base.h>
#include <hdf_log.h>
#include <hdf_sbuf_ipc.h>

namespace OHOS {
namespace HDI {
namespace Sample {
namespace V1_0 {
int32_t SampleServiceStub::SampleStubBooleanTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const bool input = data.ReadBool();
    bool output = false;

    int32_t ec = service.BooleanTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteBool(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubByteTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const int8_t input = data.ReadInt8();
    int8_t output;

    int32_t ec = service.ByteTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteInt8(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubShortTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const int16_t input = data.ReadInt16();
    int16_t output;

    int32_t ec = service.ShortTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteInt16(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubIntTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const int32_t input = data.ReadInt32();
    int32_t output;

    int32_t ec = service.IntTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteInt32(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubLongTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const int64_t input = data.ReadInt64();
    int64_t output;

    int32_t ec = service.LongTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteInt64(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubFloatTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const float input = data.ReadFloat();
    float output;

    int32_t ec = service.FloatTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteFloat(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubDoubleTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const double input = data.ReadDouble();
    double output;

    int32_t ec = service.DoubleTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteDouble(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubStringTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const std::string input = data.ReadString();
    std::string output;

    int32_t ec = service.StringTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteString(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubUcharTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint8_t input = data.ReadUint8();
    uint8_t output;

    int32_t ec = service.UcharTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint8(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubUshortTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint16_t input = data.ReadUint16();
    uint16_t output;

    int32_t ec = service.UshortTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint16(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubUintTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint32_t input = data.ReadUint32();
    uint32_t output;

    int32_t ec = service.UintTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint32(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubUlongTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint64_t input = data.ReadUint64();
    uint64_t output;

    int32_t ec = service.UlongTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint64(output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubListTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint32_t inSize = data.ReadUint32();
    std::list<int8_t> input;
    std::list<int8_t> output;

    for (uint32_t i = 0; i < inSize; i++) {
        input.push_back(data.ReadInt8());
    }

    int32_t ec = service.ListTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint32(output.size())) {
        HDF_LOGE("%{public}s: write result size failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it : output) {
        if (!reply.WriteInt8(it)) {
            HDF_LOGE("%{public}s: write result failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubMapTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint32_t inSize = data.ReadUint32();
    std::map<int8_t, int8_t> input;
    std::map<int8_t, int8_t> output;

    for (uint32_t i = 0; i < inSize; i++) {
        int8_t key = data.ReadInt8();
        int8_t val = data.ReadInt8();
        input[key] = val;
    }

    int32_t ec = service.MapTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint32(output.size())) {
        HDF_LOGE("%{public}s: write result size failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it : output) {
        if (!reply.WriteInt8(it.first) || !reply.WriteInt8(it.second)) {
            HDF_LOGE("%{public}s: write result size failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubArrayTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    const uint32_t inSize = data.ReadUint32();
    std::vector<int8_t> input;
    std::vector<int8_t> output;

    for (uint32_t i = 0; i < inSize; i++) {
        input.push_back(data.ReadInt8());
    }

    int32_t ec = service.ArrayTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint32(output.size())) {
        HDF_LOGE("%{public}s: write result size failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it : output) {
        if (!reply.WriteInt8(it)) {
            HDF_LOGE("%{public}s: write result failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubStructTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    StructSample *input = (StructSample *)data.ReadBuffer(sizeof(StructSample));
    if (input == nullptr) {
        HDF_LOGE("%{public}s: read struct data failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    StructSample output;

    int32_t ec = service.StructTypeTest(*input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteBuffer((void *)&output, sizeof(StructSample))) {
        HDF_LOGE("%{public}s: write output data failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleStubEnumTypeTest(MessageParcel& data,
    MessageParcel& reply, MessageOption& option) const
{
    EnumSample input = (EnumSample)data.ReadUint32();
    EnumSample output;

    int32_t ec = service.EnumTypeTest(input, output);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: call failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    if (!reply.WriteUint32((uint32_t)output)) {
        HDF_LOGE("%{public}s: write result failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    return HDF_SUCCESS;
}

int32_t SampleServiceStub::SampleServiceStubOnRemoteRequest(int cmdId,
    MessageParcel& data, MessageParcel& reply, MessageOption& option) const
{
    switch (cmdId) {
        case CMD_BOOLEAN_TYPE_TEST:
            return SampleStubBooleanTypeTest(data, reply, option);
        case CMD_BYTE_TYPE_TEST:
            return SampleStubByteTypeTest(data, reply, option);
        case CMD_SHORT_TYPE_TEST:
            return SampleStubShortTypeTest(data, reply, option);
        case CMD_INT_TYPE_TEST:
            return SampleStubIntTypeTest(data, reply, option);
        case CMD_LONG_TYPE_TEST:
            return SampleStubLongTypeTest(data, reply, option);
        case CMD_FLOAT_TYPE_TEST:
            return SampleStubFloatTypeTest(data, reply, option);
        case CMD_DOUBLE_TYPE_TEST:
            return SampleStubDoubleTypeTest(data, reply, option);
        case CMD_STRING_TYPE_TEST:
            return SampleStubStringTypeTest(data, reply, option);
        case CMD_UCHAR_TYPE_TEST:
            return SampleStubUcharTypeTest(data, reply, option);
        case CMD_USHORT_TYPE_TEST:
            return SampleStubUshortTypeTest(data, reply, option);
        case CMD_UINT_TYPE_TEST:
            return SampleStubUintTypeTest(data, reply, option);
        case CMD_ULONG_TYPE_TEST:
            return SampleStubUlongTypeTest(data, reply, option);
        case CMD_LIST_TYPE_TEST:
            return SampleStubListTypeTest(data, reply, option);
        case CMD_MAP_TYPE_TEST:
            return SampleStubMapTypeTest(data, reply, option);
        case CMD_ARRAY_TYPE_TEST:
            return SampleStubArrayTypeTest(data, reply, option);
        case CMD_STRUCT_TYPE_TEST:
            return SampleStubStructTypeTest(data, reply, option);
        case CMD_ENUM_TYPE_TEST:
            return SampleStubEnumTypeTest(data, reply, option);
        default: {
            HDF_LOGE("%{public}s: not support cmd %{public}d", __func__, cmdId);
            return HDF_ERR_INVALID_PARAM;
        }
    }
    return HDF_SUCCESS;
}
}  // namespace V1_0
}  // namespace Sample
}  // namespace HDI
}  // namespace OHOS

void *SampleStubInstance()
{
    using namespace OHOS::HDI::Sample::V1_0;
    return reinterpret_cast<void *>(new SampleServiceStub());
}

void SampleStubRelease(void *obj)
{
    using namespace OHOS::HDI::Sample::V1_0;
    delete reinterpret_cast<SampleServiceStub *>(obj);
}

int32_t SampleServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf& data, struct HdfSBuf& reply)
{
    using namespace OHOS::HDI::Sample::V1_0;
    SampleServiceStub *sampleStub = reinterpret_cast<SampleServiceStub *>(stub);
    OHOS::MessageParcel *dataParcel = nullptr;
    OHOS::MessageParcel *replyParcel = nullptr;

    (void)SbufToParcel(&reply, &replyParcel);
    if (SbufToParcel(&data, &dataParcel) != HDF_SUCCESS) {
        HDF_LOGE("%{public}s:invalid data sbuf object to dispatch", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    OHOS::MessageOption option;
    return sampleStub->SampleServiceStubOnRemoteRequest(cmdId, *dataParcel, *replyParcel, option);
}
