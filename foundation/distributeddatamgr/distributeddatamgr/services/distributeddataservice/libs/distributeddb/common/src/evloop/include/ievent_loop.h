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

#ifndef IEVENT_LOOP_H
#define IEVENT_LOOP_H

#include "ref_object.h"
#include "macro_utils.h"

namespace DistributedDB {
class IEvent;

// Abstract of event loop.
class IEventLoop : public virtual RefObject {
public:
    IEventLoop() = default;

    DISABLE_COPY_ASSIGN_MOVE(IEventLoop);

    // Add an event object to the loop.
    virtual int Add(IEvent *event) = 0;

    // Remove an event object from the loop.
    virtual int Remove(IEvent *event) = 0;

    // Run the loop.
    virtual int Run() = 0;

    // Create a loop object.
    static IEventLoop *CreateEventLoop(int &errCode);

protected:
    virtual ~IEventLoop() {};
};
}

#endif // IEVENT_LOOP_H
