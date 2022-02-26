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

#ifndef SERVICES_INCLUDE_TIME_SERVICE_PROXY_H
#define SERVICES_INCLUDE_TIME_SERVICE_PROXY_H

#include "iremote_proxy.h"
#include "time_service_interface.h"

namespace OHOS {
namespace MiscServices {
class TimeServiceProxy : public IRemoteProxy<ITimeService> {
public:
    explicit TimeServiceProxy(const sptr<IRemoteObject> &object);
    ~TimeServiceProxy() = default;
    DISALLOW_COPY_AND_MOVE(TimeServiceProxy);

    int32_t SetTime(const int64_t time) override;
    int32_t SetTimeZone(const std::string timeZoneId) override;
    int32_t GetTimeZone(std::string &timeZoneId) override;
    int32_t GetWallTimeMs(int64_t &times) override;
    int32_t GetWallTimeNs(int64_t &times) override;
    int32_t GetBootTimeMs(int64_t &times) override;
    int32_t GetBootTimeNs(int64_t &times) override;
    int32_t GetMonotonicTimeMs(int64_t &times) override;
    int32_t GetMonotonicTimeNs(int64_t &times) override;
    int32_t GetThreadTimeMs(int64_t &times) override;
    int32_t GetThreadTimeNs(int64_t &times) override;
    uint64_t CreateTimer(int32_t type, bool repeat, uint64_t interval, sptr<IRemoteObject> &timerCallback) override;
    bool StartTimer(uint64_t timerId, uint64_t treggerTime) override;
    bool StopTimer(uint64_t  timerId) override;
    bool DestroyTimer(uint64_t  timerId) override;

private:
    static inline BrokerDelegator<TimeServiceProxy> delegator_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_TIME_SERVICE_PROXY_H