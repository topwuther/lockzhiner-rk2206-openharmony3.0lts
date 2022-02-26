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

#ifndef EVENT_IMPL_H
#define EVENT_IMPL_H

#include <climits>
#include <condition_variable>
#include "../include/ievent.h"

namespace DistributedDB {
class EventLoopImpl;

class EventImpl : public IEvent {
public:
    explicit EventImpl(EventTime timeout);
    EventImpl(EventFd fd, EventsMask events, EventTime timeout);
    ~EventImpl() override;

    int SetAction(const EventAction &action, const EventFinalizer &finalizer) override;
    int AddEvents(EventsMask events) override;
    int RemoveEvents(EventsMask events) override;
    int SetTimeout(EventTime timeout) override;
    int Detach(bool wait) override;
    void IgnoreFinalizer() override;

    int CheckStatus() const;
    bool IsTimer() const;
    bool IsValidFd() const;
    EventFd GetEventFd() const;
    EventsMask GetEvents() const;
    bool SetLoop(EventLoopImpl *loop);
    void Wait();
    bool Attached(const EventLoopImpl *loop, bool &isLoopConfused) const;
    void SetEvents(bool isAdd, EventsMask events);
    void SetRevents(EventsMask events);
    void SetTimeoutPeriod(EventTime timeout);
    void SetStartTime(EventTime startTime);
    bool GetTimeoutPoint(EventTime &timePoint) const;
    void UpdateElapsedTime(EventTime now);
    int Dispatch();
    bool IsValidArg(EventsMask events) const;
    bool IsValidArg(EventTime timeout) const;
    DISABLE_COPY_ASSIGN_MOVE(EventImpl);

    static constexpr int MAX_TIME_VALUE = INT_MAX / 2; // half of the max

private:
    DECLARE_OBJECT_TAG(EventImpl);

    EventFd fd_;
    EventsMask events_;
    EventsMask revents_;
    EventTime timeout_; // should not < 0
    EventTime start_;
    EventLoopImpl *loop_;
    EventAction action_;
    EventFinalizer finalizer_;
    bool ignoreFinalizer_;
    std::condition_variable detached_;
};
}

#endif // EVENT_IMPL_H
