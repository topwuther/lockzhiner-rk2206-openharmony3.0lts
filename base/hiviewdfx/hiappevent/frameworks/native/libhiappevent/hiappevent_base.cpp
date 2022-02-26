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

#include "hiappevent_base.h"

#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_base" };
constexpr int SEC_TO_HOUR = 3600;
constexpr int SEC_TO_MIN = 60;

std::string TrimRightZero(const std::string& str)
{
    int endIndex = str.find_last_not_of("0");
    if (endIndex == -1) {
        return str;
    }

    return (str[endIndex] == '.') ? str.substr(0, endIndex) : str.substr(0, endIndex + 1);
}

std::string GetTimeInfo()
{
    // get system timestamp
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        HiLog::Error(LABEL, "failed to execute the gettimeofday function.");
        return "";
    }
    long long timeMillSec = tv.tv_sec * 1000 + tv.tv_usec / 1000; // one second equals 1000 milliseconds
    std::stringstream ss;
    ss << "\"" << "time_" << "\":" << std::to_string(timeMillSec) << ",";

    // Get system time zone
    time_t sysSec = tv.tv_sec;
    struct tm* tmLocal = localtime(&sysSec);
    if (tmLocal == nullptr) {
        HiLog::Error(LABEL, "failed to get local time.");
        return ss.str();
    }
    struct tm* tmUtc = gmtime(&sysSec);
    if (tmUtc == nullptr) {
        HiLog::Error(LABEL, "failed to get GMT time.");
        return ss.str();
    }
    time_t diffSec = mktime(tmLocal) - mktime(tmUtc);
    ss << "\"tz_\":\"" << ((diffSec < 0) ? "-" : "+");

    int tzHour = std::abs(diffSec) / SEC_TO_HOUR;
    if (tzHour > 12) { // max time zone is 12
        HiLog::Error(LABEL, "failed to get hours for time zone, set to 0.");
        tzHour = 0;
    }
    int tzMin = (std::abs(diffSec) % SEC_TO_HOUR) / SEC_TO_MIN;
    ss << std::setw(2) << std::setfill('0') << tzHour; // the number of digits in the hour is 2
    ss << std::setw(2) << std::setfill('0') << tzMin << "\","; // the number of digits in the min is 2

    return ss.str();
}
}

AppEventParamValue::AppEventParamValue(AppEventParamType t) : type(t), valueUnion(t)
{}

AppEventParamValue::AppEventParamValue(const AppEventParamValue& other) : type(other.type)
{
    switch (other.type) {
        case AppEventParamType::BOOL:
            valueUnion.b_ = other.valueUnion.b_;
            break;
        case AppEventParamType::CHAR:
            valueUnion.c_ = other.valueUnion.c_;
            break;
        case AppEventParamType::SHORT:
            valueUnion.sh_ = other.valueUnion.sh_;
            break;
        case AppEventParamType::INTEGER:
            valueUnion.i_ = other.valueUnion.i_;
            break;
        case AppEventParamType::LONG:
            valueUnion.l_ = other.valueUnion.l_;
            break;
        case AppEventParamType::LONGLONG:
            valueUnion.ll_ = other.valueUnion.ll_;
            break;
        case AppEventParamType::FLOAT:
            valueUnion.f_ = other.valueUnion.f_;
            break;
        case AppEventParamType::DOUBLE:
            valueUnion.d_ = other.valueUnion.d_;
            break;
        case AppEventParamType::STRING:
            new (&valueUnion.str_) auto(other.valueUnion.str_);
            break;
        case AppEventParamType::BVECTOR:
            new (&valueUnion.bs_) auto(other.valueUnion.bs_);
            break;
        case AppEventParamType::CVECTOR:
            new (&valueUnion.cs_) auto(other.valueUnion.cs_);
            break;
        case AppEventParamType::SHVECTOR:
            new (&valueUnion.shs_) auto(other.valueUnion.shs_);
            break;
        case AppEventParamType::IVECTOR:
            new (&valueUnion.is_) auto(other.valueUnion.is_);
            break;
        case AppEventParamType::LVECTOR:
            new (&valueUnion.ls_) auto(other.valueUnion.ls_);
            break;
        case AppEventParamType::LLVECTOR:
            new (&valueUnion.lls_) auto(other.valueUnion.lls_);
            break;
        case AppEventParamType::FVECTOR:
            new (&valueUnion.fs_) auto(other.valueUnion.fs_);
            break;
        case AppEventParamType::DVECTOR:
            new (&valueUnion.ds_) auto(other.valueUnion.ds_);
            break;
        case AppEventParamType::STRVECTOR:
            new (&valueUnion.strs_) auto(other.valueUnion.strs_);
            break;
        default:
            break;
    }
}

AppEventParamValue::~AppEventParamValue()
{
    switch (type) {
        case AppEventParamType::STRING:
            valueUnion.str_.~basic_string();
            break;
        case AppEventParamType::BVECTOR:
            valueUnion.bs_.~vector();
            break;
        case AppEventParamType::CVECTOR:
            valueUnion.cs_.~vector();
            break;
        case AppEventParamType::SHVECTOR:
            valueUnion.shs_.~vector();
            break;
        case AppEventParamType::IVECTOR:
            valueUnion.is_.~vector();
            break;
        case AppEventParamType::LVECTOR:
            valueUnion.ls_.~vector();
            break;
        case AppEventParamType::LLVECTOR:
            valueUnion.lls_.~vector();
            break;
        case AppEventParamType::FVECTOR:
            valueUnion.fs_.~vector();
            break;
        case AppEventParamType::DVECTOR:
            valueUnion.ds_.~vector();
            break;
        case AppEventParamType::STRVECTOR:
            valueUnion.strs_.~vector();
            break;
        default:
            break;
    }
}

AppEventParam::AppEventParam(std::string n, AppEventParamType t) : name(n), type(t), value(t)
{}

AppEventParam::AppEventParam(const AppEventParam& param) : name(param.name), type(param.type), value(param.value)
{}

AppEventParam::~AppEventParam()
{}

AppEventPack::AppEventPack(const std::string& eventName, int type): eventName_(eventName), type_(type)
{}

AppEventPack::~AppEventPack()
{}

void AppEventPack::AddParam(const std::string& key)
{
    AppEventParam appEventParam(key, AppEventParamType::EMPTY);
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, bool b)
{
    AppEventParam appEventParam(key, AppEventParamType::BOOL);
    appEventParam.value.valueUnion.b_ = b;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, char c)
{
    AppEventParam appEventParam(key, AppEventParamType::CHAR);
    appEventParam.value.valueUnion.c_ = c;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, short s)
{
    AppEventParam appEventParam(key, AppEventParamType::SHORT);
    appEventParam.value.valueUnion.sh_ = s;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, int i)
{
    AppEventParam appEventParam(key, AppEventParamType::INTEGER);
    appEventParam.value.valueUnion.i_ = i;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, long l)
{
    AppEventParam appEventParam(key, AppEventParamType::LONG);
    appEventParam.value.valueUnion.l_ = l;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, long long ll)
{
    AppEventParam appEventParam(key, AppEventParamType::LONGLONG);
    appEventParam.value.valueUnion.ll_ = ll;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, float f)
{
    AppEventParam appEventParam(key, AppEventParamType::FLOAT);
    appEventParam.value.valueUnion.f_ = f;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, double d)
{
    AppEventParam appEventParam(key, AppEventParamType::DOUBLE);
    appEventParam.value.valueUnion.d_ = d;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const char *s)
{
    AppEventParam appEventParam(key, AppEventParamType::STRING);
    appEventParam.value.valueUnion.str_ = s;
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::string& s)
{
    AppEventParam appEventParam(key, AppEventParamType::STRING);
    appEventParam.value.valueUnion.str_ = s;
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<bool>& bs)
{
    AppEventParam appEventParam(key, AppEventParamType::BVECTOR);
    appEventParam.value.valueUnion.bs_.assign(bs.begin(), bs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<char>& cs)
{
    AppEventParam appEventParam(key, AppEventParamType::CVECTOR);
    appEventParam.value.valueUnion.cs_.assign(cs.begin(), cs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<short>& shs)
{
    AppEventParam appEventParam(key, AppEventParamType::SHVECTOR);
    appEventParam.value.valueUnion.shs_.assign(shs.begin(), shs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int>& is)
{
    AppEventParam appEventParam(key, AppEventParamType::IVECTOR);
    appEventParam.value.valueUnion.is_.assign(is.begin(), is.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<long>& ls)
{
    AppEventParam appEventParam(key, AppEventParamType::LVECTOR);
    appEventParam.value.valueUnion.ls_.assign(ls.begin(), ls.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<long long>& lls)
{
    AppEventParam appEventParam(key, AppEventParamType::LLVECTOR);
    appEventParam.value.valueUnion.lls_.assign(lls.begin(), lls.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<float>& fs)
{
    AppEventParam appEventParam(key, AppEventParamType::FVECTOR);
    appEventParam.value.valueUnion.fs_.assign(fs.begin(), fs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<double>& ds)
{
    AppEventParam appEventParam(key, AppEventParamType::DVECTOR);
    appEventParam.value.valueUnion.ds_.assign(ds.begin(), ds.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<const char*>& cps)
{
    AppEventParam appEventParam(key, AppEventParamType::STRVECTOR);
    std::vector<std::string> strs;
    if (cps.size() != 0) {
        for (auto cp : cps) {
            strs.push_back(cp);
        }
    }
    appEventParam.value.valueUnion.strs_.assign(strs.begin(), strs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<const std::string>& strs)
{
    AppEventParam appEventParam(key, AppEventParamType::STRVECTOR);
    appEventParam.value.valueUnion.strs_.assign(strs.begin(), strs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddBaseInfoToJsonString(std::stringstream& jsonStr) const
{
    jsonStr << "\"" << "name_" << "\":" << "\"" << eventName_ << "\",";
    jsonStr << "\"" << "type_" << "\":" <<  type_ << ",";
    jsonStr << GetTimeInfo();
    jsonStr << "\"" << "pid_" << "\":" << getpid() << ",";
    jsonStr << "\"" << "tid_" << "\":" << gettid() << ",";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<bool>& bs) const
{
    jsonStr << "[";
    size_t len = bs.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<bool> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << ((bs[i] == true) ? "true" : "false") << ",";
    }
    jsonStr << ((bs[len - 1] == true) ? "true" : "false") << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<char>& cs) const
{
    jsonStr << "[";
    size_t len = cs.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<char> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << "\"" << cs[i] << "\""  << ",";
    }
    jsonStr << "\"" << cs[len - 1] << "\"" << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<short>& shs) const
{
    jsonStr << "[";
    size_t len = shs.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<short> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << shs[i] << ",";
    }
    jsonStr << shs[len - 1] << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<int>& is) const
{
    jsonStr << "[";
    size_t len = is.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<int> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << is[i] << ",";
    }
    jsonStr << is[len - 1] << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<long>& ls) const
{
    jsonStr << "[";
    size_t len = ls.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<long> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << ls[i] << ",";
    }
    jsonStr << ls[len - 1] << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<long long>& lls) const
{
    jsonStr << "[";
    size_t len = lls.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<long long> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << lls[i] << ",";
    }
    jsonStr << lls[len - 1] << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<float>& fs) const
{
    jsonStr << "[";
    size_t len = fs.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<float> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << TrimRightZero(std::to_string(fs[i])) << ",";
    }
    jsonStr << TrimRightZero(std::to_string(fs[len - 1])) << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<double>& ds) const
{
    jsonStr << "[";
    size_t len = ds.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<double> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << TrimRightZero(std::to_string(ds[i])) << ",";
    }
    jsonStr << TrimRightZero(std::to_string(ds[len - 1])) << "],";
}

void AppEventPack::AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<std::string>& strs) const
{
    jsonStr << "[";
    size_t len = strs.size();
    if (len == 0) {
        jsonStr << "],";
        HiLog::Info(LABEL, "The vector<string> value added to JsonString is empty.");
        return;
    }

    for (size_t i = 0; i < len - 1; i++) {
        jsonStr << "\"" << strs[i] << "\",";
    }
    jsonStr << "\"" << strs[len - 1] << "\"],";
}

void AppEventPack::AddOthersToJsonString(std::stringstream& jsonStr, const AppEventParam param) const
{
    if (param.type == AppEventParamType::BVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.bs_);
    } else if (param.type == AppEventParamType::CVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.cs_);
    } else if (param.type == AppEventParamType::SHVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.shs_);
    } else if (param.type == AppEventParamType::IVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.is_);
    } else if (param.type == AppEventParamType::LVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.ls_);
    } else if (param.type == AppEventParamType::LLVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.lls_);
    } else if (param.type == AppEventParamType::FVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.fs_);
    } else if (param.type == AppEventParamType::DVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.ds_);
    } else if (param.type == AppEventParamType::STRVECTOR) {
        AddVectorToJsonString(jsonStr, param.value.valueUnion.strs_);
    } else if (param.type == AppEventParamType::EMPTY) {
        jsonStr << "[]" << ",";
    } else {
        jsonStr << "\"\",";
    }
}

const std::string AppEventPack::GetEventName() const
{
    return eventName_;
}

int AppEventPack::GetType() const
{
    return type_;
}

std::string AppEventPack::GetJsonString() const
{
    std::stringstream jsonStr;
    jsonStr << "{";
    AddBaseInfoToJsonString(jsonStr);
    for (auto it = baseParams_.begin(); it != baseParams_.end(); it++) {
        jsonStr << "\"" << it->name << "\":";
        if (it->type == AppEventParamType::BOOL) {
            jsonStr << ((it->value.valueUnion.b_ == true) ? "true" : "false") << ",";
        } else if (it->type == AppEventParamType::CHAR) {
            jsonStr << "\"" << it->value.valueUnion.c_ << "\"" << ",";
        } else if (it->type == AppEventParamType::SHORT) {
            jsonStr << it->value.valueUnion.sh_ << ",";
        } else if (it->type == AppEventParamType::INTEGER) {
            jsonStr << it->value.valueUnion.i_ << ",";
        } else if (it->type == AppEventParamType::LONG) {
            jsonStr << it->value.valueUnion.l_ << ",";
        } else if (it->type == AppEventParamType::LONGLONG) {
            jsonStr << it->value.valueUnion.ll_ << ",";
        } else if (it->type == AppEventParamType::FLOAT) {
            jsonStr << TrimRightZero(std::to_string(it->value.valueUnion.f_)) << ",";
        } else if (it->type == AppEventParamType::DOUBLE) {
            jsonStr << TrimRightZero(std::to_string(it->value.valueUnion.d_)) << ",";
        } else if (it->type == AppEventParamType::STRING) {
            jsonStr << "\"" << it->value.valueUnion.str_ << "\",";
        } else {
            AddOthersToJsonString(jsonStr, *it);
        }
    }

    jsonStr.seekp(-1, std::ios_base::end);
    jsonStr << "}" << std::endl;
    return jsonStr.str();
}
} // HiviewDFX
} // OHOS