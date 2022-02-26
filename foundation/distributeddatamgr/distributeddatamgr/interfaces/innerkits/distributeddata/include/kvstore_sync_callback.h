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

#ifndef KVSTORE_SYNC_CALLBACK_H
#define KVSTORE_SYNC_CALLBACK_H

#include <map>
#include "types.h"

namespace OHOS {
namespace DistributedKv {
// client implememt this class to watch kvstore change.
class KvStoreSyncCallback {
public:
    KVSTORE_API KvStoreSyncCallback() = default;

    KVSTORE_API virtual ~KvStoreSyncCallback()
    {}

    // This virtual function will be called on sync callback.
    // Client needs to override this function to receive sync results.
    // Parameters:
    //     results: sync results for devices set in Sync function.
    KVSTORE_API virtual void SyncCompleted(const std::map<std::string, Status> &results) = 0;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_SYNC_CALLBACK_H
