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

#define LOG_TAG "DistributedKvDataManager"

#include "distributed_kv_data_manager.h"
#include "constant.h"
#include "mock_single_kv_store.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
DistributedKvDataManager::DistributedKvDataManager()
{}

DistributedKvDataManager::~DistributedKvDataManager()
{}

void DistributedKvDataManager::GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
    std::function<void(Status, std::unique_ptr<SingleKvStore>)> callback)
{
    std::string storeIdTmp = Constant::TrimCopy<std::string>(storeId.storeId);
    Status status = Status::SUCCESS;
    std::unique_ptr<SingleKvStore> proxyTmp = std::make_unique<AnsTestSingleKvStore>();
    callback(status, std::move(proxyTmp));
}

Status DistributedKvDataManager::CloseKvStore(
    const AppId &appId, const StoreId &storeId, std::unique_ptr<KvStore> kvStorePtr)
{
    return Status::SUCCESS;
}

Status DistributedKvDataManager::CloseKvStore(const AppId &appId, std::unique_ptr<SingleKvStore> kvStorePtr)
{
    return Status::SUCCESS;
}

Status DistributedKvDataManager::DeleteKvStore(const AppId &appId, const StoreId &storeId)
{
    return Status::SUCCESS;
}

void DistributedKvDataManager::RegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient)
{}

void DistributedKvDataManager::UnRegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipient)
{}
}  // namespace DistributedKv
}  // namespace OHOS
