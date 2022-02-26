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
#include "kv_store_delegate_impl.h"

#include <functional>
#include <string>

#include "platform_specific.h"
#include "log_print.h"
#include "param_check_utils.h"
#include "db_constant.h"
#include "db_errno.h"
#include "db_types.h"
#include "kv_store_errno.h"
#include "kvdb_pragma.h"
#include "kv_store_observer.h"
#include "kvdb_manager.h"
#include "kv_store_snapshot_delegate_impl.h"
#include "kv_store_changed_data_impl.h"

namespace DistributedDB {
namespace {
    const std::string INVALID_CONNECTION = "[KvStoreDelegate] Invalid connection for operation";
}
KvStoreDelegateImpl::KvStoreDelegateImpl(IKvDBConnection *conn, const std::string &storeId)
    : conn_(conn),
      storeId_(storeId),
      releaseFlag_(false)
{}

KvStoreDelegateImpl::~KvStoreDelegateImpl()
{
    if (!releaseFlag_) {
        LOGF("[KvStoreDelegate] can not release object directly");
        return;
    }

    LOGI("[KvStoreDelegate] deconstruct");
    conn_ = nullptr;
}

DBStatus KvStoreDelegateImpl::Put(const Key &key, const Value &value)
{
    if (conn_ != nullptr) {
        IOption option;
        int errCode = conn_->Put(option, key, value);
        if (errCode == E_OK) {
            return OK;
        }

        LOGE("[KvStoreDelegate] Put data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::PutBatch(const std::vector<Entry> &entries)
{
    if (conn_ != nullptr) {
        IOption option;
        int errCode = conn_->PutBatch(option, entries);
        if (errCode == E_OK) {
            return OK;
        }

        LOGE("[KvStoreDelegate] Put batch data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::Delete(const Key &key)
{
    if (conn_ != nullptr) {
        IOption option;
        int errCode = conn_->Delete(option, key);
        if (errCode == E_OK || errCode == -E_NOT_FOUND) {
            return OK;
        }

        LOGE("[KvStoreDelegate] Delete data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::DeleteBatch(const std::vector<Key> &keys)
{
    if (conn_ != nullptr) {
        IOption option;
        int errCode = conn_->DeleteBatch(option, keys);
        if (errCode == E_OK || errCode == -E_NOT_FOUND) {
            return OK;
        }

        LOGE("[KvStoreDelegate] Delete batch data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::Clear()
{
    if (conn_ != nullptr) {
        IOption option;
        int errCode = conn_->Clear(option);
        if (errCode == E_OK) {
            return OK;
        }

        LOGE("[KvStoreDelegate] Clear data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

std::string KvStoreDelegateImpl::GetStoreId() const
{
    return storeId_;
}

void KvStoreDelegateImpl::GetKvStoreSnapshot(KvStoreObserver *observer,
    const std::function<void(DBStatus, KvStoreSnapshotDelegate *)> &callback)
{
    if (!callback) {
        LOGE("[KvStoreDelegate] Invalid callback for snapshot!");
        return;
    }

    if (conn_ != nullptr) {
        if (observer != nullptr && RegisterObserver(observer) != E_OK) {
            LOGE("[KvStoreDelegate][GetSnapshot] Register observer failed!");
            callback(DB_ERROR, nullptr);
            return;
        }

        IKvDBSnapshot *snapshot = nullptr;
        int errCode = conn_->GetSnapshot(snapshot);
        if (errCode == E_OK) {
            auto snapshotDelegate = new (std::nothrow) KvStoreSnapshotDelegateImpl(snapshot, observer);
            if (snapshotDelegate != nullptr) {
                callback(OK, snapshotDelegate);
                return;
            }
            conn_->ReleaseSnapshot(snapshot);
            snapshot = nullptr;
        }

        // UnRegister the registered observer.
        errCode = UnRegisterObserver(observer);
        if (errCode != E_OK) {
            LOGE("[KvStoreDelegate][GetSnapshot] UnRegister observer failed:%d!", errCode);
        }
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    callback(DB_ERROR, nullptr);
}

DBStatus KvStoreDelegateImpl::ReleaseKvStoreSnapshot(KvStoreSnapshotDelegate *&snapshotDelegate)
{
    if (conn_ != nullptr && snapshotDelegate != nullptr) {
        KvStoreObserver *observer = nullptr;
        (static_cast<KvStoreSnapshotDelegateImpl *>(snapshotDelegate))->GetObserver(observer);
        if (observer != nullptr && UnRegisterObserver(observer) != E_OK) {
            LOGE("[KvStoreDelegate][ReleaseSnapshot] UnRegistObserver failed!");
            return DB_ERROR;
        }

        IKvDBSnapshot *snapshot = nullptr;
        (static_cast<KvStoreSnapshotDelegateImpl *>(snapshotDelegate))->GetSnapshot(snapshot);
        conn_->ReleaseSnapshot(snapshot);
        snapshot = nullptr;
        delete snapshotDelegate;
        snapshotDelegate = nullptr;
        return OK;
    }

    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::RegisterObserver(KvStoreObserver *observer)
{
    if (observer == nullptr) {
        return INVALID_ARGS;
    }

    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    std::lock_guard<std::mutex> lockGuard(observerMapLock_);
    if (observerMap_.find(observer) != observerMap_.end()) {
        LOGE("[KvStoreDelegate] Observer has been already registered!");
        return DB_ERROR;
    }

    Key key;
    int errCode = E_OK;
    KvDBObserverHandle *observerHandle = conn_->RegisterObserver(
        static_cast<unsigned int>(DATABASE_COMMIT_EVENT),
        key,
        [observer](const KvDBCommitNotifyData &ptr) {
            KvStoreChangedDataImpl data(&ptr);
            observer->OnChange(data);
        },
        errCode);

    if (errCode != E_OK || observerHandle == nullptr) {
        LOGE("[KvStoreDelegate] Register listener failed:%d!", errCode);
        return DB_ERROR;
    }

    observerMap_.insert(std::pair<const KvStoreObserver *, const KvDBObserverHandle *>(observer, observerHandle));
    return OK;
}

// Unregister a data change observer
DBStatus KvStoreDelegateImpl::UnRegisterObserver(const KvStoreObserver *observer)
{
    if (observer == nullptr) {
        return INVALID_ARGS;
    }

    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    std::lock_guard<std::mutex> lockGuard(observerMapLock_);
    auto iter = observerMap_.find(observer);
    if (iter == observerMap_.end()) {
        LOGE("[KvStoreDelegate] observer has not been registered!");
        return NOT_FOUND;
    }

    const KvDBObserverHandle *observerHandle = iter->second;
    int errCode = conn_->UnRegisterObserver(observerHandle);
    if (errCode != E_OK) {
        LOGE("[KvStoreDelegate] UnRegister observer failed:%d!", errCode);
        return DB_ERROR;
    }
    observerMap_.erase(iter);
    return OK;
}

DBStatus KvStoreDelegateImpl::StartTransaction()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->StartTransaction();
    if (errCode != E_OK) {
        LOGE("[KvStoreDelegate] StartTransaction failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreDelegateImpl::Commit()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Commit();
    if (errCode != E_OK) {
        LOGE("[KvStoreDelegate] Commit failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreDelegateImpl::Rollback()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->RollBack();
    if (errCode != E_OK) {
        LOGE("[KvStoreDelegate] Rollback failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreDelegateImpl::SetConflictResolutionPolicy(ResolutionPolicyType type,
    const ConflictResolution &resolution)
{
    if (type == AUTO_LAST_WIN) {
        return OK;
    }

    if (type == CUSTOMER_RESOLUTION && resolution != nullptr) {
        return OK;
    }
    LOGE("[KvStoreDelegate] Invalid conflict resolution policy:%d", type);
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::Rekey(const CipherPassword &password)
{
    if (conn_ != nullptr) {
        int errCode = conn_->Rekey(password);
        if (errCode == E_OK) {
            return OK;
        }

        LOGE("[KvStoreDelegate] rekey failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::Export(const std::string &filePath, const CipherPassword &passwd)
{
    std::string fileDir;
    std::string fileName;
    OS::SplitFilePath(filePath, fileDir, fileName);

    std::string canonicalUrl;
    if (!ParamCheckUtils::CheckDataDir(fileDir, canonicalUrl)) {
        return INVALID_ARGS;
    }

    if (!OS::CheckPathExistence(canonicalUrl)) {
        return NO_PERMISSION;
    }

    canonicalUrl = canonicalUrl + "/" + fileName;
    if (OS::CheckPathExistence(canonicalUrl)) {
        LOGE("[KvStoreDelegate] The exported file has already been existed");
        return FILE_ALREADY_EXISTED;
    }

    if (conn_ != nullptr) {
        int errCode = conn_->Export(canonicalUrl, passwd);
        if (errCode == E_OK) {
            return OK;
        }
        LOGE("[KvStoreDelegate] Export failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreDelegateImpl::Import(const std::string &filePath, const CipherPassword &passwd)
{
    std::string fileDir;
    std::string fileName;
    OS::SplitFilePath(filePath, fileDir, fileName);

    std::string canonicalUrl;
    if (!ParamCheckUtils::CheckDataDir(fileDir, canonicalUrl)) {
        return INVALID_ARGS;
    }

    canonicalUrl = canonicalUrl + "/" + fileName;
    if (!OS::CheckPathExistence(canonicalUrl)) {
        LOGE("[KvStoreDelegate] The imported file not existed:%d", errno);
        return INVALID_FILE;
    }

    if (conn_ != nullptr) {
        int errCode = conn_->Import(canonicalUrl, passwd);
        if (errCode == E_OK) {
            return OK;
        }
        LOGE("[KvStoreDelegate] Import failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

void KvStoreDelegateImpl::SetReleaseFlag(bool flag)
{
    releaseFlag_ = flag;
}

DBStatus KvStoreDelegateImpl::Close()
{
    if (conn_ != nullptr) {
        int errCode = KvDBManager::ReleaseDatabaseConnection(conn_);
        if (errCode == -E_BUSY) {
            LOGW("[KvStoreDelegate] busy for close");
            return BUSY;
        }

        LOGI("[KvStoreDelegate] Close");
        conn_ = nullptr;
    }
    return OK;
}

DBStatus KvStoreDelegateImpl::Pragma(PragmaCmd cmd, PragmaData &paramData)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode;
    switch (cmd) {
        case PERFORMANCE_ANALYSIS_GET_REPORT:
            errCode = conn_->Pragma(PRAGMA_PERFORMANCE_ANALYSIS_GET_REPORT, paramData);
            break;
        case PERFORMANCE_ANALYSIS_OPEN:
            errCode = conn_->Pragma(PRAGMA_PERFORMANCE_ANALYSIS_OPEN, paramData);
            break;
        case PERFORMANCE_ANALYSIS_CLOSE:
            errCode = conn_->Pragma(PRAGMA_PERFORMANCE_ANALYSIS_CLOSE, paramData);
            break;
        case PERFORMANCE_ANALYSIS_SET_REPORTFILENAME:
            errCode = conn_->Pragma(PRAGMA_PERFORMANCE_ANALYSIS_SET_REPORTFILENAME, paramData);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }

    if (errCode != E_OK) {
        LOGE("[KvStoreDelegate] Pragma failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}
} // namespace DistributedDB
#endif
