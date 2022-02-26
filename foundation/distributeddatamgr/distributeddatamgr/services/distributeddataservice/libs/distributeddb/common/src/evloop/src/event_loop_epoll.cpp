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

#include "event_loop_epoll.h"

#ifdef EVENT_LOOP_USE_EPOLL
#include <sys/eventfd.h>
#include "event_impl.h"
#include "log_print.h"
#include "db_errno.h"

namespace DistributedDB {
EventLoopEpoll::EventLoopEpoll()
    : pollFdCount_(0)
{
}

EventLoopEpoll::~EventLoopEpoll()
{
    if (wakeUpFd_.IsValid()) {
        wakeUpFd_.Close();
    }
    if (epollFd_.IsValid()) {
        epollFd_.Close();
    }
}

int EventLoopEpoll::Initialize()
{
    if (epollFd_.IsValid()) {
        return -E_INVALID_ARGS;
    }

    int errCode;
    wakeUpFd_ = EventFd(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC));
    if (!wakeUpFd_.IsValid()) {
        errCode = -errno;
        LOGE("Create event fd failed, err:'%d'", errCode);
        return errCode;
    }

    epollFd_ = EventFd(epoll_create(EPOLL_INIT_REVENTS));
    if (!epollFd_.IsValid()) {
        errCode = -errno;
        wakeUpFd_.Close();
        LOGE("Create epoll fd failed, err:'%d'", errCode);
        return errCode;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = this;
    errCode = epoll_ctl(epollFd_, EPOLL_CTL_ADD, wakeUpFd_, &event);
    if (errCode < 0) {
        errCode = -errno;
        epollFd_.Close();
        wakeUpFd_.Close();
        LOGE("Add wake up fd to epoll failed, err:'%d'", errCode);
        return errCode;
    }

    ++pollFdCount_;
    return E_OK;
}

int EventLoopEpoll::Prepare(const std::set<EventImpl *> &polling)
{
    if (pollFdCount_ > 0) {
        revents_.resize(pollFdCount_);
        return E_OK;
    }
    LOGE("Prepared epoll loop failed, fd count:'%d'", pollFdCount_);
    return -E_INTERNAL_ERROR;
}

int EventLoopEpoll::Poll(EventTime sleepTime)
{
    if (sleepTime > INT_MAX) {
        LOGE("[EventLoopEpoll][Poll] sleepTime is too large!");
        return -E_INVALID_ARGS;
    }
    int nReady = epoll_wait(epollFd_, &revents_[0], revents_.size(), sleepTime);
    if (nReady < 0) {
        int errCode = -errno;
        if (errCode != -EINTR) {
            LOGE("Call epoll wait failed, err:'%d'", errCode);
            return errCode;
        }
        nReady = 0;
    }

    for (int index = 0; index < nReady; ++index) {
        struct epoll_event *revent = &revents_[index];
        if (revent->data.ptr == this) {
            EpollWokenUp();
            continue;
        }
        auto event = static_cast<EventImpl *>(revent->data.ptr);
        EventsMask revents = CalEventsMask(revent->events);
        event->SetRevents(revents);
    }
    return E_OK;
}

int EventLoopEpoll::WakeUp()
{
    int64_t incValue = 1;

    while (true) {
        int nWrite = write(wakeUpFd_, &incValue, sizeof(incValue));
        if (nWrite == sizeof(incValue)) {
            break;
        }

        int errCode = -errno;
        if (errCode == -EINTR) {
            continue;
        }
        if (errCode == -EAGAIN) {
            // We have already signalled the loop.
            break;
        }
        LOGE("Write loop wake up data failed, err:'%d'", errCode);
        return errCode;
    }
    return E_OK;
}

int EventLoopEpoll::Exit(const std::set<EventImpl *> &polling)
{
    if (revents_.capacity() > 0) {
        std::vector<epoll_event> revents;
        revents.swap(revents_);
    }
    wakeUpFd_.Close();
    epollFd_.Close();
    pollFdCount_ = 0;
    return E_OK;
}

void EventLoopEpoll::EpollWokenUp()
{
    while (true) {
        int64_t intValue;
        int nRead = read(wakeUpFd_, &intValue, sizeof(intValue));
        if (nRead < 0) {
            int errCode = -errno;
            if (errCode == -EINTR) {
                continue;
            }
            if (errCode != -EAGAIN) {
                LOGE("Clear loop wake up data failed, err:'%d'", errCode);
            }
        }
        break;
    }
}

uint32_t EventLoopEpoll::CalEpollEvents(EventsMask events) const
{
    uint32_t epollEvents = 0;
    if (events & IEvent::ET_READ) {
        epollEvents |= EPOLLIN;
    }
    if (events & IEvent::ET_WRITE) {
        epollEvents |= EPOLLOUT;
    }
    if (events & IEvent::ET_ERROR) {
        epollEvents |= EPOLLERR;
    }
    return epollEvents;
}

EventsMask EventLoopEpoll::CalEventsMask(uint32_t epollEvents)
{
    EventsMask events = 0;
    if (epollEvents & EPOLLIN) {
        events |= IEvent::ET_READ;
    }
    if (epollEvents & EPOLLOUT) {
        events |= IEvent::ET_WRITE;
    }
    if (epollEvents & EPOLLERR) {
        events |= IEvent::ET_ERROR;
    }
    return events;
}

int EventLoopEpoll::EpollCtl(int operation, EventImpl *event, EventsMask events)
{
    if (operation != EPOLL_CTL_ADD &&
        operation != EPOLL_CTL_MOD &&
        operation != EPOLL_CTL_DEL) {
        return -E_INVALID_ARGS;
    }
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    EventFd fd = event->GetEventFd();
    if (fd.IsValid()) {
        return -E_INVALID_ARGS;
    }

    uint32_t epollEvents = CalEpollEvents(events);
    struct epoll_event epollEvent;
    epollEvent.events = epollEvents;
    epollEvent.data.ptr = event;

    int errCode = epoll_ctl(epollFd_, operation, fd, &epollEvent);
    if (errCode < 0) {
        errCode = -errno;
        return errCode;
    }
    return E_OK;
}

int EventLoopEpoll::AddEvent(EventImpl *event)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    EventsMask events = event->GetEvents();
    int errCode = EpollCtl(EPOLL_CTL_ADD, event, events);
    if (errCode != E_OK) {
        LOGE("Add fd to epoll set failed, err:'%d'", errCode);
        return errCode;
    }

    ++pollFdCount_;
    return E_OK;
}

int EventLoopEpoll::RemoveEvent(EventImpl *event)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    EventsMask events = event->GetEvents();
    int errCode = EpollCtl(EPOLL_CTL_DEL, event, events);
    if (errCode != E_OK) {
        LOGE("Remove fd from epoll set failed, err:'%d'", errCode);
        return errCode;
    }

    --pollFdCount_;
    return E_OK;
}

int EventLoopEpoll::ModifyEvent(EventImpl *event, bool isAdd, EventsMask events)
{
    if (event == nullptr) {
        return -E_INVALID_ARGS;
    }

    EventsMask newEvents = event->GetEvents();
    if (isAdd) {
        newEvents |= events;
    } else {
        newEvents &= ~events;
    }

    int errCode = EpollCtl(EPOLL_CTL_MOD, event, newEvents);
    if (errCode != E_OK) {
        LOGE("Modify fd in epoll set failed, err:'%d'", errCode);
        return errCode;
    }
    return E_OK;
}

DEFINE_OBJECT_TAG_FACILITIES(EventLoopEpoll)
}
#endif // EVENT_LOOP_USE_EPOLL
