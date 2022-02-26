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

#ifndef SYNCER_FACTORY_H
#define SYNCER_FACTORY_H

#include <memory>

#include "isyncer.h"

namespace DistributedDB {
class SyncerFactory final {
public:
    // Product a ISyncer for the given type
    // type can be : IKvDBSyncInterface::SYNC_SVD
    //               IKvDBSyncInterface::SYNC_MVD
    static std::shared_ptr<ISyncer> GetSyncer(int type);
};
} // namespace DistributedDB

#endif  // SYNCER_FACTORY_H
