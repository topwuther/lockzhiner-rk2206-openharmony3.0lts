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

#include <regex>

#include "pipeline.h"
#include "logger.h"
#include "plugin_factory.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(HiCollieCollector);
DEFINE_LOG_TAG("HiCollieCollector");

std::string HiCollieCollector::GetListenerName()
{
    return name_;
}

bool HiCollieCollector::ReadyToLoad()
{
    return true;
}

bool HiCollieCollector::CanProcessEvent(std::shared_ptr<Event> event)
{   
    return false;
}

void HiCollieCollector::OnLoad()
{
    SetName("HiCollieCollector");
    SetVersion("HiCollieCollector 1.0");
    HIVIEW_LOGI("HiCollieCollector OnLoad.");
    AddListenerInfo(Event::MessageType::SYS_EVENT, STRINGID_WATCHDOG);
    GetHiviewContext()->RegisterUnorderedEventListener(
        std::static_pointer_cast<HiCollieCollector>(shared_from_this()));
}

void HiCollieCollector::OnUnload()
{
    HIVIEW_LOGI("HiCollieCollector OnUnload.");
}

bool HiCollieCollector::OnEvent(std::shared_ptr<Event> &event)
{
    return true;
}

bool HiCollieCollector::OnOrderedEvent(Event &event)
{
    return false;
}

void HiCollieCollector::OnUnorderedEvent(const Event &event)
{
    HIVIEW_LOGI("received event domain=%{public}s, stringid=%{public}s.\n",
        event.domain_.c_str(), event.eventName_.c_str());
    if (GetHiviewContext() == nullptr || event.eventName_ != STRINGID_WATCHDOG) {
        return;
    }

    Event& eventRef = const_cast<Event&>(event);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);
    ProcessHiCollieEvent(sysEvent);
}

void HiCollieCollector::ProcessHiCollieEvent(SysEvent &sysEvent)
{
    std::string path = "";
    std::string info = sysEvent.GetEventValue(EventStore::EventCol::INFO);
    std::regex reg("logPath:([^,]+)");
    std::smatch result;
    if (std::regex_search(info, result, reg)) {
        path = result[1].str();
    }

    auto event = std::make_shared<PipelineEvent>("HiCollie", nullptr);
    event->happenTime_ = sysEvent.happenTime_;
    PublishPipelineEvent(std::dynamic_pointer_cast<PipelineEvent>(event));
    HIVIEW_LOGI("hicollie event msg:%{public}s path:%{public}s",
        sysEvent.GetEventValue(EVENT_MSG).c_str(), path.c_str());
}
} // namespace HiviewDFX
} // namespace OHOS
