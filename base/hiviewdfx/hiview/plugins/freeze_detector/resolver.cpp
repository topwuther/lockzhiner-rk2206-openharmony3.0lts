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

#include "resolver.h"

#include <sys/time.h>

#include "common_defines.h"
#include "db_helper.h"
#include "faultlogger_client.h"
#include "file_util.h"
#include "logger.h"
#include "plugin.h"
//#include "smart_parser.h"
#include "string_util.h"
#include "sys_event.h"
#include "sys_event_dao.h"
#include "vendor.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_RELIABILITY_LOG_TAG("FreezeDetector");

FreezeResolver::FreezeResolver() : startTime_(time(nullptr) * MILLISECOND)
{
}

FreezeResolver::~FreezeResolver()
{
}

bool FreezeResolver::Init()
{
    // freeze_config.xml
    if (Vendor::GetInstance().Init() == false) {
        HIVIEW_LOGE("failed to init vendor.");
        return false;
    }

    // freeze_rules.xml
    if (FreezeRuleCluster::GetInstance().Init() == false) {
        HIVIEW_LOGE("failed to init rule.");
        return false;
    }
    return true;
}

std::shared_ptr<PipelineEvent> FreezeResolver::ProcessHardwareEvent() const
{
    return Vendor::GetInstance().ProcessHardwareEvent(startTime_);
}

std::shared_ptr<PipelineEvent> FreezeResolver::ProcessLongPressEvent() const
{
    if (Vendor::GetInstance().GetRebootReason() == "") {
        return nullptr;
    }

    auto event = std::make_shared<SysEvent>("FreezeResolver", nullptr, "");
    event->domain_ = DOMAIN_LONGPRESS;
    event->eventName_ = STRINGID_LONGPRESS;
    event->happenTime_ = startTime_;
    event->messageType_ = Event::MessageType::SYS_EVENT;
    event->SetEventValue(EventStore::EventCol::DOMAIN, DOMAIN_LONGPRESS);
    event->SetEventValue(EventStore::EventCol::NAME, STRINGID_LONGPRESS);
    event->SetEventValue(EventStore::EventCol::TYPE, Event::MessageType::SYS_EVENT);
    event->SetEventValue(EventStore::EventCol::TS, startTime_);
    event->SetEventValue(EventStore::EventCol::TZ, GetTimeZone()); // +0800
    event->SetEventValue(FreezeDetectorPlugin::EVENT_PID, "0");
    event->SetEventValue(FreezeDetectorPlugin::EVENT_UID, "0");
    event->SetEventValue(FreezeDetectorPlugin::EVENT_PACKAGE_NAME, STRINGID_LONGPRESS);
    event->SetEventValue(FreezeDetectorPlugin::EVENT_PROCESS_NAME, STRINGID_LONGPRESS);
    event->SetEventValue(FreezeDetectorPlugin::EVENT_MSG, STRINGID_LONGPRESS);
    return event;
}

bool FreezeResolver::ResolveLongPressEvent() const
{
    if (Vendor::GetInstance().GetRebootReason() != "") {
        WatchPoint matchedWatchPoint;
        WatchPoint watchPoint = WatchPoint::Builder().InitDomain(DOMAIN_LONGPRESS)
            .InitStringId(STRINGID_LONGPRESS).InitTimestamp(time(nullptr) * MILLISECOND).Build();
        FreezeResult result;
        std::list<WatchPoint> list;
        if (ResolveEvent(watchPoint, matchedWatchPoint, list, result)) { // remove list size checking
            return true;
        }
    }
    return false;
}

bool FreezeResolver::ResolveEvent(WatchPoint& watchPoint, WatchPoint& matchedWatchPoint,
    std::list<WatchPoint>& list, FreezeResult& result) const
{
    unsigned long window = 0;
    if (FreezeRuleCluster::GetInstance().GetTimeWindow(watchPoint, window) == false) {
        return false;
    }

    if (Vendor::GetInstance().IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())) {
        if (window > DEFAULT_TIME_WINDOW) {
            window = DEFAULT_TIME_WINDOW;
        }
    }

    unsigned long start = watchPoint.GetTimestamp() - (window * MILLISECOND);
    unsigned long end = watchPoint.GetTimestamp();
    if (window == 0) {
        list.push_back(watchPoint);
    } else {
        DBHelper::SelectEventFromDB(false, start, end, list);
    }

    HIVIEW_LOGI("list size %{public}zu", list.size());
    return FreezeRuleCluster::GetInstance().GetResult(watchPoint, matchedWatchPoint, list, result);
}

std::shared_ptr<PipelineEvent> FreezeResolver::ProcessSystemEvent() const
{
    std::list<WatchPoint> list;
    DBHelper::SelectEventFromDB(true, startTime_ - REBOOT_TIME_WINDOW * MILLISECOND, startTime_, list);
    FreezeResult result = Vendor::GetInstance().MakeSystemResult();
    if (Vendor::GetInstance().ReduceRelevanceEvents(list, result) == false) {
        HIVIEW_LOGI("no relevance for default event");
        return nullptr;
    }

    if (Vendor::GetInstance().IsHardwareEvent(list.front().GetDomain(), list.front().GetStringId())) {
        result.SetScope(HARDWARE_SCOPE);
    }

    MergeEventLog(list.front(), list);

    HIVIEW_LOGI("system events size %{public}zu", list.size());
    WatchPoint dummyWatchPoint;
    return Vendor::GetInstance().MakeEvent(list.front(), dummyWatchPoint, list, result);
}

std::shared_ptr<PipelineEvent> FreezeResolver::ProcessEvent(WatchPoint &watchPoint) const
{
    HIVIEW_LOGI("process event [%{public}s, %{public}s]",
        watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());

    Vendor::GetInstance().UpdateHardwareEventLog(watchPoint);

    FreezeResult result;
    std::list<WatchPoint> list;
    WatchPoint matchedWatchPoint;
    if (ResolveEvent(watchPoint, matchedWatchPoint, list, result) == false) {
        HIVIEW_LOGI("no rule for event [%{public}s, %{public}s]",
            watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
        return nullptr;
    }

    if (Vendor::GetInstance().ReduceRelevanceEvents(list, result) == false) {
        HIVIEW_LOGI("no relevance for event [%{public}s, %{public}s]",
            watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
        return nullptr;
    }

    MergeEventLog(watchPoint, list);

    return Vendor::GetInstance().MakeEvent(watchPoint, matchedWatchPoint, list, result);
}

bool FreezeResolver::MergeEventLog(const WatchPoint &watchPoint, const std::list<WatchPoint>& list) const
{
    std::string domain = watchPoint.GetDomain();
    std::string stringId = watchPoint.GetStringId();
    std::string timestamp = GetTimeString(watchPoint.GetTimestamp());
    long pid = watchPoint.GetPid();
    long uid = watchPoint.GetUid();
    std::string packageName = watchPoint.GetPackageName();
    std::string processName = watchPoint.GetProcessName();
    std::string msg = watchPoint.GetMsg();

    std::string logPath;
    if (Vendor::GetInstance().IsApplicationEvent(domain, stringId)) {
        logPath = FAULTLOGGER_PATH + APPFREEZE + HYPHEN + packageName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
    }
    else {
        logPath = FAULTLOGGER_PATH + SYSFREEZE + HYPHEN + packageName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
    }

    std::ofstream output(logPath, std::ios::out);
    if (!output.is_open()) {
        HIVIEW_LOGE("cannot open log file for writing:%{public}s.\n", logPath.c_str());
        return false;
    }
    output << HEADER << std::endl;
    output << FreezeDetectorPlugin::EVENT_DOMAIN << FreezeDetectorPlugin::COLON << domain << std::endl;
    output << FreezeDetectorPlugin::EVENT_STRINGID << FreezeDetectorPlugin::COLON << stringId << std::endl;
    output << FreezeDetectorPlugin::EVENT_TIMESTAMP << FreezeDetectorPlugin::COLON <<
        watchPoint.GetTimestamp() << std::endl;
    output << FreezeDetectorPlugin::EVENT_PID << FreezeDetectorPlugin::COLON << pid << std::endl;
    output << FreezeDetectorPlugin::EVENT_UID << FreezeDetectorPlugin::COLON << uid << std::endl;
    output << FreezeDetectorPlugin::EVENT_PACKAGE_NAME << FreezeDetectorPlugin::COLON << packageName << std::endl;
    output << FreezeDetectorPlugin::EVENT_PROCESS_NAME << FreezeDetectorPlugin::COLON << processName << std::endl;
    output << FreezeDetectorPlugin::EVENT_MSG << FreezeDetectorPlugin::COLON << msg << std::endl;
    output.flush();
    output.close();

    HIVIEW_LOGI("merging list size %{public}zu", list.size());
    for (auto node : list) {
        std::string filePath = node.GetLogPath();
        HIVIEW_LOGI("merging file:%{public}s.\n", filePath.c_str());
        if (filePath == "" || filePath == "nolog" || FileUtil::FileExists(filePath) == false) {
            continue;
        }

        std::ifstream ifs(filePath, std::ios::in);
        if (!ifs.is_open()) {
            HIVIEW_LOGE("cannot open log file for reading:%{public}s.\n", filePath.c_str());
            continue;
        }

        std::ofstream ofs(logPath, std::ios::out | std::ios::app);
        if (!ofs.is_open()) {
            ifs.close();
            HIVIEW_LOGE("cannot open log file for writing:%{public}s.\n", logPath.c_str());
            continue;
        }

        ofs << HEADER << std::endl;
        ofs << ifs.rdbuf();

        ofs.flush();
        ofs.close();
        ifs.close();
    }

    //auto eventInfos = SmartParser::Analysis(logPath, DEFAULT_PATH, SP_APPFREEZE);
    //std::string summary = eventInfos[SP_ENDSTACK];

    FaultLogInfoInner info;
    info.time = watchPoint.GetTimestamp() / MILLISECOND;
    info.id = uid;
    info.pid = pid;
    info.faultLogType = FaultLogType::APP_FREEZE;
    info.module = packageName;
    info.reason = stringId;
    //info.summary = summary;
    info.summary = stringId;
    info.logPath = logPath;
    AddFaultLog(info);

    return true;
}

std::string FreezeResolver::GetTimeString(unsigned long timestamp) const
{
    struct tm tm;
    time_t ts;
    ts = timestamp / MILLISECOND; // ms
    localtime_r(&ts, &tm);
    char buf[TIME_STRING_LEN] = {0};

    strftime(buf, TIME_STRING_LEN - 1, "%Y%m%d%H%M%S", &tm);
    return std::string(buf, strlen(buf));
}

std::string FreezeResolver::GetTimeZone() const
{
    std::string timeZone = "";
    struct timeval tv;
    struct timezone tz;
    if (gettimeofday(&tv, &tz) != 0) {
        HIVIEW_LOGE("failed to gettimeofday");
        return timeZone;
    }

    int hour = (-tz.tz_minuteswest) / MINUTES_IN_HOUR;
    timeZone = (hour >= 0) ? "+" : "-";

    int absHour = std::abs(hour);
    if (absHour < 10) {
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absHour));

    int minute = (-tz.tz_minuteswest) % MINUTES_IN_HOUR;
    int absMinute = std::abs(minute);
    if (absMinute < 10) {
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absMinute));

    return timeZone;
}
} // namespace HiviewDFX
} // namespace OHOS
