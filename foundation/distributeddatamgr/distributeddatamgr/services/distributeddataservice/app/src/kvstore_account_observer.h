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

#ifndef KVSTORE_ACCOUNT_OBSERVER_H
#define KVSTORE_ACCOUNT_OBSERVER_H

#include "kvstore_data_service.h"
#include "account_delegate.h"
#include <atomic>

namespace OHOS {
namespace DistributedKv {
// KvStore account event proc controller.
extern std::atomic<int> g_kvStoreAccountEventStatus;
#define KVSTORE_ACCOUNT_EVENT_PROCESSING_CHECKER(result) \
do { \
    if (g_kvStoreAccountEventStatus == 1) { \
        ZLOGW("system is busy with processing account event."); \
    } \
} while (0)

class KvStoreAccountObserver : public AccountDelegate::Observer {
public:
    explicit KvStoreAccountObserver(KvStoreDataService &kvStoreDataService)
        : kvStoreDataService_(kvStoreDataService)  {}
    ~KvStoreAccountObserver() override = default;

    void OnAccountChanged(const AccountEventInfo &eventInfo) override;
    // must specify unique name for observer
    std::string Name() override
    {
        return "DistributedDataService";
    }

private:
    KvStoreDataService &kvStoreDataService_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // KVSTORE_ACCOUNT_OBSERVER_H
