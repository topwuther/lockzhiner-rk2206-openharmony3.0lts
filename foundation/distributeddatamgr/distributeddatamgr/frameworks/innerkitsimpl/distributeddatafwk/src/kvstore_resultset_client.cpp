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

#define LOG_TAG "KvStoreResultSetClient"

#include "kvstore_resultset_client.h"
#include "dds_trace.h"

namespace OHOS::DistributedKv {
KvStoreResultSetClient::KvStoreResultSetClient(sptr<IKvStoreResultSet> kvStoreProxy)
    :kvStoreResultSetProxy_(kvStoreProxy)
{}

int KvStoreResultSetClient::GetCount() const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    return kvStoreResultSetProxy_->GetCount();
}

int KvStoreResultSetClient::GetPosition() const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->GetPosition();
}

bool KvStoreResultSetClient::MoveToFirst()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->MoveToFirst();
}

bool KvStoreResultSetClient::MoveToLast()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->MoveToLast();
}

bool KvStoreResultSetClient::MoveToNext()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->MoveToNext();
}

bool KvStoreResultSetClient::MoveToPrevious()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->MoveToPrevious();
}

bool KvStoreResultSetClient::Move(int offset)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->Move(offset);
}

bool KvStoreResultSetClient::MoveToPosition(int position)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    return kvStoreResultSetProxy_->MoveToPosition(position);
}

bool KvStoreResultSetClient::IsFirst() const
{
    return kvStoreResultSetProxy_->IsFirst();
}

bool KvStoreResultSetClient::IsLast() const
{
    return kvStoreResultSetProxy_->IsLast();
}

bool KvStoreResultSetClient::IsBeforeFirst() const
{
    return kvStoreResultSetProxy_->IsBeforeFirst();
}

bool KvStoreResultSetClient::IsAfterLast() const
{
    return kvStoreResultSetProxy_->IsAfterLast();
}

Status KvStoreResultSetClient::GetEntry(Entry &entry) const
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__), true);

    return kvStoreResultSetProxy_->GetEntry(entry);
}

sptr<IKvStoreResultSet> KvStoreResultSetClient::GetKvStoreResultSetProxy() const
{
    return kvStoreResultSetProxy_;
}
} // namespace OHOS::DistributedKv