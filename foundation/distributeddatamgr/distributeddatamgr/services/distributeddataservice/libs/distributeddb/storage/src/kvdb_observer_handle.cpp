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
#include "kvdb_observer_handle.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
KvDBObserverHandle::KvDBObserverHandle(uint32_t mode)
    : mode_(mode)
{}

KvDBObserverHandle::~KvDBObserverHandle()
{
    for (auto &listener : listeners_) {
        int errCode = listener->Drop(true);
        if (errCode != E_OK) {
            LOGE("Drop listener failed!");
        }
        listener = nullptr;
    }
}

void KvDBObserverHandle::InsertListener(NotificationChain::Listener *listener)
{
    listeners_.push_back(listener);
    return;
}

uint32_t KvDBObserverHandle::GetObserverMode() const
{
    return mode_;
}
} // namespace DistributedDB
