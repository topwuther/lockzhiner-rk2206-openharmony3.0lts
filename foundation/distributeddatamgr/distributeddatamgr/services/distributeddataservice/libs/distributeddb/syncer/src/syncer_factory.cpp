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

#include "syncer_factory.h"

#include "single_ver_syncer.h"
#include "multi_ver_syncer.h"
#include "ikvdb_sync_interface.h"

namespace DistributedDB {
std::shared_ptr<ISyncer> SyncerFactory::GetSyncer(int type)
{
    if (type == IKvDBSyncInterface::SYNC_SVD) {
        std::shared_ptr<ISyncer> singleVerSyncer(std::make_shared<SingleVerSyncer>());
        return singleVerSyncer;
#ifndef OMIT_MULTI_VER
    } else if (type == IKvDBSyncInterface::SYNC_MVD) {
        std::shared_ptr<ISyncer> multiVerSyncer(std::make_shared<MultiVerSyncer>());
        return multiVerSyncer;
#endif
    } else {
        return nullptr;
    }
}
} // namespace DistributedDB
