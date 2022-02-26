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

#include "event_impl.h"
#include "db_errno.h"
#include "log_print.h"
#include "event_loop_impl.h"

namespace DistributedDB {
EventImpl::EventImpl(EventTime timeout)
    : events_(ET_TIMEOUT),
      revents_(0),
      timeout_(timeout),
      start_(0),
      loop_(nullptr),
      ignoreFinalizer_(false)
{
    if (timeout_ < 0) {
        timeout_ = MAX_TIME_VALUE;
    }

    OnKill([this]() {
        UnlockObj();
        (void)Detach(false);
        LockObj();
    });

    OnLastRef([this]() {
        if (finalizer_ && !ignoreFinalizer_) {
            finalizer_();
        }
    });
}

EventImpl::EventImpl(EventFd fd, EventsMask events, EventTime timeout)
    : fd_(fd),
      events_(events),
      revents_(0),
      timeout_(timeout),
      start_(0),
      loop_(nullptr),
      ignoreFinalizer_(false)
{
    if (!(events & ET_TIMEOUT) || (timeout_ < 0)) {
        timeout_ = MAX_TIME_VALUE;
    }
    if (!fd_.IsValid()) {
        events_ &= ~(ET_READ | ET_WRITE | ET_ERROR);
    }

    OnKill([this]() {
        UnlockObj();
        (void)Detach(false);
        LockObj();
    });

    OnLastRef([this]() {
        if (finalizer_ && !ignoreFinalizer_) {
            finalizer_();
        }
    });
}

EventImpl::~EventImpl()
{
    if (loop_ != nullptr) {
        loop_->DecObjRef(loop_);
        loop_ = nullptr;
    }
    if (fd_.IsValid()) {
        fd_.Close();
    }
}

int EventImpl::SetAction(const EventAction &action, const EventFinalizer &finalizer)
{
    if (!action || action_) {
        return -E_INVALID_ARGS;
    }
    if (IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }

    action_ = action;
    finalizer_ = finalizer;
    return E_OK;
}

int EventImpl::AddEvents(EventsMask events)
{
    if (!IsValidArg(events)) {
        return -E_INVALID_ARGS;
    }

    EventsMask genericEvents = ET_READ | ET_WRITE | ET_ERROR;
    if ((genericEvents & events) && !IsValidFd()) {
        LOGE("ev add events failed, fd is invalid.");
        return -E_INVALID_ARGS;
    }

    EventLoopImpl *loop = nullptr;
    {
        RefObject::AutoLock lockGuard(this);
        if (loop_ == nullptr) {
            events_ |= events;
            return E_OK;
        }
        loop = loop_;
        loop->IncObjRef(loop);
    }

    int errCode = loop->Modify(this, true, events);
    loop->DecObjRef(loop);
    if (errCode != E_OK) {
        LOGE("ev add events failed, err: '%d'.", errCode);
    }
    return errCode;
}

int EventImpl::RemoveEvents(EventsMask events)
{
    if (!IsValidArg(events)) {
        return -E_INVALID_ARGS;
    }

    EventsMask genericEvents = ET_READ | ET_WRITE | ET_ERROR;
    if ((genericEvents & events) && !IsValidFd()) {
        LOGE("ev remove events failed, fd is invalid.");
        return -E_INVALID_ARGS;
    }

    EventLoopImpl *loop = nullptr;
    {
        RefObject::AutoLock lockGuard(this);
        if (loop_ == nullptr) {
            events_ &= ~events;
            return E_OK;
        }
        loop = loop_;
        loop->IncObjRef(loop);
    }

    int errCode = loop->Modify(this, false, events);
    loop->DecObjRef(loop);
    if (errCode != E_OK) {
        LOGE("ev remove events failed, err: '%d'.", errCode);
    }
    return errCode;
}

int EventImpl::SetTimeout(EventTime timeout)
{
    if (!IsValidArg(timeout)) {
        return -E_INVALID_ARGS;
    }

    EventLoopImpl *loop = nullptr;
    {
        RefObject::AutoLock lockGuard(this);
        if (loop_ == nullptr) {
            timeout_ = timeout;
            return E_OK;
        }
        loop = loop_;
        loop->IncObjRef(loop);
    }

    int errCode = loop->Modify(this, timeout);
    loop->DecObjRef(loop);
    if (errCode != E_OK) {
        LOGE("ev set timeout failed, err: '%d'.", errCode);
    }
    return errCode;
}

int EventImpl::Detach(bool wait)
{
    EventLoopImpl *loop = nullptr;
    {
        RefObject::AutoLock lockGuard(this);
        if (loop_ == nullptr) {
            return E_OK;
        }
        loop = loop_;
        loop->IncObjRef(loop);
    }

    int errCode = loop->Remove(this);
    if (errCode == -E_OBJ_IS_KILLED) {
        errCode = E_OK;
    }

    if ((errCode == E_OK) && wait) {
        bool started = true;
        if (!loop->IsInLoopThread(started)) {
            Wait();
        }
        loop->DecObjRef(loop);
        return E_OK;
    }

    loop->DecObjRef(loop);
    return errCode;
}

void EventImpl::IgnoreFinalizer()
{
    ignoreFinalizer_ = true;
}

int EventImpl::CheckStatus() const
{
    RefObject::AutoLock lockGuard(this);
    if (IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }
    if (!action_) {
        return -E_INVALID_ARGS;
    }
    return E_OK;
}

bool EventImpl::IsTimer() const
{
    return !IsValidFd();
}

bool EventImpl::IsValidFd() const
{
    return fd_.IsValid();
}

EventFd EventImpl::GetEventFd() const
{
    return fd_;
}

EventsMask EventImpl::GetEvents() const
{
    return events_;
}

bool EventImpl::SetLoop(EventLoopImpl *loop)
{
    RefObject::AutoLock lockGuard(this);
    if (loop == nullptr) {
        if (loop_ != nullptr) {
            loop_->DecObjRef(loop_);
            loop_ = nullptr;
        }
        detached_.notify_one();
        return true;
    }
    if (loop_ == nullptr) {
        loop->IncObjRef(loop);
        loop_ = loop;
        return true;
    }
    return false;
}

void EventImpl::Wait()
{
    RefObject::AutoLock lockGuard(this);
    WaitLockedUntil(detached_, [this]()->bool { return loop_ == nullptr; });
}

bool EventImpl::Attached(const EventLoopImpl *loop, bool &isLoopConfused) const
{
    RefObject::AutoLock lockGuard(this);
    if (loop_ != nullptr && loop != nullptr && loop_ != loop) {
        // the event object is attached to another loop.
        isLoopConfused = true;
    } else {
        isLoopConfused = false;
    }
    // returns true when both are nullptr.
    return loop_ == loop;
}

void EventImpl::SetEvents(bool isAdd, EventsMask events)
{
    if (isAdd) {
        events_ |= events;
    } else {
        events_ &= ~events;
    }
}

void EventImpl::SetRevents(EventsMask events)
{
    EventsMask genericEvents = ET_READ | ET_WRITE | ET_ERROR;
    EventsMask revents = events & genericEvents;
    if (revents) {
        revents_ = revents;
    } else {
        revents_ = events & ET_TIMEOUT;
    }
}

void EventImpl::SetTimeoutPeriod(EventTime timeout)
{
    if (timeout < 0) {
        timeout_ = MAX_TIME_VALUE;
    } else {
        timeout_ = timeout;
    }
}

void EventImpl::SetStartTime(EventTime startTime)
{
    start_ = startTime;
}

bool EventImpl::GetTimeoutPoint(EventTime &timePoint) const
{
    if (events_ & ET_TIMEOUT) {
        timePoint = start_ + timeout_;
        return true;
    }
    timePoint = MAX_TIME_VALUE;
    return false;
}

void EventImpl::UpdateElapsedTime(EventTime now)
{
    if (events_ & ET_TIMEOUT) {
        EventTime timePoint = start_ + timeout_;
        if ((now >= timePoint) || (now < start_)) {
            start_ = now;
            if (!revents_) {
                revents_ = ET_TIMEOUT;
            }
        }
    }
}

int EventImpl::Dispatch()
{
    if (!action_) {
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    if (!IsKilled()) {
        if (revents_) {
            errCode = action_(revents_);
            if (errCode != E_OK) {
                LOGI("ev action() returns '%d'.", errCode);
            }
        }
    }
    return errCode;
}

bool EventImpl::IsValidArg(EventsMask events) const
{
    EventsMask allEvents = ET_READ | ET_WRITE | ET_ERROR | ET_TIMEOUT;
    if (!events || (events & (~allEvents))) {
        return false;
    }
    return true;
}

bool EventImpl::IsValidArg(EventTime timeout) const
{
    if ((timeout < 0) || (timeout > MAX_TIME_VALUE)) {
        return false;
    }
    return true;
}

DEFINE_OBJECT_TAG_FACILITIES(EventImpl)
}

