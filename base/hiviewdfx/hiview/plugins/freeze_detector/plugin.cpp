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

#include "plugin.h"

#include "freeze_detector_utils.h"
#include "plugin_factory.h"
#include "resolver.h"
#include "string_util.h"
#include "sys_event_dao.h"
#include "vendor.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(FreezeDetectorPlugin);
DEFINE_RELIABILITY_LOG_TAG("FreezeDetector");

std::string FreezeDetectorPlugin::GetListenerName()
{
    return FREEZE_DETECTOR_PLUGIN_NAME;
}

bool FreezeDetectorPlugin::ReadyToLoad()
{
    return FreezeResolver::GetInstance().Init();
}

void FreezeDetectorPlugin::OnLoad()
{
    HIVIEW_LOGI("OnLoad.");
    SetName(FREEZE_DETECTOR_PLUGIN_NAME);
    SetVersion(FREEZE_DETECTOR_PLUGIN_VERSION);

    flag = false;
    threadLoop_ = GetWorkLoop();
    if (threadLoop_ == nullptr) {
        HIVIEW_LOGW("thread loop is null.");
        return;
    }
    threadLoop_->StartLoop();

    AddFreezeListener();
    AddMaintenanceListener();
}

void FreezeDetectorPlugin::OnUnload()
{
    HIVIEW_LOGD("OnUnload.");
}

void FreezeDetectorPlugin::AddFreezeListener()
{
    std::set<std::string> set = Vendor::GetInstance().GetFreezeStringIds();
    AddListenerInfo(Event::MessageType::SYS_EVENT, set);

    auto context = GetHiviewContext();
    if (context != nullptr) {
        context->RegisterUnorderedEventListener(std::static_pointer_cast<FreezeDetectorPlugin>(shared_from_this()));
    }
}

void FreezeDetectorPlugin::AddMaintenanceListener()
{
    AddListenerInfo(Event::MessageType::PLUGIN_MAINTENANCE, EventListener::EventIdRange(Event::EventId::PLUGIN_LOADED));

    auto context = GetHiviewContext();
    if (context != nullptr) {
        context->RegisterUnorderedEventListener(std::static_pointer_cast<FreezeDetectorPlugin>(shared_from_this()));
    }
}

bool FreezeDetectorPlugin::OnEvent(std::shared_ptr<Event> &event)
{
    return false;
}

bool FreezeDetectorPlugin::OnOrderedEvent(Event& msg)
{
    return false;
}

bool FreezeDetectorPlugin::CanProcessEvent(std::shared_ptr<Event> event)
{
    return false;
}

bool FreezeDetectorPlugin::CanProcessRebootEvent(const Event &event)
{
    return (event.messageType_ == Event::MessageType::PLUGIN_MAINTENANCE) &&
        (event.eventId_ == Event::EventId::PLUGIN_LOADED);
}

std::string FreezeDetectorPlugin::RemoveRedundantNewline(const std::string& content)
{
    std::vector<std::string> lines;
    StringUtil::SplitStr(content, "\\n", lines, false, false);

    std::string outContent;
    for (const auto& line : lines) {
        outContent.append(line).append("\n");
    }
    return outContent;
}

WatchPoint FreezeDetectorPlugin::MakeWatchPoint(const Event& event)
{
    Event& eventRef = const_cast<Event&>(event);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);

    long seq = sysEvent.GetSeq();
    long pid = sysEvent.GetPid();
    long tid = sysEvent.GetTid();
    long uid = sysEvent.GetUid();
    std::string packageName = sysEvent.GetEventValue(EVENT_PACKAGE_NAME);
    std::string processName = sysEvent.GetEventValue(EVENT_PROCESS_NAME);
    std::string msg = RemoveRedundantNewline(sysEvent.GetEventValue(EVENT_MSG));
    WatchPoint watchPoint = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitSeq(seq)
        .InitDomain(event.domain_)
        .InitStringId(event.eventName_)
        .InitTimestamp(event.happenTime_)
        .InitPid(pid)
        .InitTid(tid)
        .InitUid(uid)
        .InitPackageName(packageName)
        .InitProcessName(processName)
        .InitMsg(msg)
        .Build();
    HIVIEW_LOGI("watchpoint domain=%{public}s, stringid=%{public}s, pid=%{public}ld, uid=%{public}ld, "
        "seq=%{public}ld, packageName=%{public}s, processName=%{public}s, msg=%{public}s.",
        event.domain_.c_str(), event.eventName_.c_str(), pid, uid,
        seq, packageName.c_str(), processName.c_str(), msg.c_str());

    return watchPoint;
}

void FreezeDetectorPlugin::OnUnorderedEvent(const Event& event)
{
    HIVIEW_LOGD("received event id=%{public}u, domain=%{public}s, stringid=%{public}s, extraInfo=%{public}s.",
        event.eventId_, event.domain_.c_str(), event.eventName_.c_str(), event.jsonExtraInfo_.c_str());

    if (CanProcessRebootEvent(event)) {
        // dispatcher context, send task to our thread
        auto task = std::bind(&FreezeDetectorPlugin::ProcessRebootEvent, this);
        threadLoop_->AddEvent(nullptr, nullptr, task);
        return;
    }

    if (Vendor::GetInstance().IsFreezeEvent(event.domain_, event.eventName_) == false) {
        HIVIEW_LOGE("not freeze event.");
        return;
    }

    // dispatcher context, send task to our thread
    WatchPoint watchPoint = MakeWatchPoint(event);
    auto task = std::bind(&FreezeDetectorPlugin::ProcessEvent, this, watchPoint);
    threadLoop_->AddEvent(nullptr, nullptr, task);
}

void FreezeDetectorPlugin::ProcessRebootEvent()
{
    flag = true;
    if (Vendor::GetInstance().GetInitFlag() == false) {
        HIVIEW_LOGE("vendor is not inited. skip reboot event");
        return;
    }

    auto event = FreezeResolver::GetInstance().ProcessHardwareEvent();
    if (event != nullptr) {
        PipelineEvent::FillPipelineInfo(shared_from_this(), SYSEVENT_PIPELINE, event, false); //sysevent pipeline
        event->OnContinue();
    }

    event = FreezeResolver::GetInstance().ProcessLongPressEvent();
    if (event != nullptr) {
        PipelineEvent::FillPipelineInfo(shared_from_this(), SYSEVENT_PIPELINE, event, false); //sysevent pipeline
        event->OnContinue();
        //GetHiviewContext()->PostAsyncEventToTarget(shared_from_this(), "EventLogger", event);
    }

    if (FreezeResolver::GetInstance().ResolveLongPressEvent() == true ||
        Vendor::GetInstance().IsBetaVersion() == false) {
        return;
    }

    event = FreezeResolver::GetInstance().ProcessSystemEvent();
    if (event != nullptr) {
        PublishPipelineEvent(std::dynamic_pointer_cast<PipelineEvent>(event));
    }
}

void FreezeDetectorPlugin::ProcessEvent(WatchPoint watchPoint)
{
    if (flag == false || Vendor::GetInstance().GetInitFlag() == false) {
        HIVIEW_LOGE("vendor is not inited. skip event");
        return;
    }

    auto event = FreezeResolver::GetInstance().ProcessEvent(watchPoint);
    if (event != nullptr) {
        PublishPipelineEvent(std::dynamic_pointer_cast<PipelineEvent>(event));
    }
}
} // namespace HiviewDFX
} // namespace OHOS
