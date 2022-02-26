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

#include "event_handler.h"
#include "event_handler_utils.h"
#include <iostream>

namespace OHOS {
namespace AppExecFwk {

std::shared_ptr<EventHandler> EventHandler::Current()
{
    const std::weak_ptr<EventHandler> &wp = std::make_shared<EventHandler>(EventRunner::Create(true));
    return wp.lock();
}

EventHandler::EventHandler(const std::shared_ptr<EventRunner> &runner) : eventRunner_(runner)
{}

EventHandler::~EventHandler()
{}

bool EventHandler::SendEvent(InnerEvent::Pointer &event, int64_t delayTime, Priority priority)
{
    std::function<void()> callback = event->GetTaskCallback();
    callback();
    return true;
}

bool EventHandler::SendTimingEvent(InnerEvent::Pointer &event, int64_t taskTime, Priority priority)
{
    return true;
}

bool EventHandler::SendSyncEvent(InnerEvent::Pointer &event, Priority priority)
{
    std::function<void()> callback = event->GetTaskCallback();
    callback();
    return true;
}

void EventHandler::RemoveAllEvents()
{}

void EventHandler::RemoveEvent(uint32_t innerEventId)
{}

void EventHandler::RemoveEvent(uint32_t innerEventId, int64_t param)
{}

void EventHandler::RemoveTask(const std::string &name)
{}

ErrCode EventHandler::AddFileDescriptorListener(
    int32_t fileDescriptor, uint32_t events, const std::shared_ptr<FileDescriptorListener> &listener)
{
    return EVENT_HANDLER_ERR_INVALID_PARAM;
}

void EventHandler::RemoveAllFileDescriptorListeners()
{}

void EventHandler::RemoveFileDescriptorListener(int32_t fileDescriptor)
{}

void EventHandler::SetEventRunner(const std::shared_ptr<EventRunner> &runner)
{}

void EventHandler::DistributeEvent(const InnerEvent::Pointer &event)
{}

void EventHandler::Dump(Dumper &dumper)
{}

bool EventHandler::HasInnerEvent(uint32_t innerEventId)
{
    return true;
}

bool EventHandler::HasInnerEvent(int64_t param)
{
    return true;
}

std::string EventHandler::GetEventName(const InnerEvent::Pointer &event)
{
    return "132123";
}

bool EventHandler::IsIdle()
{
    return true;
}

void EventHandler::ProcessEvent(const InnerEvent::Pointer &)
{}
}  // namespace AppExecFwk
}  // namespace OHOS