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

#define LOG_TAG "KvStoreAccountObserver"

#include "kvstore_account_observer.h"
#include "log_print.h"
#include <atomic>
namespace OHOS {
namespace DistributedKv {
std::atomic<int> g_kvStoreAccountEventStatus {0};
void KvStoreAccountObserver::OnAccountChanged(const AccountEventInfo &eventInfo)
{
    ZLOGI("account event %d, begin.", eventInfo.status);
    kvStoreDataService_.AccountEventChanged(eventInfo);
    ZLOGI("account event %d, end.", eventInfo.status);
}
}  // namespace DistributedKv
}  // namespace OHOS