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

#include "sample_proxy.h"
#include <hdf_base.h>
#include <message_parcel.h>

namespace OHOS {
namespace HDI {
namespace Sample {
namespace V1_0 {
int32_t SampleProxy::BooleanTypeTest(const bool input, bool& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteBool(input)) {
        HDF_LOGE("%{public}s: write bool input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_BOOLEAN_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadBool();

    return HDF_SUCCESS;
}

int32_t SampleProxy::ByteTypeTest(const int8_t input, int8_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInt8(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_BYTE_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadInt8();

    return HDF_SUCCESS;
}

int32_t SampleProxy::ShortTypeTest(const int16_t input, int16_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInt16(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_SHORT_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadInt16();

    return HDF_SUCCESS;
}

int32_t SampleProxy::IntTypeTest(const int32_t input, int32_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInt32(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_INT_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadInt32();

    return HDF_SUCCESS;
}

int32_t SampleProxy::LongTypeTest(const int64_t input, int64_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInt64(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_LONG_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadInt64();

    return HDF_SUCCESS;
}

int32_t SampleProxy::FloatTypeTest(const float input, float& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteFloat(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_FLOAT_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadFloat();

    return HDF_SUCCESS;
}

int32_t SampleProxy::DoubleTypeTest(const double input, double& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteDouble(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_DOUBLE_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadDouble();

    return HDF_SUCCESS;
}

int32_t SampleProxy::StringTypeTest(const std::string& input, std::string& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteString(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_STRING_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadString();

    return HDF_SUCCESS;
}

int32_t SampleProxy::UcharTypeTest(const uint8_t input, uint8_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint8(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_UCHAR_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadUint8();

    return HDF_SUCCESS;
}

int32_t SampleProxy::UshortTypeTest(const uint16_t input, uint16_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint16(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_UCHAR_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadUint16();

    return HDF_SUCCESS;
}

int32_t SampleProxy::UintTypeTest(const uint32_t input, uint32_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_UINT_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadUint32();

    return HDF_SUCCESS;
}

int32_t SampleProxy::UlongTypeTest(const uint64_t input, uint64_t& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint64(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_ULONG_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = reply.ReadUint64();

    return HDF_SUCCESS;
}

int32_t SampleProxy::ListTypeTest(const std::list<int8_t>& input, std::list<int8_t>& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(input.size())) {
        HDF_LOGE("%{public}s: write input size failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it : input) {
        if (!data.WriteUint8(it)) {
            HDF_LOGE("%{public}s: write input data failed!", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }

    int32_t ec = Remote()->SendRequest(CMD_LIST_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    uint32_t outSize = reply.ReadUint32();
    for (uint32_t i = 0; i < outSize; i++) {
        uint8_t curData = reply.ReadUint8();
        output.push_back(curData);
    }

    return HDF_SUCCESS;
}

int32_t SampleProxy::MapTypeTest(const std::map<int8_t, int8_t>& input, std::map<int8_t, int8_t>& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32(input.size())) {
        HDF_LOGE("%{public}s: write input size failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (auto it : input) {
        if (!data.WriteUint8(it.first)) {
            HDF_LOGE("%{public}s: write input data->first failed!", __func__);
            return HDF_ERR_INVALID_PARAM;
        }

        if (!data.WriteUint8(it.second)) {
            HDF_LOGE("%{public}s: write input data->second failed!", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
    }

    int32_t ec = Remote()->SendRequest(CMD_MAP_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    uint32_t outSize = reply.ReadUint32();
    for (uint32_t i = 0; i < outSize; i++) {
        uint8_t key = reply.ReadUint8();
        uint8_t val = reply.ReadUint8();
        output[key] = val;
    }

    return HDF_SUCCESS;
}


int32_t SampleProxy::ArrayTypeTest(const std::vector<int8_t>& input, std::vector<int8_t>& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInt8Vector(input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_ARRAY_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    reply.ReadInt8Vector(&output);

    return HDF_SUCCESS;
}

int32_t SampleProxy::StructTypeTest(const StructSample& input, StructSample& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteBuffer((void *)&input, sizeof(StructSample))) {
        HDF_LOGE("%{public}s: write input data failed", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_STRUCT_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }

    StructSample *result = (StructSample *)data.ReadBuffer(sizeof(StructSample));
    if (result == nullptr) {
        HDF_LOGE("%{public}s: read output failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    output.first = result->first;
    output.second = result->second;
    return HDF_SUCCESS;
}

int32_t SampleProxy::EnumTypeTest(const EnumSample& input, EnumSample& output)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteUint32((uint32_t)input)) {
        HDF_LOGE("%{public}s: write input failed!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t ec = Remote()->SendRequest(CMD_ENUM_TYPE_TEST, data, reply, option);
    if (ec != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: SendRequest failed, error code is %{public}d", __func__, ec);
        return ec;
    }
    output = (EnumSample)reply.ReadUint32();

    return HDF_SUCCESS;
}
}  // namespace V1_0
}  // namespace Sample
}  // namespace HDI
}  // namespace OHOS