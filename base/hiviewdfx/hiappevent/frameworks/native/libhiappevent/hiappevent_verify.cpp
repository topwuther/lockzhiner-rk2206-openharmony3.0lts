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

#include "hiappevent_verify.h"

#include <iterator>
#include <regex>

#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

using namespace OHOS::HiviewDFX::ErrorCode;

namespace OHOS {
namespace HiviewDFX {
namespace {
static constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_verify" };

static constexpr int MAX_LENGTH_OF_EVENT_NAME = 48;
static constexpr int MAX_LENGTH_OF_PARAM_NAME = 16;
static constexpr int MAX_NUM_OF_PARAMS = 32;
static constexpr int MAX_LENGTH_OF_STR_PARAM = 8 * 1024;
static constexpr int MAX_SIZE_OF_LIST_PARAM = 100;

static constexpr int HITRACE_PARAMS_NUM = 4;
static const std::string HITRACE_PARAMS[HITRACE_PARAMS_NUM] = {"traceid_", "spanid_", "pspanid_", "trace_flag_"};

bool CheckEventName(const std::string& eventName)
{
    if (eventName.empty() || eventName.length() > MAX_LENGTH_OF_EVENT_NAME) {
        return false;
    }

    /* custom and preset events */
    if (!std::regex_match(eventName, std::regex("^[a-z][a-z0-9_]*$|^hiappevent\\.[a-z][a-z0-9_]*$"))) {
        return false;
    }

    return true;
}

bool CheckParamName(const std::string& paramName)
{
    if (paramName.empty() || paramName.length() > MAX_LENGTH_OF_PARAM_NAME) {
        return false;
    }

    for (int i = 0; i < HITRACE_PARAMS_NUM; i++) {
        if (paramName == HITRACE_PARAMS[i]) {
            return true;
        }
    }

    if (!std::regex_match(paramName, std::regex("^[a-z][a-z0-9_]*[a-z0-9]$"))) {
        return false;
    }

    return true;
}

void EscapeStringValue(std::string &value)
{
    std::string escapeValue;
    for (auto it = value.begin(); it != value.end(); it++) {
        switch (*it) {
            case '\\':
                escapeValue.append("\\\\");
                break;
            case '\"':
                escapeValue.append("\\\"");
                break;
            case '\b':
                escapeValue.append("\\b");
                break;
            case '\f':
                escapeValue.append("\\f");
                break;
            case '\n':
                escapeValue.append("\\n");
                break;
            case '\r':
                escapeValue.append("\\r");
                break;
            case '\t':
                escapeValue.append("\\t");
                break;
            default:
                escapeValue.push_back(*it);
                break;
        }
    }
    value = escapeValue;
}

bool CheckStrParamLength(std::string& strParamValue)
{
    if (strParamValue.empty()) {
        HiLog::Warn(LABEL, "str param value is empty.");
        return true;
    }

    if (strParamValue.length() > MAX_LENGTH_OF_STR_PARAM) {
        return false;
    }

    EscapeStringValue(strParamValue);
    return true;
}

bool CheckListValueSize(AppEventParamType type, AppEventParamValue::ValueUnion& vu)
{
    if (type == AppEventParamType::BVECTOR && vu.bs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.bs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::CVECTOR && vu.cs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.cs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::SHVECTOR && vu.shs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.shs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::IVECTOR && vu.is_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.is_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::LVECTOR && vu.ls_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.ls_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::LLVECTOR && vu.lls_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.lls_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::FVECTOR && vu.fs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.fs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::DVECTOR && vu.ds_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.ds_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::STRVECTOR && vu.strs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.strs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else {
        return true;
    }

    return false;
}

bool CheckStringLengthOfList(std::vector<std::string>& strs)
{
    if (strs.empty()) {
        return true;
    }

    for (auto it = strs.begin(); it != strs.end(); it++) {
        if (!CheckStrParamLength(*it)) {
            return false;
        }
    }

    return true;
}

bool CheckParamsNum(std::list<AppEventParam>& baseParams)
{
    if (baseParams.size() == 0) {
        return true;
    }

    int maxParamsNum = MAX_NUM_OF_PARAMS;
    if (baseParams.begin()->name == HITRACE_PARAMS[0]) {
        maxParamsNum += HITRACE_PARAMS_NUM;
    }

    int listSize = baseParams.size();
    if (listSize > maxParamsNum) {
        auto delStartPtr = baseParams.begin();
        std::advance(delStartPtr, maxParamsNum);
        baseParams.erase(delStartPtr, baseParams.end());
        return false;
    }

    return true;
}
}

int VerifyAppEvent(std::shared_ptr<AppEventPack>& appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HiLog::Error(LABEL, "the HiAppEvent function is disabled.");
        return ERROR_HIAPPEVENT_DISABLE;
    }

    if (!CheckEventName(appEventPack->GetEventName())) {
        HiLog::Error(LABEL, "eventName=%{public}s is invalid.", appEventPack->GetEventName().c_str());
        return ERROR_INVALID_EVENT_NAME;
    }

    int verifyRes = HIAPPEVENT_VERIFY_SUCCESSFUL;
    std::list<AppEventParam>& baseParams = appEventPack->baseParams_;
    if (!CheckParamsNum(baseParams)) {
        HiLog::Warn(LABEL, "params that exceed 32 are discarded because the number of params cannot exceed 32.");
        verifyRes = ERROR_INVALID_PARAM_NUM;
    }

    for (auto it = baseParams.begin(); it != baseParams.end();) {
        if (!CheckParamName(it->name)) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the paramName is invalid.", it->name.c_str());
            verifyRes = ERROR_INVALID_PARAM_NAME;
            baseParams.erase(it++);
            continue;
        }

        if (it->type == AppEventParamType::STRING && !CheckStrParamLength(it->value.valueUnion.str_)) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the string length exceeds 8192.",
                it->name.c_str());
            verifyRes = ERROR_INVALID_PARAM_VALUE_LENGTH;
            baseParams.erase(it++);
            continue;
        }

        if (it->type > AppEventParamType::STRING && !CheckListValueSize(it->type, it->value.valueUnion)) {
            HiLog::Warn(LABEL, "list param=%{public}s is truncated because the list size exceeds 100.",
                it->name.c_str());
            verifyRes = ERROR_INVALID_LIST_PARAM_SIZE;
            continue;
        }

        if (it->type == AppEventParamType::STRVECTOR && !CheckStringLengthOfList(it->value.valueUnion.strs_)) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the string length of list exceeds 8192.",
                it->name.c_str());
            verifyRes = ERROR_INVALID_PARAM_VALUE_LENGTH;
            baseParams.erase(it++);
            continue;
        }
        it++;
    }

    return verifyRes;
}
} // HiviewDFX
} // OHOS