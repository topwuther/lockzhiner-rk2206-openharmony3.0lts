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

#include "event_loop_impl.h"

#include <ctime>

#include "db_errno.h"
#include "log_print.h"
#include "event_impl.h"

namespace DistributedDB {
class EventRequest {
public:
    enum {
        ADD_EVENT = 1,
        REMOVE_EVENT,
        SET_TIMEOUT,
        MOD_EVENTS_ADD,
        MOD_EVENTS_REMOVE,
    };

    EventRequest(int type, EventImpl *event, EventsMask events)
        : type_(type),
          event_(event),
          events_(events),
          timeout_(0)
    {
        if (event != nullptr) {
            event->IncObjRef(event);
        }
    }

    EventRequest(int type, EventImpl *event, EventTime timeout)
        : type_(type),
          event_(event),
          events_(0),
          timeout_(timeout)
    {
        if (event != nullptr) {
            event->IncObjRef(event);
        }
    }

    ~EventRequest()
    {
        if (event_ != nullptr) {
            event_->DecObjRef(event_);
            event_ = nullptr;
        }
    }

    static bool IsValidType(int type)
    {
        if (type < ADD_EVENT || type > MOD_EVENTS_REMOVE) {
            return false;
        }
        return true;
    }

    int GetType() const
    {
        return type_;
    }

    void GetEvent(EventImpl *&event) const
    {
        event = event_;
    }

    EventsMask GetEvents() const
    {
        return events_;
    }

    EventTime GetTimeout() const
    {
        return timeout_;
    }

private:
    int type_;
    EventImpl *event_;
    EventsMask events_;
    EventTime timeout_;
};

EventLoopImpl::EventLoopImpl()
    : pollingSetChanged_(false)
{
    OnKill([this](){ OnKillLoop(); });
}

EventLoopImpl::~EventLoopImpl()
{}

int EventLoopImpl::Add(IEvent *event)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    auto eventImpl = static_cast<EventImpl *>(event);
    if (!eventImpl->SetLoop(this)) {
        LOGE("Add ev to loop failed, already attached.");
        return -E_INVALID_ARGS;
    }

    EventTime timeout = 0;
    int errCode = QueueRequest(EventRequest::ADD_EVENT, eventImpl, timeout);
    if (errCode != E_OK) {
        eventImpl->SetLoop(nullptr);
        LOGE("Add ev to loop failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::Remove(IEvent *event)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    auto eventImpl = static_cast<EventImpl *>(event);
    bool isLoopConfused = false;
    if (!eventImpl->Attached(this, isLoopConfused)) {
        if (isLoopConfused) {
            LOGE("Remove ev' from loop failed, loop confused.");
            return -E_UNEXPECTED_DATA;
        }
        return E_OK;
    }

    EventTime timeout = 0;
    int errCode = QueueRequest(EventRequest::REMOVE_EVENT, eventImpl, timeout);
    if (errCode != E_OK) {
        LOGE("Remove ev from loop failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::Run()
{
    {
        RefObject::AutoLock lockGuard(this);
        if (IsKilled()) {
            LOGE("Try to run a killed loop.");
            return -E_OBJ_IS_KILLED;
        }
        if (loopThread_ != std::thread::id()) {
            LOGE("Try to run a threaded loop.");
            return -E_BUSY;
        }
        loopThread_ = std::this_thread::get_id();
    }

    int errCode;
    IncObjRef(this);

    while (true) {
        errCode = ProcessRequest();
        if (errCode != E_OK) {
            break;
        }

        errCode = Prepare(polling_);
        if (errCode != E_OK) {
            break;
        }

        EventTime sleepTime = CalSleepTime();
        errCode = Poll(sleepTime);
        if (errCode != E_OK) {
            break;
        }

        errCode = ProcessRequest();
        if (errCode != E_OK) {
            break;
        }

        errCode = DispatchAll();
        if (errCode != E_OK) {
            break;
        }
    }

    CleanLoop();
    DecObjRef(this);
    if (errCode == -E_OBJ_IS_KILLED) {
        LOGD("Loop exited.");
    } else {
        LOGE("Loop exited, err:'%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::Modify(EventImpl *event, bool isAdd, EventsMask events)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    int type = isAdd ? EventRequest::MOD_EVENTS_ADD :
        EventRequest::MOD_EVENTS_REMOVE;
    int errCode = QueueRequest(type, event, events);
    if (errCode != E_OK) {
        LOGE("Modify loop ev events failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::Modify(EventImpl *event, EventTime time)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    int errCode = QueueRequest(EventRequest::SET_TIMEOUT, event, time);
    if (errCode != E_OK) {
        LOGE("Mod loop ev time failed. err: '%d'.", errCode);
    }
    return errCode;
}

EventTime EventLoopImpl::GetTime() const
{
    uint64_t microsecond = 0;
    OS::GetMonotonicRelativeTimeInMicrosecond(microsecond); // It is not very possible to fail, if so use 0 as default
    return static_cast<EventTime>(microsecond / 1000); // 1000 is the multiple between microsecond and millisecond
}

int EventLoopImpl::SendRequestToLoop(EventRequest *eventRequest)
{
    if (eventRequest == nullptr) {
        return -E_INVALID_ARGS;
    }

    RefObject::AutoLock lockGuard(this);
    if (IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }
    requests_.push_back(eventRequest);
    WakeUp();
    return E_OK;
}

template<typename T>
int EventLoopImpl::QueueRequest(int type, EventImpl *event, T argument)
{
    if (!EventRequest::IsValidType(type)) {
        return -E_INVALID_ARGS;
    }
    if (event == nullptr ||
        !event->IsValidArg(argument)) {
        return -E_INVALID_ARGS;
    }

    if (IsKilled()) { // pre-check
        return -E_OBJ_IS_KILLED;
    }

    int errCode;
    if (event != nullptr) {
        errCode = event->CheckStatus();
        if (errCode != E_OK) {
            if (errCode != -E_OBJ_IS_KILLED ||
                type != EventRequest::REMOVE_EVENT) {
                return errCode;
            }
        }
    }

    auto eventRequest = new (std::nothrow) EventRequest(type, event, argument);
    if (eventRequest == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    errCode = SendRequestToLoop(eventRequest);
    if (errCode != E_OK) {
        delete eventRequest;
        eventRequest = nullptr;
    }
    return errCode;
}

bool EventLoopImpl::IsInLoopThread(bool &started) const
{
    if (loopThread_ == std::thread::id()) {
        started = false;
    } else {
        started = true;
    }
    return std::this_thread::get_id() == loopThread_;
}

bool EventLoopImpl::EventObjectExists(EventImpl *event) const
{
    auto it = polling_.find(event);
    if (it != polling_.end()) {
        return true;
    }
    return false;
}

bool EventLoopImpl::EventFdExists(const EventImpl *event) const
{
    if (!event->IsValidFd()) {
        return false;
    }
    for (auto ev : polling_) {
        if (ev->GetEventFd() == event->GetEventFd()) {
            return true;
        }
    }
    return false;
}

int EventLoopImpl::AddEventObject(EventImpl *event, EventTime now)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (EventObjectExists(event)) {
        LOGE("Add event object failed. ev already exists.");
        return -EEXIST;
    }
    if (EventFdExists(event)) {
        LOGE("Add event object failed. ev fd already exists.");
        return -EEXIST;
    }

    int errCode = E_OK;
    if (!event->IsTimer()) {
        errCode = AddEvent(event);
    }

    if (errCode == E_OK) {
        polling_.insert(event);
        event->SetStartTime(now);
        event->SetRevents(0);
        event->IncObjRef(event);
        pollingSetChanged_ = true;
    } else {
        LOGE("Add event failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::RemoveEventObject(EventImpl *event)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (!EventObjectExists(event)) {
        return -E_NO_SUCH_ENTRY;
    }

    int errCode = E_OK;
    if (!event->IsTimer()) {
        errCode = RemoveEvent(event);
    }

    if (errCode == E_OK) {
        polling_.erase(event);
        event->SetLoop(nullptr);
        event->DecObjRef(event);
        pollingSetChanged_ = true;
    } else {
        LOGE("Remove event failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::ModifyEventObject(EventImpl *event, bool isAdd, EventsMask events)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (!EventObjectExists(event)) {
        return -EEXIST;
    }

    int errCode = E_OK;
    if (!event->IsTimer()) {
        EventsMask genericEvents = events & (~IEvent::ET_TIMEOUT);
        if (genericEvents) {
            errCode = ModifyEvent(event, isAdd, genericEvents);
        }
    }

    if (errCode == E_OK) {
        event->SetEvents(isAdd, events);
    } else {
        LOGE("Modify event' failed. err: '%d'.", errCode);
    }
    return errCode;
}

int EventLoopImpl::ModifyEventObject(EventImpl *event, EventTime timeout)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (!EventObjectExists(event)) {
        return -E_NO_SUCH_ENTRY;
    }
    event->SetTimeoutPeriod(timeout);
    return E_OK;
}

void EventLoopImpl::ProcessRequest(std::list<EventRequest *> &requests)
{
    EventTime now = GetTime();
    while (true) {
        if (requests.empty()) {
            break;
        }

        EventRequest *request = requests.front();
        requests.pop_front();
        if (request == nullptr) {
            continue;
        }

        if (!IsKilled()) {
            EventImpl *event = nullptr;
            request->GetEvent(event);
            EventsMask events = request->GetEvents();
            EventTime timeout = request->GetTimeout();

            switch (request->GetType()) {
                case EventRequest::ADD_EVENT:
                    (void)(AddEventObject(event, now));
                    break;

                case EventRequest::REMOVE_EVENT:
                    (void)(RemoveEventObject(event));
                    break;

                case EventRequest::MOD_EVENTS_ADD:
                    (void)(ModifyEventObject(event, true, events));
                    break;

                case EventRequest::MOD_EVENTS_REMOVE:
                    (void)(ModifyEventObject(event, false, events));
                    break;

                case EventRequest::SET_TIMEOUT:
                    (void)(ModifyEventObject(event, timeout));
                    break;

                default:
                    break;
            }
        }

        delete request;
        request = nullptr;
    }
}

int EventLoopImpl::ProcessRequest()
{
    int errCode = E_OK;
    std::list<EventRequest *> requests;
    {
        RefObject::AutoLock lockGuard(this);
        if (IsKilled()) {
            errCode = -E_OBJ_IS_KILLED;
        }
        if (requests_.empty()) {
            return errCode;
        }
        std::swap(requests, requests_);
    }

    ProcessRequest(requests);
    return errCode;
}

EventTime EventLoopImpl::CalSleepTime() const
{
    EventTime now = GetTime();
    EventTime minInterval = EventImpl::MAX_TIME_VALUE;

    for (auto event : polling_) {
        if (event == nullptr) {
            continue;
        }

        EventTime t;
        bool valid = event->GetTimeoutPoint(t);
        if (!valid) {
            continue;
        }

        if (t <= now) {
            return 0;
        }

        EventTime interval = t - now;
        if (interval < minInterval) {
            minInterval = interval;
        }
    }

    return minInterval;
}

int EventLoopImpl::DispatchAll()
{
    do {
        EventTime now = GetTime();
        pollingSetChanged_ = false;

        for (auto event : polling_) {
            if (IsKilled()) {
                return -E_OBJ_IS_KILLED;
            }
            if (event == nullptr) {
                continue;
            }

            event->IncObjRef(event);
            event->UpdateElapsedTime(now);
            int errCode = event->Dispatch();
            if (errCode != E_OK) {
                RemoveEventObject(event);
            } else {
                event->SetRevents(0);
            }
            event->DecObjRef(event);

            if (pollingSetChanged_) {
                break;
            }
        }
    } while (pollingSetChanged_);
    return E_OK;
}

void EventLoopImpl::CleanLoop()
{
    if (!IsKilled()) {
        return;
    }

    ProcessRequest();
    std::set<EventImpl *> polling = std::move(polling_);
    int errCode = Exit(polling);
    if (errCode != E_OK) {
        LOGE("Exit loop failed when cleanup, err:'%d'.", errCode);
    }

    for (auto event : polling) {
        if (event != nullptr) {
            event->KillAndDecObjRef(event);
        }
    }
}

void EventLoopImpl::OnKillLoop()
{
    bool started = true;
    if (IsInLoopThread(started)) {
        // Loop object is set to state: killed,
        // everything will be done in loop.Run()
        return;
    }

    if (started) {
        // Ditto
        WakeUp();
    } else {
        // Drop the lock.
        UnlockObj();
        CleanLoop();
        LockObj();
    }
}
}
