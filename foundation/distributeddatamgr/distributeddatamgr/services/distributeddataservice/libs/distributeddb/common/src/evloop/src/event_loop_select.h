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

#ifndef EVENT_LOOP_SELECT_H
#define EVENT_LOOP_SELECT_H

#include "event_loop_impl.h"

#ifdef EVENT_LOOP_USE_SELECT
#include <mutex>
#include <condition_variable>

namespace DistributedDB {
class EventLoopSelect : public EventLoopImpl {
public:
    EventLoopSelect();
    ~EventLoopSelect();

    int Initialize() override;

private:
    int Prepare(const std::set<EventImpl *> &polling) override;
    int Poll(EventTime sleepTime) override;
    int WakeUp() override;
    int Exit(const std::set<EventImpl *> &polling) override;
    int AddEvent(EventImpl *event) override;
    int RemoveEvent(EventImpl *event) override;
    int ModifyEvent(EventImpl *event, bool isAdd, EventsMask events) override;

    DECLARE_OBJECT_TAG(EventLoopSelect);

    std::mutex wakeUpMutex_;
    std::condition_variable wakeUpCondition_;
};
}
#endif // EVENT_LOOP_USE_SELECT
#endif // EVENT_LOOP_SELECT_H
