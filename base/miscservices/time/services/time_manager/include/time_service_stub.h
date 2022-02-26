/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_TIME_SERVICE_STUB_H
#define SERVICES_INCLUDE_TIME_SERVICE_STUB_H

#include "iremote_stub.h"
#include "time_service_interface.h"
#include "itimer_call_back.h"
#include "ipc_skeleton.h"
#include <map>

namespace OHOS {
namespace MiscServices {
class TimeServiceStub : public IRemoteStub<ITimeService> {
public:
    TimeServiceStub();
    ~TimeServiceStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using TimeServiceFunc = int32_t (TimeServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetTime(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetTimeZone(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetTimeZone(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallTimeMs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallTimeNs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetBootTimeMs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetBootTimeNs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetMonotonicTimeMs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetMonotonicTimeNs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetThreadTimeMs(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetThreadTimeNs(MessageParcel &data, MessageParcel &reply);

    int32_t OnCreateTimer(MessageParcel &data, MessageParcel &reply);
    int32_t OnStartTimer(MessageParcel &data, MessageParcel &reply);
    int32_t OnStopTimer(MessageParcel &data, MessageParcel &reply);
    int32_t OnDestoryTimer(MessageParcel &data, MessageParcel &reply);
    
    std::map<uint32_t, TimeServiceFunc> memberFuncMap_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // SERVICES_INCLUDE_TIME_SERVICE_STUB_H