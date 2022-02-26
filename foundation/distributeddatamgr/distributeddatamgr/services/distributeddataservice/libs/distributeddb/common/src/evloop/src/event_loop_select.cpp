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

#include "event_loop_select.h"

#ifdef EVENT_LOOP_USE_SELECT
#include <chrono>
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
EventLoopSelect::EventLoopSelect()
{
}

EventLoopSelect::~EventLoopSelect()
{
}

int EventLoopSelect::Initialize()
{
    return E_OK;
}

int EventLoopSelect::Prepare(const std::set<EventImpl *> &polling)
{
    return E_OK;
}

int EventLoopSelect::Poll(EventTime sleepTime)
{
    std::unique_lock<std::mutex> lockGuard(wakeUpMutex_);
    auto now = std::chrono::system_clock::now();
    auto to = now + std::chrono::milliseconds(sleepTime);
    wakeUpCondition_.wait_until(lockGuard, to);
    return E_OK;
}

int EventLoopSelect::WakeUp()
{
    wakeUpCondition_.notify_one();
    return E_OK;
}

int EventLoopSelect::Exit(const std::set<EventImpl *> &polling)
{
    return E_OK;
}

int EventLoopSelect::AddEvent(EventImpl *event)
{
    return E_OK;
}

int EventLoopSelect::RemoveEvent(EventImpl *event)
{
    return E_OK;
}

int EventLoopSelect::ModifyEvent(EventImpl *event, bool isAdd, EventsMask events)
{
    return E_OK;
}

DEFINE_OBJECT_TAG_FACILITIES(EventLoopSelect)
}

#endif
