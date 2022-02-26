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

#ifndef IEVENT_H
#define IEVENT_H

#include "ref_object.h"
#include "macro_utils.h"
#include "event_fd.h"

namespace DistributedDB {
using EventTime = int64_t;
using EventsMask = unsigned int;
using EventAction = std::function<int(EventsMask revents)>;
using EventFinalizer = std::function<void(void)>;

class IEvent : public virtual RefObject {
public:
    enum EventType {
        ET_READ = 0x01,
        ET_WRITE = 0x02,
        ET_ERROR = 0x04,
        ET_TIMEOUT = 0x08,
    };

    IEvent() = default;
    DISABLE_COPY_ASSIGN_MOVE(IEvent);

    virtual int SetAction(const EventAction &action, const EventFinalizer &finalizer = nullptr) = 0;
    virtual int AddEvents(EventsMask events) = 0;
    virtual int RemoveEvents(EventsMask events) = 0;
    virtual int SetTimeout(EventTime timeout) = 0;
    virtual int Detach(bool wait) = 0;
    virtual void IgnoreFinalizer() = 0;

    // The following 2 static methods is used to create real event objects,
    // instead of an event object factory.
    static IEvent *CreateEvent(EventTime timeout, int &errCode);
    static IEvent *CreateEvent(EventFd fd, EventsMask events, EventTime timeout, int &errCode);

protected:
    virtual ~IEvent() {};
};
}

#endif // IEVENT_H
