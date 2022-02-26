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

#include "sample_service.h"
#include <hdf_log.h>

namespace OHOS {
namespace HDI {
namespace Sample {
namespace V1_0 {
int32_t SampleService::BooleanTypeTest(const bool input, bool& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::ByteTypeTest(const int8_t input, int8_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::ShortTypeTest(const int16_t input, int16_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::IntTypeTest(const int32_t input, int32_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::LongTypeTest(const int64_t input, int64_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::FloatTypeTest(const float input, float& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::DoubleTypeTest(const double input, double& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::StringTypeTest(const std::string& input, std::string& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::UcharTypeTest(const uint8_t input, uint8_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::UshortTypeTest(const uint16_t input, uint16_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::UintTypeTest(const uint32_t input, uint32_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::UlongTypeTest(const uint64_t input, uint64_t& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::ListTypeTest(const std::list<int8_t>& input, std::list<int8_t>& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::MapTypeTest(const std::map<int8_t, int8_t>& input, std::map<int8_t, int8_t>& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::ArrayTypeTest(const std::vector<int8_t>& input, std::vector<int8_t>& output) const
{
    output = input;
    return 0;
}

int32_t SampleService::StructTypeTest(const StructSample& input, StructSample& output) const
{
    output.first = input.first;
    output.second = input.second;
    return 0;
}

int32_t SampleService::EnumTypeTest(const EnumSample& input, EnumSample& output) const
{
    output = input;
    return 0;
}
}  // namespace V1_0
}  // namespace Sample
}  // namespace HDI
}  // namespace OHOS