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

#include "hiappevent_read.h"

#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>

#include "hiappevent_base.h"
#include "hilog/log.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
    constexpr HiLogLabel LABEL = {LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_read"};
    constexpr int COUNT_NOT_SET = -1;
    constexpr int FORMAT_TZ_SIZE = 9;
    constexpr int MAX_LOG_COUNT = 1000;
    constexpr int MILLI_TO_MICRO = 1000;
    // same as definition in hiappevent_write.cpp
    constexpr char APP_EVENT_DIR[] = "/hiappevent/";
    // Date format of time stamp: YYYYmmdd, eg. 20140510
    constexpr char DATE_FORMAT[] = "%Y%m%d";
    // the default value of the beginTimestamp/endTimeStamp
    // is -1LL
    constexpr TimeStampVarType INVALID_TIMESTAMP = -1LL;
    // The count of the regex matched results is always more
    // than one and all results except the first one are the matched
    // results we want, so the total count is 2 and 1 is the right
    // index
    constexpr smatch::size_type REGEX_MATCHED_RESULTS_COUNT = 2;
    constexpr smatch::size_type REGEX_MATCHED_RESULT_INDEX = 1;
    // Pattern for parsing the timestamp in the name of log files
    // eg. 20140510
    const regex TIME_STAMP_REGEX_PATTERN(".*_(.{8})\\..*");
    // Pattern for reading the timestamp from the
    // persisted log record, eg. 1626266996728
    const regex MATCHED_LOG_REGEX_PATTERN(".*\"time_\":([0-9]{13}).*");
} // __UNIQUE_NAME_

LogAssistant& LogAssistant::Instance()
{
    static LogAssistant assistant;
    return assistant;
}

LogAssistant::LogAssistant()
    : logPersistDir(""),
    realTimeLogUpdateListener(nullptr),
    historyLogPulledListener(nullptr)
{}

LogAssistant::~LogAssistant()
{
    this->RemoveAllListeners();
}

// A util method to find the matched result by the input
// pattern
string LogAssistant::FindMatchedRegex(const string& origin, const regex& regex)
{
    smatch regexMatchedResult;
    if (std::regex_match(origin, regexMatchedResult, regex)) {
        if (regexMatchedResult.size() == REGEX_MATCHED_RESULTS_COUNT) {
            return regexMatchedResult[REGEX_MATCHED_RESULT_INDEX].str();
        }
    }
    return string();
}

// Use regex to parse the timestamp from each persisted event log record
string LogAssistant::ParseLogLineTime(const string& line)
{
    return FindMatchedRegex(line, MATCHED_LOG_REGEX_PATTERN);
}

bool LogAssistant::CheckMatchedLogLine(const string& logTimeStamp,
    TimeStampVarType beginTimeStamp, TimeStampVarType endTimeStamp)
{
    // Comparing the timestamp value in the persited log record with
    // the timestamp from libjvmtiagent calling
    return ((beginTimeStamp == INVALID_TIMESTAMP) ||
        logTimeStamp >= to_string(beginTimeStamp)) &&
        ((endTimeStamp == INVALID_TIMESTAMP) ||
        logTimeStamp <= to_string(endTimeStamp));
}

// Read the single log file and then find out all matched event record
void LogAssistant::ParseSingFileLogs(vector<string>& logs,
    const string& fileName, TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp, int count)
{
    ifstream fin(logPersistDir + string(APP_EVENT_DIR) + fileName);
    if (!fin) {
        HiLog::Error(LABEL, "single log file read failed.");
        return;
    }
    string currentLine;
    vector<string> currentFileLogs;
    while (fin >> currentLine) {
        string currentTimeStamp = ParseLogLineTime(currentLine);
        if (CheckMatchedLogLine(currentTimeStamp, beginTimeStamp, endTimeStamp)) {
            currentFileLogs.emplace_back(currentLine);
        }
    }
    int logsSize = static_cast<int>(logs.size());
    int currentFileLogSize = static_cast<int>(currentFileLogs.size());
    if ((logsSize + currentFileLogSize) > count) {
        logs.insert(logs.end(), currentFileLogs.rbegin(), currentFileLogs.rbegin() +
            count - logsSize);
    } else {
        logs.insert(logs.end(), currentFileLogs.rbegin(), currentFileLogs.rend());
    }
}

void LogAssistant::ParseAllHistoryLogs(vector<string>& logs,
    const vector<string>& logFiles, TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp, int count)
{
    logs.clear();
    // Read the matched log files one by one
    for (auto &logFileName : logFiles) {
        ParseSingFileLogs(logs, logFileName, beginTimeStamp,
            endTimeStamp, count);
        int logSize = static_cast<int>(logs.size());
        if (logSize >= count) {
            break;
        }
    }
}

string LogAssistant::ParseLogFileTimeStamp(const string& fileName)
{
    return FindMatchedRegex(fileName, TIME_STAMP_REGEX_PATTERN);
}

string LogAssistant::TranslateLongToFormattedTimeStamp(TimeStampVarType timeStamp)
{
    time_t ftt = (time_t)(timeStamp / MILLI_TO_MICRO);
    struct tm* ptm = localtime(&ftt);
    if (ptm == nullptr) {
        return string();
    }
    char formatTz[FORMAT_TZ_SIZE] = {0};
    if (strftime(formatTz, sizeof(formatTz), DATE_FORMAT, ptm) == 0) {
        return string();
    }
    return string(formatTz);
}

/**
 * Check whether the log file contains logs whose timestamp is between beginTimeStamp
 * and endTimeStamp or not
 */
bool LogAssistant::IsMatchedLogFile(const string& fileName, TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp)
{
    string logFileTimeStamp = ParseLogFileTimeStamp(fileName);
    if (logFileTimeStamp.empty()) {
        // The name of this log file isn't standard, so we ignore it directly
        return false;
    }
    if (beginTimeStamp == INVALID_TIMESTAMP && endTimeStamp == INVALID_TIMESTAMP) {
        return true;
    } else if (beginTimeStamp == INVALID_TIMESTAMP) {
        return logFileTimeStamp <= TranslateLongToFormattedTimeStamp(endTimeStamp);
    } else if (endTimeStamp == INVALID_TIMESTAMP) {
        return logFileTimeStamp >= TranslateLongToFormattedTimeStamp(beginTimeStamp);
    } else {
        return logFileTimeStamp >= TranslateLongToFormattedTimeStamp(beginTimeStamp) &&
            logFileTimeStamp <= TranslateLongToFormattedTimeStamp(endTimeStamp);
    }
}

/**
 * Find out all macthed log files by comparing the parsed timestamp from file name with
 * the beginTimeStamp/endTimeStamp
 */
void LogAssistant::AllMatchedLogFiles(vector<string>& logFiles, TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp)
{
    logFiles.clear();
    DIR* dir = opendir((logPersistDir + APP_EVENT_DIR).c_str());
    if (dir == nullptr) {
        HiLog::Error(LABEL, "log persisted directory opened failed.");
        return;
    }
    struct dirent* ent;
    while ((ent = readdir(dir))) {
        if (IsMatchedLogFile(std::string(ent->d_name), beginTimeStamp, endTimeStamp)) {
            logFiles.emplace_back(ent->d_name);
        }
    }
    if (!logFiles.empty()) {
        // Sort all the matched log files in descending order
        sort(logFiles.begin(),
            logFiles.end(),
            [](string file1, string file2)->bool {
                return file1 > file2;
            });
    }
    closedir(dir);
}

void LogAssistant::ReadHistoryLogFromPersistFile(vector<string>& historyLogs,
    TimeStampVarType beginTimeStamp, TimeStampVarType endTimeStamp,
    int count)
{
    historyLogs.clear();
    if (!logPersistDir.empty()) {
        vector<string> logFiles;
        AllMatchedLogFiles(logFiles, beginTimeStamp, endTimeStamp);
        if (logFiles.size() > 0) {
            ParseAllHistoryLogs(historyLogs, logFiles, beginTimeStamp,
                endTimeStamp, count);
        }
    }
}

void LogAssistant::UpdateHiAppEventLogDir(const std::string& dir)
{
    logPersistDir = dir;
}

void LogAssistant::PullEventHistoryLog(TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp, int count)
{
    if (count == COUNT_NOT_SET) {
        count = MAX_LOG_COUNT;
    }
    vector<string> historyLogs;
    historyLogs.reserve(count);
    HiLog::Debug(LABEL, "log capacity set to %{public}d", count);
    ReadHistoryLogFromPersistFile(historyLogs, beginTimeStamp, endTimeStamp,
        count);
    // Sort all the matched logs in ascending order
    sort(historyLogs.begin(),
        historyLogs.end(),
        [](string log1, string log2)->bool {
            return log1 < log2;
        });
    if (historyLogPulledListener != nullptr) {
        historyLogPulledListener(historyLogs);
    }
}

void LogAssistant::RegRealTimeAppLogListener(RealTimeEventLogListener listener)
{
    realTimeLogUpdateListener = listener;
}

void LogAssistant::RegHistoryAppLogListener(HistoryEventLogListener listener)
{
    historyLogPulledListener = listener;
}

void LogAssistant::RemoveAllListeners()
{
    realTimeLogUpdateListener = nullptr;
    historyLogPulledListener = nullptr;
}

void LogAssistant::RealTimeAppLogUpdate(const string& realTimeLog)
{
    if (realTimeLogUpdateListener != nullptr) {
        realTimeLogUpdateListener(realTimeLog);
    }
}
} // HiviewDFX
} // OHOS

void RegRealTimeAppLogListener(RealTimeEventLogListener listener)
{
    OHOS::HiviewDFX::LogAssistant::Instance().RegRealTimeAppLogListener(listener);
}

void RegHistoryAppLogListener(HistoryEventLogListener listener)
{
    OHOS::HiviewDFX::LogAssistant::Instance().RegHistoryAppLogListener(listener);
}

void RemoveAllListeners()
{
    OHOS::HiviewDFX::LogAssistant::Instance().RemoveAllListeners();
}

void RealTimeAppLogUpdate(const string& realTimeLog)
{
    OHOS::HiviewDFX::LogAssistant::Instance().RealTimeAppLogUpdate(realTimeLog);
}

void UpdateHiAppEventLogDir(const string& path)
{
    OHOS::HiviewDFX::LogAssistant::Instance().UpdateHiAppEventLogDir(path);
}

void PullEventHistoryLog(TimeStampVarType beginTimeStamp,
    TimeStampVarType endTimeStamp, int count)
{
    OHOS::HiviewDFX::LogAssistant::Instance().PullEventHistoryLog(beginTimeStamp,
        endTimeStamp, count);
}