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

#include "single_ver_sync_engine.h"
#include "single_ver_sync_task_context.h"
#include "log_print.h"

namespace DistributedDB {
ISyncTaskContext *SingleVerSyncEngine::CreateSyncTaskContext()
{
    auto context = new (std::nothrow) SingleVerSyncTaskContext;
    if (context == nullptr) {
        LOGE("[SingleVerSyncEngine][CreateSyncTaskContext] create failed, may be out of memory");
        return nullptr;
    }
    context->EnableClearRemoteStaleData(needClearRemoteStaleData_);
    return context;
}

void SingleVerSyncEngine::EnableClearRemoteStaleData(bool enable)
{
    LOGI("[SingleVerSyncEngine][EnableClearRemoteStaleData] enabled %d", enable);
    needClearRemoteStaleData_ = enable;
    std::unique_lock<std::mutex> lock(contextMapLock_);
    for (auto &iter : syncTaskContextMap_) {
        auto context = static_cast<SingleVerSyncTaskContext *>(iter.second);
        if (context != nullptr) {
            context->EnableClearRemoteStaleData(enable);
        }
    }
}

DEFINE_OBJECT_TAG_FACILITIES(SingleVerSyncEngine);
} // namespace DistributedDB