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
#include "event_dispatcher.h"

#include <algorithm>

#include "logger.h"
namespace OHOS {
namespace HiviewDFX {
void EventDispatcher::AddInterestType(int32_t type)
{
    types_.insert(type);
}

void EventDispatcher::ClearInvalidListeners()
{
    std::lock_guard<std::mutex> lock(lock_);
    auto channelMapperIter = channelMapper_.begin();
    while (channelMapperIter != channelMapper_.end()) {
        auto listeners = channelMapperIter->second;
        auto listenerIter = listeners.begin();
        while (listenerIter != listeners.end()) {
            auto listener = listenerIter->lock();
            if (listener == nullptr) {
                listenerIter = listeners.erase(listenerIter);
                continue;
            }
            listenerIter++;
        }
        channelMapperIter++;
    }
}

void EventDispatcher::DispatchEvent(Event event)
{
    if (types_.find(event.messageType_) == types_.end()) {
        return;
    }

    auto listeners = channelMapper_[event.messageType_];
    for (auto listener : listeners) {
        std::shared_ptr<EventListener> sp = listener.lock();
        if (sp == nullptr) {
            continue;
        }

        std::set<EventListener::EventIdRange> listenerInfo;
        if (!sp->GetListenerInfo(event.messageType_, listenerInfo)) {
            continue;
        }

        if (std::any_of(listenerInfo.begin(), listenerInfo.end(),
            [&](const EventListener::EventIdRange &range) {
            return ((event.eventId_ >= range.begin) && (event.eventId_ <= range.end));
            })) {
            sp->OnUnorderedEvent(event);
        }
    }
}

void EventDispatcher::RegisterListener(std::weak_ptr<EventListener> listener)
{
    std::shared_ptr<EventListener> sp = listener.lock();
    if (sp == nullptr) {
        return;
    }

    for (auto type : types_) {
        std::set<EventListener::EventIdRange> listenerInfo;
        if (sp->GetListenerInfo(type, listenerInfo) && (!listenerInfo.empty())) {
            std::lock_guard<std::mutex> lock(lock_);
            channelMapper_[type].push_back(listener);
        }
    }
}
}  // namespace HiviewDFX
}  // namespace OHOS