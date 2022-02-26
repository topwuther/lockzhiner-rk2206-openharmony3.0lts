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

#ifndef KVSTORE_DEATH_RECIPIENT_IMPL_H
#define KVSTORE_DEATH_RECIPIENT_IMPL_H

#include <memory>
#include "kvstore_death_recipient.h"

namespace OHOS {
namespace DistributedKv {

class KvStoreDeathRecipientImpl {
public:
    explicit KvStoreDeathRecipientImpl(std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient);
    ~KvStoreDeathRecipientImpl();
    void OnRemoteDied();
private:
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient_;
friend struct KvStoreDeathRecipientImplCompare;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KVSTORE_DEATH_RECIPIENT_IMPL_H