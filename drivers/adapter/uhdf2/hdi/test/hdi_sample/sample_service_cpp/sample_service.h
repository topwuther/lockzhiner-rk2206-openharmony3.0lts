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

#ifndef HDI_SAMPLE_SERVICE_CPP_INF_H
#define HDI_SAMPLE_SERVICE_CPP_INF_H

#include "isample.h"

namespace OHOS {
namespace HDI {
namespace Sample {
namespace V1_0 {
class SampleService : public ISample {
public:
    SampleService(){}
    virtual ~SampleService() override {}

    virtual int32_t BooleanTypeTest(const bool input, bool& output) const override;

    virtual int32_t ByteTypeTest(const int8_t input, int8_t& output) const override;

    virtual int32_t ShortTypeTest(const int16_t input, int16_t& output) const override;

    virtual int32_t IntTypeTest(const int32_t input, int32_t& output) const override;

    virtual int32_t LongTypeTest(const int64_t input, int64_t& output) const override;

    virtual int32_t FloatTypeTest(const float input, float& output) const override;

    virtual int32_t DoubleTypeTest(const double input, double& output) const override;

    virtual int32_t StringTypeTest(const std::string& input, std::string& output) const override;

    virtual int32_t UcharTypeTest(const uint8_t input, uint8_t& output) const override;

    virtual int32_t UshortTypeTest(const uint16_t input, uint16_t& output) const override;

    virtual int32_t UintTypeTest(const uint32_t input, uint32_t& output) const override;

    virtual int32_t UlongTypeTest(const uint64_t input, uint64_t& output) const override;

    virtual int32_t ListTypeTest(const std::list<int8_t>& input, std::list<int8_t>& output) const override;

    virtual int32_t MapTypeTest(const std::map<int8_t, int8_t>& input, std::map<int8_t, int8_t>& output) const override;

    virtual int32_t ArrayTypeTest(const std::vector<int8_t>& input, std::vector<int8_t>& output) const override;

    virtual int32_t StructTypeTest(const StructSample& input, StructSample& output) const override;

    virtual int32_t EnumTypeTest(const EnumSample& input, EnumSample& output) const override;
};
}  // namespace V1_0
}  // namespace Sample
}  // namespace HDI
}  // namespace OHOS

#endif // HDI_SAMPLE_SERVICE_CPP_INF_H
