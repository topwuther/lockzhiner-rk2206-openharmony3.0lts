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

#ifndef SERVICES_INCLUDE_TIME_SERVICES_H
#define SERVICES_INCLUDE_TIME_SERVICES_H

#include "time_service_stub.h"
#include "time_service_notify.h"
#include "timer_manager.h"
#include "time_permission.h"
#include "system_ability.h"
#include "event_handler.h"
#include "time.h"
#include "securec.h"
#include <mutex>
#include <inttypes.h>


namespace OHOS {
namespace MiscServices {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class TimeService : public SystemAbility, public TimeServiceStub {
    DECLARE_SYSTEM_ABILITY(TimeService);
public:
    DISALLOW_COPY_AND_MOVE(TimeService);
    TimeService(int32_t systemAbilityId, bool runOnCreate);
    TimeService();
    ~TimeService();
    static sptr<TimeService> GetInstance();
    int32_t SetTime(const int64_t time) override;
    int32_t SetTimeZone(const std::string timezoneId) override;
    int32_t GetTimeZone(std::string &timezoneId) override;
    int32_t GetWallTimeMs(int64_t &times) override;
    int32_t GetWallTimeNs(int64_t &times) override;
    int32_t GetBootTimeMs(int64_t &times) override;
    int32_t GetBootTimeNs(int64_t &times) override;
    int32_t GetMonotonicTimeMs(int64_t &times) override;
    int32_t GetMonotonicTimeNs(int64_t &times) override;
    int32_t GetThreadTimeMs(int64_t &times) override;
    int32_t GetThreadTimeNs(int64_t &times) override;

    uint64_t CreateTimer(int32_t type, bool repeat, uint64_t interval, sptr<IRemoteObject> &timerCallback) override;
    bool StartTimer(uint64_t timerId, uint64_t triggerTime) override;
    bool StopTimer(uint64_t  timerId) override;
    bool DestroyTimer(uint64_t  timerId) override;
    
protected:
    void OnStart() override;
    void OnStop() override;

private:
    struct TimerPara {
        int timerType;
        int64_t windowLength;
        uint64_t interval;
        int flag;
    };
    int32_t Init();
    void InitServiceHandler();
    void InitNotifyHandler();
    void InitTimeZone();
    void InitTimerHandler();
    void PaserTimerPara(int32_t type, bool repeat, uint64_t interval, TimerPara &paras);
    bool GetTimeByClockid(clockid_t clockID, struct timespec* tv);
    int set_rtc_time(time_t sec);

    bool check_rtc(std::string rtc_path, uint64_t rtc_id);
    int get_wall_clock_rtc_id();

    ServiceRunningState state_;
    static std::mutex instanceLock_;
    static sptr<TimeService> instance_;
    const std::string setTimePermName_ = "ohos.permission.SET_TIME";
    const std::string setTimezonePermName_ = "ohos.permission.SET_TIME_ZONE";
    const int rtc_id;
    static std::shared_ptr<MiscServices::TimeServiceNotify> timeServiceNotify_;
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    static std::shared_ptr<TimerManager> timerManagerHandler_;
};
} // MiscServices
} // OHOS
#endif // SERVICES_INCLUDE_TIME_SERVICES_H
