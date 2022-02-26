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

#ifndef HDI_SAMPLE_SERVICE_STUB_CPP_INF_H
#define HDI_SAMPLE_SERVICE_STUB_CPP_INF_H

#include <message_parcel.h>
#include <message_option.h>
#include <refbase.h>
#include "sample_service.h"

namespace OHOS {
namespace HDI {
namespace Sample {
namespace V1_0 {
class SampleServiceStub {
public:
    SampleServiceStub(){}
    virtual ~SampleServiceStub(){}

    int32_t SampleStubBooleanTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubByteTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubShortTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option)  const;
    int32_t SampleStubIntTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubLongTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubFloatTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubDoubleTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubStringTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubUcharTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubUshortTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubUintTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubUlongTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubListTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubMapTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubArrayTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubStructTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;
    int32_t SampleStubEnumTypeTest(MessageParcel& data, MessageParcel& reply, MessageOption& option) const;

    int32_t SampleServiceStubOnRemoteRequest(int cmdId, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) const;
private:
    SampleService service;
};
}  // namespace V1_0
}  // namespace Sample
}  // namespace HDI
}  // namespace OHOS

void *SampleStubInstance();

void SampleStubRelease(void *obj);

int32_t SampleServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf& data, struct HdfSBuf& reply);

#endif // HDI_SAMPLE_SERVICE_STUB_CPP_INF_H