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

#include "kv_store_delegate_manager.h"

#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <map>
#include <thread>

#include "db_constant.h"
#include "platform_specific.h"
#include "log_print.h"
#include "db_common.h"
#include "kv_store_errno.h"
#include "kvdb_pragma.h"
#include "kvdb_properties.h"
#include "kvdb_manager.h"
#include "kv_store_nb_delegate_impl.h"
#include "network_adapter.h"
#include "runtime_context.h"
#include "param_check_utils.h"
#include "auto_launch.h"
#ifndef OMIT_MULTI_VER
#include "kv_store_delegate_impl.h"
#endif

namespace DistributedDB {
const std::string KvStoreDelegateManager::DEFAULT_PROCESS_APP_ID = "default";
std::mutex KvStoreDelegateManager::communicatorMutex_;
std::shared_ptr<IProcessCommunicator> KvStoreDelegateManager::processCommunicator_ = nullptr;

namespace {
    const int GET_CONNECT_RETRY = 3;
    const int RETRY_GET_CONN_INTER = 30;

    IKvDBConnection *GetOneConnectionWithRetry(const KvDBProperties &properties, int &errCode)
    {
        for (int i = 0; i < GET_CONNECT_RETRY; i++) {
            auto conn = KvDBManager::GetDatabaseConnection(properties, errCode);
            if (conn != nullptr) {
                return conn;
            }
            if (errCode == -E_STALE) {
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_GET_CONN_INTER));
            } else {
                return nullptr;
            }
        }
        return nullptr;
    }

    DBStatus CheckAndGetSchema(bool isMemoryDb, const std::string &schema, SchemaObject &schemaObj)
    {
        if (isMemoryDb && !schema.empty()) {
            LOGW("[KvStoreDelegateManager] memory database doesn't support the schema.");
            return NOT_SUPPORT;
        }
        if (schema.empty()) {
            return OK;
        }
        schemaObj.ParseFromSchemaString(schema);
        if (!schemaObj.IsSchemaValid()) {
            return INVALID_SCHEMA;
        }
        return OK;
    }

    void InitPropWithNbOption(KvDBProperties &properties,  const std::string &storePath,
        const SchemaObject &schema, const KvStoreNbDelegate::Option &option)
    {
        properties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, option.createIfNecessary);
        properties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
        properties.SetBoolProp(KvDBProperties::MEMORY_MODE, option.isMemoryDb);
        properties.SetBoolProp(KvDBProperties::ENCRYPTED_MODE, option.isEncryptedDb);
        properties.SetStringProp(KvDBProperties::DATA_DIR, storePath);
        properties.SetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, option.createDirByStoreIdOnly);
        properties.SetSchema(schema);
        if (RuntimeContext::GetInstance()->IsProcessSystemApiAdapterValid()) {
            properties.SetIntProp(KvDBProperties::SECURITY_LABEL, option.secOption.securityLabel);
            properties.SetIntProp(KvDBProperties::SECURITY_FLAG, option.secOption.securityFlag);
        }
        properties.SetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, option.conflictResolvePolicy);

        if (option.isEncryptedDb) {
            properties.SetPassword(option.cipher, option.passwd);
        }
    }

    bool CheckObserverConflictParam(const KvStoreNbDelegate::Option &option)
    {
        if ((option.notifier && !ParamCheckUtils::CheckConflictNotifierType(option.conflictType)) ||
            (!option.notifier && option.conflictType != 0)) {
            LOGE("Invalid conflict type, conflict type is [%d]", option.conflictType);
            return false;
        }
        if ((option.observer != nullptr && !ParamCheckUtils::CheckObserver(option.key, option.mode)) ||
            (option.observer == nullptr && (!option.key.empty() || option.mode != 0))) {
            LOGE("Invalid observer param, observer mode is [%u]", option.mode);
            return false;
        }
        return true;
    }

#ifndef OMIT_MULTI_VER
    void InitPropWithOption(KvDBProperties &properties, const std::string &storePath,
        const KvStoreDelegate::Option &option)
    {
        properties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, option.createIfNecessary);
        properties.SetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, option.createDirByStoreIdOnly);
        properties.SetIntProp(KvDBProperties::DATABASE_TYPE,
            ((option.localOnly == true) ? KvDBProperties::LOCAL_TYPE : KvDBProperties::MULTI_VER_TYPE));
        properties.SetBoolProp(KvDBProperties::MEMORY_MODE, false);
        properties.SetBoolProp(KvDBProperties::ENCRYPTED_MODE, option.isEncryptedDb);
        properties.SetStringProp(KvDBProperties::DATA_DIR, storePath);
        if (option.isEncryptedDb) {
            properties.SetPassword(option.cipher, option.passwd);
        }
    }
#endif
}

KvStoreDelegateManager::KvStoreDelegateManager(const std::string &appId, const std::string &userId)
    : appId_(appId),
      userId_(userId)
{}

KvStoreDelegateManager::~KvStoreDelegateManager() {}

DBStatus KvStoreDelegateManager::SetKvStoreConfig(const KvStoreConfig &kvStoreConfig)
{
    std::string canonicalDir;
    if (!IsDataDirSafe(kvStoreConfig.dataDir, canonicalDir)) {
        return INVALID_ARGS;
    }
    if (!OS::CheckPathExistence(canonicalDir)) {
        LOGE("[KvStoreMgr] Data dir doesn't exist or no perm");
        return INVALID_ARGS;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        kvStoreConfig_ = kvStoreConfig;
        kvStoreConfig_.dataDir = canonicalDir;
    }
    return OK;
}

#ifndef OMIT_MULTI_VER
void KvStoreDelegateManager::GetKvStore(const std::string &storeId, const KvStoreDelegate::Option &option,
    const std::function<void(DBStatus, KvStoreDelegate *)> &callback)
{
    if (!callback) {
        LOGE("[KvStoreMgr] Invalid callback for kv store!");
        return;
    }

    // Multi version and local database mode not allow the creation of a memory database
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId_, userId_) || GetKvStorePath().empty()) {
        callback(INVALID_ARGS, nullptr);
        return;
    }

    if (option.isEncryptedDb) {
        if (!ParamCheckUtils::CheckEncryptedParameter(option.cipher, option.passwd)) {
            callback(INVALID_ARGS, nullptr);
            return;
        }
    }

    KvDBProperties properties;
    InitPropWithOption(properties, GetKvStorePath(), option);
    DBCommon::SetDatabaseIds(properties, appId_, userId_, storeId);

    int errCode;
    IKvDBConnection *conn = GetOneConnectionWithRetry(properties, errCode);
    if (conn == nullptr) {
        DBStatus status = TransferDBErrno(errCode);
        callback(status, nullptr);
        return;
    }

    auto kvStore = new (std::nothrow) KvStoreDelegateImpl(conn, storeId);
    if (kvStore == nullptr) {
        LOGE("[KvStoreMgr] Failed to alloc the delegate");
        conn->Close();
        conn = nullptr;
        callback(DB_ERROR, nullptr);
        return;
    }
    callback(OK, kvStore);
}
#endif

DBStatus KvStoreDelegateManager::SetObserverNotifier(KvStoreNbDelegate *kvStore,
    const KvStoreNbDelegate::Option &option)
{
    DBStatus status;
    if (option.observer != nullptr) {
        status = kvStore->RegisterObserver(option.key, option.mode, option.observer);
        if (status != OK) {
            LOGE("[KvStoreMgr] RegisterObserver failed.");
            return status;
        }
    }
    if (option.notifier != nullptr) {
        status = kvStore->SetConflictNotifier(option.conflictType, option.notifier);
        if (status != OK) {
            LOGE("[KvStoreMgr] SetConflictNotifier failed.");
            return status;
        }
    }
    return OK;
}

bool KvStoreDelegateManager::GetKvStoreParamCheck(const std::string &storeId, const KvStoreNbDelegate::Option &option,
    const std::function<void(DBStatus, KvStoreNbDelegate *)> &callback) const
{
    if (!callback) {
        LOGE("[KvStoreMgr] Invalid callback for kv store");
        return false;
    }
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId_, userId_) ||
        (GetKvStorePath().empty() && !option.isMemoryDb)) {
        LOGE("[KvStoreMgr] Invalid id or path info for the store");
        callback(INVALID_ARGS, nullptr);
        return false;
    }

    // check if want an encrypted db
    if (option.isEncryptedDb) {
        if (option.isMemoryDb) {
            LOGE("Memory db not support encrypt!");
            callback(NOT_SUPPORT, nullptr);
            return false;
        }
        if (!ParamCheckUtils::CheckEncryptedParameter(option.cipher, option.passwd)) {
            callback(INVALID_ARGS, nullptr);
            return false;
        }
    }
    // check secOption
    if (!option.isMemoryDb) {
        if (!ParamCheckUtils::CheckSecOption(option.secOption)) {
            callback(INVALID_ARGS, nullptr);
            return false;
        }
    } else {
        if (option.secOption.securityLabel != SecurityLabel::NOT_SET ||
            option.secOption.securityFlag != 0) {
            LOGE("Memory db has no physical files, Is not controlled by security labels, so not support set labels");
            callback(INVALID_ARGS, nullptr);
            return false;
        }
    }

    if (!CheckObserverConflictParam(option)) {
        callback(INVALID_ARGS, nullptr);
        return false;
    }
    return true;
}

void KvStoreDelegateManager::GetKvStore(const std::string &storeId, const KvStoreNbDelegate::Option &option,
    const std::function<void(DBStatus, KvStoreNbDelegate *)> &callback)
{
    if (!GetKvStoreParamCheck(storeId, option, callback)) {
        return;
    }
    // check if schema is supported and valid
    SchemaObject schema;
    DBStatus retCode = CheckAndGetSchema(option.isMemoryDb, option.schema, schema);
    if (retCode != OK) {
        callback(retCode, nullptr);
        return;
    }
    KvDBProperties properties;
    InitPropWithNbOption(properties, GetKvStorePath(), schema, option);
    DBCommon::SetDatabaseIds(properties, appId_, userId_, storeId);

    int errCode;
    IKvDBConnection *conn = GetOneConnectionWithRetry(properties, errCode);
    DBStatus status = TransferDBErrno(errCode);
    if (conn == nullptr) {
        callback(status, nullptr);
        return;
    }

    auto kvStore = new (std::nothrow) KvStoreNbDelegateImpl(conn, storeId);
    if (kvStore == nullptr) {
        conn->Close();
        conn = nullptr;
        callback(DB_ERROR, nullptr);
        return;
    }

    status = SetObserverNotifier(kvStore, option);
    if (status != OK) {
        CloseKvStore(kvStore);
        callback(status, nullptr);
        return;
    }

    bool enAutoSync = false;
    (void)conn->Pragma(PRAGMA_AUTO_SYNC, static_cast<void *>(&enAutoSync));

    SecurityOption secOption = option.secOption;
    (void)conn->Pragma(PRAGMA_TRIGGER_TO_MIGRATE_DATA, &secOption);

    callback(OK, kvStore);
}

#ifndef OMIT_MULTI_VER
DBStatus KvStoreDelegateManager::CloseKvStore(KvStoreDelegate *kvStore)
{
    if (kvStore == nullptr) {
        return INVALID_ARGS;
    }

    auto kvStoreImpl = static_cast<KvStoreDelegateImpl *>(kvStore);
    DBStatus status = kvStoreImpl->Close();
    if (status == BUSY) {
        LOGD("DelegateImpl is busy now.");
        return BUSY;
    }

    kvStoreImpl->SetReleaseFlag(true);
    delete kvStore;
    kvStore = nullptr;
    return OK;
}
#endif

DBStatus KvStoreDelegateManager::CloseKvStore(KvStoreNbDelegate *kvStore)
{
    if (kvStore == nullptr) {
        return INVALID_ARGS;
    }

    auto kvStoreImpl = static_cast<KvStoreNbDelegateImpl *>(kvStore);
    DBStatus status = kvStoreImpl->Close();
    if (status == BUSY) {
        LOGD("NbDelegateImpl is busy now.");
        return BUSY;
    }
    kvStoreImpl->SetReleaseFlag(true);
    delete kvStore;
    kvStore = nullptr;
    return OK;
}

DBStatus KvStoreDelegateManager::DeleteKvStore(const std::string &storeId)
{
    if (!ParamCheckUtils::IsStoreIdSafe(storeId) || GetKvStorePath().empty()) {
        LOGE("Invalid store info for deleting");
        return INVALID_ARGS;
    }

    KvDBProperties properties;
    properties.SetStringProp(KvDBProperties::DATA_DIR, GetKvStorePath());
    DBCommon::SetDatabaseIds(properties, appId_, userId_, storeId);
    int errCode = KvDBManager::RemoveDatabase(properties);
    if (errCode == E_OK) {
        LOGI("Database deleted successfully!");
        return OK;
    }
    LOGE("Delete the kv store error:%d", errCode);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreDelegateManager::SetProcessLabel(const std::string &appId, const std::string &userId)
{
    if (appId.size() > DBConstant::MAX_APP_ID_LENGTH || appId.empty() ||
        userId.size() > DBConstant::MAX_USER_ID_LENGTH || userId.empty()) {
        LOGE("Invalid app or user info[%zu]-[%zu]", appId.length(), userId.length());
        return INVALID_ARGS;
    }

    int errCode = KvDBManager::SetProcessLabel(appId, userId);
    if (errCode != E_OK) {
        LOGE("Failed to set the process label:%d", errCode);
        return DB_ERROR;
    }
    return OK;
}

DBStatus KvStoreDelegateManager::SetProcessCommunicator(const std::shared_ptr<IProcessCommunicator> &inCommunicator)
{
    std::lock_guard<std::mutex> lock(communicatorMutex_);
    if (processCommunicator_ != nullptr) {
        LOGE("processCommunicator_ is not null!");
        return DB_ERROR;
    }

    std::string processLabel = RuntimeContext::GetInstance()->GetProcessLabel();
    if (processLabel.empty()) {
        LOGE("ProcessLabel is not set!");
        return DB_ERROR;
    }

    NetworkAdapter *adapter = new (std::nothrow) NetworkAdapter(processLabel, inCommunicator);
    if (adapter == nullptr) {
        LOGE("New NetworkAdapter failed!");
        return DB_ERROR;
    }
    processCommunicator_ = inCommunicator;
    if (RuntimeContext::GetInstance()->SetCommunicatorAdapter(adapter) != E_OK) {
        LOGE("SetProcessCommunicator not support!");
        delete adapter;
        return DB_ERROR;
    }
    KvDBManager::RestoreSyncableKvStore();
    return OK;
}

DBStatus KvStoreDelegateManager::GetKvStoreDiskSize(const std::string &storeId, uint64_t &size)
{
    std::string dataDir = GetKvStorePath();
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId_, userId_)) {
        LOGE("[KvStoreMgr] Invalid store info for size");
        return INVALID_ARGS;
    }
    KvDBProperties properties;
    properties.SetStringProp(KvDBProperties::DATA_DIR, dataDir);
    DBCommon::SetDatabaseIds(properties, appId_, userId_, storeId);
    int errCode = KvDBManager::CalculateKvStoreSize(properties, size);
    if (errCode != E_OK) {
        if (errCode == -E_NOT_FOUND) {
            return NOT_FOUND;
        }

        LOGE("[KvStoreMgr] Get the file size failed[%d]", errCode);
        return DB_ERROR;
    }
    return OK;
}

void KvStoreDelegateManager::SetKvStoreCorruptionHandler(const KvStoreCorruptionHandler &handler)
{
    KvDBManager::SetDatabaseCorruptionHandler(handler);
}

DBStatus KvStoreDelegateManager::GetDatabaseDir(const std::string &storeId, const std::string &appId,
    const std::string &userId, std::string &directory)
{
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId, userId)) {
        return INVALID_ARGS;
    }

    std::string identifier = DBCommon::GenerateIdentifierId(storeId, appId, userId);
    std::string dir = DBCommon::TransferHashString(identifier);
    if (dir.empty()) {
        return DB_ERROR;
    }
    directory = DBCommon::TransferStringToHex(dir);
    return OK;
}

DBStatus KvStoreDelegateManager::GetDatabaseDir(const std::string &storeId, std::string &directory)
{
    if (!ParamCheckUtils::IsStoreIdSafe(storeId)) {
        return INVALID_ARGS;
    }

    if (storeId.find(DBConstant::ID_CONNECTOR) != std::string::npos) {
        return INVALID_ARGS;
    }

    std::string dir = DBCommon::TransferHashString(storeId);
    if (dir.empty()) {
        return DB_ERROR;
    }
    directory = DBCommon::TransferStringToHex(dir);
    return OK;
}

// private
bool KvStoreDelegateManager::IsDataDirSafe(const std::string &dataDir, std::string &canonicalDir) const
{
    return ParamCheckUtils::CheckDataDir(dataDir, canonicalDir);
}

const std::string &KvStoreDelegateManager::GetKvStorePath() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return kvStoreConfig_.dataDir;
}

DBStatus KvStoreDelegateManager::SetPermissionCheckCallback(const PermissionCheckCallback &callback)
{
    int errCode = RuntimeContext::GetInstance()->SetPermissionCheckCallback(callback);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreDelegateManager::SetPermissionCheckCallback(const PermissionCheckCallbackV2 &callback)
{
    int errCode = RuntimeContext::GetInstance()->SetPermissionCheckCallback(callback);
    return TransferDBErrno(errCode);
}

DBStatus KvStoreDelegateManager::EnableKvStoreAutoLaunch(const std::string &userId, const std::string &appId,
    const std::string &storeId, const AutoLaunchOption &option, const AutoLaunchNotifier &notifier)
{
    if (RuntimeContext::GetInstance() == nullptr) {
        return DB_ERROR;
    }
    AutoLaunchParam param{ userId, appId, storeId, option, notifier };
    KvDBProperties properties;
    int errCode = AutoLaunch::GetAutoLaunchProperties(param, properties);
    if (errCode != E_OK) {
        LOGE("[KvStoreManager] Enable auto launch failed:%d", errCode);
        return TransferDBErrno(errCode);
    }

    errCode = RuntimeContext::GetInstance()->EnableKvStoreAutoLaunch(properties, notifier, option.observer,
        option.conflictType, option.notifier);
    if (errCode != E_OK) {
        LOGE("[KvStoreManager] Enable auto launch failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    LOGI("[KvStoreManager] Enable auto launch");
    return OK;
}

DBStatus KvStoreDelegateManager::DisableKvStoreAutoLaunch(const std::string &userId, const std::string &appId,
    const std::string &storeId)
{
    if (RuntimeContext::GetInstance() == nullptr) {
        return DB_ERROR;
    }

    std::string syncIdentifier = DBCommon::GenerateIdentifierId(storeId, appId, userId);
    std::string hashIdentifier = DBCommon::TransferHashString(syncIdentifier);
    int errCode = RuntimeContext::GetInstance()->DisableKvStoreAutoLaunch(hashIdentifier);
    if (errCode != E_OK) {
        LOGE("[KvStoreManager] Disable auto launch failed:%d", errCode);
        return TransferDBErrno(errCode);
    }
    LOGI("[KvStoreManager] Disable auto launch");
    return OK;
}

void KvStoreDelegateManager::SetAutoLaunchRequestCallback(const AutoLaunchRequestCallback &callback)
{
    RuntimeContext::GetInstance()->SetAutoLaunchRequestCallback(callback);
}

std::string KvStoreDelegateManager::GetKvStoreIdentifier(const std::string &userId, const std::string &appId,
    const std::string &storeId)
{
    if (!ParamCheckUtils::CheckStoreParameter(storeId, appId, userId)) {
        return "";
    }
    return DBCommon::TransferHashString(userId + "-" + appId + "-" + storeId);
}

DBStatus KvStoreDelegateManager::SetProcessSystemAPIAdapter(const std::shared_ptr<IProcessSystemApiAdapter> &adapter)
{
    return TransferDBErrno(RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(adapter));
}
} // namespace DistributedDB
