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

#include "../include/ievent.h"
#include "db_errno.h"
#include "event_impl.h"

namespace DistributedDB {
IEvent *IEvent::CreateEvent(EventTime timeout, int &errCode)
{
    if (timeout < 0) {
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }

    IEvent *event = new (std::nothrow) EventImpl(timeout);
    if (event == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = E_OK;
    return event;
}

IEvent *IEvent::CreateEvent(EventFd fd, EventsMask events,
    EventTime timeout, int &errCode)
{
    errCode = -E_INVALID_ARGS;
    if (!events) {
        return nullptr;
    }
    if ((events & ET_TIMEOUT) && (timeout < 0)) {
        return nullptr;
    }
    if (!(events & ET_TIMEOUT) && !fd.IsValid()) {
        return nullptr;
    }

    IEvent *event = new (std::nothrow) EventImpl(fd, events, timeout);
    if (event == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = E_OK;
    return event;
}
}
