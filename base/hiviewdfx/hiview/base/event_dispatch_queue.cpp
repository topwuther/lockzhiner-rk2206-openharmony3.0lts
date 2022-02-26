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
#include "event_dispatch_queue.h"

#include <algorithm>
#include <memory>

#include "file_util.h"
#include "thread_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventDispatchQueue");
EventDispatchQueue::EventDispatchQueue(const std::string& name, Event::ManageType type)
    : stop_(false), isRunning_(false), threadName_(name), type_(type)
{}

EventDispatchQueue::~EventDispatchQueue()
{
    Stop();
}

void EventDispatchQueue::Run()
{
    const int threadNameLen = 15;
    Thread::SetThreadDescription(threadName_.substr(0, threadNameLen));
    isRunning_ = true;
    while (true) {
        std::shared_ptr<Event> event = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutexLock_);
            while (pendingEvents_.empty()) {
                condition_.wait(lock);
                if (stop_) {
                    return;
                }
            }
            event = pendingEvents_.front();
            pendingEvents_.pop_front();
        }

        if (event == nullptr) {
            continue;
        }

        if (type_ == Event::ManageType::ORDERED) {
            ProcessOrderedEvent(*(event.get()));
        } else {
            ProcessUnorderedEvent(*(event.get()));
        }

        if (stop_) {
            break;
        }
    }
}

void EventDispatchQueue::RegisterListener(std::weak_ptr<EventListener> listener)
{
    HIVIEW_LOGI("EventDispatchQueue RegisterListener");
    listeners_.push_back(std::move(listener));
}

void EventDispatchQueue::ProcessOrderedEvent(Event& event)
{
    bool skip = true;
    bool stop = false;
    for (const auto& listener : listeners_) {
        if (std::shared_ptr<EventListener> sp = listener.lock()) {
            // dispatch anonymous event from head of the listeners
            if (event.sender_.empty()) {
                skip = false;
            }

            if (skip && (sp->GetListenerName() == event.sender_)) {
                skip = false;
                continue;
            }

            if (skip) {
                continue;
            }

            if (IsEventMatchCurrentListener(event, sp)) {
                stop = sp->OnOrderedEvent(event);
            }

            if (stop) {
                HIVIEW_LOGI("Event %d consumed by %{public}s.", event.eventId_, sp->GetListenerName().c_str());
                break;
            }
        }
    }
}

void EventDispatchQueue::ProcessUnorderedEvent(const Event& event)
{
    for (const auto& listener : listeners_) {
        if (std::shared_ptr<EventListener> sp = listener.lock()) {
            if (IsEventMatchCurrentListener(event, sp)) {
                sp->OnUnorderedEvent(event);
            }
        }
    }
}

bool EventDispatchQueue::IsEventMatchCurrentListener(const Event& event, std::shared_ptr<EventListener> listener)
{
    if (!event.eventName_.empty() && !event.domain_.empty()) {
        auto eventName = event.domain_ + "_" + event.eventName_;
        std::set<std::string> strListenerInfo;
        if (listener->GetListenerInfo(event.messageType_, strListenerInfo)) {
            return std::any_of(strListenerInfo.begin(), strListenerInfo.end(), [&](const std::string& name) {
                return (eventName.find(name) != std::string::npos);
            });
        }
        return false;
    }

    std::set<EventListener::EventIdRange> listenerInfo;
    if (listener->GetListenerInfo(event.messageType_, listenerInfo)) {
        return std::any_of(listenerInfo.begin(), listenerInfo.end(), [&](const EventListener::EventIdRange& range) {
            return ((event.eventId_ >= range.begin) && (event.eventId_ <= range.end));
        });
    }
    return false;
}

void EventDispatchQueue::Stop()
{
    stop_ = true;
    condition_.notify_all();
    if (thread_ != nullptr && thread_->joinable()) {
        thread_->join();
    }
    isRunning_ = false;
}

void EventDispatchQueue::Start()
{
    std::unique_lock<std::mutex> lock(mutexLock_);
    if (!IsRunning()) {
        thread_ = std::make_unique<std::thread>(&EventDispatchQueue::Run, this);
    }
}

void EventDispatchQueue::Enqueue(std::shared_ptr<Event> event)
{
    HIVIEW_LOGD("EventDispatchQueue Enqueue");
    std::unique_lock<std::mutex> lock(mutexLock_);
    pendingEvents_.push_back(std::move(event));
    condition_.notify_one();
}

int EventDispatchQueue::GetWaitQueueSize() const
{
    return pendingEvents_.size();
}
} // namespace HiviewDFX
} // namespace OHOS