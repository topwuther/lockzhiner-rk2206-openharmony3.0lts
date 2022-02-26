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

#ifndef KV_DB_OBSERVER_HANDLE_H
#define KV_DB_OBSERVER_HANDLE_H

#include <list>

#include "macro_utils.h"
#include "notification_chain.h"

namespace DistributedDB {
class KvDBObserverHandle {
public:
    explicit KvDBObserverHandle(uint32_t mode);
    ~KvDBObserverHandle();
    DISABLE_COPY_ASSIGN_MOVE(KvDBObserverHandle);
    void InsertListener(NotificationChain::Listener *listener);
    uint32_t GetObserverMode() const;
private:
    std::list<NotificationChain::Listener *> listeners_;
    uint32_t mode_;
};
} // namespace DistributedDB

#endif // KV_DB_OBSERVER_H
