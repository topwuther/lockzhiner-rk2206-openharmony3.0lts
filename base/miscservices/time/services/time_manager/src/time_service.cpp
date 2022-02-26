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
#include <ctime>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <cerrno>
#include <mutex>
#include <dirent.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

#include "pthread.h"
#include "time_service.h"
#include "time_zone_info.h" 
#include "time_common.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

namespace OHOS {
namespace MiscServices {
namespace {
// Unit of measure conversion , BASE: second
static const int MILLI_TO_BASE = 1000LL;
static const int MICR_TO_BASE = 1000000LL;
static const int NANO_TO_BASE = 1000000000LL;
static const int FIVE_THOUSANDS = 5000LL;
static const std::int32_t INIT_INTERVAL = 10000L;
static const uint32_t TIMER_TYPE_REALTIME_MASK = 1 << 0;
static const uint32_t TIMER_TYPE_REALTIME_WAKEUP_MASK = 1 << 1;
static const uint32_t TIMER_TYPE_EXACT_MASK = 1 << 2;
constexpr int MIN_TRIGGER_TIMES = 5000;
constexpr int MILLI_TO_MICR = MICR_TO_BASE / MILLI_TO_BASE;
constexpr int NANO_TO_MILLI = NANO_TO_BASE / MILLI_TO_BASE;
}

REGISTER_SYSTEM_ABILITY_BY_ID(TimeService, TIME_SERVICE_ID, true);

std::mutex TimeService::instanceLock_;
sptr<TimeService> TimeService::instance_;
std::shared_ptr<AppExecFwk::EventHandler> TimeService::serviceHandler_ = nullptr;
std::shared_ptr<MiscServices::TimeServiceNotify> TimeService::timeServiceNotify_  = nullptr;
std::shared_ptr<TimerManager> TimeService::timerManagerHandler_  = nullptr;

TimeService::TimeService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), 
      state_(ServiceRunningState::STATE_NOT_START), 
      rtc_id(get_wall_clock_rtc_id())
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " TimeService Start.");
}

TimeService::TimeService()
    :state_(ServiceRunningState::STATE_NOT_START), rtc_id(get_wall_clock_rtc_id())
{
}

TimeService::~TimeService() {};

sptr<TimeService> TimeService::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new TimeService;
        }
    }
    return instance_;
}

void TimeService::OnStart()
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " TimeService OnStart.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " TimeService is already running.");
        return;
    }

    InitServiceHandler();
    InitTimerHandler();
    InitNotifyHandler();
    DelayedSingleton<TimeZoneInfo>::GetInstance()->Init();
    if (Init() != ERR_OK) {
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        TIME_HILOGE(TIME_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }

    TIME_HILOGI(TIME_MODULE_SERVICE, "Start TimeService success."); 
    return;
}

int32_t TimeService::Init()
{
    bool ret = Publish(TimeService::GetInstance());
    if (!ret) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Init Failed.");
        return E_TIME_PUBLISH_FAIL;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Init Success.");
    state_ = ServiceRunningState::STATE_RUNNING;
    return ERR_OK;
}

void TimeService::OnStop()
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "OnStop Started.");
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    serviceHandler_ = nullptr;
    timeServiceNotify_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
    TIME_HILOGI(TIME_MODULE_SERVICE, "OnStop End.");
}

void TimeService::InitNotifyHandler()
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "InitNotify started.");
    if (timeServiceNotify_ != nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " Already init.");
        return;
    }
    timeServiceNotify_ = std::make_shared<MiscServices::TimeServiceNotify>();
    timeServiceNotify_->RegisterPublishEvents();
}

void TimeService::InitServiceHandler()
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " Already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(TIME_SERVICE_NAME);
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    
    TIME_HILOGI(TIME_MODULE_SERVICE, "InitServiceHandler Succeeded.");
}

void TimeService::InitTimerHandler()
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "Init Timer started.");
    if (timerManagerHandler_ != nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " Already init.");
        return;
    }
    timerManagerHandler_ = TimerManager::Create();
}

void TimeService::PaserTimerPara(int32_t type, bool repeat, uint64_t interval, TimerPara &paras)
{
    bool isRealtime = false;
    bool isWakeup = false;
    auto uIntType = static_cast<uint32_t>(type);
    paras.flag = 0;
    if ((uIntType & TIMER_TYPE_REALTIME_MASK) > 0) {
        isRealtime = true;
    }
    if ((uIntType & TIMER_TYPE_REALTIME_WAKEUP_MASK) > 0) {
        isWakeup = true;
    }
    if ((uIntType & TIMER_TYPE_EXACT_MASK) > 0) {
        paras.windowLength = 0;
    } else {
        paras.windowLength = -1;
    }

    if (isRealtime && isWakeup) {
        paras.timerType = ITimerManager::TimerType::ELAPSED_REALTIME_WAKEUP;
    } else if (isRealtime) {
        paras.timerType = ITimerManager::TimerType::ELAPSED_REALTIME;
    } else if (isWakeup) {
        paras.timerType = ITimerManager::TimerType::RTC_WAKEUP;
    } else {
        paras.timerType = ITimerManager::TimerType::RTC;
    }
    if (repeat) {
        paras.interval =  (interval < FIVE_THOUSANDS) ? FIVE_THOUSANDS : interval;
    } else {
        paras.interval = 0;
    }
    return;
}

uint64_t TimeService::CreateTimer(int32_t type, bool repeat, uint64_t interval, 
    sptr<IRemoteObject> &obj)
{
    if (obj == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Input nullptr.");
        return 0;
    }
    struct TimerPara paras {};
    PaserTimerPara(type, repeat, interval, paras);
    sptr<ITimerCallback> timerCallback = iface_cast<ITimerCallback>(obj);
    if (timerCallback == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "ITimerCallback nullptr.");
        return 0;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Start create timer.");
    auto callbackFunc = [timerCallback](uint64_t id) {
        timerCallback->NotifyTimer(id);
    };
    
    if (timerManagerHandler_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Timer manager nullptr.");
        timerManagerHandler_ = TimerManager::Create();
        if (timerManagerHandler_ == nullptr) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "Redo Timer manager Init Failed.");
            return 0;
        }
    }
    return timerManagerHandler_->CreateTimer(paras.timerType,
                                             paras.windowLength,
                                             paras.interval,
                                             paras.flag,
                                             callbackFunc,
                                             0);
}

bool TimeService::StartTimer(uint64_t timerId, uint64_t triggerTimes) 
{
    uint64_t triggerTimesIn;
    triggerTimesIn = (triggerTimes < MIN_TRIGGER_TIMES) ? MIN_TRIGGER_TIMES : triggerTimes;
    if (timerManagerHandler_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Timer manager nullptr.");
        timerManagerHandler_ = TimerManager::Create();
        if (timerManagerHandler_ == nullptr) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "Redo Timer manager Init Failed.");
            return 0;
        }
    }
    auto ret = timerManagerHandler_->StartTimer(timerId, triggerTimesIn);
    if (!ret) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "TimerId Not found.");
    }
    return ret;
}

bool TimeService::StopTimer(uint64_t  timerId) 
{
    if (timerManagerHandler_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Timer manager nullptr.");
        timerManagerHandler_ = TimerManager::Create();
        if (timerManagerHandler_ == nullptr) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "Redo Timer manager Init Failed.");
            return 0;
        }
    }
    auto ret = timerManagerHandler_->StopTimer(timerId);
    if (!ret) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "TimerId Not found.");
    }
    return ret;
}

bool TimeService::DestroyTimer(uint64_t  timerId) 
{
    if (timerManagerHandler_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Timer manager nullptr.");
        timerManagerHandler_ = TimerManager::Create();
        if (timerManagerHandler_ == nullptr) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "Redo Timer manager Init Failed.");
            return 0;
        }
    }
    auto ret = timerManagerHandler_->DestroyTimer(timerId);
    if (!ret) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "TimerId Not found.");
    }
    return ret;
}

int32_t TimeService::SetTime(const int64_t time)
{
    pid_t uid = IPCSkeleton::GetCallingUid();
    auto hasPerm = DelayedSingleton<TimePermission>::GetInstance()->CheckCallingPermission(uid, setTimePermName_);
    if (!hasPerm) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Permission check failed, uid : %{public}d", uid);
        return E_TIME_NO_PERMISSION;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Setting time of day to milliseconds: %{public}" PRId64 "", time);
    if (time < 0 || time / 1000LL >= LONG_MAX) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "input param error");
        return E_TIME_PARAMETERS_INVALID;
    }
    struct timeval tv {};
    tv.tv_sec = (time_t) (time / MILLI_TO_BASE);
    tv.tv_usec = (suseconds_t)((time % MILLI_TO_BASE) * MILLI_TO_MICR);

    int result = settimeofday(&tv, NULL);
    if (result < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "settimeofday fail: %{public}d.", result);
        return E_TIME_DEAL_FAILED;
    }
    auto ret = set_rtc_time(tv.tv_sec);
    if (ret < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "set rtc fail: %{public}d.", ret);
        return E_TIME_SET_RTC_FAILED;
    }
    int64_t currentTime = 0;
    GetWallTimeMs(currentTime);
    if (timeServiceNotify_ != nullptr) {
        timeServiceNotify_->PublishTimeChanageEvents(currentTime);
    }
    
    return  ERR_OK;
}

int TimeService::set_rtc_time(time_t sec) {
    struct rtc_time rtc {};
    struct tm tm {}; 
    struct tm *gmtime_res = nullptr;
    int fd = 0;
    int res;
    if (rtc_id < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "invalid rtc id: %{public}s:", strerror(ENODEV));
        return -1;
    }
    std::stringstream strs;
    strs << "/dev/rtc" << rtc_id;
    auto rtc_dev = strs.str();
    TIME_HILOGI(TIME_MODULE_SERVICE, "rtc_dev : %{public}s:", rtc_dev.data());
    auto rtc_data = rtc_dev.data();
    fd = open(rtc_data, O_RDWR);
    if (fd < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "open failed %{public}s: %{public}s", rtc_dev.data(), strerror(errno));
        return -1;
    }

    gmtime_res = gmtime_r(&sec, &tm);
    if (gmtime_res) {
        rtc.tm_sec = tm.tm_sec;
        rtc.tm_min = tm.tm_min;
        rtc.tm_hour = tm.tm_hour;
        rtc.tm_mday = tm.tm_mday;
        rtc.tm_mon = tm.tm_mon;
        rtc.tm_year = tm.tm_year;
        rtc.tm_wday = tm.tm_wday;
        rtc.tm_yday = tm.tm_yday;
        rtc.tm_isdst = tm.tm_isdst;
        res = ioctl(fd, RTC_SET_TIME, &rtc);
        if (res < 0) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "ioctl RTC_SET_TIME failed: %{public}s", strerror(errno));
        }
    } else {
        TIME_HILOGE(TIME_MODULE_SERVICE, "convert rtc time failed: %{public}s", strerror(errno));
        res = -1;
    }
    close(fd);
    return res;
}

bool TimeService::check_rtc(std::string rtc_path, uint64_t rtc_id_t)
{
    std::stringstream strs;
    strs << rtc_path << "/rtc" << rtc_id_t << "/hctosys";
    auto hctosys_path = strs.str();

    uint32_t hctosys;
    std::fstream file(hctosys_path.data(), std::ios_base::in);
    if (file.is_open()) {
        file >> hctosys;
    } else {
        TIME_HILOGE(TIME_MODULE_SERVICE, "failed to open %{public}s", hctosys_path.data());
        return false;
    }
    return true;
}

int TimeService::get_wall_clock_rtc_id()
{
    std::string rtc_path = "/sys/class/rtc";

    std::unique_ptr<DIR, int(*)(DIR*)> dir(opendir(rtc_path.c_str()), closedir);
    if (!dir.get()) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "failed to open %{public}s: %{public}s", rtc_path.c_str(), strerror(errno));
        return -1;
    }

    struct dirent *dirent;
    std::string s = "rtc";
    while (errno = 0,
           dirent = readdir(dir.get())) {
        
        std::string name(dirent->d_name);
        unsigned long rtc_id_t = 0;
        auto index = name.find(s);
        if (index == std::string::npos) {
            continue;
        } else {
            auto rtc_id_str = name.substr(index + s.length());
            rtc_id_t = std::stoul(rtc_id_str);
        }
       
        if (check_rtc(rtc_path, rtc_id_t)) {
            TIME_HILOGD(TIME_MODULE_SERVICE, "found wall clock rtc %{public}ld", rtc_id_t);
            return rtc_id_t;
        }
    }

    if (errno == 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "no wall clock rtc found");
    } else {
        TIME_HILOGE(TIME_MODULE_SERVICE, "failed to check rtc: %{public}s", strerror(errno));
    }
    return -1;
}

int32_t TimeService::SetTimeZone(const std::string timeZoneId)
{
    pid_t uid = IPCSkeleton::GetCallingUid();
    auto hasPerm = DelayedSingleton<TimePermission>::GetInstance()->CheckCallingPermission(uid, setTimezonePermName_);
    if (!hasPerm) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Permission check failed, uid : %{public}d", uid);
        return E_TIME_NO_PERMISSION;
    }
    if (!DelayedSingleton<TimeZoneInfo>::GetInstance()->SetTimezone(timeZoneId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Set timezone failed :%{public}s", timeZoneId.c_str());
        return E_TIME_DEAL_FAILED;
    }
    int64_t currentTime = 0;
    GetWallTimeMs(currentTime);
    if (timeServiceNotify_ != nullptr) {
        timeServiceNotify_->PublishTimeZoneChangeEvents(currentTime);
    }
    return ERR_OK;
}

int32_t TimeService::GetTimeZone(std::string &timeZoneId)
{
    if (!DelayedSingleton<TimeZoneInfo>::GetInstance()->GetTimezone(timeZoneId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "get timezone failed.");
        return E_TIME_DEAL_FAILED;
    }
    TIME_HILOGD(TIME_MODULE_SERVICE, "Current timezone : %{public}s", timeZoneId.c_str());
    return ERR_OK;
}

int32_t TimeService::GetWallTimeMs(int64_t &times)
{   
    struct timespec tv {};
    
    if (GetTimeByClockid(CLOCK_REALTIME, &tv)) {
        times = tv.tv_sec * MILLI_TO_BASE + tv.tv_nsec / NANO_TO_MILLI;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetWallTimeNs(int64_t &times)
{   
    struct timespec tv {};

    if (GetTimeByClockid(CLOCK_REALTIME, &tv)) {
        times = tv.tv_sec * NANO_TO_BASE + tv.tv_nsec;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetBootTimeMs(int64_t &times)
{   
    struct timespec tv {};

    if (GetTimeByClockid(CLOCK_BOOTTIME, &tv)) {
        times = tv.tv_sec * MILLI_TO_BASE + tv.tv_nsec / NANO_TO_MILLI;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetBootTimeNs(int64_t &times)
{   
    struct timespec tv {};

    if (GetTimeByClockid(CLOCK_BOOTTIME, &tv)) {
        times = tv.tv_sec * NANO_TO_BASE + tv.tv_nsec;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetMonotonicTimeMs(int64_t &times)
{   
    struct timespec tv {};

    if (GetTimeByClockid(CLOCK_MONOTONIC, &tv)) {
        times = tv.tv_sec * MILLI_TO_BASE + tv.tv_nsec / NANO_TO_MILLI;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetMonotonicTimeNs(int64_t &times)
{   
    struct timespec tv {};

    if (GetTimeByClockid(CLOCK_MONOTONIC, &tv)) {
        times = tv.tv_sec * NANO_TO_BASE + tv.tv_nsec;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetThreadTimeMs(int64_t &times)
{   
    struct timespec tv {};
    int ret;
    clockid_t cid;
    ret = pthread_getcpuclockid(pthread_self(), &cid);
    if (ret != 0) {
        return E_TIME_PARAMETERS_INVALID;
    }

    if (GetTimeByClockid(cid, &tv)) {
        times = tv.tv_sec * MILLI_TO_BASE + tv.tv_nsec / NANO_TO_MILLI;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

int32_t TimeService::GetThreadTimeNs(int64_t &times)
{   
    struct timespec tv {};
    int ret;
    clockid_t cid;
    ret = pthread_getcpuclockid(pthread_self(), &cid);
    if (ret != 0) {
        return E_TIME_PARAMETERS_INVALID;
    }
               
    if (GetTimeByClockid(cid, &tv)) {
        times = tv.tv_sec * NANO_TO_BASE + tv.tv_nsec;
        return ERR_OK;
    }
    return  E_TIME_DEAL_FAILED;
}

bool TimeService::GetTimeByClockid(clockid_t clk_id, struct timespec *tv)
{
    if (clock_gettime(clk_id, tv) < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Failed clock_gettime.");
        return false;
    }
    return true;
}

} // namespace MiscServices
} // namespace OHOS
