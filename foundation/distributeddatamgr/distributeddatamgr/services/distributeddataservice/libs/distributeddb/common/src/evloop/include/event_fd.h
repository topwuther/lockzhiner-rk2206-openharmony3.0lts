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

#ifndef EVENT_FD_H
#define EVENT_FD_H

#include "platform_specific.h"

#if defined EVLOOP_TIMER_ONLY
using Handle = int;
static const int INVALID_HANDLE = -1;
#define IS_VALID_HANDLE(h) false
#define CLOSE_HANDLE(h)
#else
#include <unistd.h>
using Handle = int;
static const int INVALID_HANDLE = -1;
#define IS_VALID_HANDLE(h) ((h) > 0)
#define CLOSE_HANDLE(h) do { close(h); } while (0)
#endif

namespace DistributedDB {
class EventFd final {
public:
    EventFd() : fd_(INVALID_HANDLE) {}
    explicit EventFd(Handle handle) : fd_(handle) {}

    ~EventFd()
    {
        // we can't close it.
        fd_ = INVALID_HANDLE;
    }

    bool IsValid() const
    {
        return IS_VALID_HANDLE(fd_);
    }

    operator Handle() const
    {
        return fd_;
    }

    bool operator==(const EventFd &other) const
    {
        return other.fd_ == fd_;
    }

    void Close()
    {
        if (IsValid()) {
            CLOSE_HANDLE(fd_);
            fd_ = INVALID_HANDLE;
        }
    }

private:
    Handle fd_;
};
}

#endif // EVENT_FD_H
