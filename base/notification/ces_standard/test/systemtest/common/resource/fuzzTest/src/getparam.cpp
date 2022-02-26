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
#include "../include/getparam.h"
#include <functional>
#include <random>
#include <limits.h>
#include <float.h>
#include <chrono>
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"
using namespace std;
using namespace OHOS::AppExecFwk;
using Uri = OHOS::Uri;
namespace OHOS {
namespace EventFwk {
bool GetBoolParam()
{
    bool param;
    if (GetIntParam() % 2 == 0) {
        param = true;
    } else {
        param = false;
    }
    cout << "Bool param is: " << param << endl;
    return param;
}

size_t GenRandom(size_t min, size_t max)
{
    std::random_device rd;
    static uniform_int_distribution<size_t> u(min, max);
    static default_random_engine e(rd());
    size_t param = u(e);
    return param;
}

int8_t GetS8Param()
{
    std::random_device rd;
    static uniform_int_distribution<int8_t> u(INT8_MIN, INT8_MAX);
    static default_random_engine e(rd());
    int8_t param = u(e);
    cout << "Int8_t param is: " << param << endl;
    return param;
}
int16_t GetS16Param()
{
    std::random_device rd;
    static uniform_int_distribution<int16_t> u(INT16_MIN, INT16_MAX);
    static default_random_engine e(rd());
    int16_t param = u(e);
    cout << "Int16_t param is: " << param << endl;
    return param;
}
int32_t GetS32Param()
{
    std::random_device rd;
    static uniform_int_distribution<int32_t> u(INT32_MIN, INT32_MAX);
    static default_random_engine e(rd());
    int32_t param = u(e);
    cout << "Int32_t param is: " << param << endl;
    return param;
}

int64_t GetS64Param()
{
    std::random_device rd;
    static uniform_int_distribution<int64_t> u(INT64_MIN, INT64_MAX);
    static default_random_engine e(rd());
    int64_t param = u(e);
    cout << "Int64_t param is: " << param << endl;
    return param;
}

template <class T>
T GetUnsignParam()
{
    std::random_device rd;
    static uniform_int_distribution<T> u;
    static default_random_engine e(rd());
    T param = u(e);
    return param;
}

size_t GetSizeTParam()
{
    size_t param = GetUnsignParam<size_t>();
    return param;
}

uint8_t GetU8Param()
{
    uint8_t param = GetUnsignParam<uint8_t>();
    cout << "Uint8_t param is: " << param << endl;
    return param;
}

unsigned int GetUIntParam()
{
    unsigned int param = GetUnsignParam<unsigned int>();
    cout << "Unsigned int param is: " << param << endl;
    return param;
}

uint16_t GetU16Param()
{
    uint16_t param = GetUnsignParam<uint16_t>();
    cout << "Uint16_t param is: " << param << endl;
    return param;
}

uint32_t GetU32Param()
{
    uint32_t param = GetUnsignParam<uint32_t>();
    cout << "Uint32_t param is: " << param << endl;
    return param;
}

uint64_t GetU64Param()
{
    uint64_t param = GetUnsignParam<uint64_t>();
    cout << "Uint64_t param is: " << param << endl;
    return param;
}

short GetShortParam()
{
    std::random_device rd;
    static uniform_int_distribution<short> u(SHRT_MIN, SHRT_MAX);
    static default_random_engine e(rd());
    short param = u(e);
    cout << "Short param is: " << param << endl;
    return param;
}

long GetLongParam()
{
    std::random_device rd;
    static uniform_int_distribution<long> u(LONG_MIN, LONG_MAX);
    static default_random_engine e(rd());
    long param = u(e);
    cout << "Long param is: " << param << endl;
    return param;
}

int GetIntParam()
{
    std::random_device rd;
    static uniform_int_distribution<> u(INT_MIN, INT_MAX);
    static default_random_engine e(rd());
    int param = u(e);
    cout << "Int param is: " << param << endl;
    return param;
}

double GetDoubleParam()
{
    double param = 0;
    std::random_device rd;
    static uniform_real_distribution<double> u(DBL_MIN, DBL_MAX);
    static default_random_engine e(rd());
    param = u(e);
    cout << "double param is: " << param << endl;
    return param;
}

float GetFloatParam()
{
    float param = 0;
    std::random_device rd;
    static uniform_real_distribution<float> u(FLT_MIN, FLT_MAX);
    static default_random_engine e(rd());
    param = u(e);
    cout << "Float param is: " << param << endl;
    return param;
}

char GetCharParam()
{
    std::random_device rd;
    static uniform_int_distribution<> u(-128, 127);
    static default_random_engine e(rd());
    char param = u(e);
    return param;
}

char32_t GetChar32Param()
{
    char32_t param = ' ';
    std::random_device rd;
    static uniform_int_distribution<char32_t> u;
    static default_random_engine e(rd());
    param = u(e);
    return param;
}

char *GetCharArryParam()
{
    static char param[256];
    size_t len = 0;
    string strparam = GetStringParam();
    if (!strparam.empty()) {
        len = strparam.size() + 1;
        if (len > sizeof(param)) {
            len = sizeof(param) - 1;
        }

        int ret = strcpy_s(param, len, strparam.c_str());
        if(ret == 0){
            return param;    
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }  
}

string GetStringParam()
{
    string param = "";
    char ch = GetCharParam();
    size_t len = GenRandom(0, 255);
    while (len--) {
        ch = GetCharParam();
        param += ch;
    }
    cout << "String param length is: " << param.length() << endl;
    return param;
}

template <class T>
vector<T> GetUnsignVectorParam()
{
    vector<T> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        T t = GetUnsignParam<T>();
        param.push_back(t);
    }
    cout << "Uint vector param length is: " << param.size() << endl;
    return param;
}

template <class T>
T GetClassParam()
{
    T param;
    return param;
}

std::vector<bool> GetBoolVectorParam()
{
    vector<bool> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int t = GetBoolParam();
        param.push_back(t);
    }
    cout << "Bool vector param length is: " << param.size() << endl;
    return param;
}

std::vector<short> GetShortVectorParam()
{
    vector<short> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        short t = GetShortParam();
        param.push_back(t);
    }
    cout << "Short vector param length is: " << param.size() << endl;
    return param;
}

std::vector<long> GetLongVectorParam()
{
    vector<long> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        long t = GetLongParam();
        param.push_back(t);
    }
    cout << "Long vector param length is: " << param.size() << endl;
    return param;
}

vector<int> GetIntVectorParam()
{
    vector<int> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int t = GetIntParam();
        param.push_back(t);
    }
    cout << "Int vector param length is: " << param.size() << endl;
    return param;
}

std::vector<float> GetFloatVectorParam()
{
    vector<float> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        float t = GetIntParam();
        param.push_back(t);
    }
    cout << "Float vector param length is: " << param.size() << endl;
    return param;
}

std::vector<double> GetDoubleVectorParam()
{
    vector<double> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        double t = GetIntParam();
        param.push_back(t);
    }
    cout << "Double vector param length is: " << param.size() << endl;
    return param;
}

vector<char> GetCharVectorParam()
{
    vector<char> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        char t = GetCharParam();
        param.push_back(t);
    }
    cout << "Char vector param length is: " << param.size() << endl;
    return param;
}

vector<char32_t> GetChar32VectorParam()
{
    vector<char32_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        char32_t t = GetChar32Param();
        param.push_back(t);
    }
    cout << "Char32_t vector param length is: " << param.size() << endl;
    return param;
}

vector<string> GetStringVectorParam()
{
    vector<string> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        string t = GetStringParam();
        param.push_back(t);
    }
    cout << "String vector param length is: " << param.size() << endl;
    return param;
}

vector<int8_t> GetS8VectorParam()
{
    vector<int8_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int8_t temp = GetS8Param();
        param.push_back(temp);
    }
    cout << "Int8_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int16_t> GetS16VectorParam()
{
    vector<int16_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int16_t temp = GetS16Param();
        param.push_back(temp);
    }
    cout << "Int16_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int32_t> GetS32VectorParam()
{
    vector<int32_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int32_t temp = GetS32Param();
        param.push_back(temp);
    }
    cout << "Int32_t vector param length is: " << param.size() << endl;
    return param;
}

vector<int64_t> GetS64VectorParam()
{
    vector<int64_t> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        int64_t temp = GetS64Param();
        param.push_back(temp);
    }
    cout << "Int32_t vector param length is: " << param.size() << endl;
    return param;
}

std::shared_ptr<Parcel> GetParamParcel()
{
    return make_shared<Parcel>();
}
std::shared_ptr<Want> GetParamWant()
{
    return make_shared<Want>();
}

OHOS::AAFwk::Operation GetParamOperation()
{
    return OHOS::AAFwk::Operation();
}

std::shared_ptr<AsyncCommonEventResult> GetParamAsyncCommonEventResult()
{
    return make_shared<AsyncCommonEventResult>(
        GetIntParam(), GetStringParam(), GetBoolParam(), GetBoolParam(), GetParamSptrRemote());
}

std::shared_ptr<CommonEventData> GetParamCommonEventData()
{
    return make_shared<CommonEventData>();
}

std::shared_ptr<CommonEventManager> GetParamCommonEventManager()
{
    return make_shared<CommonEventManager>();
}

std::shared_ptr<CommonEventPublishInfo> GetParamCommonEventPublishInfo()
{
    return make_shared<CommonEventPublishInfo>();
}

std::shared_ptr<CommonEventSubscribeInfo> GetParamCommonEventSubscribeInfo()
{
    return make_shared<CommonEventSubscribeInfo>();
}

std::shared_ptr<CommonEventSubscriber> GetParamCommonEventSubscriber()
{
    return make_shared<TestCommonEventSubscriber>();
}

std::shared_ptr<CommonEventSupport> GetParamCommonEventSupport()
{
    return make_shared<CommonEventSupport>();
}

std::shared_ptr<MatchingSkills> GetParamMatchingSkills()
{
    return make_shared<MatchingSkills>();
}

sptr<IRemoteObject> GetParamSptrRemote()
{
    return sptr<TestRemoteObject>();
}

std::shared_ptr<EventRunner> GetParamEventRunner()
{
    return EventRunner::Create(GetCharArryParam());
}

std::shared_ptr<EventHandler> GetParamEventHandler()
{
    return make_shared<EventHandler>(GetParamEventRunner());
}

std::shared_ptr<OHOS::AppExecFwk::EventQueue> GetParamEventQueue()
{
    return make_shared<OHOS::AppExecFwk::EventQueue>();
}

std::shared_ptr<EventRunnerNativeImplement> GetParamEventRunnerNativeImplement()
{
    return make_shared<EventRunnerNativeImplement>(GetBoolParam());
}

std::shared_ptr<OHOS::AppExecFwk::FileDescriptorListener> GetParamFileDescriptorListener()
{
    return make_shared<TestFileDescriptorListener>();
}

TestDumper GetParamDumper()
{
    return GetClassParam<TestDumper>();
}

InnerEvent::Pointer GetParamInnerEvent()
{
    return InnerEvent::Get(GetU32Param(), GetS64Param());
}

CommonEventSubscribeInfo::ThreadMode GetParamThreadMode()
{
    switch (GetIntParam() % 4) {
        case 0:
            return CommonEventSubscribeInfo::ThreadMode::HANDLER;
            break;
        case 1:
            return CommonEventSubscribeInfo::ThreadMode::POST;
            break;
        case 2:
            return CommonEventSubscribeInfo::ThreadMode::ASYNC;
            break;
        case 3:
            return CommonEventSubscribeInfo::ThreadMode::BACKGROUND;
            break;
        default:
            return CommonEventSubscribeInfo::ThreadMode::HANDLER;
            break;
    }
}

EventQueue::Priority GetParamPriority()
{
    switch (GetIntParam() % 4) {
        case 0:
            return EventQueue::Priority::IMMEDIATE;
            break;
        case 1:
            return EventQueue::Priority::HIGH;
            break;
        case 2:
            return EventQueue::Priority::LOW;
            break;
        case 3:
            return EventQueue::Priority::IDLE;
            break;
        default:
            return EventQueue::Priority::LOW;
            break;
    }
}

std::shared_ptr<Logger> GetParamLogger()
{
    return make_shared<TestLogger>();
}

InnerEvent::Callback GetParamCallback()
{
    auto callback = []() { printf("Fuzz Test Inner Event Callback."); };
    return callback;
}

OHOS::AppExecFwk::InnerEvent::TimePoint GetParamTimePoint()
{
    std::chrono::steady_clock::time_point param = std::chrono::steady_clock::now();
    return param;
}

std::shared_ptr<AbilityStartSetting> GetParamAbilityStartSetting()
{
    return AbilityStartSetting::GetEmptySetting();
}

sptr<OHOS::AAFwk::IAbilityConnection> GetParamIAbilityConnection()
{
    return sptr<OHOS::AAFwk::IAbilityConnection>();
}

std::shared_ptr<OHOS::AppExecFwk::AbilityContext> GetParamAbilityContext()
{
    return make_shared<OHOS::AppExecFwk::AbilityContext>();
}
std::shared_ptr<OHOS::AppExecFwk::IAbilityEvent> GetParamIAbilityEvent()
{
    return make_shared<TestAbilityEvent>();
}

sptr<OHOS::AppExecFwk::AbilityThread> GetParamAbilityThread()
{
    return sptr<OHOS::AppExecFwk::AbilityThread>();
}

std::shared_ptr<OHOS::AppExecFwk::AbilityHandler> GetParamAbilityHandler()
{
    return make_shared<OHOS::AppExecFwk::AbilityHandler>(GetParamEventRunner(), GetParamAbilityThread());
}

std::shared_ptr<OHOS::AppExecFwk::Ability> GetParamAbility()
{
    return make_shared<OHOS::AppExecFwk::Ability>();
}

std::shared_ptr<OHOS::AppExecFwk::ComponentContainer> GetParamComponentContainer()
{
    return make_shared<OHOS::AppExecFwk::ComponentContainer>();
}

// std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> GetParamAbilityInfo()
// {
//     return make_shared<OHOS::AppExecFwk::AbilityInfo>();
// }

std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> GetParamOHOSApplication()
{
    return make_shared<OHOS::AppExecFwk::OHOSApplication>();
}

std::shared_ptr<OHOS::KeyEvent> GetParamKeyEvent()
{
    return make_shared<OHOS::KeyEvent>();
}

OHOS::Uri GetParamUri()
{
    return OHOS::Uri(GetStringParam());
}

NativeRdb::ValuesBucket GetParamValuesBucket()
{
    if (GetBoolParam()) {
        NativeRdb::ValuesBucket val;
        val.PutNull(GetStringParam());
        return val;
    } else {
        return NativeRdb::ValuesBucket();
    }
}

OHOS::AppExecFwk::Configuration GetParamConfiguration()
{
    if (GetBoolParam()) {
        return OHOS::AppExecFwk::Configuration(GetStringParam());
    } else {
        return OHOS::AppExecFwk::Configuration();
    }
}

NativeRdb::DataAbilityPredicates GetParamDataAbilityPredicates()
{
    if (GetBoolParam()) {
        return NativeRdb::DataAbilityPredicates(GetStringParam());
    } else {
        return NativeRdb::DataAbilityPredicates();
    }
}

OHOS::AppExecFwk::PacMap GetParamPacMap()
{
    return OHOS::AppExecFwk::PacMap();
}

OHOS::AppExecFwk::NotificationRequest GetParamNotificationRequest()
{
    return OHOS::AppExecFwk::NotificationRequest();
}

std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> GetParamProcessInfo()
{
    pid_t id = GetIntParam();
    if (GetBoolParam()) {
        return make_shared<OHOS::AppExecFwk::ProcessInfo>(GetStringParam(), id);
    } else {
        return make_shared<OHOS::AppExecFwk::ProcessInfo>();
    }
}

std::shared_ptr<OHOS::AppExecFwk::DataUriUtils> GetParamDataUriUtils()
{
    return make_shared<OHOS::AppExecFwk::DataUriUtils>();
}

std::shared_ptr<OHOS::AppExecFwk::Context> GetParamContext()
{
    return make_shared<OHOS::AppExecFwk::Ability>();
}

std::shared_ptr<OHOS::AppExecFwk::LifeCycle> GetParamLifeCycle()
{
    return make_shared<OHOS::AppExecFwk::LifeCycle>();
}

OHOS::AppExecFwk::LifeCycle::Event GetParamLifeCycleEvent()
{
    switch (GetIntParam() % 7) {
        case 0:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_ACTIVE;
            break;
        case 1:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_BACKGROUND;
            break;
        case 2:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_FOREGROUND;
            break;
        case 3:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_INACTIVE;
            break;
        case 4:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_START;
            break;
        case 5:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_STOP;
            break;
        case 6:
            return OHOS::AppExecFwk::LifeCycle::Event::UNDEFINED;
            break;
        default:
            return OHOS::AppExecFwk::LifeCycle::Event::ON_ACTIVE;
            break;
    }
}

std::shared_ptr<OHOS::AppExecFwk::ElementName> GetParamElementName()
{
    if (GetBoolParam()) {
        return make_shared<OHOS::AppExecFwk::ElementName>(GetStringParam(), GetStringParam(), GetStringParam());
    } else {
        return make_shared<OHOS::AppExecFwk::ElementName>();
    }
}

std::shared_ptr<OHOS::AAFwk::WantParams> GetParamWantParams()
{
    return make_shared<OHOS::AAFwk::WantParams>();
}

std::shared_ptr<OHOS::AppExecFwk::AbilityManager> GetParamAbilityManager()
{
    return make_shared<OHOS::AppExecFwk::AbilityManager>();
}

OHOS::AAFwk::PatternsMatcher GetParamPatternsMatcher()
{
    return OHOS::AAFwk::PatternsMatcher();
}

OHOS::AAFwk::MatchType GetParamMatchType()
{
    switch (GetIntParam() % 4) {
        case 0:
            return OHOS::AAFwk::MatchType::DEFAULT;
            break;
        case 1:
            return OHOS::AAFwk::MatchType::PREFIX;
            break;
        case 2:
            return OHOS::AAFwk::MatchType::PATTERN;
            break;
        case 3:
            return OHOS::AAFwk::MatchType::GLOBAL;
            break;
        default:
            return OHOS::AAFwk::MatchType::DEFAULT;
            break;
    }
}

std::shared_ptr<OHOS::AppExecFwk::BundleMgrProxy> GetParamBundleMgrProxy()
{
    return make_shared<OHOS::AppExecFwk::BundleMgrProxy>(GetParamSptrRemote());
}

OHOS::AppExecFwk::ApplicationFlag GetParamApplicationFlag()
{
    if (GetBoolParam()) {
        return OHOS::AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO;
    } else {
        return OHOS::AppExecFwk::ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMS;
    }
}

OHOS::AppExecFwk::ApplicationInfo GetParamApplicationInfo()
{
    return OHOS::AppExecFwk::ApplicationInfo();
}

std::vector<OHOS::AppExecFwk::ApplicationInfo> GetParamApplicationInfoVector()
{
    vector<OHOS::AppExecFwk::ApplicationInfo> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        OHOS::AppExecFwk::ApplicationInfo t = GetParamApplicationInfo();
        param.push_back(t);
    }
    cout << "ApplicationInfo vector param length is: " << param.size() << endl;
    return param;
}

OHOS::AppExecFwk::BundleFlag GetParamBundleFlag()
{
    if (GetBoolParam()) {
        return OHOS::AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT;
    } else {
        return OHOS::AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES;
    }
}

OHOS::AppExecFwk::BundleInfo GetParamBundleInfo()
{
    return OHOS::AppExecFwk::BundleInfo();
}

OHOS::AppExecFwk::HapModuleInfo GetParamHapModuleInfo()
{
    return OHOS::AppExecFwk::HapModuleInfo();
}

OHOS::AppExecFwk::PermissionDef GetParamPermissionDef()
{
    return OHOS::AppExecFwk::PermissionDef();
}

std::vector<OHOS::AppExecFwk::PermissionDef> GetParamPermissionDefVector()
{
    vector<OHOS::AppExecFwk::PermissionDef> param;
    size_t len = GenRandom(0, 255);
    while (len--) {
        OHOS::AppExecFwk::PermissionDef t = GetParamPermissionDef();
        param.push_back(t);
    }
    cout << "PermissionDef vector param length is: " << param.size() << endl;
    return param;
}

OHOS::AppExecFwk::IBundleMgr::Message GetParamIBundleMgrMessage()
{
    return (OHOS::AppExecFwk::IBundleMgr::Message)(GetIntParam() % 36);
}

OHOS::MessageParcel GetParamMessageParcel()
{
    return OHOS::MessageParcel();
}

OHOS::AppExecFwk::DumpFlag GetParamDumpFlag()
{
    switch (GetIntParam() % 3) {
        case 0:
            return OHOS::AppExecFwk::DumpFlag::DUMP_BUNDLE_LIST;
            break;
        case 1:
            return OHOS::AppExecFwk::DumpFlag::DUMP_ALL_BUNDLE_INFO;
            break;
        case 2:
            return OHOS::AppExecFwk::DumpFlag::DUMP_BUNDLE_INFO;
            break;
        default:
            return OHOS::AppExecFwk::DumpFlag::DUMP_BUNDLE_LIST;
            break;
    }
}

sptr<OHOS::AppExecFwk::ICleanCacheCallback> GetParamICleanCacheCallback()
{
    return sptr<TestICleanCacheCallback>();
}

sptr<OHOS::AppExecFwk::IBundleStatusCallback> GetParamIBundleStatusCallback()
{
    return sptr<TestIBundleStatusCallback>();
}

std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper>GetParamDataAbilityHelper()
{
    return OHOS::AppExecFwk::DataAbilityHelper::Creator(std::make_shared<OHOS::AppExecFwk::Ability>());
}

}  // namespace EventFwk
}  // namespace OHOS
