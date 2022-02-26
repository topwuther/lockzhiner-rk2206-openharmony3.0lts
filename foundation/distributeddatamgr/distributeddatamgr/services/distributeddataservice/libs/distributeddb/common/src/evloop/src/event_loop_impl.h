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

#ifndef EVENT_LOOP_IMPL_H
#define EVENT_LOOP_IMPL_H

#include <list>
#include <set>
#include <thread>
#include "platform_specific.h"
#include "../include/ievent_loop.h"
#include "../include/ievent.h"

#if defined EVLOOP_TIMER_ONLY
#define EVENT_LOOP_USE_SELECT
#else
#define EVENT_LOOP_USE_EPOLL
#endif

namespace DistributedDB {
class EventImpl;
class EventRequest;

class EventLoopImpl : public IEventLoop {
public:
    EventLoopImpl();
    ~EventLoopImpl() override;
    DISABLE_COPY_ASSIGN_MOVE(EventLoopImpl);

    int Add(IEvent *event) override;
    int Remove(IEvent *event) override;
    int Run() override;
    int Modify(EventImpl *event, bool isAdd, EventsMask events);
    int Modify(EventImpl *event, EventTime time);

    // Initialize the loop, code removed from the constructor.
    virtual int Initialize() = 0;
    bool IsInLoopThread(bool &started) const;

private:
    virtual int Prepare(const std::set<EventImpl *> &polling) = 0;
    virtual int Poll(EventTime sleepTime) = 0;
    virtual int WakeUp() = 0;
    virtual int Exit(const std::set<EventImpl *> &polling) = 0;
    virtual int AddEvent(EventImpl *event) = 0;
    virtual int RemoveEvent(EventImpl *event) = 0;
    virtual int ModifyEvent(EventImpl *event, bool isAdd, EventsMask events) = 0;
    virtual EventTime GetTime() const;

    template<typename T>
    int QueueRequest(int type, EventImpl *event, T argument);
    int SendRequestToLoop(EventRequest *eventRequest);
    bool EventObjectExists(EventImpl *event) const;
    bool EventFdExists(const EventImpl *event) const;
    int AddEventObject(EventImpl *event, EventTime now);
    int RemoveEventObject(EventImpl *event);
    int ModifyEventObject(EventImpl *event, bool isAdd, EventsMask events);
    int ModifyEventObject(EventImpl *event, EventTime timeout);
    void ProcessRequest(std::list<EventRequest *> &requests);
    int ProcessRequest();
    EventTime CalSleepTime() const;
    int DispatchAll();
    void CleanLoop();
    void OnKillLoop();

    std::list<EventRequest *> requests_;
    std::set<EventImpl *> polling_;
    bool pollingSetChanged_;
    std::thread::id loopThread_;
};
}

#endif // EVENT_LOOP_IMPL_H
