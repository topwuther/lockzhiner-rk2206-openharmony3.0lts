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

#ifndef KVSTORE_OBSERVER_H
#define KVSTORE_OBSERVER_H

#include <memory>
#include "change_notification.h"
#include "kvstore_snapshot.h"

namespace OHOS {
namespace DistributedKv {
// client implement this class to watch kvstore change.
class KvStoreObserver {
public:
    KVSTORE_API KvStoreObserver() = default;

    KVSTORE_API virtual ~KvStoreObserver()
    {}

    // client override this function to receive change notification.
    KVSTORE_API
    virtual void OnChange(const ChangeNotification &changeNotification, std::unique_ptr<KvStoreSnapshot> snapshot) = 0;

    // client override this function to receive change notification.
    KVSTORE_API
    virtual void OnChange(const ChangeNotification &changeNotification)
    {}
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_OBSERVER_H
