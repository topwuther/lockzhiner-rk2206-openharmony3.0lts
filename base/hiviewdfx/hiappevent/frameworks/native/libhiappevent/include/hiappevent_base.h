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

#ifndef HI_APP_EVENT_BASE_H
#define HI_APP_EVENT_BASE_H

#include <ctime>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
/**
 * HiAppEvent write app event error code
 */
namespace ErrorCode {
const int HIAPPEVENT_VERIFY_SUCCESSFUL = 0;
const int ERROR_INVALID_EVENT_NAME = -1;
const int ERROR_INVALID_PARAM_TYPE_JS = -2;
const int ERROR_INVALID_PARAM_NUM_JS = -3;
const int ERROR_INVALID_PARAM_NAME = 1;
const int ERROR_INVALID_PARAM_KEY_TYPE = 2;
const int ERROR_INVALID_PARAM_VALUE_TYPE = 3;
const int ERROR_INVALID_PARAM_VALUE_LENGTH = 4;
const int ERROR_INVALID_PARAM_NUM = 5;
const int ERROR_INVALID_LIST_PARAM_SIZE = 6;
const int ERROR_INVALID_LIST_PARAM_TYPE = 7;
const int ERROR_HIAPPEVENT_DISABLE = -99;
const int ERROR_UNKNOWN = -100;
}

/**
 * HiLog hiappevent domain code
 */
const unsigned int HIAPPEVENT_DOMAIN = 0xD002D07;

enum AppEventParamType {
    EMPTY = 0,
    BOOL = 1,
    CHAR = 2,
    SHORT = 3,
    INTEGER = 4,
    LONG = 5,
    LONGLONG = 6,
    FLOAT = 7,
    DOUBLE = 8,
    STRING = 9,
    BVECTOR = 10,
    CVECTOR = 11,
    SHVECTOR = 12,
    IVECTOR = 13,
    LVECTOR = 14,
    LLVECTOR = 15,
    FVECTOR = 16,
    DVECTOR = 17,
    STRVECTOR = 18
};

struct AppEventParamValue {
    AppEventParamType type;
    union ValueUnion {
        bool b_;
        char c_;
        short sh_;
        int i_;
        long l_;
        long long ll_;
        float f_;
        double d_;
        std::string str_;
        std::vector<bool> bs_;
        std::vector<char> cs_;
        std::vector<short> shs_;
        std::vector<int> is_;
        std::vector<long> ls_;
        std::vector<long long> lls_;
        std::vector<float> fs_;
        std::vector<double> ds_;
        std::vector<std::string> strs_;

        ValueUnion() {}

        ValueUnion(AppEventParamType type)
        {
            switch (type) {
                case AppEventParamType::STRING:
                    new (&str_) std::string;
                    break;
                case AppEventParamType::BVECTOR:
                    new (&bs_) std::vector<bool>;
                    break;
                case AppEventParamType::CVECTOR:
                    new (&cs_) std::vector<char>;
                    break;
                case AppEventParamType::SHVECTOR:
                    new (&shs_) std::vector<short>;
                    break;
                case AppEventParamType::IVECTOR:
                    new (&is_) std::vector<int>;
                    break;
                case AppEventParamType::LVECTOR:
                    new (&ls_) std::vector<long>;
                    break;
                case AppEventParamType::LLVECTOR:
                    new (&lls_) std::vector<long long>;
                    break;
                case AppEventParamType::FVECTOR:
                    new (&fs_) std::vector<float>;
                    break;
                case AppEventParamType::DVECTOR:
                    new (&ds_) std::vector<double>;
                    break;
                case AppEventParamType::STRVECTOR:
                    new (&strs_) std::vector<std::string>;
                    break;
                default:
                    break;
            }
        }

        ~ValueUnion() {}
    } valueUnion;

    AppEventParamValue(AppEventParamType t);
    AppEventParamValue(const AppEventParamValue& value);
    ~AppEventParamValue();
};
using AppEventParamValue = struct AppEventParamValue;

struct AppEventParam {
    std::string name;
    AppEventParamType type;
    AppEventParamValue value;

    AppEventParam(std::string n, AppEventParamType t);
    AppEventParam(const AppEventParam& param);
    ~AppEventParam();
};
using AppEventParam = struct AppEventParam;

class AppEventPack {
public:
    AppEventPack(const std::string &eventName, int type);
    ~AppEventPack();

public:
    void AddParam(const std::string& key);
    void AddParam(const std::string& key, bool b);
    void AddParam(const std::string& key, char c);
    void AddParam(const std::string& key, short s);
    void AddParam(const std::string& key, int i);
    void AddParam(const std::string& key, long l);
    void AddParam(const std::string& key, long long ll);
    void AddParam(const std::string& key, float f);
    void AddParam(const std::string& key, double d);
    void AddParam(const std::string& key, const char *s);
    void AddParam(const std::string& key, const std::string& s);
    void AddParam(const std::string& key, const std::vector<bool>& bs);
    void AddParam(const std::string& key, const std::vector<char>& cs);
    void AddParam(const std::string& key, const std::vector<short>& shs);
    void AddParam(const std::string& key, const std::vector<int>& is);
    void AddParam(const std::string& key, const std::vector<long>& ls);
    void AddParam(const std::string& key, const std::vector<long long>& lls);
    void AddParam(const std::string& key, const std::vector<float>& fs);
    void AddParam(const std::string& key, const std::vector<double>& ds);
    void AddParam(const std::string& key, const std::vector<const char*>& cps);
    void AddParam(const std::string& key, const std::vector<const std::string>& strs);
    const std::string GetEventName() const;
    int GetType() const;
    std::string GetJsonString() const;
    friend int VerifyAppEvent(std::shared_ptr<AppEventPack>& appEventPack);

private:
    void AddBaseInfoToJsonString(std::stringstream& jsonStr) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<bool>& bs) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<char>& cs) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<short>& shs) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<int>& is) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<long>& ls) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<long long>& lls) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<float>& fs) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<double>& ds) const;
    void AddVectorToJsonString(std::stringstream& jsonStr, const std::vector<std::string>& strs) const;
    void AddOthersToJsonString(std::stringstream& jsonStr, const AppEventParam param) const;

private:
    std::string eventName_;
    int type_;
    std::list<AppEventParam> baseParams_;
};
} // HiviewDFX
} // OHOS
#endif // HI_APP_EVENT_BASE_H