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

#define LOG_TAG "BackupHandler"

#include "backup_handler.h"
#include <directory_ex.h>
#include <fcntl.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include "account_delegate.h"
#include "battery_info.h"
#include "battery_srv_client.h"
#include "constant.h"
#include "crypto_utils.h"
#include "kv_store_delegate_manager.h"
#include "kv_scheduler.h"
#include "kvstore_data_service.h"
#include "log_print.h"
#include "kvstore_meta_manager.h"
#include "power_mgr_client.h"
#include "time_utils.h"

namespace OHOS::DistributedKv {
using json = nlohmann::json;

BackupHandler::BackupHandler(IKvStoreDataService *kvStoreDataService)
{
}

BackupHandler::BackupHandler()
{
}
BackupHandler::~BackupHandler()
{
}
void BackupHandler::BackSchedule()
{
    std::chrono::duration<int> delay(1800); // delay 30 minutes
    std::chrono::duration<int> internal(1800); // duration is 30 minutes
    ZLOGI("BackupHandler Schedule start.");
    scheduler_.Every(delay, internal, [&]() {
        if (!CheckNeedBackup()) {
            ZLOGE("it is not meet the condition of backup.");
            return;
        }
        std::map<std::string, MetaData> results;
        ZLOGI("BackupHandler Schedule Every start.");
        if (KvStoreMetaManager::GetInstance().GetFullMetaData(results)) {
            ZLOGE("GetFullMetaData failed.");
            return;
        }

        for (auto const &entry : results) {
            if (!entry.second.kvStoreMetaData.isBackup || entry.second.kvStoreMetaData.isDirty) {
                continue;
            }

            KvStoreType type = entry.second.kvStoreMetaData.kvStoreType;
            if (type == KvStoreType::MULTI_VERSION) {
                MultiKvStoreBackup(entry.second);
            } else if (type == KvStoreType::SINGLE_VERSION) {
                SingleKvStoreBackup(entry.second);
            }
        }
        backupSuccessTime_ = TimeUtils::CurrentTimeMicros();
    });
}

void BackupHandler::SingleKvStoreBackup(const MetaData &metaData)
{
    ZLOGI("SingleKvStoreBackup start.");
    auto pathType = KvStoreAppManager::ConvertPathType(
        metaData.kvStoreMetaData.bundleName, metaData.kvStoreMetaData.securityLevel);
    if (!ForceCreateDirectory(BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType))) {
        ZLOGE("SingleKvStoreBackup backup create directory failed.");
        return;
    }

    DistributedDB::CipherPassword password;
    const std::vector<uint8_t> &secretKey = metaData.secretKeyMetaData.secretKey;
    if (password.SetValue(secretKey.data(), secretKey.size()) != DistributedDB::CipherPassword::OK) {
        ZLOGE("Set secret key failed.");
        return;
    }

    DistributedDB::KvStoreNbDelegate::Option dbOption;
    dbOption.createIfNecessary = false;
    dbOption.isEncryptedDb = password.GetSize() > 0;
    dbOption.passwd = password;
    dbOption.createDirByStoreIdOnly = true;
    dbOption.secOption = KvStoreAppManager::ConvertSecurity(metaData.kvStoreMetaData.securityLevel);

    auto *delegateMgr = new DistributedDB::KvStoreDelegateManager(metaData.kvStoreMetaData.appId,
        AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(metaData.kvStoreMetaData.bundleName));

    std::string appDataStoragePath = KvStoreAppManager::GetDataStoragePath(metaData.kvStoreMetaData.deviceAccountId,
        metaData.kvStoreMetaData.bundleName, pathType);
    delegateMgr->SetKvStoreConfig(
        { Constant::Concatenate({appDataStoragePath, "/", metaData.kvStoreMetaData.bundleName })});

    std::function<void(DistributedDB::DBStatus, DistributedDB::KvStoreNbDelegate *)> fun =
        [&](DistributedDB::DBStatus status, DistributedDB::KvStoreNbDelegate *delegate) {
            auto del = std::shared_ptr<DistributedDB::KvStoreDelegateManager>(delegateMgr);
            if (delegate == nullptr) {
                ZLOGE("SingleKvStoreBackup delegate is null");
                return;
            }
            if (metaData.kvStoreMetaData.isAutoSync) {
                bool autoSync = true;
                DistributedDB::PragmaData data = static_cast<DistributedDB::PragmaData>(&autoSync);
                auto pragmaStatus = delegate->Pragma(DistributedDB::PragmaCmd::AUTO_SYNC, data);
                if (pragmaStatus != DistributedDB::DBStatus::OK) {
                    ZLOGE("pragmaStatus: %d", static_cast<int>(pragmaStatus));
                }
            }

            ZLOGW("SingleKvStoreBackup export");
            if (status == DistributedDB::DBStatus::OK) {
                std::string backupName = Constant::Concatenate(
                    { metaData.kvStoreMetaData.userId, "_", metaData.kvStoreMetaData.appId, "_",
                      metaData.kvStoreMetaData.storeId });
                auto backupFullName = Constant::Concatenate({
                    BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType), "/",
                    GetHashedBackupName(backupName)
                });
                auto backupBackFullName = Constant::Concatenate({ backupFullName, ".", "backup" });
                RenameFile(backupFullName, backupBackFullName);
                status = delegate->Export(backupFullName, dbOption.passwd);
                if (status == DistributedDB::DBStatus::OK) {
                    ZLOGD("SingleKvStoreBackup export success.");
                    RemoveFile(backupBackFullName);
                } else {
                    ZLOGE("SingleKvStoreBackup export failed, status is %d.", status);
                    RenameFile(backupBackFullName, backupFullName);
                }
            }
            del->CloseKvStore(delegate);
        };
    delegateMgr->GetKvStore(metaData.kvStoreMetaData.storeId, dbOption, fun);
}

void BackupHandler::MultiKvStoreBackup(const MetaData &metaData)
{
    auto pathType = KvStoreAppManager::ConvertPathType(metaData.kvStoreMetaData.bundleName,
                                                       metaData.kvStoreMetaData.securityLevel);
    if (!ForceCreateDirectory(BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType))) {
        ZLOGE("MultiKvStoreBackup backup create directory failed.");
        return;
    }
    ZLOGI("MultiKvStoreBackup start.");

    DistributedDB::CipherPassword password;
    const std::vector<uint8_t> &secretKey = metaData.secretKeyMetaData.secretKey;
    if (password.SetValue(secretKey.data(), secretKey.size()) != DistributedDB::CipherPassword::OK) {
        ZLOGE("Set secret key value failed. len is (%d)", int32_t(secretKey.size()));
        return;
    }

    DistributedDB::KvStoreDelegate::Option option;
    option.createIfNecessary = false;
    option.isEncryptedDb = password.GetSize() > 0;
    option.passwd = password;
    option.createDirByStoreIdOnly = true;

    auto *delegateMgr = new DistributedDB::KvStoreDelegateManager(metaData.kvStoreMetaData.appId,
        AccountDelegate::GetInstance()->GetCurrentHarmonyAccountId(metaData.kvStoreMetaData.bundleName));
    std::string appDataStoragePath = KvStoreAppManager::GetDataStoragePath(metaData.kvStoreMetaData.deviceAccountId,
        metaData.kvStoreMetaData.bundleName, pathType);
    delegateMgr->SetKvStoreConfig(
        {Constant::Concatenate({appDataStoragePath, "/", metaData.kvStoreMetaData.bundleName})});
    std::function<void(DistributedDB::DBStatus, DistributedDB::KvStoreDelegate *)> fun =
        [&](DistributedDB::DBStatus status, DistributedDB::KvStoreDelegate *delegate) {
            auto del = std::shared_ptr<DistributedDB::KvStoreDelegateManager>(delegateMgr);
            if (delegate == nullptr) {
                ZLOGE("MultiKvStoreBackup delegate is null");
                return;
            }
            ZLOGW("MultiKvStoreBackup export");
            if (status == DistributedDB::DBStatus::OK) {
                std::string backupName =
                    Constant::Concatenate({metaData.kvStoreMetaData.userId, "_",
                                           metaData.kvStoreMetaData.appId, "_", metaData.kvStoreMetaData.storeId});
                auto backupFullName = Constant::Concatenate({
                    BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType), "/",
                    GetHashedBackupName(backupName)
                });
                auto backupBackFullName = Constant::Concatenate({backupFullName, ".", "backup"});
                RenameFile(backupFullName, backupBackFullName);
                status = delegate->Export(backupFullName, option.passwd);
                if (status == DistributedDB::DBStatus::OK) {
                    ZLOGD("MultiKvStoreBackup export success.");
                    RemoveFile(backupBackFullName);
                    ZLOGD("MultiKvStoreBackup export success.");
                } else {
                    ZLOGE("MultiKvStoreBackup export failed.");
                    RenameFile(backupBackFullName, backupFullName);
                }
            }
            del->CloseKvStore(delegate);
        };
    delegateMgr->GetKvStore(metaData.kvStoreMetaData.storeId, option, fun);
}

bool BackupHandler::SingleKvStoreRecover(MetaData &metaData, DistributedDB::KvStoreNbDelegate *delegate)
{
    ZLOGI("start.");
    if (delegate == nullptr) {
        ZLOGE("SingleKvStoreRecover failed, delegate is null.");
        return false;
    }
    auto pathType = KvStoreAppManager::ConvertPathType(metaData.kvStoreMetaData.bundleName,
                                                       metaData.kvStoreMetaData.securityLevel);
    if (!BackupHandler::FileExists(BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType))) {
        ZLOGE("SingleKvStoreRecover failed, backupDir_ file is not exist.");
        return false;
    }

    DistributedDB::CipherPassword password;
    const std::vector<uint8_t> &secretKey = metaData.secretKeyMetaData.secretKey;
    if (password.SetValue(secretKey.data(), secretKey.size()) != DistributedDB::CipherPassword::OK) {
        ZLOGE("Set secret key failed.");
        return false;
    }

    std::string backupName = Constant::Concatenate(
        {metaData.kvStoreMetaData.userId, "_", metaData.kvStoreMetaData.appId, "_",
         metaData.kvStoreMetaData.storeId});
    auto backupFullName = Constant::Concatenate({
        BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType), "/",
        GetHashedBackupName(backupName)
    });
    DistributedDB::DBStatus dbStatus = delegate->Import(backupFullName, password);
    if (dbStatus == DistributedDB::DBStatus::OK) {
        ZLOGI("SingleKvStoreRecover success.");
        return true;
    }
    ZLOGI("SingleKvStoreRecover failed.");
    return false;
}

bool BackupHandler::MultiKvStoreRecover(MetaData &metaData,
                                        DistributedDB::KvStoreDelegate *delegate)
{
    ZLOGI("start.");
    if (delegate == nullptr) {
        ZLOGE("MultiKvStoreRecover failed, delegate is null.");
        return false;
    }
    auto pathType = KvStoreAppManager::ConvertPathType(metaData.kvStoreMetaData.bundleName,
                                                       metaData.kvStoreMetaData.securityLevel);
    if (!BackupHandler::FileExists(BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType))) {
        ZLOGE("MultiKvStoreRecover failed, backupDir_ file is not exist.");
        return false;
    }

    ZLOGI("MultiKvStoreRecover start.");
    DistributedDB::CipherPassword password;
    const std::vector<uint8_t> &secretKey = metaData.secretKeyMetaData.secretKey;
    if (password.SetValue(secretKey.data(), secretKey.size()) != DistributedDB::CipherPassword::OK) {
        ZLOGE("Set secret key failed.");
        return false;
    }

    std::string backupName = Constant::Concatenate(
        {metaData.kvStoreMetaData.userId, "_", metaData.kvStoreMetaData.appId, "_",
         metaData.kvStoreMetaData.storeId});
    auto backupFullName = Constant::Concatenate({
        BackupHandler::GetBackupPath(metaData.kvStoreMetaData.deviceAccountId, pathType), "/",
        GetHashedBackupName(backupName)
    });
    DistributedDB::DBStatus dbStatus = delegate->Import(backupFullName, password);
    if (dbStatus == DistributedDB::DBStatus::OK) {
        ZLOGI("MultiKvStoreRecover success.");
        return true;
    }
    ZLOGI("MultiKvStoreRecover failed.");
    return false;
}

std::string BackupHandler::backupDirCe_;
std::string BackupHandler::backupDirDe_;
const std::string &BackupHandler::GetBackupPath(const std::string &deviceAccountId, int type)
{
    if (type == KvStoreAppManager::PATH_DE) {
        if (backupDirDe_.empty()) {
            backupDirDe_ = Constant::Concatenate({ Constant::ROOT_PATH_DE, "/", Constant::SERVICE_NAME, "/",
                                                   deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(),
                                                   "/", "backup" });
        }
        return backupDirDe_;
    } else {
        if (backupDirCe_.empty()) {
            backupDirCe_ = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
                                                   deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(),
                                                   "/", "backup" });
        }
        return backupDirCe_;
    }
}

bool BackupHandler::RenameFile(const std::string &oldPath, const std::string &newPath)
{
    if (oldPath.empty() || newPath.empty()) {
        ZLOGE("RenameFile failed: path is empty");
        return false;
    }
    if (!RemoveFile(newPath)) {
        ZLOGE("RenameFile failed: newPath file is already exist");
        return false;
    }
    if (rename(oldPath.c_str(), newPath.c_str()) != 0) {
        ZLOGE("RenameFile: rename error, errno[%d].", errno);
        return false;
    }
    return true;
}

bool BackupHandler::RemoveFile(const std::string &path)
{
    if (path.empty()) {
        ZLOGI("RemoveFile: path is empty");
        return true;
    }

    if (unlink(path.c_str()) != 0 && (errno != ENOENT)) {
        ZLOGE("RemoveFile: failed to RemoveFile, errno[%d].", errno);
        return false;
    }
    return true;
}

bool BackupHandler::FileExists(const std::string &path)
{
    if (path.empty()) {
        ZLOGI("FileExists: path is empty");
        return false;
    }

    if (access(path.c_str(), F_OK) != 0) {
        ZLOGI("FileExists: file is not exist");
        return false;
    }
    return true;
}

bool BackupHandler::CheckNeedBackup()
{
    auto &batterySrvClient = PowerMgr::BatterySrvClient::GetInstance();
    auto chargingStatus = batterySrvClient.GetChargingStatus();
    if (chargingStatus != PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE) {
        if (chargingStatus != PowerMgr::BatteryChargeState::CHARGE_STATE_FULL) {
            ZLOGE("the device is not in charge state, chargingStatus=%{public}d.", chargingStatus);
            return false;
        }
        auto batteryPluggedType = batterySrvClient.GetPluggedType();
        if (batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_AC &&
            batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_USB &&
            batteryPluggedType != PowerMgr::BatteryPluggedType::PLUGGED_TYPE_WIRELESS) {
            ZLOGE("the device is not in charge full statue, the batteryPluggedType is %{public}d.", batteryPluggedType);
            return false;
        }
    }
    auto &powerMgrClient = PowerMgr::PowerMgrClient::GetInstance();
    if (powerMgrClient.IsScreenOn()) {
        ZLOGE("the device screen is on.");
        return false;
    }
    int64_t currentTime = TimeUtils::CurrentTimeMicros();
    if (currentTime - backupSuccessTime_ < 36000000 && backupSuccessTime_ > 0) { // 36000000 is 10 hours
        ZLOGE("no more than 10 hours since the last backup success.");
        return false;
    }
    return true;
}

std::string BackupHandler::GetHashedBackupName(const std::string &bundleName)
{
    if (bundleName.empty()) {
        return bundleName;
    }
    return CryptoUtils::Sha256(bundleName);
}
} // namespace OHOS::DistributedKv
