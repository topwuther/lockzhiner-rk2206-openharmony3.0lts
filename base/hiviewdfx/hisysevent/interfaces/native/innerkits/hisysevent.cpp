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

#include "hisysevent.h"

#include <chrono>
#include <cinttypes>
#include <list>
#include <mutex>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include <securec.h>
#include "hilog/log.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
static constexpr int SUCCESS = 0;
static constexpr int ERR_EVENT_NAME_INVALID = -1;
static constexpr int ERR_DOMAIN_NAME_INVALID = -4;
static constexpr int ERR_DOES_NOT_INIT = -5;
static constexpr int ERR_OVER_SIZE = -7;
static constexpr int ERR_SEND_FAIL = -8;
static constexpr int ERR_KEY_NAME_INVALID = 1;
static constexpr int ERR_VALUE_LENGTH_TOO_LONG = 4;
static constexpr int ERR_KEY_NUMBER_TOO_MUCH = 5;
static constexpr int ERR_ARRAY_TOO_MUCH = 6;
static constexpr int SECS_IN_MINUTE = 60;
static constexpr unsigned int MAX_DOMAIN_LENGTH = 16;
static constexpr unsigned int MAX_EVENT_NAME_LENGTH = 32;
static constexpr unsigned int MAX_PARAM_NAME_LENGTH = 48;
static constexpr unsigned int MAX_ARRAY_SIZE = 100;
static constexpr unsigned int MAX_PARAM_NUMBER = 128;
static constexpr unsigned int MAX_STRING_LENGTH = 256 * 1024;
static constexpr unsigned int MAX_JSON_SIZE = 384 * 1024;

#ifdef USE_MUSL
static constexpr char SOCKET_FILE_DIR[] = "/dev/unix/socket/hisysevent";
#else
static constexpr char SOCKET_FILE_DIR[] = "/dev/socket/hisysevent";
#endif

static constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENT" };

static inline uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

static std::string GetTimeZone()
{
    struct timeval tv;
    struct timezone tz;
    if (gettimeofday(&tv, &tz) != 0) {
        HiLog::Error(LABEL, "can not get tz");
        return "";
    }
    int tzHour = (-tz.tz_minuteswest) / SECS_IN_MINUTE;
    std::string timeZone = (tzHour >= 0) ? "+" : "-";
    int absTzHour = std::abs(tzHour);
    if (absTzHour < 10) { // less than 10 hour
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absTzHour));

    int tzMin = (-tz.tz_minuteswest) % SECS_IN_MINUTE;
    int absTzMin = std::abs(tzMin);
    if (absTzMin < 10) { // less than 10 minute
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absTzMin));
    return timeZone;
}

static bool IsValidName(const std::string &value, unsigned int maxSize)
{
    if (value.empty()) {
        return false;
    }

    if (value.size() > maxSize) {
        return false;
    }

    for (unsigned int index = 0; index < value.size(); index++) {
        const char c = value.at(index);
        if ((c >= '0' && c <= '9') && (index > 0)) {
            continue;
        }
        if (c >= 'a' && c <= 'z') {
            continue;
        }
        if (c >= 'A' && c <= 'Z') {
            continue;
        }
        if ((c == '_') && (index > 0)) {
            continue;
        }
        return false;
    }
    return true;
}

static const char* GetMessageOfCode(int retCode)
{
    switch (retCode) {
        case SUCCESS:
            return "SUCCESS";
        case ERR_EVENT_NAME_INVALID:
            return "invalid event name";
        case ERR_DOMAIN_NAME_INVALID:
            return "invalid domain";
        case ERR_DOES_NOT_INIT:
            return "socket init error";
        case ERR_OVER_SIZE:
            return "over size";
        case ERR_SEND_FAIL:
            return "can not connect socket";
        case ERR_KEY_NAME_INVALID:
            return "key invalid";
        case ERR_VALUE_LENGTH_TOO_LONG:
            return "string value too length";
        case ERR_KEY_NUMBER_TOO_MUCH:
            return "more than 128 keys";
        case ERR_ARRAY_TOO_MUCH:
            return "more than 100 item";
        default:
            return "other reason";
    };
}

static std::string EscapeStringValue(const std::string &value)
{
    std::string escapeValue;
    for (auto it = value.begin(); it != value.end(); it++) {
        switch (*it) {
            case '\\':
                escapeValue.push_back('\\');
                escapeValue.push_back('\\');
                break;
            case '\"':
                escapeValue.push_back('\\');
                escapeValue.push_back('"');
                break;
            case '\b':
                escapeValue.push_back('\\');
                escapeValue.push_back('b');
                break;
            case '\f':
                escapeValue.push_back('\\');
                escapeValue.push_back('f');
                break;
            case '\n':
                escapeValue.push_back('\\');
                escapeValue.push_back('n');
                break;
            case '\r':
                escapeValue.push_back('\\');
                escapeValue.push_back('r');
                break;
            case '\t':
                escapeValue.push_back('\\');
                escapeValue.push_back('t');
                break;
            default:
                escapeValue.push_back(*it);
                break;
        }
    }
    return escapeValue;
}

static void InitRecvBuffer(int socketId)
{
    int oldN = 0;
    socklen_t oldOutSize = sizeof(int);
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&oldN), &oldOutSize) < 0) {
        HiLog::Error(LABEL, "get socket send buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int sendBuffSize = MAX_JSON_SIZE;
    if (setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&sendBuffSize), sizeof(int)) < 0) {
        HiLog::Error(LABEL, "set socket send buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int newN = 0;
    socklen_t newOutSize = sizeof(int);
    if (getsockopt(socketId, SOL_SOCKET, SO_SNDBUF, static_cast<void *>(&newN), &newOutSize) < 0) {
        HiLog::Error(LABEL, "get new socket send buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }
    HiLog::Debug(LABEL, "reset send buffer size old=%{public}d, new=%{public}d", oldN, newN);
}

static int SendToHiSysEventDataSource(const std::string &jsonStr)
{
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), SOCKET_FILE_DIR) != EOK) {
        HiLog::Error(LABEL, "can not assign server path");
        return ERR_DOES_NOT_INIT;
    }
    serverAddr.sun_path[sizeof(serverAddr.sun_path) - 1] = '\0';

    int socketId = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (socketId < 0) {
        HiLog::Error(LABEL, "create hisysevent client socket failed, error=%{public}d, msg=%{public}s",
            errno, strerror(errno));
        return ERR_DOES_NOT_INIT;
    }
    InitRecvBuffer(socketId);
    if (sendto(socketId, jsonStr.c_str(), jsonStr.size(), 0, reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)) < 0) {
        close(socketId);
        HiLog::Error(LABEL, "send data to hisysevent server failed, error=%{public}d, msg=%{public}s",
            errno, strerror(errno));
        return ERR_SEND_FAIL;
    }
    close(socketId);
    HiLog::Debug(LABEL, "HiSysEvent send data successful");
    return SUCCESS;
}

class FailDataQueue : public DelayedSingleton<FailDataQueue> {
public:
    void AddData(const std::string &jsonStr);
    void RetrySendFailedData();

    DECLARE_DELAYED_SINGLETON(FailDataQueue);
    DISALLOW_COPY_AND_MOVE(FailDataQueue);
private:
    std::size_t max_;
    std::mutex qmutex_;
    std::list<std::string> jsonStrs_;
};

FailDataQueue::FailDataQueue(): max_(10) // 10 the max size of list
{}

FailDataQueue::~FailDataQueue()
{}

void FailDataQueue::AddData(const std::string &jsonStr)
{
    std::lock_guard<std::mutex> lock(qmutex_);
    if (jsonStrs_.size() >= max_) {
        HiLog::Info(LABEL, "dispatch retry sysevent data as reach max size");
        jsonStrs_.pop_front();
    }
    jsonStrs_.push_back(jsonStr);
}

void FailDataQueue::RetrySendFailedData()
{
    std::lock_guard<std::mutex> lock(qmutex_);
    while (!jsonStrs_.empty()) {
        std::string jsonStr = jsonStrs_.front();
        HiLog::Debug(LABEL, "resend data size=%{public}lu, sysevent=%{public}s",
            static_cast<unsigned long>(jsonStr.size()), jsonStr.c_str());
        if (SendToHiSysEventDataSource(jsonStr) != SUCCESS) {
            return;
        }
        jsonStrs_.pop_front();
    }
}

int HiSysEvent::CheckDomain(HiSysEvent::EventBase &eventBase)
{
    if (!IsValidName(eventBase.domain_, MAX_DOMAIN_LENGTH)) {
        return ERR_DOMAIN_NAME_INVALID;
    }
    return SUCCESS;
}

int HiSysEvent::CheckEventName(HiSysEvent::EventBase &eventBase)
{
    if (!IsValidName(eventBase.eventName_, MAX_EVENT_NAME_LENGTH)) {
        return ERR_EVENT_NAME_INVALID;
    }
    return SUCCESS;
}

int HiSysEvent::CheckKey(const std::string &key)
{
    if (!IsValidName(key, MAX_PARAM_NAME_LENGTH)) {
        return ERR_KEY_NAME_INVALID;
    }
    return SUCCESS;
}

int HiSysEvent::CheckValue(const std::string &value)
{
    if (value.size() > MAX_STRING_LENGTH) {
        return ERR_VALUE_LENGTH_TOO_LONG;
    }
    return SUCCESS;
}

int HiSysEvent::CheckArraySize(unsigned long size)
{
    if (size > MAX_ARRAY_SIZE) {
        return ERR_ARRAY_TOO_MUCH;
    }
    return SUCCESS;
}

unsigned int HiSysEvent::GetArrayMax()
{
    return MAX_ARRAY_SIZE;
}

void HiSysEvent::ExplainRetCode(HiSysEvent::EventBase &eventBase)
{
    if (eventBase.retCode_ > SUCCESS) {
        HiLog::Warn(LABEL, "some value of param discard as invalid data, error=%{public}d, message=%{public}s",
            eventBase.retCode_, GetMessageOfCode(eventBase.retCode_));
    } else if (eventBase.retCode_ < SUCCESS) {
        HiLog::Error(LABEL, "discard data, error=%{public}d, message=%{public}s",
            eventBase.retCode_, GetMessageOfCode(eventBase.retCode_));
    }
}

bool HiSysEvent::IsError(HiSysEvent::EventBase &eventBase)
{
    if (eventBase.retCode_ < SUCCESS) {
        return true;
    }
    return false;
}

bool HiSysEvent::IsErrorAndUpdate(int retCode, HiSysEvent::EventBase &eventBase)
{
    if (retCode < SUCCESS) {
        eventBase.retCode_ = retCode;
        return true;
    }
    return false;
}

bool HiSysEvent::IsWarnAndUpdate(int retCode, EventBase &eventBase)
{
    if (retCode != SUCCESS) {
        eventBase.retCode_ = retCode;
        return true;
    }
    return false;
}

bool HiSysEvent::UpdateAndCheckKeyNumIsOver(HiSysEvent::EventBase &eventBase, bool isDefKey)
{
    if (!isDefKey) {
        return false;
    }
    eventBase.keyCnt_++;
    if (eventBase.keyCnt_ > MAX_PARAM_NUMBER) {
        eventBase.retCode_ = ERR_KEY_NUMBER_TOO_MUCH;
        return true;
    }
    return false;
}

void HiSysEvent::AppendHexData(HiSysEvent::EventBase &eventBase, const std::string &key, uint64_t value)
{
    eventBase.jsonStr_ << "\"" << key << "\":\"" << std::hex << value << "\"," << std::dec;
}

void HiSysEvent::AppendData(HiSysEvent::EventBase &eventBase,
    const std::string &key, const std::string &value, bool isDefKey)
{
    if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
        return;
    }
    if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
        return;
    }
    if (IsWarnAndUpdate(CheckValue(value), eventBase)) {
        return;
    }
    eventBase.jsonStr_ << "\"" << key << "\":\"" << EscapeStringValue(value) << "\",";
}

void HiSysEvent::AppendArrayData(HiSysEvent::EventBase &eventBase,
    const std::string &key, const std::vector<char> &value, bool isDefKey)
{
    if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
        return;
    }

    if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
        return;
    }

    if (value.empty()) {
        eventBase.jsonStr_ << "\"" << key << "\":[]";
        return;
    }

    IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);

    unsigned int index = 0;
    eventBase.jsonStr_ << "\"" << key << "\":[";
    for (auto item = value.begin(); item != value.end(); item++) {
        index++;
        if (index > MAX_ARRAY_SIZE) {
            break;
        }
        eventBase.jsonStr_ << static_cast<short>(*item) << ",";
    }
    if (eventBase.jsonStr_.tellp() != 0) {
        eventBase.jsonStr_.seekp(-1, std::ios_base::end);
    }
    eventBase.jsonStr_ << "],";
}

void HiSysEvent::AppendArrayData(HiSysEvent::EventBase &eventBase,
    const std::string &key, const std::vector<unsigned char> &value, bool isDefKey)
{
    if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
        return;
    }

    if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
        return;
    }

    if (value.empty()) {
        eventBase.jsonStr_ << "\"" << key << "\":[]";
        return;
    }

    IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);

    unsigned int index = 0;
    eventBase.jsonStr_ << "\"" << key << "\":[";
    for (auto item = value.begin(); item != value.end(); item++) {
        index++;
        if (index > MAX_ARRAY_SIZE) {
            break;
        }
        eventBase.jsonStr_ << static_cast<short>(*item) << ",";
    }
    if (eventBase.jsonStr_.tellp() != 0) {
        eventBase.jsonStr_.seekp(-1, std::ios_base::end);
    }
    eventBase.jsonStr_ << "],";
}

void HiSysEvent::AppendArrayData(HiSysEvent::EventBase &eventBase,
    const std::string &key, const std::vector<std::string> &value, bool isDefKey)
{
    if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
        return;
    }

    if (UpdateAndCheckKeyNumIsOver(eventBase, isDefKey)) {
        return;
    }

    if (value.empty()) {
        eventBase.jsonStr_ << "\"" << key << "\":[]";
        return;
    }

    IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);

    bool isAdd = false;
    unsigned int index = 0;
    eventBase.jsonStr_ << "\"" << key << "\":[";
    for (auto item = value.begin(); item != value.end(); item++) {
        index++;
        if (index > MAX_ARRAY_SIZE) {
            break;
        }
        if (IsWarnAndUpdate(CheckValue(*item), eventBase)) {
            continue;
        }
        isAdd = true;
        eventBase.jsonStr_ << "\"" << EscapeStringValue((*item)) << "\",";
    }
    if (isAdd) {
        eventBase.jsonStr_.seekp(-1, std::ios_base::end);
    }
    eventBase.jsonStr_ << "],";
}

int HiSysEvent::WritebaseInfo(HiSysEvent::EventBase &eventBase)
{
    int retCode = SUCCESS;
    retCode = CheckDomain(eventBase);
    if (IsErrorAndUpdate(retCode, eventBase)) {
        return retCode;
    }
    retCode = CheckEventName(eventBase);
    if (IsErrorAndUpdate(retCode, eventBase)) {
        return retCode;
    }
    AppendData(eventBase, "domain_", eventBase.domain_, false);
    AppendData(eventBase, "name_", eventBase.eventName_, false);
    AppendData(eventBase, "type_", eventBase.type_, false);
    AppendData(eventBase, "time_", GetMilliseconds(), false);
    AppendData(eventBase, "tz_", GetTimeZone(), false);
    AppendData(eventBase, "pid_", getpid(), false);
    AppendData(eventBase, "tid_", gettid(), false);
    AppendData(eventBase, "uid_", getuid(), false);
    return retCode;
}

void HiSysEvent::InnerWrite(HiSysEvent::EventBase &eventBase)
{
    if (eventBase.jsonStr_.tellp() != 0) {
        eventBase.jsonStr_.seekp(-1, std::ios_base::end);
    }
}

int HiSysEvent::SendSysEvent(HiSysEvent::EventBase &eventBase)
{
    std::string jsonStr = eventBase.jsonStr_.str();
    if (jsonStr.size() > MAX_JSON_SIZE) {
        HiLog::Error(LABEL, "data is too long %{public}lu", static_cast<unsigned long>(jsonStr.length()));
        return ERR_OVER_SIZE;
    }
    HiLog::Debug(LABEL, "size=%{public}lu, sysevent=%{public}s",
        static_cast<unsigned long>(jsonStr.size()), jsonStr.c_str());

    FailDataQueue::GetInstance()->RetrySendFailedData();
    int tryTimes = 3;
    int retCode = SUCCESS;
    while (tryTimes > 0) {
        tryTimes--;
        retCode = SendToHiSysEventDataSource(jsonStr);
        if (retCode == SUCCESS) {
            return SUCCESS;
        }
    }

    FailDataQueue::GetInstance()->AddData(jsonStr);
    return retCode;
}
} // HiviewDFX
} // OHOS
