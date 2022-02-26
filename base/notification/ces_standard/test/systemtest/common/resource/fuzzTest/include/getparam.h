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
#ifndef GET_PARAM_H
#define GET_PARAM_H
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#include "ability.h"
#include "ability_context.h"
#include "ability_event_interface.h"
#include "ability_info.h"
#include "ability_handler.h"
#include "ability_handler.h"
#include "ability_manager.h"
#include "ability_manager_interface.h"
#include "async_common_event_result.h"
#include "bundle_info.h"
#include "common_event_manager.h"
#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "data_uri_utils.h"
#include "dummy_component_container.h"
#include "dumper.h"
#include "element_name.h"
#include "event_handler.h"
#include "event_runner.h"
#include "inner_event.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "key_event.h"
#include "logger.h"
#include "matching_skills.h"
#include "module_info.h"
#include "native_implement_eventhandler.h"
#include "pac_map.h"
#include "parcel.h"
#include "patterns_matcher.h"
#include "uri.h"
#include "want.h"
#include "bundle_mgr_proxy.h"

namespace OHOS {
namespace EventFwk {
class TestDumper;
bool GetBoolParam();
uint8_t GetU8Param();
unsigned int GetUIntParam();
uint16_t GetU16Param();
uint32_t GetU32Param();
uint64_t GetU64Param();
int8_t GetS8Param();
int16_t GetS16Param();
int32_t GetS32Param();
int64_t GetS64Param();
char32_t GetChar32Param();

short GetShortParam();
long GetLongParam();
int GetIntParam();
double GetDoubleParam();
float GetFloatParam();
char GetCharParam();
char *GetCharArryParam();
std::string GetStringParam();
std::vector<bool> GetBoolVectorParam();
std::vector<short> GetShortVectorParam();
std::vector<long> GetLongVectorParam();
std::vector<int> GetIntVectorParam();
std::vector<float> GetFloatVectorParam();
std::vector<double> GetDoubleVectorParam();
std::vector<char> GetCharVectorParam();
std::vector<char32_t> GetChar32VectorParam();
std::vector<std::string> GetStringVectorParam();
template <class T>
std::vector<T> GetUnsignVectorParam();
std::vector<int8_t> GetS8VectorParam();
std::vector<int16_t> GetS16VectorParam();
std::vector<int32_t> GetS32VectorParam();
std::vector<int64_t> GetS64VectorParam();

std::shared_ptr<Parcel> GetParamParcel();
std::shared_ptr<Want> GetParamWant();
OHOS::AAFwk::Operation GetParamOperation();
std::shared_ptr<AsyncCommonEventResult> GetParamAsyncCommonEventResult();
std::shared_ptr<CommonEventData> GetParamCommonEventData();
std::shared_ptr<CommonEventManager> GetParamCommonEventManager();
std::shared_ptr<CommonEventPublishInfo> GetParamCommonEventPublishInfo();
std::shared_ptr<CommonEventSubscribeInfo> GetParamCommonEventSubscribeInfo();
std::shared_ptr<CommonEventSubscriber> GetParamCommonEventSubscriber();
std::shared_ptr<CommonEventSupport> GetParamCommonEventSupport();
std::shared_ptr<MatchingSkills> GetParamMatchingSkills();
sptr<IRemoteObject> GetParamSptrRemote();
std::shared_ptr<OHOS::AppExecFwk::EventRunner> GetParamEventRunner();
std::shared_ptr<OHOS::AppExecFwk::EventHandler> GetParamEventHandler();
std::shared_ptr<OHOS::AppExecFwk::EventQueue> GetParamEventQueue();
std::shared_ptr<EventRunnerNativeImplement> GetParamEventRunnerNativeImplement();
std::shared_ptr<OHOS::AppExecFwk::FileDescriptorListener> GetParamFileDescriptorListener();
std::shared_ptr<OHOS::AppExecFwk::Logger> GetParamLogger();
OHOS::AppExecFwk::EventQueue::Priority GetParamPriority();
TestDumper GetParamDumper();
OHOS::AppExecFwk::InnerEvent::Pointer GetParamInnerEvent();
OHOS::AppExecFwk::InnerEvent::Callback GetParamCallback();
OHOS::AppExecFwk::InnerEvent::TimePoint GetParamTimePoint();
CommonEventSubscribeInfo::ThreadMode GetParamThreadMode();

std::shared_ptr<OHOS::AppExecFwk::AbilityContext> GetParamAbilityContext();
std::shared_ptr<OHOS::AppExecFwk::IAbilityEvent> GetParamIAbilityEvent();
sptr<OHOS::AppExecFwk::AbilityThread> GetParamAbilityThread();
std::shared_ptr<OHOS::AppExecFwk::AbilityHandler> GetParamAbilityHandler();
std::shared_ptr<OHOS::AppExecFwk::AbilityStartSetting> GetParamAbilityStartSetting();
std::shared_ptr<OHOS::AppExecFwk::Ability> GetParamAbility();
std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> GetParamOHOSApplication();
std::shared_ptr<OHOS::KeyEvent> GetParamKeyEvent();
OHOS::Uri GetParamUri();

NativeRdb::ValuesBucket GetParamValuesBucket();
OHOS::AppExecFwk::Configuration GetParamConfiguration();
NativeRdb::DataAbilityPredicates GetParamDataAbilityPredicates();
OHOS::AppExecFwk::PacMap GetParamPacMap();
OHOS::AppExecFwk::NotificationRequest GetParamNotificationRequest();
std::shared_ptr<OHOS::AppExecFwk::ComponentContainer> GetParamComponentContainer();
sptr<OHOS::AAFwk::IAbilityConnection> GetParamIAbilityConnection();

std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> GetParamProcessInfo();
std::shared_ptr<OHOS::AppExecFwk::DataUriUtils> GetParamDataUriUtils();
std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> GetParamDataAbilityHelper();
std::shared_ptr<OHOS::AppExecFwk::LifeCycle> GetParamLifeCycle();
OHOS::AppExecFwk::LifeCycle::Event GetParamLifeCycleEvent();
std::shared_ptr<OHOS::AppExecFwk::ElementName> GetParamElementName();
std::shared_ptr<OHOS::AAFwk::WantParams> GetParamWantParams();
std::shared_ptr<OHOS::AppExecFwk::AbilityManager> GetParamAbilityManager();
OHOS::AAFwk::PatternsMatcher GetParamPatternsMatcher();
OHOS::AAFwk::MatchType GetParamMatchType();
std::shared_ptr<OHOS::AppExecFwk::Context> GetParamContext();

std::shared_ptr<OHOS::AppExecFwk::BundleMgrProxy> GetParamBundleMgrProxy();
OHOS::AppExecFwk::ApplicationFlag GetParamApplicationFlag();
OHOS::AppExecFwk::ApplicationInfo GetParamApplicationInfo();
std::vector<OHOS::AppExecFwk::ApplicationInfo> GetParamApplicationInfoVector();
OHOS::AppExecFwk::BundleFlag GetParamBundleFlag();
OHOS::AppExecFwk::BundleInfo GetParamBundleInfo();
std::vector<OHOS::AppExecFwk::BundleInfo> GetParamBundleInfoVector();
OHOS::AppExecFwk::HapModuleInfo GetParamHapModuleInfo();
OHOS::AppExecFwk::PermissionDef GetParamPermissionDef();
std::vector<OHOS::AppExecFwk::PermissionDef> GetParamPermissionDefVector();
OHOS::AppExecFwk::IBundleMgr::Message GetParamIBundleMgrMessage();
OHOS::MessageParcel GetParamMessageParcel();
OHOS::AppExecFwk::DumpFlag GetParamDumpFlag();
sptr<OHOS::AppExecFwk::ICleanCacheCallback> GetParamICleanCacheCallback();
sptr<OHOS::AppExecFwk::IBundleStatusCallback> GetParamIBundleStatusCallback();

class TestRemoteObject : public IRemoteObject {
public:
    TestRemoteObject();
    ~TestRemoteObject();

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class TestCommonEventSubscriber : public CommonEventSubscriber {
public:
    TestCommonEventSubscriber(){};
    ~TestCommonEventSubscriber(){};
    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        printf("Fuzz Test Reveive Event\n");
    }
};

class TestDumper : public OHOS::AppExecFwk::Dumper {
public:
    void Dump(const std::string &message)
    {
        return;
    }
    std::string GetTag()
    {
        return GetStringParam();
    }
};

class TestFileDescriptorListener : public OHOS::AppExecFwk::FileDescriptorListener {
public:
    TestFileDescriptorListener()
    {}
    ~TestFileDescriptorListener()
    {}
};

class TestIAbilityConnection : public OHOS::AAFwk::IAbilityConnection {
public:
    void OnAbilityConnectDone(
        const OHOS::AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const OHOS::AppExecFwk::ElementName &element, int resultCode) override
    {}
    virtual ~TestIAbilityConnection()
    {}
};

class TestAbilityEvent : public OHOS::AppExecFwk::IAbilityEvent {
public:
    virtual void OnBackPressed()
    {
        printf("Fuzz Test Back Pressed.");
    }
};

class TestLogger : public OHOS::AppExecFwk::Logger {
public:
    void Log(const std::string &line)
    {}
    virtual ~TestLogger()
    {}
};

class TestICleanCacheCallback : public OHOS::AppExecFwk::ICleanCacheCallback {
public:
    TestICleanCacheCallback()
    {}
    void OnCleanCacheFinished(bool succeeded) override
    {}
    virtual ~TestICleanCacheCallback()
    {}
};

class TestIBundleStatusCallback : public OHOS::AppExecFwk::IBundleStatusCallback {
public:
    TestIBundleStatusCallback()
    {}
    void OnBundleStateChanged(const uint8_t installType, const int32_t resultCode, const std::string &resultMsg,
        const std::string &bundleName) override
    {}
    virtual ~TestIBundleStatusCallback()
    {}
};

}  // namespace EventFwk
}  // namespace OHOS
#endif