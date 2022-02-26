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

#ifndef HI_APP_EVENT_READ_H
#define HI_APP_EVENT_READ_H

#include <regex>
#include <string>
#include <vector>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Value type of the timestamp, timestamp is always translated
 * into milliseconds, eg. 1626266996728
 * WARNING: long long type is always 64 bits in any platform
 */
using TimeStampVarType = const long long;

// Function type of the listener which observe on any update of
// the hiappevent real time log
using RealTimeEventLogListener = void(*)(const std::string&);

// Function type of the listener which observe on the pull of the hiappevent
// history log
using HistoryEventLogListener = void(*)(const std::vector<std::string>&);

/**
 * A interface method for libjvmtiagent to call to set the unique
 * listener which observe on any update of the hiappevent real
 * time log
 * WARNING: Any modification on the name of this method ISN'T permitted
 */
void RegRealTimeAppLogListener(RealTimeEventLogListener listener);

/**
 * A interface method for libjvmtiagent to call to set the unique
 * listener which observe on the pull of the hiappevent history
 * log
 * WARNING: Any modification on the name of this method ISN'T permitted
 */
void RegHistoryAppLogListener(HistoryEventLogListener listener);

/**
 * A interaface method for libjvmtiagent to call to remove all listeners
 * WARNING: Any modification on the name of this method ISN'T permitted
 */
void RemoveAllListeners();

// This method would always be called when a new real time hiappevent
// log record is persisted(hiappevent_write.cpp)
void RealTimeAppLogUpdate(const std::string& realTimeLog);

// This method would always be called when the directory for the
// hiappevent log file to persist is setted.(hiappevent_config.cpp)
void UpdateHiAppEventLogDir(const std::string& logPersistDir);

/**
 * A interface method for libjvmtiagent to call to pull hiappevent history logs
 * WARNING: Any modification on the name of this method ISN'T permitted.
 */
void PullEventHistoryLog(TimeStampVarType beginTimeStamp, TimeStampVarType endTimeStamp,
    int count);

namespace OHOS {
namespace HiviewDFX {
class LogAssistant final {
public:
    static LogAssistant& Instance();
public:
    void RegRealTimeAppLogListener(RealTimeEventLogListener);
    void RegHistoryAppLogListener(HistoryEventLogListener);
    void RemoveAllListeners();
    void RealTimeAppLogUpdate(const std::string&);
    void UpdateHiAppEventLogDir(const std::string&);
    void PullEventHistoryLog(TimeStampVarType, TimeStampVarType, int);
private:
    LogAssistant();
    LogAssistant(const LogAssistant&) = delete;
    LogAssistant(const LogAssistant&&) = delete;
    LogAssistant& operator=(const LogAssistant&) = delete;
    LogAssistant& operator=(const LogAssistant&&) = delete;
    ~LogAssistant();
private:
    std::string FindMatchedRegex(const std::string&, const std::regex&);
    std::string ParseLogLineTime(const std::string&);
    bool CheckMatchedLogLine(const std::string&, TimeStampVarType, TimeStampVarType);
    void ParseSingFileLogs(std::vector<std::string>&, const std::string&,
        TimeStampVarType, TimeStampVarType, int);
    void ParseAllHistoryLogs(std::vector<std::string>&, const std::vector<std::string>&,
        TimeStampVarType, TimeStampVarType, int);
    std::string ParseLogFileTimeStamp(const std::string&);
    std::string TranslateLongToFormattedTimeStamp(TimeStampVarType);
    bool IsMatchedLogFile(const std::string&, TimeStampVarType,
        TimeStampVarType);
    void AllMatchedLogFiles(std::vector<std::string>&, TimeStampVarType,
        TimeStampVarType);
    void ReadHistoryLogFromPersistFile(std::vector<std::string>&, TimeStampVarType,
        TimeStampVarType, int);
private:
    std::string logPersistDir;
    // Listener which observe on any update of the hiappevent
    // real time log
    RealTimeEventLogListener realTimeLogUpdateListener;
    // Listener which observe on the pull of the hiappevent
    // history log
    HistoryEventLogListener historyLogPulledListener;
};
} // HiviewDFX
} // OHOS

#ifdef __cplusplus
}
#endif

#endif // HI_APP_EVENT_WRITE_H