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

#ifndef OMIT_MULTI_VER
#include "multi_ver_sync_engine.h"
#include "multi_ver_sync_task_context.h"

namespace DistributedDB {
ISyncTaskContext *MultiVerSyncEngine::CreateSyncTaskContext()
{
    return new (std::nothrow) MultiVerSyncTaskContext;
}

DEFINE_OBJECT_TAG_FACILITIES(MultiVerSyncEngine);
} // namespace DistributedDB
#endif