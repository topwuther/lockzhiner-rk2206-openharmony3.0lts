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
#define LOG_TAG "KvStoreMetaManager"

#include "kvstore_meta_manager.h"
#include <chrono>
#include <condition_variable>
#include <directory_ex.h>
#include <file_ex.h>
#include <thread>
#include <unistd.h>
#include "security_adapter.h"

#include "account_delegate.h"
#include "constant.h"
#include "kvstore_utils.h"
#include "crypto_utils.h"
#include "device_kvstore_impl.h"
#include "kvstore_data_service.h"
#include "log_print.h"
#include "reporter.h"
#include "directory_utils.h"

namespace OHOS {
namespace DistributedKv {
using json = nlohmann::json;
using namespace std::chrono;

// APPID: distributeddata
// USERID: default
// STOREID: service_meta
// dataDir: /data/misc_de/0/mdds/Meta/${storeId}/sin_gen.db
std::condition_variable KvStoreMetaManager::cv_;
std::mutex KvStoreMetaManager::cvMutex_;
KvStoreMetaManager::MetaDeviceChangeListenerImpl KvStoreMetaManager::listener_;

KvStoreMetaManager::KvStoreMetaManager()
    : metaDBDirectory_(Constant::Concatenate({
      Constant::ROOT_PATH_DE, "/", Constant::SERVICE_NAME, "/", Constant::META_DIR_NAME })),
      kvStoreDelegateManager_(META_DB_APP_ID, Constant::GetDefaultHarmonyAccountName())
{
    ZLOGI("begin.");
}

KvStoreMetaManager::~KvStoreMetaManager()
{
}

KvStoreMetaManager &KvStoreMetaManager::GetInstance()
{
    static KvStoreMetaManager instance;
    return instance;
}

void KvStoreMetaManager::InitMetaListener(std::function<void(const KvStoreMetaData &metaData)> observer)
{
    metaObserver_.notify_ = observer;

    InitMetaData();
    auto status = KvStoreUtils::GetProviderInstance().StartWatchDeviceChange(&listener_, {"metaMgr"});
    if (status != AppDistributedKv::Status::SUCCESS) {
        ZLOGW("register failed.");
    }
    ZLOGI("register meta device change success.");
    GetInstance().SubscribeMetaKvStore();
}

void KvStoreMetaManager::InitMetaData()
{
    ZLOGI("start.");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGI("get meta failed.");
        return;
    }
    const std::string userId = AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId();
    auto metaKey = GetMetaKey(AccountDelegate::MAIN_DEVICE_ACCOUNT_ID, "default", META_DB_APP_ID,
                              Constant::SERVICE_META_DB_NAME);
    struct KvStoreMetaData metaData {
        .appId = META_DB_APP_ID,
        .appType = "default",
        .bundleName = META_DB_APP_ID,
        .dataDir = "default",
        .deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID,
        .deviceId = DeviceKvStoreImpl::GetLocalDeviceId(),
        .isAutoSync = false,
        .isBackup = false,
        .isEncrypt = false,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
        .schema = "",
        .storeId = Constant::SERVICE_META_DB_NAME,
        .userId = userId,
        .uid = -1,
        .version = KvStoreDataService::KVSTORE_META_VERSION,
        .securityLevel = SecurityLevel::NO_LABEL,
    };
    std::string jsonStr = metaData.Marshal();
    std::vector<uint8_t> value(jsonStr.begin(), jsonStr.end());
    if (CheckUpdateServiceMeta(metaKey, UPDATE, value) != Status::SUCCESS) {
        ZLOGW("CheckUpdateServiceMeta database failed.");
    }
    ZLOGI("end.");
}

void KvStoreMetaManager::InitMetaParameter()
{
    ZLOGI("start.");

    bool ret = ForceCreateDirectory(metaDBDirectory_);
    if (!ret) {
        FaultMsg msg = {FaultType::SERVICE_FAULT, "user", __FUNCTION__, Fault::SF_CREATE_DIR};
        Reporter::GetInstance()->ServiceFault()->Report(msg);
        ZLOGE("create directories failed");
        return;
    }
    // change mode for directories to 0755, and for files to 0600.
    DirectoryUtils::ChangeModeDirOnly(metaDBDirectory_, Constant::DEFAULT_MODE_DIR);
    DirectoryUtils::ChangeModeFileOnly(metaDBDirectory_, Constant::DEFAULT_MODE_FILE);

    DistributedDB::KvStoreConfig kvStoreConfig {metaDBDirectory_};
    kvStoreDelegateManager_.SetKvStoreConfig(kvStoreConfig);

    vecRootKeyAlias_ = std::vector<uint8_t>(ROOT_KEY_ALIAS, ROOT_KEY_ALIAS + strlen(ROOT_KEY_ALIAS));
    vecNonce_ = std::vector<uint8_t>(HKS_BLOB_TYPE_NONCE, HKS_BLOB_TYPE_NONCE + strlen(HKS_BLOB_TYPE_NONCE));
    vecAad_ = std::vector<uint8_t>(HKS_BLOB_TYPE_AAD, HKS_BLOB_TYPE_AAD + strlen(HKS_BLOB_TYPE_AAD));
}

const KvStoreMetaManager::NbDelegate &KvStoreMetaManager::GetMetaKvStore()
{
    if (metaDelegate_ == nullptr) {
        metaDelegate_ = CreateMetaKvStore();
    }
    return metaDelegate_;
}

KvStoreMetaManager::NbDelegate KvStoreMetaManager::CreateMetaKvStore()
{
    DistributedDB::DBStatus dbStatusTmp = DistributedDB::DBStatus::NOT_SUPPORT;
    DistributedDB::KvStoreNbDelegate::Option option;
    option.createIfNecessary = true;
    option.isMemoryDb = false;
    option.createDirByStoreIdOnly = true;
    option.isEncryptedDb = false;
    DistributedDB::KvStoreNbDelegate *kvStoreNbDelegatePtr = nullptr;
    kvStoreDelegateManager_.GetKvStore(
        Constant::SERVICE_META_DB_NAME, option,
        [&kvStoreNbDelegatePtr, &dbStatusTmp](DistributedDB::DBStatus dbStatus,
                                              DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate) {
            kvStoreNbDelegatePtr = kvStoreNbDelegate;
            dbStatusTmp = dbStatus;
        });

    if (dbStatusTmp != DistributedDB::DBStatus::OK) {
        ZLOGE("GetKvStore return error status: %d", static_cast<int>(dbStatusTmp));
        return nullptr;
    }
    auto release = [this](DistributedDB::KvStoreNbDelegate *delegate) {
        if (delegate == nullptr) {
            return;
        }

        auto result = kvStoreDelegateManager_.CloseKvStore(delegate);
        if (result != DistributedDB::DBStatus::OK) {
            ZLOGE("CloseMetaKvStore return error status: %d", static_cast<int>(result));
        }
    };
    return NbDelegate(kvStoreNbDelegatePtr, release);
}

std::vector<uint8_t> KvStoreMetaManager::GetMetaKey(const std::string &deviceAccountId,
                                                    const std::string &groupId, const std::string &bundleName,
                                                    const std::string &storeId, const std::string &key)
{
    std::string originKey;
    if (key.empty()) {
        originKey = DeviceKvStoreImpl::GetLocalDeviceId() + Constant::KEY_SEPARATOR +
                    deviceAccountId + Constant::KEY_SEPARATOR +
                    groupId + Constant::KEY_SEPARATOR +
                    bundleName + Constant::KEY_SEPARATOR +
                    storeId;
        return KvStoreMetaRow::GetKeyFor(originKey);
    }

    originKey = deviceAccountId + Constant::KEY_SEPARATOR +
                groupId + Constant::KEY_SEPARATOR +
                bundleName + Constant::KEY_SEPARATOR +
                storeId + Constant::KEY_SEPARATOR +
                key;
    return SecretMetaRow::GetKeyFor(originKey);
}

std::string KvStoreMetaManager::GetSecretKeyFile(const std::string &deviceAccountId, const std::string &appId,
                                                 const std::string &storeId)
{
    std::string hashedStoreId;
    DistributedDB::DBStatus result = DistributedDB::KvStoreDelegateManager::GetDatabaseDir(storeId, hashedStoreId);
    if (DistributedDB::OK != result) {
        ZLOGE("get data base directory by kvstore store id failed, result = %d.", result);
        return "";
    }
    return Constant::ROOT_PATH_DE + "/" + Constant::SERVICE_NAME + "/" +
           deviceAccountId + "/" + Constant::GetDefaultHarmonyAccountName() + "/" +
           appId + "/" + hashedStoreId + ".mul.key";
}

std::string KvStoreMetaManager::GetSecretSingleKeyFile(const std::string &deviceAccountId, const std::string &appId,
                                                       const std::string &storeId)
{
    std::string hashedStoreId;
    DistributedDB::DBStatus result = DistributedDB::KvStoreDelegateManager::GetDatabaseDir(storeId, hashedStoreId);
    if (DistributedDB::OK != result) {
        ZLOGE("get data base directory by kvstore store id failed, result = %d.", result);
        return "";
    }
    return Constant::ROOT_PATH_DE + "/" + Constant::SERVICE_NAME + "/" +
           deviceAccountId + "/" + Constant::GetDefaultHarmonyAccountName() + "/" +
           appId + "/" + hashedStoreId + ".sig.key";
}

Status KvStoreMetaManager::CheckUpdateServiceMeta(const std::vector<uint8_t> &metaKey, FLAG flag,
                                                  const std::vector<uint8_t> &val)
{
    ZLOGD("begin.");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    KvStoreAppManager::PathType pathType = KvStoreAppManager::PATH_CE;
    DistributedDB::Key dbKey = metaKey;
    DistributedDB::Value dbValue = val;
    DistributedDB::DBStatus dbStatus;
    DistributedDB::CipherPassword dbPassword;
    const std::string deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    switch (flag) {
        case UPDATE:
            dbStatus = metaDelegate->Put(dbKey, dbValue);
            metaDelegate->Export(BackupHandler::GetBackupPath(deviceAccountId, pathType), dbPassword);
            break;
        case DELETE:
            dbStatus = metaDelegate->Delete(dbKey);
            metaDelegate->Export(BackupHandler::GetBackupPath(deviceAccountId, pathType), dbPassword);
            break;
        case CHECK_EXIST:
            dbStatus = metaDelegate->Get(dbKey, dbValue);
            break;
        case UPDATE_LOCAL:
            dbStatus = metaDelegate->PutLocal(dbKey, dbValue);
            metaDelegate->Export(BackupHandler::GetBackupPath(deviceAccountId, pathType), dbPassword);
            break;
        case DELETE_LOCAL:
            dbStatus = metaDelegate->DeleteLocal(dbKey);
            metaDelegate->Export(BackupHandler::GetBackupPath(deviceAccountId, pathType), dbPassword);
            break;
        case CHECK_EXIST_LOCAL:
            dbStatus = metaDelegate->GetLocal(dbKey, dbValue);
            break;
        default:
            break;
    }
    ZLOGI("Flag: %d status: %d", static_cast<int>(flag), static_cast<int>(dbStatus));
    SyncMeta();
    return (dbStatus != DistributedDB::DBStatus::OK) ? Status::DB_ERROR : Status::SUCCESS;
}

Status KvStoreMetaManager::GenerateRootKey()
{
    ZLOGI("GenerateRootKey.");
    struct HksBlob rootKeyName = { vecRootKeyAlias_.size(), &(vecRootKeyAlias_[0]) };
    struct HksParamSet *paramSet = nullptr;
    int32_t ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksInitParamSet() failed with error %{public}d", ret);
        return Status::ERROR;
    }

    struct HksParam genKeyParams[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    };

    ret = HksAddParams(paramSet, genKeyParams, sizeof(genKeyParams) / sizeof(genKeyParams[0]));
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return Status::ERROR;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return Status::ERROR;
    }

    ret = HksGenerateKey(&rootKeyName, paramSet, NULL);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksGenerateKey failed with error %{public}d", ret);
        HksFreeParamSet(&paramSet);
        return Status::ERROR;
    }
    HksFreeParamSet(&paramSet);

    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    DistributedDB::Key dbKey = std::vector<uint8_t>(Constant::ROOT_KEY_GENERATED.begin(),
        Constant::ROOT_KEY_GENERATED.end());
    if (metaDelegate->PutLocal(dbKey, {ROOT_KEY_ALIAS, ROOT_KEY_ALIAS + strlen(ROOT_KEY_ALIAS)}) !=
        DistributedDB::DBStatus::OK) {
        return Status::ERROR;
    }
    ZLOGI("GenerateRootKey Succeed.");
    return Status::SUCCESS;
}

Status KvStoreMetaManager::CheckRootKeyExist()
{
    ZLOGI("GenerateRootKey.");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    DistributedDB::Key dbKey = std::vector<uint8_t>(Constant::ROOT_KEY_GENERATED.begin(),
                                                    Constant::ROOT_KEY_GENERATED.end());
    DistributedDB::Value dbValue;
    if (metaDelegate->GetLocal(dbKey, dbValue) == DistributedDB::DBStatus::OK) {
        ZLOGI("root key exist.");
        return Status::SUCCESS;
    }
    return Status::ERROR;
}

std::vector<uint8_t> KvStoreMetaManager::EncryptWorkKey(const std::vector<uint8_t> &key)
{
    struct HksBlob blobAad = { vecAad_.size(), &(vecAad_[0]) };
    struct HksBlob blobNonce = { vecNonce_.size(), &(vecNonce_[0]) };
    struct HksBlob rootKeyName = { vecRootKeyAlias_.size(), &(vecRootKeyAlias_[0]) };
    struct HksBlob plainKey = { key.size(), const_cast<uint8_t *>(&(key[0])) };
    uint8_t cipherBuf[256] = { 0 };
    struct HksBlob encryptedKey = { sizeof(cipherBuf), cipherBuf };
    std::vector<uint8_t> encryptedKeyVec;
    struct HksParamSet *encryptParamSet = nullptr;
    int32_t ret = HksInitParamSet(&encryptParamSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksInitParamSet() failed with error %{public}d", ret);
        return {};
    }
    struct HksParam encryptParams[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_NONCE, .blob = blobNonce },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = blobAad },
    };
    ret = HksAddParams(encryptParamSet, encryptParams, sizeof(encryptParams) / sizeof(encryptParams[0]));
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&encryptParamSet);
        return {};
    }

    ret = HksBuildParamSet(&encryptParamSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&encryptParamSet);
        return {};
    }

    ret = HksEncrypt(&rootKeyName, encryptParamSet, &plainKey, &encryptedKey);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksEncrypt failed with error %{public}d", ret);
        HksFreeParamSet(&encryptParamSet);
        return {};
    }
    (void)HksFreeParamSet(&encryptParamSet);

    for (uint32_t i = 0; i < encryptedKey.size; i++) {
        encryptedKeyVec.push_back(encryptedKey.data[i]);
    }
    return encryptedKeyVec;
}

bool KvStoreMetaManager::DecryptWorkKey(const std::vector<uint8_t> &encryptedKey, std::vector<uint8_t> &key)
{
    struct HksBlob blobAad = { vecAad_.size(), &(vecAad_[0]) };
    struct HksBlob blobNonce = { vecNonce_.size(), &(vecNonce_[0]) };
    struct HksBlob rootKeyName = { vecRootKeyAlias_.size(), &(vecRootKeyAlias_[0]) };
    struct HksBlob encryptedKeyBlob = { encryptedKey.size(), const_cast<uint8_t *>(&(encryptedKey[0])) };
    uint8_t plainBuf[256] = { 0 };
    struct HksBlob plainKeyBlob = { sizeof(plainBuf), plainBuf };
    struct HksParamSet *decryptParamSet = nullptr;
    int32_t ret = HksInitParamSet(&decryptParamSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksInitParamSet() failed with error %{public}d", ret);
        return false;
    }
    struct HksParam decryptParams[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_NONCE, .blob = blobNonce },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = blobAad },
    };
    ret = HksAddParams(decryptParamSet, decryptParams, sizeof(decryptParams) / sizeof(decryptParams[0]));
    if (ret !=  HKS_SUCCESS) {
        ZLOGE("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&decryptParamSet);
        return false;
    }

    ret = HksBuildParamSet(&decryptParamSet);
    if (ret != HKS_SUCCESS) {
        ZLOGE("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&decryptParamSet);
        return false;
    }

    ret = HksDecrypt(&rootKeyName, decryptParamSet, &encryptedKeyBlob, &plainKeyBlob);
    if (ret != HKS_SUCCESS) {
        ZLOGW("HksDecrypt failed with error %{public}d", ret);
        HksFreeParamSet(&decryptParamSet);
        return false;
    }
    (void)HksFreeParamSet(&decryptParamSet);

    for (uint32_t i = 0; i < plainKeyBlob.size; i++) {
        key.push_back(plainKeyBlob.data[i]);
    }
    return true;
}

Status KvStoreMetaManager::WriteSecretKeyToMeta(const std::vector<uint8_t> &metaKey, const std::vector<uint8_t> &key)
{
    ZLOGD("start");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    SecretKeyMetaData secretKey;
    secretKey.kvStoreType = KvStoreType::DEVICE_COLLABORATION;
    secretKey.timeValue = TransferTypeToByteArray<time_t>(system_clock::to_time_t(system_clock::now()));
    secretKey.secretKey = EncryptWorkKey(key);
    if (secretKey.secretKey.empty()) {
        ZLOGE("encrypt work key error.");
        return Status::CRYPT_ERROR;
    }

    DistributedDB::DBStatus dbStatus = metaDelegate->PutLocal(metaKey, secretKey);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("end with %d", static_cast<int>(dbStatus));
        return Status::DB_ERROR;
    } else {
        ZLOGD("normal end");
        return Status::SUCCESS;
    }
}

Status KvStoreMetaManager::WriteSecretKeyToFile(const std::string &secretKeyFile, const std::vector<uint8_t> &key)
{
    ZLOGD("start");
    std::vector<uint8_t> secretKey = EncryptWorkKey(key);
    if (secretKey.empty()) {
        ZLOGW("encrypt work key error.");
        return Status::CRYPT_ERROR;
    }
    std::string dbDir = secretKeyFile.substr(0, secretKeyFile.find_last_of('/'));
    if (!ForceCreateDirectory(dbDir)) {
        return Status::ERROR;
    }

    std::vector<uint8_t> secretKeyInByte =
        TransferTypeToByteArray<time_t>(system_clock::to_time_t(system_clock::now()));
    std::vector<char> secretKeyInChar;
    secretKeyInChar.insert(secretKeyInChar.end(), secretKeyInByte.begin(), secretKeyInByte.end());
    secretKeyInChar.insert(secretKeyInChar.end(), secretKey.begin(), secretKey.end());
    if (SaveBufferToFile(secretKeyFile, secretKeyInChar)) {
        ZLOGD("normal end");
        return Status::SUCCESS;
    }
    ZLOGW("failure end");
    return Status::ERROR;
}

Status KvStoreMetaManager::RemoveSecretKey(const std::string &deviceAccountId, const std::string &bundleName,
                                           const std::string &storeId)
{
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    Status status = Status::SUCCESS;
    DistributedDB::Key secretDbKey = GetMetaKey(deviceAccountId, "default", bundleName, storeId, "KEY");
    DistributedDB::Key secretSingleDbKey = GetMetaKey(deviceAccountId, "default", bundleName, storeId, "SINGLE_KEY");
    DistributedDB::DBStatus dbStatus = metaDelegate->DeleteLocal(secretDbKey);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delete secretDbKey fail Status %d", static_cast<int>(dbStatus));
        status = Status::DB_ERROR;
    }
    dbStatus = metaDelegate->DeleteLocal(secretSingleDbKey);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("delete secretSingleDbKey fail Status %d", static_cast<int>(dbStatus));
        status = Status::DB_ERROR;
    }

    std::string secretKeyFile = GetSecretKeyFile(deviceAccountId, bundleName, storeId);
    bool rmFile = RemoveFile(secretKeyFile);
    if (!rmFile) {
        ZLOGW("remove secretKeyFile fail.");
        status = Status::DB_ERROR;
    }
    secretKeyFile = GetSecretSingleKeyFile(deviceAccountId, bundleName, storeId);
    rmFile = RemoveFile(secretKeyFile);
    if (!rmFile) {
        ZLOGW("remove secretKeyFile Single fail.");
        status = Status::DB_ERROR;
    }
    return status;
}

Status KvStoreMetaManager::GetSecretKeyFromMeta(const std::vector<uint8_t> &metaSecretKey, std::vector<uint8_t> &key,
                                                bool &outdated)
{
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    DistributedDB::Key dbKey = metaSecretKey;
    DistributedDB::Value dbValue;
    DistributedDB::DBStatus dbStatus = metaDelegate->GetLocal(dbKey, dbValue);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        return Status::DB_ERROR;
    }
    std::string jsonStr(dbValue.begin(), dbValue.end());
    json jsonObj = json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        ZLOGE("parse json error");
        return Status::ERROR;
    }
    SecretKeyMetaData sKeyValue(jsonObj);
    time_t createTime = TransferByteArrayToType<time_t>(sKeyValue.timeValue);
    DecryptWorkKey(sKeyValue.secretKey, key);
    system_clock::time_point createTimeChrono = system_clock::from_time_t(createTime);
    outdated = ((createTimeChrono + hours(HOURS_PER_YEAR)) < system_clock::now()); // secretKey valid for 1 year.
    return Status::SUCCESS;
}

Status KvStoreMetaManager::RecoverSecretKeyFromFile(const std::string &secretKeyFile,
                                                    const std::vector<uint8_t> &metaSecretKey,
                                                    std::vector<uint8_t> &key, bool &outdated)
{
    std::vector<char> fileBuffer;
    if (!LoadBufferFromFile(secretKeyFile, fileBuffer)) {
        return Status::ERROR;
    }
    if (fileBuffer.size() < sizeof(time_t) / sizeof(uint8_t) + KEY_SIZE) {
        return Status::ERROR;
    }
    std::vector<uint8_t> timeVec;
    auto iter = fileBuffer.begin();
    for (int i = 0; i < static_cast<int>(sizeof(time_t) / sizeof(uint8_t)); i++) {
        timeVec.push_back(*iter);
        iter++;
    }
    time_t createTime = TransferByteArrayToType<time_t>(timeVec);
    SecretKeyMetaData secretKey;
    secretKey.secretKey.insert(secretKey.secretKey.end(), iter, fileBuffer.end());
    if (!DecryptWorkKey(secretKey.secretKey, key)) {
        return Status::ERROR;
    }
    system_clock::time_point createTimeChrono = system_clock::from_time_t(createTime);
    outdated = ((createTimeChrono + hours(HOURS_PER_YEAR)) < system_clock::now()); // secretKey valid for 1 year.

    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }

    secretKey.timeValue = TransferTypeToByteArray<time_t>(createTime);
    secretKey.kvStoreType = KvStoreType::DEVICE_COLLABORATION;

    DistributedDB::DBStatus dbStatus = metaDelegate->PutLocal(metaSecretKey, secretKey);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("put work key failed.");
        return Status::DB_ERROR;
    }
    return Status::SUCCESS;
}

void KvStoreMetaManager::ReKey(const std::string &deviceAccountId, const std::string &bundleName,
                               const std::string &storeId, sptr<IKvStoreImpl> store)
{
    if (store == nullptr) {
        return;
    }
    KvStoreImpl *kvStoreimpl = static_cast<KvStoreImpl *>(store.GetRefPtr());
    std::vector<uint8_t> key;
    CryptoUtils::GetRandomKey(KEY_SIZE, key);
    WriteSecretKeyToMeta(GetMetaKey(deviceAccountId, "default", bundleName, storeId, "KEY"), key);
    Status status = kvStoreimpl->ReKey(key);
    if (status == Status::SUCCESS) {
        WriteSecretKeyToFile(GetSecretKeyFile(deviceAccountId, bundleName, storeId), key);
    }
    key.assign(key.size(), 0);
}

void KvStoreMetaManager::ReKey(const std::string &deviceAccountId, const std::string &bundleName,
                               const std::string &storeId, sptr<ISingleKvStore> store)
{
    if (store == nullptr) {
        return;
    }
    SingleKvStoreImpl *kvStoreImpl = static_cast<SingleKvStoreImpl *>(store.GetRefPtr());
    std::vector<uint8_t> key;
    CryptoUtils::GetRandomKey(KEY_SIZE, key);
    WriteSecretKeyToMeta(GetMetaKey(deviceAccountId, "default", bundleName, storeId, "SINGLE_KEY"), key);
    Status status = kvStoreImpl->ReKey(key);
    if (status == Status::SUCCESS) {
        WriteSecretKeyToFile(GetSecretSingleKeyFile(deviceAccountId, bundleName, storeId), key);
    }
    key.assign(key.size(), 0);
}

// StrategyMetaData###deviceId###deviceAccountID###${groupId}###bundleName###storeId
void KvStoreMetaManager::GetStrategyMetaKey(const StrategyMeta &params, std::string &retVal)
{
    std::vector<std::string> keys = {STRATEGY_META_PREFIX, params.devId, params.devAccId, params.grpId,
                                     params.bundleName, params.storeId};
    ConcatWithSharps(keys, retVal);
}

void KvStoreMetaManager::ConcatWithSharps(const std::vector<std::string> &params, std::string &retVal)
{
    int32_t len = static_cast<int32_t>(params.size());
    for (int32_t i = 0; i < len; i++) {
        retVal.append(params.at(i));
        if (i != (len - 1)) {
            retVal.append(Constant::KEY_SEPARATOR);
        }
    }
}

Status KvStoreMetaManager::SaveStrategyMetaEnable(const std::string &key, bool enable)
{
    ZLOGD("begin");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        return Status::ERROR;
    }
    auto dbkey = std::vector<uint8_t>(key.begin(), key.end());
    std::vector<uint8_t> values;
    auto dbStatus = metaDelegate->Get(dbkey, values);
    if (dbStatus == DistributedDB::DBStatus::NOT_FOUND) {
        json j;
        j[CAPABILITY_ENABLED] = enable;
        std::string json = j.dump();
        if (metaDelegate->Put(dbkey, std::vector<uint8_t>(json.begin(), json.end())) != DistributedDB::OK) {
            ZLOGE("save failed.");
            return Status::DB_ERROR;
        }
        ZLOGD("save end");
    } else if (dbStatus == DistributedDB::DBStatus::OK) {
        std::string jsonStr(values.begin(), values.end());
        auto jsonObj = json::parse(jsonStr, nullptr, false);
        if (jsonObj.is_discarded()) {
            ZLOGE("invalid json.");
            return Status::ERROR;
        }
        jsonObj[CAPABILITY_ENABLED] = enable;
        std::string json = jsonObj.dump();
        if (metaDelegate->Put(dbkey, std::vector<uint8_t>(json.begin(), json.end())) != DistributedDB::OK) {
            ZLOGE("save failed.");
            return Status::DB_ERROR;
        }
        ZLOGD("update end");
    } else {
        ZLOGE("failed.");
        return Status::DB_ERROR;
    }
    SyncMeta();
    return Status::SUCCESS;
}

Status KvStoreMetaManager::SaveStrategyMetaLabels(const std::string &key,
                                                  const std::vector<std::string> &localLabels,
                                                  const std::vector<std::string> &remoteSupportLabels)
{
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        return Status::ERROR;
    }
    auto dbkey = std::vector<uint8_t>(key.begin(), key.end());
    std::vector<uint8_t> values;
    auto dbStatus = metaDelegate->Get(dbkey, values);
    if (dbStatus == DistributedDB::DBStatus::NOT_FOUND) {
        json j;
        j[CAPABILITY_RANGE][LOCAL_LABEL] = localLabels;
        j[CAPABILITY_RANGE][REMOTE_LABEL] = remoteSupportLabels;
        std::string metaJson = j.dump();
        if (metaDelegate->Put(dbkey, std::vector<uint8_t>(metaJson.begin(), metaJson.end())) != DistributedDB::OK) {
            ZLOGE("save failed.");
            return Status::DB_ERROR;
        }
    } else if (dbStatus == DistributedDB::DBStatus::OK) {
        std::string jsonStr(values.begin(), values.end());
        auto j = json::parse(jsonStr, nullptr, false);
        if (j.is_discarded()) {
            return Status::ERROR;
        }
        j[CAPABILITY_RANGE][LOCAL_LABEL] = localLabels;
        j[CAPABILITY_RANGE][REMOTE_LABEL] = remoteSupportLabels;
        std::string metaJson = j.dump();
        if (metaDelegate->Put(dbkey, std::vector<uint8_t>(metaJson.begin(), metaJson.end())) != DistributedDB::OK) {
            ZLOGE("save failed.");
            return Status::DB_ERROR;
        }
    } else {
        ZLOGE("failed.");
        return Status::DB_ERROR;
    }
    SyncMeta();
    return Status::SUCCESS;
}

Status KvStoreMetaManager::DeleteStrategyMeta(const std::string &bundleName, const std::string &storeId)
{
    ZLOGI("start");
    std::string key;
    std::string devId = DeviceKvStoreImpl::GetLocalDeviceId();
    if (devId.empty()) {
        ZLOGE("get device id empty.");
        return Status::ERROR;
    }
    std::string devAccId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    StrategyMeta params = {devId, devAccId, Constant::DEFAULT_GROUP_ID, bundleName, storeId};
    GetStrategyMetaKey(params, key);

    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        return Status::ERROR;
    }
    auto dbkey = std::vector<uint8_t>(key.begin(), key.end());
    auto dbStatus = metaDelegate->Delete(dbkey);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("failed.");
        return Status::DB_ERROR;
    }
    return Status::SUCCESS;
}

void KvStoreMetaManager::SyncMeta()
{
    std::vector<std::string> devs;
    auto deviceList = KvStoreUtils::GetProviderInstance().GetDeviceList();
    for (auto const &dev : deviceList) {
        devs.push_back(dev.deviceId);
    }

    if (devs.empty()) {
        ZLOGW("meta db sync fail, devices is empty.");
        return;
    }

    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGW("meta db sync failed.");
        return;
    }
    auto onComplete = [this](const std::map<std::string, DistributedDB::DBStatus> &) {
        ZLOGD("meta db sync complete.");
        cv_.notify_all();
        ZLOGD("meta db sync complete end.");
    };
    auto dbStatus = metaDelegate->Sync(devs, DistributedDB::SyncMode::SYNC_MODE_PUSH_PULL, onComplete);
    if (dbStatus != DistributedDB::OK) {
        ZLOGW("meta db sync error %d.", dbStatus);
    }
}

void KvStoreMetaManager::SubscribeMetaKvStore()
{
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGW("register meta observer failed.");
        return;
    }

    int mode = DistributedDB::OBSERVER_CHANGES_NATIVE;
    auto dbStatus = metaDelegate->RegisterObserver(DistributedDB::Key(), mode, &metaObserver_);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGW("register meta observer failed :%d.", dbStatus);
    }
}

Status KvStoreMetaManager::CheckSyncPermission(const std::string &userId, const std::string &appId,
                                               const std::string &storeId, uint8_t flag, const std::string &deviceId)
{
    std::string devId = DeviceKvStoreImpl::GetLocalDeviceId();
    if (devId.empty()) {
        ZLOGE("get device id empty.");
        return Status::ERROR;
    }
    KvStoreMetaData val;
    auto queryStatus = QueryKvStoreMetaDataByDeviceIdAndAppId(devId, appId, val);
    if (queryStatus != Status::SUCCESS) {
        ZLOGE("get kvstore by deviceId and appId empty.");
        return Status::ERROR;
    }

    std::string devAccId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    StrategyMeta params = {devId, devAccId, Constant::DEFAULT_GROUP_ID, val.bundleName, storeId};
    std::string localKey;
    GetStrategyMetaKey(params, localKey);
    if (localKey.empty()) {
        ZLOGE("get key empty.");
        return Status::ERROR;
    }

    std::string remoteKey;
    params.devId = deviceId;
    GetStrategyMetaKey(params, remoteKey);
    if (remoteKey.empty()) {
        ZLOGE("get key empty.");
        return Status::ERROR;
    }

    std::map<std::string, std::vector<std::string>> localStrategies;
    std::map<std::string, std::vector<std::string>> remoteStrategies;
    GetStategyMeta(localKey, localStrategies);
    GetStategyMeta(remoteKey, remoteStrategies);
    if (localStrategies.empty() || remoteStrategies.empty()) {
        ZLOGD("no range, sync permission success.");
        return Status::SUCCESS;
    }

    auto localSupportRemotes = localStrategies.find(REMOTE_LABEL);
    auto remoteSupportLocals = remoteStrategies.find(LOCAL_LABEL);
    if (localSupportRemotes != localStrategies.end() && remoteSupportLocals != remoteStrategies.end()) {
        std::vector<std::string> lremotes = localSupportRemotes->second;
        for (auto const &lremote : lremotes) {
            std::vector<std::string> rlocals = remoteSupportLocals->second;
            if (std::find(rlocals.begin(), rlocals.end(), lremote) != rlocals.end()) {
                ZLOGD("find range, sync permission success.");
                return Status::SUCCESS;
            }
        }
    }
    ZLOGD("check strategy failed, sync permission fail.");
    return Status::ERROR;
}

Status KvStoreMetaManager::GetStategyMeta(const std::string &key,
                                          std::map<std::string, std::vector<std::string>> &strategies)
{
    ZLOGD("get meta key:%s.", key.c_str());
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGW("get delegate error.");
        return Status::ERROR;
    }

    DistributedDB::Value values;
    auto dbStatus = metaDelegate->Get(DistributedDB::Key(key.begin(), key.end()), values);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGW("get meta error %d.", dbStatus);
        return Status::DB_ERROR;
    }

    std::string jsonStr(values.begin(), values.end());
    auto jsonObj = json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        jsonObj = json::parse(jsonStr.substr(1), nullptr, false); // 1 drop for a
        if (jsonObj.is_discarded()) {
            ZLOGW("get meta parse error.");
            return Status::ERROR;
        }
    }

    auto range = jsonObj.find(CAPABILITY_RANGE);
    if (range == jsonObj.end()) {
        ZLOGW("get meta parse no range.");
        return Status::ERROR;
    }

    auto local = range->find(LOCAL_LABEL);
    if (local != range->end()) {
        json obj = *local;
        if (obj.is_array()) {
            std::vector<std::string> v;
            obj.get_to(v);
            strategies.insert({LOCAL_LABEL, v});
        }
    }
    auto remote = range->find(REMOTE_LABEL);
    if (remote != range->end()) {
        json obj = *remote;
        if (obj.is_array()) {
            std::vector<std::string> v;
            obj.get_to(v);
            strategies.insert({REMOTE_LABEL, v});
        }
    }
    return Status::SUCCESS;
}

KvStoreMetaManager::KvStoreMetaObserver::~KvStoreMetaObserver()
{
    ZLOGW("meta observer destruct.");
}

void KvStoreMetaManager::KvStoreMetaObserver::OnChange(const DistributedDB::KvStoreChangedData &data)
{
    ZLOGD("on data change.");
    if (notify_ != nullptr) {
        auto &updated = data.GetEntriesUpdated();
        for (const auto &entry : updated) {
            std::string key(entry.key.begin(), entry.key.end());
            if (key.find(KvStoreMetaRow::KEY_PREFIX) != 0) {
                continue;
            }

            KvStoreMetaData metaData;
            std::string json(entry.value.begin(), entry.value.end());
            metaData.Unmarshal(Serializable::ToJson(json));
            ZLOGD("meta data info appType:%s, storeId:%s isDirty:%d",
                  metaData.appType.c_str(), metaData.storeId.c_str(), metaData.isDirty);
            if (!metaData.isDirty || metaData.appType != HARMONY_APP) {
                continue;
            }
            ZLOGI("dirty kv store. storeId:%s", metaData.storeId.c_str());
            notify_(metaData);
        }
    }
    KvStoreMetaManager::GetInstance().SyncMeta();
}

void KvStoreMetaManager::MetaDeviceChangeListenerImpl::OnDeviceChanged(
    const AppDistributedKv::DeviceInfo &info, const AppDistributedKv::DeviceChangeType &type) const
{
    if (type == AppDistributedKv::DeviceChangeType::DEVICE_OFFLINE) {
        ZLOGD("offline ignore.");
        return;
    }

    ZLOGD("begin to sync.");
    KvStoreMetaManager::GetInstance().SyncMeta();
    ZLOGD("end.");
}

AppDistributedKv::ChangeLevelType KvStoreMetaManager::MetaDeviceChangeListenerImpl::GetChangeLevelType() const
{
    return AppDistributedKv::ChangeLevelType::HIGH;
}

void KvStoreMetaManager::ToJson(json &j, const KvStoreMetaData &k)
{
    j = json(k.Marshal());
}

void KvStoreMetaManager::FromJson(const json &j, KvStoreMetaData &k)
{
    k.Unmarshal(j);
}

Status KvStoreMetaManager::QueryKvStoreMetaDataByDeviceIdAndAppId(const std::string &devId, const std::string &appId,
                                                                  KvStoreMetaData &val)
{
    ZLOGD("query meta start.");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGW("get delegate error.");
        return Status::ERROR;
    }
    std::string dbPrefixKey;
    std::string prefix = KvStoreMetaRow::KEY_PREFIX;
    ConcatWithSharps({prefix, devId}, dbPrefixKey);
    std::vector<DistributedDB::Entry> values;
    auto status = metaDelegate->GetEntries(DistributedDB::Key(dbPrefixKey.begin(), dbPrefixKey.end()), values);
    if (status != DistributedDB::DBStatus::OK) {
        status = metaDelegate->GetEntries(DistributedDB::Key(prefix.begin(), prefix.end()), values);
        if (status != DistributedDB::DBStatus::OK) {
            ZLOGW("query db failed key:%s, ret:%d.", dbPrefixKey.c_str(), static_cast<int>(status));
            return Status::ERROR;
        }
    }

    for (auto const &entry : values) {
        std::string str(entry.value.begin(), entry.value.end());
        json j = Serializable::ToJson(str);
        val.Unmarshal(j);
        if (val.appId == appId) {
            ZLOGD("query meta success.");
            return Status::SUCCESS;
        }
    }

    ZLOGW("find meta failed id:%s.", appId.c_str());
    return Status::ERROR;
}

Status KvStoreMetaManager::GetKvStoreMeta(const std::vector<uint8_t> &metaKey, KvStoreMetaData &metaData)
{
    ZLOGD("begin.");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        ZLOGE("GetMetaKvStore return nullptr.");
        return Status::DB_ERROR;
    }
    DistributedDB::Value dbValue;
    DistributedDB::DBStatus dbStatus = metaDelegate->Get(metaKey, dbValue);
    ZLOGI("status: %d", static_cast<int>(dbStatus));
    if (dbStatus == DistributedDB::DBStatus::NOT_FOUND) {
        ZLOGI("key not found.");
        return Status::KEY_NOT_FOUND;
    }
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("GetKvStoreMeta failed.");
        return Status::DB_ERROR;
    }

    std::string jsonStr(dbValue.begin(), dbValue.end());
    metaData.Unmarshal(Serializable::ToJson(jsonStr));
    return Status::SUCCESS;
}

std::string KvStoreMetaData::Marshal() const
{
    json jval = {
        {DEVICE_ID, deviceId},
        {USER_ID, userId},
        {APP_ID, appId},
        {STORE_ID, storeId},
        {BUNDLE_NAME, bundleName},
        {KVSTORE_TYPE, kvStoreType},
        {ENCRYPT, isEncrypt},
        {BACKUP, isBackup},
        {AUTO_SYNC, isAutoSync},
        {SCHEMA, schema},
        {DATA_DIR, dataDir}, // Reserved for kvstore data storage directory.
        {APP_TYPE, appType}, // Reserved for the APP type which used kvstore.
        {DEVICE_ACCOUNT_ID, deviceAccountId},
        {UID, uid},
        {VERSION, version},
        {SECURITY_LEVEL, securityLevel},
        {DIRTY_KEY, isDirty},
    };
    return jval.dump();
}

json Serializable::ToJson(const std::string &jsonStr)
{
    json jsonObj = json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        // if the string size is less than 1, means the string is invalid.
        if (jsonStr.empty()) {
            ZLOGE("empty jsonStr, error.");
            return {};
        }
        jsonObj = json::parse(jsonStr.substr(1), nullptr, false); // drop first char to adapt A's value;
        if (jsonObj.is_discarded()) {
            ZLOGE("parse jsonStr, error.");
            return {};
        }
    }
    return jsonObj;
}

void KvStoreMetaData::Unmarshal(const nlohmann::json &jObject)
{
    kvStoreType = Serializable::GetVal<KvStoreType>(jObject, KVSTORE_TYPE, json::value_t::number_unsigned, kvStoreType);
    isBackup = Serializable::GetVal<bool>(jObject, BACKUP, json::value_t::boolean, isBackup);
    isEncrypt = Serializable::GetVal<bool>(jObject, ENCRYPT, json::value_t::boolean, isEncrypt);
    isAutoSync = Serializable::GetVal<bool>(jObject, AUTO_SYNC, json::value_t::boolean, isAutoSync);
    appId = Serializable::GetVal<std::string>(jObject, APP_ID, json::value_t::string, appId);
    userId = Serializable::GetVal<std::string>(jObject, USER_ID, json::value_t::string, userId);
    storeId = Serializable::GetVal<std::string>(jObject, STORE_ID, json::value_t::string, storeId);
    bundleName = Serializable::GetVal<std::string>(jObject, BUNDLE_NAME, json::value_t::string, bundleName);
    deviceAccountId = Serializable::GetVal<std::string>(jObject, DEVICE_ACCOUNT_ID, json::value_t::string,
                                                        deviceAccountId);
    dataDir = Serializable::GetVal<std::string>(jObject, DATA_DIR, json::value_t::string, dataDir);
    appType = Serializable::GetVal<std::string>(jObject, APP_TYPE, json::value_t::string, appType);
    deviceId = Serializable::GetVal<std::string>(jObject, DEVICE_ID, json::value_t::string, deviceId);
    schema = Serializable::GetVal<std::string>(jObject, SCHEMA, json::value_t::string, schema);
    uid = Serializable::GetVal<int32_t>(jObject, UID, json::value_t::number_unsigned, uid);
    version = Serializable::GetVal<uint32_t>(jObject, VERSION, json::value_t::number_unsigned, version);
    securityLevel = Serializable::GetVal<uint32_t>(jObject, SECURITY_LEVEL, json::value_t::number_unsigned,
                                                   securityLevel);
    isDirty = Serializable::GetVal<uint32_t>(jObject, DIRTY_KEY, json::value_t::boolean, isDirty);
}

bool KvStoreMetaData::CheckChiefValues(const nlohmann::json &jObject)
{
    return Serializable::CheckJsonValue(jObject, KVSTORE_TYPE, json::value_t::number_unsigned) &&
           Serializable::CheckJsonValue(jObject, BACKUP, json::value_t::boolean) &&
           Serializable::CheckJsonValue(jObject, ENCRYPT, json::value_t::boolean) &&
           Serializable::CheckJsonValue(jObject, AUTO_SYNC, json::value_t::boolean) &&
           Serializable::CheckJsonValue(jObject, APP_ID, json::value_t::string) &&
           Serializable::CheckJsonValue(jObject, USER_ID, json::value_t::string) &&
           Serializable::CheckJsonValue(jObject, STORE_ID, json::value_t::string) &&
           Serializable::CheckJsonValue(jObject, BUNDLE_NAME, json::value_t::string) &&
           Serializable::CheckJsonValue(jObject, DEVICE_ACCOUNT_ID, json::value_t::string) &&
           Serializable::CheckJsonValue(jObject, DATA_DIR, json::value_t::string);
}

bool Serializable::CheckJsonValue(const nlohmann::json &j, const std::string &name, json::value_t type)
{
    auto it = j.find(name);
    return it != j.end() && it->type() == type;
}

template<typename T>
T Serializable::GetVal(const json &j, const std::string &name, json::value_t type, const T &val)
{
    auto it = j.find(name);
    if (it != j.end() && it->type() == type) {
        return *it;
    }
    ZLOGW("not found name:%s.", name.c_str());
    return val;
}

std::vector<uint8_t> SecretKeyMetaData::Marshal() const
{
    json jval = {
        {TIME, timeValue},
        {SKEY, secretKey},
        {KVSTORE_TYPE, kvStoreType}
    };
    auto value = jval.dump();
    return std::vector<uint8_t>(value.begin(), value.end());
}

void SecretKeyMetaData::Unmarshal(const nlohmann::json &jObject)
{
    timeValue = Serializable::GetVal<std::vector<uint8_t>>(jObject, TIME, json::value_t::array, timeValue);
    secretKey = Serializable::GetVal<std::vector<uint8_t>>(jObject, SKEY, json::value_t::array, secretKey);
    kvStoreType = Serializable::GetVal<KvStoreType>(jObject, KVSTORE_TYPE, json::value_t::number_unsigned, kvStoreType);
}

bool KvStoreMetaManager::GetFullMetaData(std::map<std::string, MetaData> &entries)
{
    ZLOGI("start");
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        return false;
    }

    std::vector<DistributedDB::Entry> kvStoreMetaEntries;
    const std::string &metaKey = KvStoreMetaRow::KEY_PREFIX;
    DistributedDB::DBStatus dbStatus = metaDelegate->GetEntries({metaKey.begin(), metaKey.end()}, kvStoreMetaEntries);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("Get kvstore meta data entries from metaDB failed, dbStatus: %d.", static_cast<int>(dbStatus));
        return false;
    }

    for (auto const &kvStoreMeta : kvStoreMetaEntries) {
        std::string jsonStr(kvStoreMeta.value.begin(), kvStoreMeta.value.end());
        ZLOGD("kvStoreMetaData get json: %s", jsonStr.c_str());
        auto metaObj = Serializable::ToJson(jsonStr);
        MetaData metaData {0};
        metaData.kvStoreType = MetaData::GetKvStoreType(metaObj);
        if (metaData.kvStoreType == KvStoreType::INVALID_TYPE) {
            ZLOGE("Failed to find KVSTORE_TYPE in jsonStr.");
            continue;
        }

        metaData.kvStoreMetaData.Unmarshal(metaObj);
        std::vector<uint8_t> decryptKey;
        if (metaData.kvStoreMetaData.isEncrypt) {
            ZLOGE("isEncrypt.");
            const std::string keyType = ((metaData.kvStoreType == KvStoreType::SINGLE_VERSION) ? "SINGLE_KEY" : "KEY");
            const std::vector<uint8_t> metaSecretKey = KvStoreMetaManager::GetInstance().GetMetaKey(
                metaData.kvStoreMetaData.deviceAccountId, "default", metaData.kvStoreMetaData.bundleName,
                metaData.kvStoreMetaData.storeId, keyType);
            DistributedDB::Value secretValue;
            metaDelegate->GetLocal(metaSecretKey, secretValue);
            auto secretObj = Serializable::ToJson({secretValue.begin(), secretValue.end()});
            if (secretObj.empty()) {
                ZLOGE("Failed to find SKEY in SecretKeyMetaData.");
                continue;
            }
            metaData.secretKeyMetaData.Unmarshal(secretObj);
            KvStoreMetaManager::GetInstance().DecryptWorkKey(metaData.secretKeyMetaData.secretKey, decryptKey);
        }
        entries.insert({{kvStoreMeta.key.begin(), kvStoreMeta.key.end()}, {metaData}});
        std::fill(decryptKey.begin(), decryptKey.end(), 0);
    }

    return true;
}

bool KvStoreMetaManager::GetKvStoreMetaByType(const std::string &name, const std::string &val,
                                              KvStoreMetaData &metaData)
{
    auto &metaDelegate = GetMetaKvStore();
    if (metaDelegate == nullptr) {
        return false;
    }

    DistributedDB::Key metaKeyPrefix = KvStoreMetaRow::GetKeyFor(KvStoreMetaRow::KEY_PREFIX);
    std::vector<DistributedDB::Entry> metaEntries;
    DistributedDB::DBStatus dbStatus = metaDelegate->GetEntries(metaKeyPrefix, metaEntries);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("Get meta entries from metaDB failed, dbStatus: %d.", static_cast<int>(dbStatus));
        return false;
    }

    for (auto const &metaEntry : metaEntries) {
        std::string jsonStr(metaEntry.value.begin(), metaEntry.value.end());
        ZLOGD("KvStore get json: %s", jsonStr.c_str());
        json jsonObj = json::parse(jsonStr, nullptr, false);
        if (jsonObj.is_discarded()) {
            ZLOGE("parse json error");
            continue;
        }

        std::string metaTypeVal;
        jsonObj[name].get_to(metaTypeVal);
        if (metaTypeVal == val) {
            metaData.Unmarshal(Serializable::ToJson(jsonStr));
        }
    }
    return true;
}

bool KvStoreMetaManager::GetKvStoreMetaDataByBundleName(const std::string &bundleName, KvStoreMetaData &metaData)
{
    return GetKvStoreMetaByType(KvStoreMetaData::BUNDLE_NAME, bundleName, metaData);
}

bool KvStoreMetaManager::GetKvStoreMetaDataByAppId(const std::string &appId, KvStoreMetaData &metaData)
{
    return GetKvStoreMetaByType(KvStoreMetaData::APP_ID, appId, metaData);
}
}  // namespace DistributedKv
}  // namespace OHOS
