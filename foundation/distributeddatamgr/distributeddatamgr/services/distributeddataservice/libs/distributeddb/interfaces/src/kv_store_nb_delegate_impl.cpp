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

#include "kv_store_nb_delegate_impl.h"

#include <functional>
#include <string>

#include "platform_specific.h"
#include "log_print.h"
#include "db_constant.h"
#include "db_errno.h"
#include "db_types.h"
#include "param_check_utils.h"
#include "types.h"
#include "kvdb_pragma.h"
#include "kvdb_manager.h"
#include "kv_store_errno.h"
#include "kv_store_observer.h"
#include "kv_store_changed_data_impl.h"
#include "kv_store_nb_conflict_data_impl.h"
#include "kv_store_result_set_impl.h"
#include "sync_operation.h"
#include "performance_analysis.h"

namespace DistributedDB {
namespace {
    struct PragmaCmdPair {
        int externCmd = 0;
        int innerCmd = 0;
    };

    const PragmaCmdPair g_pragmaMap[] = {
        {GET_DEVICE_IDENTIFIER_OF_ENTRY, PRAGMA_GET_DEVICE_IDENTIFIER_OF_ENTRY},
        {AUTO_SYNC, PRAGMA_AUTO_SYNC},
        {PERFORMANCE_ANALYSIS_GET_REPORT, PRAGMA_PERFORMANCE_ANALYSIS_GET_REPORT},
        {PERFORMANCE_ANALYSIS_OPEN, PRAGMA_PERFORMANCE_ANALYSIS_OPEN},
        {PERFORMANCE_ANALYSIS_CLOSE, PRAGMA_PERFORMANCE_ANALYSIS_CLOSE},
        {PERFORMANCE_ANALYSIS_SET_REPORTFILENAME, PRAGMA_PERFORMANCE_ANALYSIS_SET_REPORTFILENAME},
        {GET_IDENTIFIER_OF_DEVICE, PRAGMA_GET_IDENTIFIER_OF_DEVICE},
        {GET_QUEUED_SYNC_SIZE, PRAGMA_GET_QUEUED_SYNC_SIZE},
        {SET_QUEUED_SYNC_LIMIT, PRAGMA_SET_QUEUED_SYNC_LIMIT},
        {GET_QUEUED_SYNC_LIMIT, PRAGMA_GET_QUEUED_SYNC_LIMIT},
        {SET_WIPE_POLICY, PRAGMA_SET_WIPE_POLICY},
        {RESULT_SET_CACHE_MODE, PRAGMA_RESULT_SET_CACHE_MODE},
        {RESULT_SET_CACHE_MAX_SIZE, PRAGMA_RESULT_SET_CACHE_MAX_SIZE},
    };

    const std::string INVALID_CONNECTION = "[KvStoreNbDelegate] Invalid connection for operation";
}

KvStoreNbDelegateImpl::KvStoreNbDelegateImpl(IKvDBConnection *conn, const std::string &storeId)
    : conn_(conn),
      storeId_(storeId),
      releaseFlag_(false)
{}

KvStoreNbDelegateImpl::~KvStoreNbDelegateImpl()
{
    if (!releaseFlag_) {
        LOGF("[KvStoreNbDelegate] Can't release directly");
        return;
    }

    conn_ = nullptr;
}

DBStatus KvStoreNbDelegateImpl::Get(const Key &key, Value &value) const
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    return GetInner(option, key, value);
}

DBStatus KvStoreNbDelegateImpl::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    return GetEntriesInner(option, keyPrefix, entries);
}

DBStatus KvStoreNbDelegateImpl::GetEntries(const Key &keyPrefix, KvStoreResultSet *&resultSet) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    IKvDBResultSet *kvDbResultSet = nullptr;
    int errCode = conn_->GetResultSet(option, keyPrefix, kvDbResultSet);
    if (errCode == E_OK) {
        resultSet = new (std::nothrow) KvStoreResultSetImpl(kvDbResultSet);
        if (resultSet != nullptr) {
            return OK;
        }

        LOGE("[KvStoreNbDelegate] Alloc result set failed.");
        conn_->ReleaseResultSet(kvDbResultSet);
        kvDbResultSet = nullptr;
        return DB_ERROR;
    }

    LOGE("[KvStoreNbDelegate] Get result set failed: %d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::GetEntries(const Query &query, std::vector<Entry> &entries) const
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    if (conn_ != nullptr) {
        int errCode = conn_->GetEntries(option, query, entries);
        if (errCode == E_OK) {
            return OK;
        } else if (errCode == -E_NOT_FOUND) {
            LOGD("[KvStoreNbDelegate] Not found the data by query");
            return NOT_FOUND;
        }

        LOGE("[KvStoreNbDelegate] Get the batch data by query err:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreNbDelegateImpl::GetEntries(const Query &query, KvStoreResultSet *&resultSet) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    IKvDBResultSet *kvDbResultSet = nullptr;
    int errCode = conn_->GetResultSet(option, query, kvDbResultSet);
    if (errCode == E_OK) {
        resultSet = new (std::nothrow) KvStoreResultSetImpl(kvDbResultSet);
        if (resultSet != nullptr) {
            return OK;
        }

        LOGE("[KvStoreNbDelegate] Alloc result set failed.");
        conn_->ReleaseResultSet(kvDbResultSet);
        kvDbResultSet = nullptr;
        return DB_ERROR;
    }

    LOGE("[KvStoreNbDelegate] Get result set for query failed: %d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::GetCount(const Query &query, int &count) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    int errCode = conn_->GetCount(option, query, count);
    if (errCode == E_OK) {
        if (count == 0) {
            return NOT_FOUND;
        }
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Get count for query failed: %d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::CloseResultSet(KvStoreResultSet *&resultSet)
{
    if (resultSet == nullptr) {
        return INVALID_ARGS;
    }

    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    // release inner result set
    IKvDBResultSet *kvDbResultSet = nullptr;
    (static_cast<KvStoreResultSetImpl *>(resultSet))->GetResultSet(kvDbResultSet);
    conn_->ReleaseResultSet(kvDbResultSet);
    // release external result set
    delete resultSet;
    resultSet = nullptr;
    return OK;
}

DBStatus KvStoreNbDelegateImpl::Put(const Key &key, const Value &value)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    return PutInner(option, key, value);
}

DBStatus KvStoreNbDelegateImpl::PutBatch(const std::vector<Entry> &entries)
{
    if (conn_ != nullptr) {
        IOption option;
        option.dataType = IOption::SYNC_DATA;
        int errCode = conn_->PutBatch(option, entries);
        if (errCode == E_OK) {
            return OK;
        }

        LOGE("[KvStoreNbDelegate] Put batch data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreNbDelegateImpl::DeleteBatch(const std::vector<Key> &keys)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    int errCode = conn_->DeleteBatch(option, keys);
    if (errCode == E_OK || errCode == -E_NOT_FOUND) {
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Delete batch data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::Delete(const Key &key)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    return DeleteInner(option, key);
}

DBStatus KvStoreNbDelegateImpl::GetLocal(const Key &key, Value &value) const
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    return GetInner(option, key, value);
}

DBStatus KvStoreNbDelegateImpl::GetLocalEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    return GetEntriesInner(option, keyPrefix, entries);
}

DBStatus KvStoreNbDelegateImpl::PutLocal(const Key &key, const Value &value)
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    return PutInner(option, key, value);
}

DBStatus KvStoreNbDelegateImpl::DeleteLocal(const Key &key)
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    return DeleteInner(option, key);
}

DBStatus KvStoreNbDelegateImpl::PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
    const KvStoreNbPublishOnConflict &onConflict)
{
    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        LOGW("[KvStoreNbDelegate][Publish] Invalid para");
        return INVALID_ARGS;
    }

    if (conn_ != nullptr) {
        PragmaPublishInfo publishInfo{ key, deleteLocal, updateTimestamp, onConflict };
        int errCode = conn_->Pragma(PRAGMA_PUBLISH_LOCAL, static_cast<PragmaData>(&publishInfo));
        if (errCode != E_OK) {
            LOGD("[KvStoreNbDelegate] Publish local err:%d", errCode);
            return TransferDBErrno(errCode);
        }
        return OK;
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreNbDelegateImpl::UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp)
{
    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        LOGW("[KvStoreNbDelegate][Unpublish] Invalid para");
        return INVALID_ARGS;
    }

    if (conn_ != nullptr) {
        PragmaUnpublishInfo unpublishInfo{ key, deletePublic, updateTimestamp };
        int errCode = conn_->Pragma(PRAGMA_UNPUBLISH_SYNC, static_cast<PragmaData>(&unpublishInfo));
        if (errCode != E_OK) {
            LOGD("[KvStoreNbDelegate] Unpublish result:%d", errCode);
            return TransferDBErrno(errCode);
        }
        return OK;
    }

    LOGE("%s", INVALID_CONNECTION.c_str());
    return DB_ERROR;
}

DBStatus KvStoreNbDelegateImpl::PutLocalBatch(const std::vector<Entry> &entries)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    int errCode = conn_->PutBatch(option, entries);
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Put local batch data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    return OK;
}

DBStatus KvStoreNbDelegateImpl::DeleteLocalBatch(const std::vector<Key> &keys)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    int errCode = conn_->DeleteBatch(option, keys);
    if (errCode == E_OK || errCode == -E_NOT_FOUND) {
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Delete local batch data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::RegisterObserver(const Key &key, unsigned int mode, KvStoreObserver *observer)
{
    if (key.size() > DBConstant::MAX_KEY_SIZE) {
        return INVALID_ARGS;
    }

    if (!ParamCheckUtils::CheckObserver(key, mode)) {
        LOGE("Register nb observer by illegal mode or key size!");
        return INVALID_ARGS;
    }

    if (observer == nullptr) {
        return INVALID_ARGS;
    }

    std::lock_guard<std::mutex> lockGuard(observerMapLock_);
    if (observerMap_.find(observer) != observerMap_.end()) {
        LOGE("[KvStoreNbDelegate] Observer has been already registered!");
        return DB_ERROR;
    }

    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    if (conn_->IsTransactionStarted()) {
        return BUSY;
    }

    int errCode = E_OK;
    KvDBObserverHandle *observerHandle = conn_->RegisterObserver(
        mode, key,
        [observer](const KvDBCommitNotifyData &notifyData) {
            KvStoreChangedDataImpl data(&notifyData);
            observer->OnChange(data);
        },
        errCode);

    if (errCode != E_OK || observerHandle == nullptr) {
        LOGE("[KvStoreNbDelegate] RegisterListener failed:%d!", errCode);
        return DB_ERROR;
    }

    observerMap_.insert(std::pair<const KvStoreObserver *, const KvDBObserverHandle *>(observer, observerHandle));
    LOGI("[KvStoreNbDelegate] RegisterObserver ok mode:%u", mode);
    return OK;
}

DBStatus KvStoreNbDelegateImpl::UnRegisterObserver(const KvStoreObserver *observer)
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
        LOGE("[KvStoreNbDelegate] Observer has not been registered!");
        return NOT_FOUND;
    }

    const KvDBObserverHandle *observerHandle = iter->second;
    int errCode = conn_->UnRegisterObserver(observerHandle);
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] UnRegistObserver failed:%d!", errCode);
        return DB_ERROR;
    }
    observerMap_.erase(iter);
    return OK;
}

DBStatus KvStoreNbDelegateImpl::RemoveDeviceData(const std::string &device)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Pragma(PRAGMA_RM_DEVICE_DATA,
        const_cast<void *>(static_cast<const void *>(&device)));
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Remove device data failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

std::string KvStoreNbDelegateImpl::GetStoreId() const
{
    return storeId_;
}

DBStatus KvStoreNbDelegateImpl::Sync(const std::vector<std::string> &devices, SyncMode mode,
    const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete,
    bool wait = false)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    PragmaSync pragmaData(devices, mode, std::bind(&KvStoreNbDelegateImpl::OnSyncComplete,
        this, std::placeholders::_1, onComplete), wait);
    int errCode = conn_->Pragma(PRAGMA_SYNC_DEVICES, &pragmaData);
    if (errCode < E_OK) {
        if (errCode == -E_BUSY) {
            return BUSY;
        }

        if (errCode == -E_INVALID_ARGS) {
            return INVALID_ARGS;
        }

        LOGE("[KvStoreNbDelegate] Sync data failed:%d", errCode);
        return DB_ERROR;
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::Pragma(PragmaCmd cmd, PragmaData &paramData)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = -E_NOT_SUPPORT;
    for (const auto &item : g_pragmaMap) {
        if (item.externCmd == cmd) {
            errCode = conn_->Pragma(item.innerCmd, paramData);
            break;
        }
    }

    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Pragma failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::SetConflictNotifier(int conflictType, const KvStoreNbConflictNotifier &notifier)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    if (!ParamCheckUtils::CheckConflictNotifierType(conflictType)) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return INVALID_ARGS;
    }

    int errCode;
    if (!notifier) {
        errCode = conn_->SetConflictNotifier(conflictType, nullptr);
        goto END;
    }

    errCode = conn_->SetConflictNotifier(conflictType,
        [conflictType, notifier](const KvDBCommitNotifyData &data) {
            int resultCode;
            const std::list<KvDBConflictEntry> entries = data.GetCommitConflicts(resultCode);
            if (resultCode != E_OK) {
                LOGE("Get commit conflicted entries failed:%d!", resultCode);
                return;
            }

            for (const auto &entry : entries) {
                // Prohibit signed numbers to perform bit operations
                uint32_t entryType = static_cast<uint32_t>(entry.type);
                uint32_t type = static_cast<uint32_t>(conflictType);
                if (entryType & type) {
                    KvStoreNbConflictDataImpl dataImpl;
                    dataImpl.SetConflictData(entry);
                    notifier(dataImpl);
                }
            }
        });

END:
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Register conflict failed:%d!", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::Rekey(const CipherPassword &password)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Rekey(password);
    if (errCode == E_OK) {
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Rekey failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::Export(const std::string &filePath, const CipherPassword &passwd)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

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
        return FILE_ALREADY_EXISTED;
    }

    int errCode = conn_->Export(canonicalUrl, passwd);
    if (errCode == E_OK) {
        return OK;
    }
    LOGE("[KvStoreNbDelegate] Export failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    std::string fileDir;
    std::string fileName;
    OS::SplitFilePath(filePath, fileDir, fileName);

    std::string canonicalUrl;
    if (!ParamCheckUtils::CheckDataDir(fileDir, canonicalUrl)) {
        return INVALID_ARGS;
    }

    canonicalUrl = canonicalUrl + "/" + fileName;
    if (!OS::CheckPathExistence(canonicalUrl)) {
        LOGE("Import file path err, DBStatus = INVALID_FILE errno = [%d]", errno);
        return INVALID_FILE;
    }

    int errCode = conn_->Import(canonicalUrl, passwd);
    if (errCode == E_OK) {
        LOGI("[KvStoreNbDelegate] Import ok");
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Import failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::StartTransaction()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->StartTransaction();
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] StartTransaction failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::Commit()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Commit();
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Commit failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::Rollback()
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->RollBack();
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Rollback failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    return OK;
}

void KvStoreNbDelegateImpl::SetReleaseFlag(bool flag)
{
    releaseFlag_ = flag;
}

DBStatus KvStoreNbDelegateImpl::Close()
{
    if (conn_ != nullptr) {
        int errCode = KvDBManager::ReleaseDatabaseConnection(conn_);
        if (errCode == -E_BUSY) {
            LOGI("[KvStoreNbDelegate] Busy for close");
            return BUSY;
        }

        LOGI("[KvStoreNbDelegateImpl] Database connection Close");
        conn_ = nullptr;
    }
    return OK;
}

DBStatus KvStoreNbDelegateImpl::GetSecurityOption(SecurityOption &option) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }
    return TransferDBErrno(conn_->GetSecurityOption(option.securityLabel, option.securityFlag));
}

DBStatus KvStoreNbDelegateImpl::SetRemotePushFinishedNotify(const RemotePushFinishedNotifier &notifier)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    PragmaRemotePushNotify notify(notifier);
    int errCode = conn_->Pragma(PRAGMA_REMOTE_PUSH_FINISHED_NOTIFY, reinterpret_cast<void *>(&notify));
    if (errCode != E_OK) {
        LOGE("[KvStoreNbDelegate] Set remote push finished notify failed : %d", errCode);
    }
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::GetInner(const IOption &option, const Key &key, Value &value) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Get(option, key, value);
    if (errCode == E_OK) {
        return OK;
    }
    LOGW("[KvStoreNbDelegate] Get the data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::GetEntriesInner(const IOption &option,
    const Key &keyPrefix, std::vector<Entry> &entries) const
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->GetEntries(option, keyPrefix, entries);
    if (errCode == E_OK) {
        return OK;
    }
    LOGW("[KvStoreNbDelegate] Get the batch data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::PutInner(const IOption &option, const Key &key, const Value &value)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_PUT_DATA);
    }

    int errCode = conn_->Put(option, key, value);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_PUT_DATA);
    }

    if (errCode == E_OK) {
        return OK;
    }
    LOGE("[KvStoreNbDelegate] Put the data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreNbDelegateImpl::DeleteInner(const IOption &option, const Key &key)
{
    if (conn_ == nullptr) {
        LOGE("%s", INVALID_CONNECTION.c_str());
        return DB_ERROR;
    }

    int errCode = conn_->Delete(option, key);
    if (errCode == E_OK || errCode == -E_NOT_FOUND) {
        return OK;
    }

    LOGE("[KvStoreNbDelegate] Delete the data failed:%d", errCode);
    return TransferDBErrno(errCode);
}

void KvStoreNbDelegateImpl::OnSyncComplete(const std::map<std::string, int> &statuses,
    const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete) const
{
    std::map<std::string, DBStatus> result;
    for (const auto &pair : statuses) {
        DBStatus status = DB_ERROR;
        static const std::map<int, DBStatus> statusMap = {
            { static_cast<int>(SyncOperation::FINISHED_ALL),                  DBStatus::OK },
            { static_cast<int>(SyncOperation::TIMEOUT),                       DBStatus::TIME_OUT },
            { static_cast<int>(SyncOperation::PERMISSION_CHECK_FAILED),       DBStatus::PERMISSION_CHECK_FORBID_SYNC },
            { static_cast<int>(SyncOperation::COMM_ABNORMAL),                 DBStatus::COMM_FAILURE },
            { static_cast<int>(SyncOperation::SECURITY_OPTION_CHECK_FAILURE), DBStatus::SECURITY_OPTION_CHECK_ERROR },
            { static_cast<int>(SyncOperation::EKEYREVOKED_FAILURE),           DBStatus::EKEYREVOKED_ERROR },
            { static_cast<int>(SyncOperation::SCHEMA_INCOMPATIBLE),           DBStatus::SCHEMA_MISMATCH },
            { static_cast<int>(SyncOperation::BUSY_FAILURE),                  DBStatus::BUSY },
        };
        auto iter = statusMap.find(pair.second);
        if (iter != statusMap.end()) {
            status = iter->second;
        }
        result.insert(std::pair<std::string, DBStatus>(pair.first, status));
    }
    if (onComplete) {
        onComplete(result);
    }
}
} // namespace DistributedDB
