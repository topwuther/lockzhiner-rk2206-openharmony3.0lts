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

#define LOG_TAG "KvStoreDeathRecipientImpl"

#include "kvstore_death_recipient_impl.h"

#include <utility>
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreDeathRecipientImpl::KvStoreDeathRecipientImpl(std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient)
    : kvStoreDeathRecipient_(std::move(kvStoreDeathRecipient))
{
    ZLOGI("constructor");
}

KvStoreDeathRecipientImpl::~KvStoreDeathRecipientImpl()
{
    ZLOGI("destructor");
}

void KvStoreDeathRecipientImpl::OnRemoteDied()
{
    ZLOGI("OnRemoteDied");
    if (kvStoreDeathRecipient_ != nullptr) {
        ZLOGI("call client");
        kvStoreDeathRecipient_->OnRemoteDied();
    }
}
}  // namespace DistributedKv
}  // namespace OHOS