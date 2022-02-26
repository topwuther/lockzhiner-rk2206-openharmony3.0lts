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
#define private public
#define protected public

#include "hilog_wrapper.h"

// event/commonevent
#include "async_common_event_result.h"
#include "common_event_manager.h"
#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "matching_skills.h"

// event/eventhandle
#include "event_handler.h"
#include "event_queue.h"
#include "event_runner.h"
#include "inner_event.h"
#include "dumper.h"
#include "logger.h"

// aafwk/ability
#include "ability.h"
#include "data_ability_helper.h"
#include "ability_lifecycle_interface.h"
#include "ability_lifecycle.h"
#include "data_uri_utils.h"
#include "ability_manager.h"

// AMS
#include "ohos_application.h"
#include "ability_context.h"
#include "data_uri_utils.h"
#include "element_name.h"
#include "want.h"
#include "ability_record_mgr.h"

// aafwk/bundle
#include "bundle_mgr_proxy.h"
#include "ohos/aafwk/base/short_wrapper.h"
#include "../include/getparam.h"
#include "../include/fuzzTestManager.h"

#include "../include/fuzzConfigParser.h"

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"

#include <unistd.h>
#include <signal.h>

#undef private
#undef protected
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace EventFwk {
std::shared_ptr<fuzzTestManager> fuzzTestManager::instance_ = nullptr;

// RegisterAsyncCommonEventResult
void fuzzTestManager::RegisterAsyncCommonEventResult()
{
    callFunctionMap_["AsyncCommonEventResultSetCode"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp.get()->SetCode(GetU32Param());
        std::cout << temp->GetCode();
    };
    callFunctionMap_["AsyncCommonEventResultGetCode"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->GetCode();
    };
    callFunctionMap_["AsyncCommonEventResultSetData"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->SetData(GetStringParam());
    };
    callFunctionMap_["AsyncCommonEventResultGetData"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->GetData();
    };
    callFunctionMap_["AsyncCommonEventResultSetCodeAndData"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->SetCodeAndData(GetIntParam(), GetStringParam());
    };
    callFunctionMap_["AsyncCommonEventResultAbortCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->AbortCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultClearAbortCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->ClearAbortCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultGetAbortCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->GetAbortCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultFinishCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->FinishCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultIsOrderedCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->IsOrderedCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultIsStickyCommonEvent"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->IsStickyCommonEvent();
    };
    callFunctionMap_["AsyncCommonEventResultCheckSynchronous"] = []() {
        std::shared_ptr<AsyncCommonEventResult> temp = GetParamAsyncCommonEventResult();
        temp->CheckSynchronous();
    };
}

// RegisterCommonEventData
void fuzzTestManager::RegisterCommonEventData()
{
    callFunctionMap_["CommonEventDataSetWant"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->SetWant(*(GetParamWant().get()));
    };
    callFunctionMap_["CommonEventDataGetWant"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->GetWant();
    };
    callFunctionMap_["CommonEventDataSetCode"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->SetCode(GetIntParam());
    };
    callFunctionMap_["CommonEventDataGetCode"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->GetCode();
    };
    callFunctionMap_["CommonEventDataSetData"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->SetData(GetStringParam());
    };
    callFunctionMap_["CommonEventDataGetData"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        temp->GetData();
    };
    callFunctionMap_["CommonEventDataMarshalling"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["CommonEventDataUnmarshalling"] = []() {
        std::shared_ptr<CommonEventData> temp = GetParamCommonEventData();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// RegisterCommonEventManager
void fuzzTestManager::RegisterCommonEventManager()
{
    callFunctionMap_["CommonEventManagerPublishCommonEventCommonEventData"] = []() {
        std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
        temp->PublishCommonEvent(*(GetParamCommonEventData().get()));
    };
    callFunctionMap_["CommonEventManagerPublishCommonEventCommonEventDataCommonEventPublishInfo"] = []() {
        std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
        temp->PublishCommonEvent(*(GetParamCommonEventData().get()), *(GetParamCommonEventPublishInfo().get()));
    };
    callFunctionMap_["CommonEventManagerPublishCommonEventCommonEventDataCommonEventPublishInfoCommonEventSubscriber"] =
        []() {
            std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
            temp->PublishCommonEvent(*(GetParamCommonEventData().get()),
                *(GetParamCommonEventPublishInfo().get()),
                GetParamCommonEventSubscriber());
        };
    callFunctionMap_["CommonEventManagerSubscribeCommonEvent"] = []() {
        std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
        temp->SubscribeCommonEvent(GetParamCommonEventSubscriber());
    };
    callFunctionMap_["CommonEventManagerUnSubscribeCommonEvent"] = []() {
        std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
        temp->UnSubscribeCommonEvent(GetParamCommonEventSubscriber());
    };
    callFunctionMap_["CommonEventManagerGetStickyCommonEvent"] = []() {
        std::shared_ptr<CommonEventManager> temp = GetParamCommonEventManager();
        temp->GetStickyCommonEvent(GetStringParam(), *(GetParamCommonEventData()));
    };
}

// RegisterCommonEventPublishInfo
void fuzzTestManager::RegisterCommonEventPublishInfo()
{
    callFunctionMap_["CommonEventPublishInfoSetSticky"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->SetSticky(GetBoolParam());
    };
    callFunctionMap_["CommonEventPublishInfoIsSticky"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->IsSticky();
    };
    callFunctionMap_["CommonEventPublishInfoSetSubscriberPermissions"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->SetSubscriberPermissions(GetStringVectorParam());
    };
    callFunctionMap_["CommonEventPublishInfoGetSubscriberPermissions"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->GetSubscriberPermissions();
    };
    callFunctionMap_["CommonEventPublishInfoSetOrdered"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->SetOrdered(GetBoolParam());
    };
    callFunctionMap_["CommonEventPublishInfoIsOrdered"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        temp->IsOrdered();
    };
    callFunctionMap_["CommonEventPublishInfoMarshalling"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        Parcel p;
        temp->Marshalling(p);
    };

    callFunctionMap_["CommonEventPublishInfoUnmarshalling"] = []() {
        std::shared_ptr<CommonEventPublishInfo> temp = GetParamCommonEventPublishInfo();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// RegisterCommonEventSubscribeInfo
void fuzzTestManager::RegisterCommonEventSubscribeInfo()
{
    callFunctionMap_["CommonEventSubscribeInfoSetPriority"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->SetPriority(GetS32Param());
    };
    callFunctionMap_["CommonEventSubscribeInfoGetPriority"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->GetPriority();
    };
    callFunctionMap_["CommonEventSubscribeInfoSetPermission"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->SetPermission(GetStringParam());
    };
    callFunctionMap_["CommonEventSubscribeInfoGetPermission"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->GetPermission();
    };
    callFunctionMap_["CommonEventSubscribeInfoSetThreadMode"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->SetThreadMode(GetParamThreadMode());
    };
    callFunctionMap_["CommonEventSubscribeInfoSetDeviceId"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->SetDeviceId(GetStringParam());
    };
    callFunctionMap_["CommonEventSubscribeInfoGetDeviceId"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->GetDeviceId();
    };
    callFunctionMap_["CommonEventSubscribeInfoGetMatchingSkills"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        temp->GetMatchingSkills();
    };
    callFunctionMap_["CommonEventSubscribeInfoMarshalling"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["CommonEventSubscribeInfoUnmarshalling"] = []() {
        std::shared_ptr<CommonEventSubscribeInfo> temp = GetParamCommonEventSubscribeInfo();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// RegisterCommonEventSubscriber
void fuzzTestManager::RegisterCommonEventSubscriber()
{
    callFunctionMap_["CommonEventSubscriberOnReceiveEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->OnReceiveEvent(*(GetParamCommonEventData().get()));
    };
    callFunctionMap_["CommonEventSubscriberGetSubscribeInfo"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->GetSubscribeInfo();
    };
    callFunctionMap_["CommonEventSubscriberSetCode"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->SetCode(GetIntParam());
    };
    callFunctionMap_["CommonEventSubscriberGetCode"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->GetCode();
    };
    callFunctionMap_["CommonEventSubscriberSetData"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->SetData(GetStringParam());
    };
    callFunctionMap_["CommonEventSubscriberGetData"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->GetData();
    };
    callFunctionMap_["CommonEventSubscriberSetCodeAndData"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->SetCodeAndData(GetIntParam(), GetStringParam());
    };
    callFunctionMap_["CommonEventSubscriberAbortCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->AbortCommonEvent();
    };
    callFunctionMap_["CommonEventSubscriberClearAbortCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->ClearAbortCommonEvent();
    };
    callFunctionMap_["CommonEventSubscriberGetAbortCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->GetAbortCommonEvent();
    };
    callFunctionMap_["CommonEventSubscriberGoAsyncCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->GoAsyncCommonEvent();
    };
    callFunctionMap_["CommonEventSubscriberIsOrderedCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->IsOrderedCommonEvent();
    };
    callFunctionMap_["CommonEventSubscriberIsStickyCommonEvent"] = []() {
        std::shared_ptr<CommonEventSubscriber> temp = GetParamCommonEventSubscriber();
        temp->IsStickyCommonEvent();
    };
}

// RegisterCommonEventSupport
void fuzzTestManager::RegisterCommonEventSupport()
{
    callFunctionMap_["CommonEventSupportIsSystemEvent"] = []() {
        std::shared_ptr<CommonEventSupport> temp = GetParamCommonEventSupport();
        std::string s = GetStringParam();
        temp->IsSystemEvent(s);
    };
}

// RegisterMatchingSkills
void fuzzTestManager::RegisterMatchingSkills()
{
    callFunctionMap_["MatchingSkillsAddEntity"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->AddEntity(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsHasEntity"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->HasEntity(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsRemoveEntity"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->RemoveEntity(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsCountEntities"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->CountEntities();
    };
    callFunctionMap_["MatchingSkillsAddEvent"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->AddEvent(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsCountEvent"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->CountEvent();
    };
    callFunctionMap_["MatchingSkillsGetEvents"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->GetEvents();
    };
    callFunctionMap_["MatchingSkillsRemoveEvent"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->RemoveEvent(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsHasEvent"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->HasEvent(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsAddScheme"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->AddScheme(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsHasScheme"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->HasScheme(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsRemoveScheme"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->RemoveScheme(GetStringParam());
    };
    callFunctionMap_["MatchingSkillsCountSchemes"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->CountSchemes();
    };
    callFunctionMap_["MatchingSkillsMatch"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        temp->Match(*(GetParamWant().get()));
    };
    callFunctionMap_["MatchingSkillsMarshalling"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["MatchingSkillsUnmarshalling"] = []() {
        std::shared_ptr<MatchingSkills> temp = GetParamMatchingSkills();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// RegisterDumper
void fuzzTestManager::RegisterDumper()
{
    callFunctionMap_["DumperDump"] = []() {
        TestDumper temp;
        temp.Dump(GetStringParam());
    };
    callFunctionMap_["DumperGetTag"] = []() {
        TestDumper temp;
        temp.GetTag();
    };
}

// RegisterEventHandler
void fuzzTestManager::RegisterEventHandler()
{
    callFunctionMap_["EventHandlerCurrent"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->Current();
    };
    callFunctionMap_["EventHandlerSendEventPointer&int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendTimingEvent"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventPointerPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerSendEventPointer&&int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tint64_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetU32Param(), GetS64Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetU32Param(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetU32Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tshared_ptr<T>int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetU32Param(), GetParamEventHandler(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tweak_ptr<T>int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->SendEvent(GetU32Param(), wp, GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventuint32_tunique_ptr<T, D>&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendEvent(GetU32Param(), p, GetS64Param());
    };

    callFunctionMap_["EventHandlerSendEventuint32_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendEvent(GetU32Param());
    };

    callFunctionMap_["EventHandlerSendEventunique_ptr<T, D>&&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerSendImmediateEventPointer"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerSendImmediateEventuint32_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetU32Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendImmediateEventuint32_tshared_ptr<T>"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetU32Param(), GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerSendImmediateEventuint32_t:unique_ptr<T, D>&"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetU32Param(), GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerSendImmediateEventuint32_t:unique_ptr<T, D>&&"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendImmediateEvent(GetU32Param(), p);
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventPointer&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendImmediateEvent(GetU32Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventPointer&&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendHighPriorityEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventPointer&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendHighPriorityEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventuint32_tint64_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendHighPriorityEvent(GetU32Param(), GetS64Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventuint32_tshared_ptr<T>&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendHighPriorityEvent(GetU32Param(), GetParamEventHandler(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventuint32_tweak_ptr<T>&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->SendHighPriorityEvent(GetU32Param(), wp, GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventuint32_tunique_ptr<T, D>&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendHighPriorityEvent(GetU32Param(), p, GetS64Param());
    };

    callFunctionMap_["EventHandlerSendHighPriorityEventuint32_tunique_ptr<T, D>&&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendHighPriorityEvent(GetU32Param(), p, GetS64Param());
    };

    callFunctionMap_["EventHandlerPostTaskCallbackstringint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostTask(GetParamCallback(), GetStringParam(), GetS64Param());
    };

    callFunctionMap_["EventHandlerPostTaskCallbackPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostTask(GetParamCallback(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerPostTaskCallbackint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostTask(GetParamCallback(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerPostImmediateTaskCallbackstring"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostImmediateTask(GetParamCallback(), GetStringParam());
    };

    callFunctionMap_["EventHandlerPostHighPriorityTaskCallbackstringint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostHighPriorityTask(GetParamCallback(), GetStringParam(), GetS64Param());
    };

    callFunctionMap_["EventHandlerPostHighPriorityTaskCallbackint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostHighPriorityTask(GetParamCallback(), GetS64Param());
    };

    callFunctionMap_["EventHandlerPostIdleTaskCallbackstringint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostIdleTask(GetParamCallback(), GetStringParam(), GetS64Param());
    };

    callFunctionMap_["EventHandlerPostIdleTaskCallbackint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostIdleTask(GetParamCallback(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendSyncEventPointer&Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendSyncEvent(GetParamInnerEvent(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventPointer&&Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendSyncEvent(GetParamInnerEvent(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendSyncEvent(GetU32Param(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendSyncEvent(GetU32Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tshared_ptr<T>Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendSyncEvent(GetU32Param(), GetParamEventHandler(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tweak_ptr<T>Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->SendSyncEvent(GetU32Param(), wp, GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tunique_ptr<T, D>&Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendSyncEvent(GetU32Param(), p, GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendSyncEventuint32_tunique_ptr<T, D>&&Priority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendSyncEvent(GetU32Param(), p, GetParamPriority());
    };

    callFunctionMap_["EventHandlerPostSyncTaskCallbackstringPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostSyncTask(GetParamCallback(), GetStringParam(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerPostSyncTaskCallbackPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostSyncTask(GetParamCallback(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventPointerint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetParamInnerEvent(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventPointerint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetParamInnerEvent(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tint64_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetU32Param(), GetS64Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetU32Param(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetU32Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tshared_ptr<T>int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SendTimingEvent(GetU32Param(), GetParamEventHandler(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tweak_ptr<T>int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->SendTimingEvent(GetU32Param(), wp, GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tunique_ptr<T, D>&int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendTimingEvent(GetU32Param(), p, GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerSendTimingEventuint32_tunique_ptr<T, D>&&int64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->SendTimingEvent(GetU32Param(), p, GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerPostTimingTaskCallbackint64_tstringPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostTimingTask(GetParamCallback(), GetS64Param(), GetStringParam());
    };

    callFunctionMap_["EventHandlerPostTimingTaskCallbackint64_tPriority"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->PostTimingTask(GetParamCallback(), GetS64Param(), GetParamPriority());
    };

    callFunctionMap_["EventHandlerRemoveAllEvents"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveAllEvents();
    };

    callFunctionMap_["EventHandlerRemoveEvent"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveEvent(GetU32Param());
    };

    callFunctionMap_["EventHandlerRemoveEventuint32_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveEvent(GetU32Param(), GetS64Param());
    };

    callFunctionMap_["EventHandlerRemoveTask"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveTask(GetStringParam());
    };

    callFunctionMap_["EventHandlerAddFileDescriptorListener"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->AddFileDescriptorListener(GetS32Param(), GetU32Param(), GetParamFileDescriptorListener());
    };

    callFunctionMap_["EventHandlerRemoveAllFileDescriptorListeners"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveAllFileDescriptorListeners();
    };

    callFunctionMap_["EventHandlerRemoveFileDescriptorListener"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->RemoveFileDescriptorListener(GetS32Param());
    };

    callFunctionMap_["EventHandlerSetEventRunner"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->SetEventRunner(GetParamEventRunner());
    };

    callFunctionMap_["EventHandlerGetEventRunner"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->GetEventRunner();
    };

    callFunctionMap_["EventHandlerDistributeEvent"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->DistributeEvent(GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerDump"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        TestDumper param;
        temp->Dump(param);
    };

    callFunctionMap_["EventHandlerHasInnerEventuint32_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->HasInnerEvent(GetU32Param());
    };

    callFunctionMap_["EventHandlerHasInnerEventint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->HasInnerEvent(GetS64Param());
    };

    callFunctionMap_["EventHandlerGetEventNamePointer"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->GetEventName(GetParamInnerEvent());
    };

    callFunctionMap_["EventHandlerIsIdle"] = []() {
        std::shared_ptr<AppExecFwk::EventHandler> temp = GetParamEventHandler();
        temp->IsIdle();
    };
}

// RegisterEventQueue
void fuzzTestManager::RegisterEventQueue()
{
    callFunctionMap_["EventQueueInsert"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        AppExecFwk::InnerEvent::Pointer param = GetParamInnerEvent();
        temp->Insert(param, GetParamPriority());
    };

    callFunctionMap_["EventQueueRemoveOrphan"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->RemoveOrphan();
    };
    callFunctionMap_["EventQueueRemoveshared_ptr<EventHandler>"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Remove(GetParamEventHandler());
    };
    callFunctionMap_["EventQueueRemoveshared_ptr<EventHandler>uint32_t"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Remove(GetParamEventHandler(), GetU32Param());
    };
    callFunctionMap_["EventQueueRemoveshared_ptr<EventHandler>uint32_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Remove(GetParamEventHandler(), GetU32Param(), GetS64Param());
    };
    callFunctionMap_["EventQueueRemoveshared_ptr<EventHandler>string"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Remove(GetParamEventHandler(), GetStringParam());
    };
    callFunctionMap_["EventQueueAddFileDescriptor"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->AddFileDescriptorListener(GetS32Param(), GetU32Param(), GetParamFileDescriptorListener());
    };
    callFunctionMap_["EventQueueRemoveFileDescriptorListenershared_ptr<EventHandler>"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->RemoveFileDescriptorListener(GetParamEventHandler());
    };
    callFunctionMap_["EventQueueRemoveFileDescriptorListenerint32_t"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->RemoveFileDescriptorListener(GetS32Param());
    };
    callFunctionMap_["EventQueuePrepare"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Prepare();
    };
    callFunctionMap_["EventQueueFinish"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->Finish();
    };
    callFunctionMap_["EventQueueGetEvent"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->GetEvent();
    };
    callFunctionMap_["EventQueueGetExpiredEvent"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        AppExecFwk::InnerEvent::TimePoint param = GetParamTimePoint();
        temp->GetExpiredEvent(param);
    };
    callFunctionMap_["EventQueueDump"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        TestDumper param;
        temp->Dump(param);
    };
    callFunctionMap_["EventQueueIsIdle"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->IsIdle();
    };
    callFunctionMap_["EventQueueIsQueueEmpty"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->IsQueueEmpty();
    };
    callFunctionMap_["EventQueueHasInnerEventuint32_t"] = []() {
        std::shared_ptr<AppExecFwk::EventQueue> temp = GetParamEventQueue();
        temp->HasInnerEvent(GetParamEventHandler(), GetU32Param());
    };
}

// RegisterEventRunner
void fuzzTestManager::RegisterEventRunner()
{
    callFunctionMap_["EventRunnerCreatebool"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->Create(GetBoolParam());
    };
    callFunctionMap_["EventRunnerCreatechar*"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->Create(GetCharArryParam());
    };
    callFunctionMap_["EventRunnerCurrent"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->Current();
    };
    callFunctionMap_["EventRunnerRun"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->Run();
    };
    callFunctionMap_["EventRunnerStop"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->Stop();
    };
    callFunctionMap_["EventRunnerGetEventQueue"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->GetEventQueue();
    };
    callFunctionMap_["EventRunnerGetCurrentEventQueue"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->GetCurrentEventQueue();
    };
    callFunctionMap_["EventRunnerDump"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        TestDumper param;
        temp->Dump(param);
    };
    callFunctionMap_["EventRunnerSetLogger"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->SetLogger(GetParamLogger());
    };
    callFunctionMap_["EventRunnerGetThreadId"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->GetThreadId();
    };
    callFunctionMap_["EventRunnerIsCurrentRunnerThread"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->IsCurrentRunnerThread();
    };
    callFunctionMap_["EventRunnerGetMainEventRunner"] = []() {
        std::shared_ptr<AppExecFwk::EventRunner> temp = GetParamEventRunner();
        temp->GetMainEventRunner();
    };
}

// RegisterFileDescriptorListener
void fuzzTestManager::RegisterFileDescriptorListener()
{
    callFunctionMap_["FileDescriptorListenerOnReadable"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->OnReadable(GetS32Param());
    };
    callFunctionMap_["FileDescriptorListenerOnWritable"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->OnWritable(GetS32Param());
    };
    callFunctionMap_["FileDescriptorListenerOnShutdown"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->OnShutdown(GetS32Param());
    };
    callFunctionMap_["FileDescriptorListenerOnException"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->OnException(GetS32Param());
    };
    callFunctionMap_["FileDescriptorListenerGetOwner"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->GetOwner();
    };
    callFunctionMap_["FileDescriptorListenerSetOwner"] = []() {
        std::shared_ptr<AppExecFwk::FileDescriptorListener> temp = GetParamFileDescriptorListener();
        temp->SetOwner(GetParamEventHandler());
    };
}

// RegisterInnerEvent
void fuzzTestManager::RegisterInnerEvent()
{
    callFunctionMap_["InnerEventGetuint32_tint64_t"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Get(GetU32Param(), GetS64Param());
    };
    callFunctionMap_["InnerEventGetuint32_tshared_ptr<T>int64_t"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Get<AppExecFwk::EventRunner>(GetU32Param(), GetParamEventRunner(), GetS64Param());
    };
    callFunctionMap_["InnerEventGetuint32_tstd::weak_ptr<T>int64_t"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->Get(GetU32Param(), wp, GetS64Param());
    };

    callFunctionMap_["InnerEventGetuint32_tunique_ptr<T, D>&&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->Get(GetU32Param(), p, GetS64Param());
    };

    callFunctionMap_["InnerEventGetuint32_tunique_ptr<T, D>&int64_t"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->Get(GetU32Param(), p, GetS64Param());
    };

    callFunctionMap_["InnerEventGetuint32_tint64_tshared_ptr<T>"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Get<AppExecFwk::EventRunner>(GetU32Param(), GetS64Param(), GetParamEventRunner());
    };

    callFunctionMap_["InnerEventGetuint32_tint64_tweak_ptr<T>"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::weak_ptr<int> wp = std::make_shared<int>(1);
        temp->Get(GetU32Param(), GetS64Param(), wp);
    };

    callFunctionMap_["InnerEventGetuint32_tint64_tunique_ptr<T, D>&&"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->Get(GetU32Param(), GetS64Param(), p);
    };
    callFunctionMap_["InnerEventGetuint32_tint64_tunique_ptr<T, D>&"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        std::unique_ptr<int> p(new int, std::default_delete<int>());
        temp->Get(GetU32Param(), GetS64Param(), p);
    };
    callFunctionMap_["InnerEventGetCallbackstring"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Get(GetParamCallback(), GetStringParam());
    };
    callFunctionMap_["InnerEventGet"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Get();
    };
    callFunctionMap_["InnerEventGetOwner"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetOwner();
    };
    callFunctionMap_["InnerEventSetOwner"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->SetOwner(GetParamEventHandler());
    };
    callFunctionMap_["InnerEventGetHandleTime"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetHandleTime();
    };
    callFunctionMap_["InnerEventSetHandleTime"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        AppExecFwk::InnerEvent::TimePoint param = GetParamTimePoint();
        temp->SetHandleTime(param);
    };
    callFunctionMap_["InnerEventGetSendTime"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetSendTime();
    };
    callFunctionMap_["InnerEventSetSendTime"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        AppExecFwk::InnerEvent::TimePoint param = GetParamTimePoint();
        temp->SetSendTime(param);
    };
    callFunctionMap_["InnerEventGetInnerEventId"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetInnerEventId();
    };
    callFunctionMap_["InnerEventGetParam"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetParam();
    };
    callFunctionMap_["InnerEventGetSharedObject"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetSharedObject<int>();
    };
    callFunctionMap_["InnerEventGetUniqueObject"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetUniqueObject<int>();
    };
    callFunctionMap_["InnerEventGetUniqueObject2"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        using deleter = void (*)(uint32_t *);
        temp->GetUniqueObject<uint32_t, deleter>();
    };
    callFunctionMap_["InnerEventGetTaskName"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetTaskName();
    };
    callFunctionMap_["InnerEventGetTask"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->GetTask();
    };
    callFunctionMap_["InnerEventHasTask"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->HasTask();
    };
    callFunctionMap_["InnerEventDump"] = []() {
        std::shared_ptr<AppExecFwk::InnerEvent> temp = GetParamInnerEvent();
        temp->Dump();
    };
}

// RegisterEventRunnerNativeImplement
void fuzzTestManager::RegisterEventRunnerNativeImplement()
{
    callFunctionMap_["EventRunnerNativeImplementGetEventRunnerNativeObj"] = []() {
        std::shared_ptr<EventRunnerNativeImplement> temp = GetParamEventRunnerNativeImplement();
        temp->GetEventRunnerNativeObj();
    };
    callFunctionMap_["EventRunnerNativeImplementCreateEventRunnerNativeObj"] = []() {
        std::shared_ptr<EventRunnerNativeImplement> temp = GetParamEventRunnerNativeImplement();
        temp->CreateEventRunnerNativeObj();
    };
    callFunctionMap_["EventRunnerNativeImplementStopEventRunnerNativeObj"] = []() {
        std::shared_ptr<EventRunnerNativeImplement> temp = GetParamEventRunnerNativeImplement();
        temp->StopEventRunnerNativeObj();
    };
}

// RegisterAbilityManager
void fuzzTestManager::RegisterAbilityManager()
{
    callFunctionMap_["AbilityManagerStartAbility"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->StartAbility(*(GetParamWant().get()), GetIntParam());
    };
    callFunctionMap_["AbilityManagerMoveMissionToTop"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->MoveMissionToTop(GetIntParam());
    };
    callFunctionMap_["AbilityManagerGetAllStackInfo"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->GetAllStackInfo();
    };
    callFunctionMap_["AbilityManagerQueryRecentAbilityMissionInfo"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->QueryRecentAbilityMissionInfo(GetIntParam(), GetIntParam());
    };
    callFunctionMap_["AbilityManagerQueryRunningAbilityMissionInfo"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->QueryRunningAbilityMissionInfo(GetIntParam());
    };
    callFunctionMap_["AbilityManagerQueryRunningAbilityMissionInfo2"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->QueryRunningAbilityMissionInfo(GetIntParam());
    };
    callFunctionMap_["AbilityManagerRemoveMissions"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->RemoveMissions(GetIntVectorParam());
    };
    callFunctionMap_["AbilityManagerClearUpApplicationData"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->ClearUpApplicationData(GetStringParam());
    };
    callFunctionMap_["AbilityManagerGetAllRunningProcesses"] = []() {
        std::shared_ptr<AppExecFwk::AbilityManager> temp = GetParamAbilityManager();
        temp->GetAllRunningProcesses();
    };
}

// RegisterWantParams
void fuzzTestManager::RegisterWantParams()
{
    callFunctionMap_["WantParamsoperator="] = []() {
        AAFwk::WantParams want1;
        AAFwk::WantParams want2;
        want1 = want2;
    };

    callFunctionMap_["WantParamsGetParam"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->GetParam(GetStringParam());
    };
    callFunctionMap_["WantParamsGetParams"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->GetParams();
    };
    callFunctionMap_["WantParamsKeySet"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->KeySet();
    };
    callFunctionMap_["WantParamsRemove"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->Remove(GetStringParam());
    };
    callFunctionMap_["WantParamsHasParam"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->HasParam(GetStringParam());
    };
    callFunctionMap_["WantParamsSize"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->Size();
    };
    callFunctionMap_["WantParamsIsEmpty"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        temp->IsEmpty();
    };
    callFunctionMap_["WantParamsMarshalling"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["WantParamsUnmarshalling"] = []() {
        std::shared_ptr<AAFwk::WantParams> temp = GetParamWantParams();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// bundle
void fuzzTestManager::RegisterBundleMgrProxy()
{
    callFunctionMap_["BundleMgrProxyGetApplicationInfo"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        ApplicationInfo a;
        temp->GetApplicationInfo(GetStringParam(), GetParamApplicationFlag(), GetIntParam(), a);
    };

    callFunctionMap_["BundleMgrProxyGetApplicationInfos"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<ApplicationInfo> a;
        temp->GetApplicationInfos(GetParamApplicationFlag(), GetIntParam(), a);
    };

    callFunctionMap_["BundleMgrProxyGetBundleInfo"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        BundleInfo a;
        temp->GetBundleInfo(GetStringParam(), GetParamBundleFlag(), a);
    };

    callFunctionMap_["BundleMgrProxyGetBundleInfos"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<BundleInfo> a;
        temp->GetBundleInfos(GetParamBundleFlag(), a);
    };

    callFunctionMap_["BundleMgrProxyGetUidByBundleName"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->GetUidByBundleName(GetStringParam(), GetIntParam());
    };

    callFunctionMap_["BundleMgrProxyGetBundleNameForUid"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::string a;
        temp->GetBundleNameForUid(GetIntParam(), a);
    };

    callFunctionMap_["BundleMgrProxyGetBundleGids"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<int> a;
        temp->GetBundleGids(GetStringParam(), a);
    };

    callFunctionMap_["BundleMgrProxyGetAppType"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->GetAppType(GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyCheckIsSystemAppByUid"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->GetAppType(GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyGetBundleInfosByMetaData"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<BundleInfo> a;
        temp->GetBundleInfosByMetaData(GetStringParam(), a);
    };

    callFunctionMap_["BundleMgrProxyQueryAbilityInfo"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        AbilityInfo a;
        Want want;
        temp->QueryAbilityInfo(want, a);
    };

    callFunctionMap_["BundleMgrProxyQueryAbilityInfoByUri"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        AbilityInfo a;
        temp->QueryAbilityInfoByUri(GetStringParam(), a);
    };

    callFunctionMap_["BundleMgrProxyQueryKeepAliveBundleInfos"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<BundleInfo> a;
        temp->QueryKeepAliveBundleInfos(a);
    };

    callFunctionMap_["BundleMgrProxyGetAbilityLabel"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->GetAbilityLabel(GetStringParam(), GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyGetBundleArchiveInfo"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        BundleInfo a;
        temp->GetBundleArchiveInfo(GetStringParam(), GetParamBundleFlag(), a);
    };

    callFunctionMap_["BundleMgrProxyGetHapModuleInfo"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        HapModuleInfo moduleInfo;
        AbilityInfo abilityInfo;
        temp->GetHapModuleInfo(abilityInfo, moduleInfo);
    };

    callFunctionMap_["BundleMgrProxyGetLaunchWantForBundle"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        Want want;
        temp->GetLaunchWantForBundle(GetStringParam(), want);
    };

    callFunctionMap_["BundleMgrProxyCheckPublicKeys"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->CheckPublicKeys(GetStringParam(), GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyCheckPermission"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->CheckPermission(GetStringParam(), GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyGetPermissionDef"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        PermissionDef a;
        temp->GetPermissionDef(GetStringParam(), a);
    };
    callFunctionMap_["BundleMgrProxyGetAllPermissionGroupDefs"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<PermissionDef> a;
        temp->GetAllPermissionGroupDefs(a);
    };

    callFunctionMap_["BundleMgrProxyGetAppsGrantedPermissions"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<std::string> a;
        temp->GetAppsGrantedPermissions(GetStringVectorParam(), a);
    };

    callFunctionMap_["BundleMgrProxyGetSystemAvailableCapabilities"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::vector<std::string> a;
        temp->GetSystemAvailableCapabilities(a);
    };

    callFunctionMap_["BundleMgrProxyIsSafeMode"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->IsSafeMode();
    };

    callFunctionMap_["BundleMgrProxyCleanBundleCacheFiles"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->CleanBundleCacheFiles(GetStringParam(), GetParamICleanCacheCallback());
    };

    callFunctionMap_["BundleMgrProxyCleanBundleDataFiles"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->CleanBundleDataFiles(GetStringParam());
    };

    callFunctionMap_["BundleMgrProxyRegisterBundleStatusCallback"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->RegisterBundleStatusCallback(GetParamIBundleStatusCallback());
    };

    callFunctionMap_["BundleMgrProxyClearBundleStatusCallback"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->ClearBundleStatusCallback(GetParamIBundleStatusCallback());
    };

    callFunctionMap_["BundleMgrProxyUnregisterBundleStatusCallback"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->UnregisterBundleStatusCallback();
    };

    callFunctionMap_["BundleMgrProxyDumpInfos"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        std::string a;
        temp->DumpInfos(GetParamDumpFlag(), GetStringParam(), a);
    };

    callFunctionMap_["BundleMgrProxyIsApplicationEnabled"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->IsApplicationEnabled(GetStringParam());
    };

    callFunctionMap_["BundleMgrProxySetApplicationEnabled"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->SetApplicationEnabled(GetStringParam(), GetBoolParam());
    };

    callFunctionMap_["BundleMgrProxyGetBundleInstaller"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->GetBundleInstaller();
    };

    callFunctionMap_["BundleMgrProxyCanRequestPermission"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->CanRequestPermission(GetStringParam(), GetStringParam(), GetIntParam());
    };

    callFunctionMap_["BundleMgrProxyRequestPermissionFromUser"] = []() {
        std::shared_ptr<BundleMgrProxy> temp = GetParamBundleMgrProxy();
        temp->RequestPermissionFromUser(GetStringParam(), GetStringParam(), GetIntParam());
    };
}

// RegisterWant
void fuzzTestManager::RegisterWant()
{
    callFunctionMap_["Wantoperator="] = []() {
        Want want1;
        Want want2;
        want1 = want2;
    };
    callFunctionMap_["WantSetFlags"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetFlags(GetUIntParam());
    };
    callFunctionMap_["WantGetFlags"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetFlags();
    };
    callFunctionMap_["WantAddFlags"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->AddFlags(GetUIntParam());
    };
    callFunctionMap_["WantRemoveFlags"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->RemoveFlags(GetUIntParam());
    };
    callFunctionMap_["WantSetElementName"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetElementName(GetStringParam(), GetStringParam());
    };
    callFunctionMap_["WantSetElementName2"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetElementName(GetStringParam(), GetStringParam(), GetStringParam());
    };
    callFunctionMap_["WantSetElement"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetElement(*(GetParamElementName()));
    };
    callFunctionMap_["WantGetElement"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetElement();
    };
    callFunctionMap_["WantMakeMainAbility"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->MakeMainAbility(*(GetParamElementName()));
    };
    callFunctionMap_["WantWantParseUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->WantParseUri(GetCharArryParam());
    };
    callFunctionMap_["WantParseUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->ParseUri(GetStringParam());
    };
    callFunctionMap_["WantGetUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetUri();
    };
    callFunctionMap_["WantGetUriString"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetUriString();
    };
    callFunctionMap_["WantSetUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetUri(GetStringParam());
    };
    callFunctionMap_["WantSetUri2"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetUri(GetParamUri());
    };
    callFunctionMap_["WantSetUriAndType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetUriAndType(GetParamUri(), GetStringParam());
    };
    callFunctionMap_["WantWantToUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->WantToUri(*(GetParamWant().get()));
    };
    callFunctionMap_["WantToUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->ToUri();
    };
    callFunctionMap_["WantFormatUri"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->FormatUri(GetStringParam());
    };
    callFunctionMap_["WantFormatUri2"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->FormatUri(GetParamUri());
    };
    callFunctionMap_["WantGetAction"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetAction();
    };
    callFunctionMap_["WantSetAction"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetAction(GetStringParam());
    };
    callFunctionMap_["WantGetBundle"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetBundle();
    };
    callFunctionMap_["WantSetBundle"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetBundle(GetStringParam());
    };
    callFunctionMap_["WantGetEntities"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetEntities();
    };
    callFunctionMap_["WantAddEntity"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->AddEntity(GetStringParam());
    };
    callFunctionMap_["WantRemoveEntity"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->RemoveEntity(GetStringParam());
    };
    callFunctionMap_["WantHasEntity"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->HasEntity(GetStringParam());
    };
    callFunctionMap_["WantCountEntities"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->CountEntities();
    };
    callFunctionMap_["WantGetScheme"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetScheme();
    };
    callFunctionMap_["WantGetType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetType();
    };
    callFunctionMap_["WantSetType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetType(GetStringParam());
    };
    callFunctionMap_["WantFormatType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->FormatType(GetStringParam());
    };
    callFunctionMap_["WantFormatUriAndType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->FormatUriAndType(GetParamUri(), GetStringParam());
    };
    callFunctionMap_["WantFormatMimeType"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->FormatMimeType(GetStringParam());
    };
    callFunctionMap_["WantClearWant"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->ClearWant(GetParamWant().get());
    };
    callFunctionMap_["WantGetParams"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetParams();
    };
    callFunctionMap_["WantSetParams"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParams(*(GetParamWantParams()));
    };
    callFunctionMap_["WantGetBoolParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetBoolParam(GetStringParam(), GetBoolParam());
    };
    callFunctionMap_["WantGetBoolArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetBoolArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetBoolParam());
    };
    callFunctionMap_["WantSetParam2"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetBoolVectorParam());
    };
    callFunctionMap_["WantGetByteParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetByteParam(GetStringParam(), GetCharParam());
    };
    callFunctionMap_["WantGetByteArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetByteArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam3"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetCharParam());
    };
    callFunctionMap_["WantSetParam4"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetCharVectorParam());
    };
    callFunctionMap_["WantGetCharParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetCharParam(GetStringParam(), GetChar32Param());
    };
    callFunctionMap_["WantGetCharArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetCharArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam5"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetChar32Param());
    };
    callFunctionMap_["WantSetParam6"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetChar32VectorParam());
    };
    callFunctionMap_["WantGetIntParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetIntParam(GetStringParam(), GetIntParam());
    };
    callFunctionMap_["WantGetIntArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetIntArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam7"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetIntParam());
    };
    callFunctionMap_["WantSetParam8"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetIntVectorParam());
    };
    callFunctionMap_["WantGetDoubleParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetDoubleParam(GetStringParam(), GetDoubleParam());
    };
    callFunctionMap_["WantGetDoubleArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetDoubleArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam9"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetDoubleParam());
    };
    callFunctionMap_["WantSetParam10"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetDoubleVectorParam());
    };
    callFunctionMap_["WantGetFloatParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetFloatParam(GetStringParam(), GetFloatParam());
    };
    callFunctionMap_["WantGetFloatArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetFloatArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam11"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetFloatParam());
    };
    callFunctionMap_["WantSetParam12"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetFloatVectorParam());
    };
    callFunctionMap_["WantGetLongParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetLongParam(GetStringParam(), GetLongParam());
    };
    callFunctionMap_["WantGetLongArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetLongArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam13"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetLongParam());
    };
    callFunctionMap_["WantSetParam14"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetLongVectorParam());
    };
    callFunctionMap_["WantGetShortParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetShortParam(GetStringParam(), GetShortParam());
    };
    callFunctionMap_["WantGetShortArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetShortArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam15"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetShortParam());
    };
    callFunctionMap_["WantSetParam16"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetShortVectorParam());
    };
    callFunctionMap_["WantGetStringParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetStringParam(GetStringParam());
    };
    callFunctionMap_["WantGetStringArrayParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetStringArrayParam(GetStringParam());
    };
    callFunctionMap_["WantSetParam17"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetStringParam());
    };
    callFunctionMap_["WantSetParam18"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetParam(GetStringParam(), GetStringVectorParam());
    };
    callFunctionMap_["WantHasParameter"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->HasParameter(GetStringParam());
    };
    callFunctionMap_["WantReplaceParams"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->ReplaceParams(*(GetParamWantParams().get()));
    };
    callFunctionMap_["WantReplaceParams2"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->ReplaceParams(*(GetParamWant().get()));
    };
    callFunctionMap_["WantRemoveParam"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->RemoveParam(GetStringParam());
    };
    callFunctionMap_["WantGetOperation"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->GetOperation();
    };
    callFunctionMap_["WantSetOperation"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->SetOperation(GetParamOperation());
    };
    callFunctionMap_["WantOperationEquals"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->OperationEquals(*(GetParamWant().get()));
    };
    callFunctionMap_["WantCloneOperation"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        temp->CloneOperation();
    };
    callFunctionMap_["WantMarshalling"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["WantUnmarshalling"] = []() {
        std::shared_ptr<AAFwk::Want> temp = GetParamWant();
        Parcel p;
        temp->Unmarshalling(p);
    };
}

// RegisterElementName
void fuzzTestManager::RegisterElementName()
{
    callFunctionMap_["ElementNameGetURI"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->GetURI();
    };
    callFunctionMap_["ElementNameoperator=="] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        std::shared_ptr<AppExecFwk::ElementName> temp2 = GetParamElementName();
        if (*temp == *temp2) {
            return;
        };
    };
    callFunctionMap_["ElementNameSetDeviceID"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetDeviceID(GetStringParam());
    };
    callFunctionMap_["ElementNameGetDeviceID"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->GetDeviceID();
    };
    callFunctionMap_["ElementNameSetBundleName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetBundleName(GetStringParam());
    };
    callFunctionMap_["ElementNameGetBundleName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->GetBundleName();
    };
    callFunctionMap_["ElementNameSetAbilityName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetAbilityName(GetStringParam());
    };
    callFunctionMap_["ElementNameGetAbilityName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->GetAbilityName();
    };
    callFunctionMap_["ElementNameReadFromParcel"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        Parcel p;
        temp->ReadFromParcel(p);
    };
    callFunctionMap_["ElementNameMarshalling"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        Parcel p;
        temp->Marshalling(p);
    };
    callFunctionMap_["ElementNameUnmarshalling"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        Parcel p;
        temp->Unmarshalling(p);
    };
    callFunctionMap_["ElementNameSetElementDeviceID"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetElementDeviceID(GetParamElementName().get(), GetCharArryParam());
    };
    callFunctionMap_["ElementNameSetElementBundleName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetElementBundleName(GetParamElementName().get(), GetCharArryParam());
    };
    callFunctionMap_["ElementNameSetElementAbilityName"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->SetElementAbilityName(GetParamElementName().get(), GetCharArryParam());
    };
    callFunctionMap_["ElementNameClearElement"] = []() {
        std::shared_ptr<AppExecFwk::ElementName> temp = GetParamElementName();
        temp->ClearElement(GetParamElementName().get());
    };
}

void fuzzTestManager::RegisterOHOSApplication()
{
    callFunctionMap_.emplace("OHOSApplicationDumpApplication", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->DumpApplication();
    });

    callFunctionMap_.emplace("OHOSApplicationSetAbilityRecordMgr", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        auto ar = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
        temp->SetAbilityRecordMgr(ar);
    });

    callFunctionMap_.emplace("OHOSApplicationRegisterAbilityLifecycleCallbacks", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->RegisterAbilityLifecycleCallbacks(std::shared_ptr<OHOS::AppExecFwk::AbilityLifecycleCallbacks>(nullptr));
    });

    callFunctionMap_.emplace("OHOSApplicationUnregisterAbilityLifecycleCallbacks", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->UnregisterAbilityLifecycleCallbacks(
            std::shared_ptr<OHOS::AppExecFwk::AbilityLifecycleCallbacks>(nullptr));
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityStart", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityStart(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityInactive", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityInactive(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityBackground", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityBackground(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityForeground", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityForeground(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityActive", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityActive(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilityStop", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilityStop(GetParamAbility());
    });

    callFunctionMap_.emplace("OHOSApplicationDispatchAbilitySavedState", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->DispatchAbilitySavedState(GetParamPacMap());
    });

    callFunctionMap_.emplace("OHOSApplicationOnAbilitySaveState", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnAbilitySaveState(GetParamPacMap());
    });

    callFunctionMap_.emplace("OHOSApplicationRegisterElementsCallbacks", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->RegisterElementsCallbacks(std::shared_ptr<OHOS::AppExecFwk::ElementsCallback>(nullptr));
    });

    callFunctionMap_.emplace("OHOSApplicationUnregisterElementsCallbacks", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->UnregisterElementsCallbacks(std::shared_ptr<OHOS::AppExecFwk::ElementsCallback>(nullptr));
    });

    callFunctionMap_.emplace("OHOSApplicationOnConfigurationUpdated", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnConfigurationUpdated(OHOS::AppExecFwk::Configuration());
    });

    callFunctionMap_.emplace("OHOSApplicationOnMemoryLevel", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnMemoryLevel(GetIntParam());
    });

    callFunctionMap_.emplace("OHOSApplicationOnForeground", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnForeground();
    });

    callFunctionMap_.emplace("OHOSApplicationOnBackground", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnBackground();
    });

    callFunctionMap_.emplace("OHOSApplicationOnStart", []() {
        std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> temp = GetParamOHOSApplication();
        temp->OnStart();
    });
}

void fuzzTestManager::RegisterAbility()
{
    const int COLOR_R = 100;
    const int COLOR_G = 100;
    const int COLOR_B = 100;

    callFunctionMap_.emplace("AbilityTerminateAbilityWant", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        OHOS::AAFwk::Want param;
        temp->TerminateAbility(param);
    });

    callFunctionMap_.emplace("AbilityAddActionRoute", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->AddActionRoute("", "");
    });

    callFunctionMap_.emplace("AbilitySetWindowBackgroundColor", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetWindowBackgroundColor(COLOR_R, COLOR_G, COLOR_B);
    });

    callFunctionMap_.emplace("AbilityTerminateAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->TerminateAbility();
    });

    callFunctionMap_.emplace("AbilityGetLifecycle", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetLifecycle();
    });

    callFunctionMap_.emplace("AbilityGetResourceManager", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetResourceManager();
    });

    callFunctionMap_.emplace("AbilitySetUIContentCC", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetUIContent(OHOS::AppExecFwk::ComponentContainer());
    });

    callFunctionMap_.emplace("AbilitySetUIContentlayoutRes", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetUIContent(GetIntParam());
    });

    callFunctionMap_.emplace("AbilityStartAbilityForResultWi", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->StartAbilityForResult(OHOS::AAFwk::Want(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityStartAbilityForResultWiA", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->StartAbilityForResult(OHOS::AAFwk::Want(), GetIntParam(), OHOS::AppExecFwk::AbilityStartSetting());
    });

    callFunctionMap_.emplace("AbilityStartAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->StartAbility(OHOS::AAFwk::Want(), OHOS::AppExecFwk::AbilityStartSetting());
    });

    callFunctionMap_.emplace("AbilityInitAOAI", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();

        auto server = sptr<OHOS::AppExecFwk::AbilityThread>(new OHOS::AppExecFwk::AbilityThread());
        auto ah = std::make_shared<OHOS::AppExecFwk::AbilityHandler>(OHOS::AppExecFwk::EventRunner::Create(), server);
        temp->Init(std::make_shared<OHOS::AppExecFwk::AbilityInfo>(),
            std::make_shared<OHOS::AppExecFwk::OHOSApplication>(),
            ah,
            sptr<IRemoteObject>());
    });

    callFunctionMap_.emplace("AbilityOnStart", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnStart(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityOnStop", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnStop();
    });

    callFunctionMap_.emplace("AbilityOnActive", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnActive();
    });

    callFunctionMap_.emplace("AbilityOnInactive", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnInactive();
    });

    callFunctionMap_.emplace("AbilityOnForeground", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnForeground(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityOnBackground", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnBackground();
    });

    callFunctionMap_.emplace("AbilityOnKeyDown", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnKeyDown(1, KeyEvent());
    });

    callFunctionMap_.emplace("AbilityOnKeyUp", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnKeyUp(1, KeyEvent());
    });

    callFunctionMap_.emplace("AbilityOnTouchEvent", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnTouchEvent(TouchEvent());
    });

    callFunctionMap_.emplace("AbilityOnConnect", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnConnect(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityOnDisconnect", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnDisconnect(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilitySetUIContentCCi", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        std::shared_ptr<OHOS::AppExecFwk::Context> con = std::make_shared<OHOS::AppExecFwk::Ability>();
        temp->SetUIContent(OHOS::AppExecFwk::ComponentContainer(), con, GetIntParam());
    });

    callFunctionMap_.emplace("AbilitySetUIContentlCi", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        std::shared_ptr<OHOS::AppExecFwk::Context> con = std::make_shared<OHOS::AppExecFwk::Ability>();
        temp->SetUIContent(GetIntParam(), con, GetIntParam());
    });

    callFunctionMap_.emplace("AbilityHasWindowFocus", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->HasWindowFocus();
    });

    callFunctionMap_.emplace("AbilityOnKeyPressAndHold", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnKeyPressAndHold(1, std::shared_ptr<KeyEvent>());
    });

    callFunctionMap_.emplace("AbilityOnRequestPermissionsFromUserResult", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnRequestPermissionsFromUserResult(1, std::vector<std::string>(), std::vector<int>());
    });

    callFunctionMap_.emplace("AbilityOnLeaveForeground", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnLeaveForeground();
    });

    callFunctionMap_.emplace("AbilityGetType", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetType(GetParamUri());
    });

    callFunctionMap_.emplace("AbilityInsert", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Insert(GetParamUri(), NativeRdb::ValuesBucket());
    });

    callFunctionMap_.emplace("AbilityOnConfigurationUpdated", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnConfigurationUpdated(OHOS::AppExecFwk::Configuration());
    });

    callFunctionMap_.emplace("AbilityOnMemoryLevel", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnMemoryLevel(GetIntParam());
    });

    callFunctionMap_.emplace("AbilityOpenRawFile", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OpenRawFile(GetParamUri(), std::string());
    });

    callFunctionMap_.emplace("AbilityUpdate", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Update(GetParamUri(), NativeRdb::ValuesBucket(), GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("AbilityGetApplication", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetApplication();
    });

    callFunctionMap_.emplace("AbilityGetAbilityName", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetAbilityName();
    });

    callFunctionMap_.emplace("AbilityIsTerminating", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->IsTerminating();
    });

    callFunctionMap_.emplace("AbilityOnAbilityResult", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnAbilityResult(GetIntParam(), GetIntParam(), OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityOnBackPressed", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnBackPressed();
    });

    callFunctionMap_.emplace("AbilityOnNewWant", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnNewWant(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityOnRestoreAbilityState", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnRestoreAbilityState(GetParamPacMap());
    });

    callFunctionMap_.emplace("AbilityOnSaveAbilityState", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        auto pm = GetParamPacMap();
        temp->OnSaveAbilityState(pm);
    });

    callFunctionMap_.emplace("AbilityOnEventDispatch", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnEventDispatch();
    });

    callFunctionMap_.emplace("AbilityOnWindowFocusChanged", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnWindowFocusChanged(GetBoolParam());
    });

    callFunctionMap_.emplace("AbilitySetWant", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetWant(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityGetWant", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetWant();
    });

    callFunctionMap_.emplace("AbilitySetResult", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetResult(GetIntParam(), OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilitySetVolumeTypeAdjustedByKey", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetVolumeTypeAdjustedByKey(GetIntParam());
    });

    callFunctionMap_.emplace("AbilityOnCommand", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OnCommand(OHOS::AAFwk::Want(), GetBoolParam(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityDump", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Dump("info");
    });

    callFunctionMap_.emplace("AbilityKeepBackgroundRunning", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->KeepBackgroundRunning(GetIntParam(), OHOS::AppExecFwk::NotificationRequest());
    });

    callFunctionMap_.emplace("AbilityCancelBackgroundRunning", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->CancelBackgroundRunning();
    });

    callFunctionMap_.emplace("AbilityNormalizeUri", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->NormalizeUri(GetParamUri());
    });

    callFunctionMap_.emplace("AbilityDelete", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Delete(GetParamUri(), GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("AbilityGetFileTypes", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetFileTypes(GetParamUri(), "* /jpg");
    });

    callFunctionMap_.emplace("AbilityOpenFile", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->OpenFile(GetParamUri(), "r");
    });

    callFunctionMap_.emplace("AbilityQuery", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Query(GetParamUri(), std::vector<std::string>(), GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("AbilitySetMainRoute", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->SetMainRoute("");
    });

    callFunctionMap_.emplace("AbilityContinueAbilityReversiblyS", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->ContinueAbilityReversibly("id");
    });

    callFunctionMap_.emplace("AbilityGetOriginalDeviceId", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetOriginalDeviceId();
    });

    callFunctionMap_.emplace("AbilityGetContinuationState", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetContinuationState();
    });

    callFunctionMap_.emplace("AbilityGetAbilityPackage", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetAbilityPackage();
    });

    callFunctionMap_.emplace("AbilityDenormalizeUri", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->DenormalizeUri(GetParamUri());
    });

    callFunctionMap_.emplace("AbilityReload", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->Reload(GetParamUri(), GetParamPacMap());
    });

    callFunctionMap_.emplace("AbilityBatchInsert", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->BatchInsert(GetParamUri(), std::vector<NativeRdb::ValuesBucket>());
    });

    callFunctionMap_.emplace("AbilityGetVolumeTypeAdjustedByKey", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetVolumeTypeAdjustedByKey();
    });

    callFunctionMap_.emplace("AbilityGetState", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->GetState();
    });

    callFunctionMap_.emplace("AbilityStartAbilityW", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->StartAbility(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityConnectAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->ConnectAbility(OHOS::AAFwk::Want(), sptr<AAFwk::IAbilityConnection>());
    });

    callFunctionMap_.emplace("AbilityDisconnectAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->DisconnectAbility(sptr<AAFwk::IAbilityConnection>());
    });

    callFunctionMap_.emplace("AbilityStopAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->StopAbility(OHOS::AAFwk::Want());
    });

    callFunctionMap_.emplace("AbilityPostTask", []() {
        std::shared_ptr<OHOS::AppExecFwk::Ability> temp = GetParamAbility();
        temp->PostTask([]() {}, GetIntParam());
    });
}

void fuzzTestManager::RegisterDataAbilityHelper()
{
    callFunctionMap_.emplace("DataAbilityHelperRelease", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        temp->Release();
    });

    callFunctionMap_.emplace("DataAbilityHelperGetFileTypes", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->GetFileTypes(uri, "* /jpg");
    });

    callFunctionMap_.emplace("DataAbilityHelperOpenFile", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->OpenFile(uri, "r");
    });

    callFunctionMap_.emplace("DataAbilityHelperOpenRawFile", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->OpenRawFile(uri, "r");
    });

    callFunctionMap_.emplace("DataAbilityHelperInsert", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->Insert(uri, NativeRdb::ValuesBucket());
    });

    callFunctionMap_.emplace("DataAbilityHelperUpdate", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->Update(uri, NativeRdb::ValuesBucket(), GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("DataAbilityHelperDelete", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->Delete(uri, GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("DataAbilityHelperQuery", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        std::vector<std::string> columns{};
        temp->Query(uri, columns, GetParamDataAbilityPredicates());
    });

    callFunctionMap_.emplace("DataAbilityHelperGetType", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->GetType(uri);
    });

    callFunctionMap_.emplace("DataAbilityHelperReload", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        temp->Reload(uri, GetParamPacMap());
    });

    callFunctionMap_.emplace("DataAbilityHelperBatchInsert", []() {
        std::shared_ptr<OHOS::AppExecFwk::DataAbilityHelper> temp = GetParamDataAbilityHelper();
        auto uri = GetParamUri();
        std::vector<NativeRdb::ValuesBucket> values{};
        temp->BatchInsert(uri, values);
    });
}

void fuzzTestManager::RegisterDataUriUtils()
{
    callFunctionMap_.emplace(
        "DataUriUtilsAttachId", []() { OHOS::AppExecFwk::DataUriUtils::AttachId(GetParamUri(), GetIntParam()); });

    callFunctionMap_.emplace("DataUriUtilsGetId", []() { OHOS::AppExecFwk::DataUriUtils::GetId(GetParamUri()); });

    callFunctionMap_.emplace("DataUriUtilsDeleteId", []() { OHOS::AppExecFwk::DataUriUtils::DeleteId(GetParamUri()); });

    callFunctionMap_.emplace(
        "DataUriUtilsUpdateId", []() { OHOS::AppExecFwk::DataUriUtils::UpdateId(GetParamUri(), GetIntParam()); });

    callFunctionMap_.emplace(
        "DataUriUtilsIsAttachedId", []() { OHOS::AppExecFwk::DataUriUtils::IsAttachedId(GetParamUri()); });

    callFunctionMap_.emplace("DataUriUtilsIsNumber", []() { OHOS::AppExecFwk::DataUriUtils::IsNumber("a1"); });

    callFunctionMap_.emplace(
        "DataUriUtilsUriUpateLastPath", []() { OHOS::AppExecFwk::DataUriUtils::UriUpateLastPath(GetParamUri(), "1"); });
}

void fuzzTestManager::RegisterLifeCycle()
{
    callFunctionMap_.emplace("LifeCycleGetLifecycleState", []() {
        std::shared_ptr<OHOS::AppExecFwk::LifeCycle> temp = GetParamLifeCycle();
        temp->GetLifecycleState();
    });

    callFunctionMap_.emplace("LifeCycleAddObserver", []() {
        std::shared_ptr<OHOS::AppExecFwk::LifeCycle> temp = GetParamLifeCycle();
        temp->AddObserver(nullptr);
    });

    callFunctionMap_.emplace("LifeCycleDispatchLifecycleEW", []() {
        std::shared_ptr<OHOS::AppExecFwk::LifeCycle> temp = GetParamLifeCycle();
        temp->DispatchLifecycle(GetParamLifeCycleEvent(), OHOS::AppExecFwk::Want());
    });

    callFunctionMap_.emplace("LifeCycleDispatchLifecycleE", []() {
        std::shared_ptr<OHOS::AppExecFwk::LifeCycle> temp = GetParamLifeCycle();
        temp->DispatchLifecycle(GetParamLifeCycleEvent());
    });

    callFunctionMap_.emplace("LifeCycleRemoveObserver", []() {
        std::shared_ptr<OHOS::AppExecFwk::LifeCycle> temp = GetParamLifeCycle();
        temp->RemoveObserver(nullptr);
    });
}

void fuzzTestManager::RegisterAbilityContext()
{
    callFunctionMap_.emplace("AbilityContextGetExternalCacheDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetExternalCacheDir();
    });

    callFunctionMap_.emplace("AbilityContextGetExternalFilesDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        std::string t{};
        temp->GetExternalFilesDir(t);
    });

    callFunctionMap_.emplace("AbilityContextGetFilesDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetFilesDir();
    });

    callFunctionMap_.emplace("AbilityContextGetNoBackupFilesDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetNoBackupFilesDir();
    });

    callFunctionMap_.emplace("AbilityContextUnauthUriPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->UnauthUriPermission("", GetParamUri(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextGetDistributedDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetDistributedDir();
    });

    callFunctionMap_.emplace("AbilityContextSetPattern", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->SetPattern(GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextGetAbilityPackageContext", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetAbilityPackageContext();
    });

    callFunctionMap_.emplace("AbilityContextGetProcessName", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetProcessName();
    });

    callFunctionMap_.emplace("AbilityContextStartAbilityWi", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        OHOS::AAFwk::Want param;
        temp->StartAbility(param, GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextStartAbilityWiA", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        OHOS::AAFwk::Want param1;
        OHOS::AppExecFwk::AbilityStartSetting param2;
        temp->StartAbility(param1, GetIntParam(), param2);
    });

    callFunctionMap_.emplace("AbilityContextTerminateAbilityi", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->TerminateAbility(GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextTerminateAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->TerminateAbility();
    });

    callFunctionMap_.emplace("AbilityContextGetCallingBundle", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetCallingBundle();
    });

    callFunctionMap_.emplace("AbilityContextGetElementName", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetElementName();
    });

    callFunctionMap_.emplace("AbilityContextGetCallingAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetCallingAbility();
    });

    callFunctionMap_.emplace("AbilityContextConnectAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->ConnectAbility(OHOS::AppExecFwk::Want(), GetParamIAbilityConnection());
    });

    callFunctionMap_.emplace("AbilityContextDisconnectAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->DisconnectAbility(GetParamIAbilityConnection());
    });

    callFunctionMap_.emplace("AbilityContextStopAbility", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->StopAbility(OHOS::AppExecFwk::Want());
    });

    callFunctionMap_.emplace("AbilityContextGetApplicationInfo", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetApplicationInfo();
    });

    callFunctionMap_.emplace("AbilityContextGetCacheDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetCacheDir();
    });

    callFunctionMap_.emplace("AbilityContextGetCodeCacheDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetCodeCacheDir();
    });

    callFunctionMap_.emplace("AbilityContextGetDatabaseDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetDatabaseDir();
    });

    callFunctionMap_.emplace("AbilityContextGetDataDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetDataDir();
    });

    callFunctionMap_.emplace("AbilityContextGetDir", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetDir(GetStringParam(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextGetBundleManager", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetBundleManager();
    });

    callFunctionMap_.emplace("AbilityContextGetBundleCodePath", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetBundleCodePath();
    });

    callFunctionMap_.emplace("AbilityContextGetBundleName", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetBundleName();
    });

    callFunctionMap_.emplace("AbilityContextGetBundleResourcePath", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetBundleResourcePath();
    });

    callFunctionMap_.emplace("AbilityContextGetApplicationContext", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetApplicationContext();
    });

    callFunctionMap_.emplace("AbilityContextGetContext", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetContext();
    });

    callFunctionMap_.emplace("AbilityContextGetAbilityManager", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetAbilityManager();
    });

    callFunctionMap_.emplace("AbilityContextGetProcessInfo", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetProcessInfo();
    });

    callFunctionMap_.emplace("AbilityContextGetAppType", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetAppType();
    });

    callFunctionMap_.emplace("AbilityContextGetAbilityInfo", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetAbilityInfo();
    });

    callFunctionMap_.emplace("AbilityContextGetHapModuleInfo", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetHapModuleInfo();
    });

    callFunctionMap_.emplace("AbilityContextCreateBundleContext", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->CreateBundleContext(GetStringParam(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextGetResourceManager", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetResourceManager();
    });

    callFunctionMap_.emplace("AbilityContextVerifySelfPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->VerifySelfPermission(GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextVerifyCallingPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->VerifyCallingPermission(GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextCanRequestPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->CanRequestPermission(GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextVerifyCallingOrSelfPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->VerifyCallingOrSelfPermission(GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextVerifyPermission", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->VerifyPermission(GetStringParam(), GetIntParam(), GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextRequestPermissionsFromUser", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        std::vector<std::string> permissions{};
        temp->RequestPermissionsFromUser(permissions, GetIntParam());
    });

    callFunctionMap_.emplace("AbilityContextDeleteFile", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->DeleteFile(GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextSetCallingContext", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->SetCallingContext(GetStringParam(), GetStringParam(), GetStringParam());
    });

    callFunctionMap_.emplace("AbilityContextGetCaller", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetCaller();
    });

    callFunctionMap_.emplace("AbilityContextGetAbilityInfoType", []() {
        std::shared_ptr<OHOS::AppExecFwk::AbilityContext> temp = GetParamAbilityContext();
        temp->GetAbilityInfoType();
    });
}

void fuzzTestManager::RegisterProcessInfo()
{
    callFunctionMap_.emplace("ProcessInfoGetProcessName", []() {
        std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> temp = GetParamProcessInfo();
        temp->GetProcessName();
    });

    callFunctionMap_.emplace("ProcessInfoGetPid", []() {
        std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> temp = GetParamProcessInfo();
        temp->GetPid();
    });

    callFunctionMap_.emplace("ProcessInfoReadFromParcel", []() {
        std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> temp = GetParamProcessInfo();
        auto p = Parcel();
        temp->ReadFromParcel(p);
    });

    callFunctionMap_.emplace("ProcessInfoMarshalling", []() {
        std::shared_ptr<OHOS::AppExecFwk::ProcessInfo> temp = GetParamProcessInfo();
        auto p = Parcel();
        temp->Marshalling(p);
    });

    callFunctionMap_.emplace("ProcessInfoUnmarshalling", []() {
        auto p = Parcel();
        OHOS::AppExecFwk::ProcessInfo::Unmarshalling(p);
    });
}

fuzzTestManager::fuzzTestManager()
{
    RegisterAsyncCommonEventResult();
    RegisterCommonEventData();
    RegisterCommonEventManager();
    RegisterCommonEventPublishInfo();
    RegisterCommonEventSubscribeInfo();
    RegisterCommonEventSubscriber();
    RegisterCommonEventSupport();
    RegisterMatchingSkills();
    RegisterDumper();
    RegisterEventHandler();
    RegisterEventQueue();
    RegisterEventRunner();
    RegisterFileDescriptorListener();
    RegisterInnerEvent();
    RegisterEventRunnerNativeImplement();
    RegisterAbilityManager();
    RegisterWantParams();
    RegisterWant();
    RegisterElementName();
    RegisterBundleMgrProxy();

    RegisterOHOSApplication();
    RegisterAbility();
    RegisterDataAbilityHelper();
    RegisterDataUriUtils();
    RegisterLifeCycle();

    RegisterAbilityContext();
    RegisterProcessInfo();
}

void fuzzTestManager::SetJsonFunction(std::string functionName)
{
    remainderMap_.emplace(functionName, cycle_);
}

void fuzzTestManager::SetCycle(uint16_t cycle)
{
    cycle_ = cycle;
    for_each(remainderMap_.begin(), remainderMap_.end(), [cycle](std::unordered_map<std::string, int>::reference temp) {
        temp.second = cycle;
    });
}

int GetRandomInt(int minNum, int maxNum)
{
    return GetU16Param() % (maxNum - minNum + 1) + minNum;
}

void action(int a)
{
    std::cout << "Interrupt signal (" << a << ") received.\n";
}

void fuzzTestManager::StartFuzzTest()
{
    std::cout << __func__ << std::endl;
    OHOS::FuzzConfigParser jsonParser;
    OHOS::FuzzTestData tempData;

    std::cout << "parseFromFile start" << std::endl;
    jsonParser.ParseFromFile4FuzzTest(FUZZ_TEST_CONFIG_FILE_PATH, tempData);
    std::cout << "flag :" << tempData.mainLoopFlag << std::endl;
    for_each(tempData.methodVec.begin(), tempData.methodVec.end(), [this](std::vector<std::string>::reference s) {
        SetJsonFunction(s);
    });
    SetCycle(tempData.mainLoopFlag);

    std::vector<std::string> index;
    std::unordered_map<std::string, int>::iterator it = remainderMap_.begin();
    while (it != remainderMap_.end()) {
        if (it->second <= 0) {
            it = remainderMap_.erase(it);
        } else {
            index.push_back(it->first);
            it++;
        }
    }

    std::cout << remainderMap_.size() << "--------fuzz test start--------" << callFunctionMap_.size() << std::endl;
    for (; remainderMap_.size() > 0;) {
        std::string functionName;
        int offset = GetRandomInt(0, index.size() - 1);
        functionName = index[offset];
        if (callFunctionMap_.find(functionName) != callFunctionMap_.end()) {
            std::cout << "call function : " << functionName << std::endl;
            callFunctionMap_[functionName]();
            std::cout << "function end  :" << functionName << std::endl;
        } else {
            std::cout << "can't find function : " << functionName << std::endl;
        }
        remainderMap_[functionName]--;
        if (remainderMap_[functionName] <= 0) {
            remainderMap_.erase(functionName);
            index.erase(index.begin() + offset);
        };
    }
    std::cout << remainderMap_.size() << "--------fuzz test end--------" << std::endl;
}
}  // namespace EventFwk
}  // namespace OHOS
