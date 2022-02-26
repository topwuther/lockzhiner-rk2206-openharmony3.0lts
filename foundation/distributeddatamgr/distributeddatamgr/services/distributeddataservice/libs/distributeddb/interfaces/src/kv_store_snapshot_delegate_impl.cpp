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

#ifndef OMIT_MULTI_VER
#include "kv_store_snapshot_delegate_impl.h"

#include "kv_store_errno.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
KvStoreSnapshotDelegateImpl::KvStoreSnapshotDelegateImpl(IKvDBSnapshot *snapshot, KvStoreObserver *observer)
    : snapShot_(snapshot),
      observer_(observer)
{}

void KvStoreSnapshotDelegateImpl::Get(
    const Key &key, const std::function<void(DBStatus, const Value &)> &callback) const
{
    if (!callback) {
        LOGE("[KvStoreSnapshot] Invalid callback!");
        return;
    }

    DBStatus status = DB_ERROR;
    Value value;
    if (snapShot_ != nullptr) {
        int errCode = snapShot_->Get(key, value);
        if (errCode == E_OK) {
            status = OK;
        } else {
            if (errCode != -E_NOT_FOUND) {
                LOGE("[KvStoreSnapshot] Get data failed:%d", errCode);
            }
            status = TransferDBErrno(errCode);
        }
    }

    callback(status, value);
}

void KvStoreSnapshotDelegateImpl::GetEntries(
    const Key &keyPrefix, const std::function<void(DBStatus, const std::vector<Entry> &)> &callback) const
{
    if (!callback) {
        LOGE("[KvStoreSnapshot] Invalid callback!");
        return;
    }

    DBStatus status = DB_ERROR;
    std::vector<Entry> entries;
    if (snapShot_ != nullptr) {
        int errCode = snapShot_->GetEntries(keyPrefix, entries);
        if (errCode == E_OK) {
            status = OK;
        } else {
            if (errCode != -E_NOT_FOUND) {
                LOGE("[KvStoreSnapshot] Get entries failed:%d", errCode);
            }
            status = TransferDBErrno(errCode);
        }
    }

    callback(status, entries);
}

void KvStoreSnapshotDelegateImpl::GetSnapshot(IKvDBSnapshot *&snapshot) const
{
    snapshot = snapShot_;
}

void KvStoreSnapshotDelegateImpl::GetObserver(KvStoreObserver *&observer) const
{
    observer = observer_;
}
} // namespace DistributedDB
#endif
