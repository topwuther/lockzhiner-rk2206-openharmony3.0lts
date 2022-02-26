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
#include "db_errno.h"

#ifdef EVENT_LOOP_USE_EPOLL
#include "event_loop_epoll.h"
using EventLoop = DistributedDB::EventLoopEpoll;
#else
#include "event_loop_select.h"
using EventLoop = DistributedDB::EventLoopSelect;
#endif

namespace DistributedDB {
IEventLoop *IEventLoop::CreateEventLoop(int &errCode)
{
    EventLoopImpl *loop = new (std::nothrow) EventLoop;
    if (loop == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    errCode = loop->Initialize();
    if (errCode != E_OK) {
        delete loop;
        loop = nullptr;
    }
    return loop;
}
}
