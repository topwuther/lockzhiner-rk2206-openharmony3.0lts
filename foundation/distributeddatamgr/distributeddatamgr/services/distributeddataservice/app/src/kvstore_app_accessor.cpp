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

#define LOG_TAG "KvStoreAppAccessor"

#include "kvstore_app_accessor.h"
#include <map>
#include <autils/constant.h>
#include "broadcast_sender.h"
#include "kv_store_delegate_manager.h"
#include "kvstore_app_manager.h"
#include "kvstore_meta_manager.h"
#include "log_print.h"
#include "permission_validator.h"

namespace OHOS::DistributedKv {
KvStoreAppAccessor::~KvStoreAppAccessor()
{
    ZLOGD("destructor.");
}

KvStoreAppAccessor::KvStoreAppAccessor()
{
    ZLOGD("constructor.");
}

KvStoreAppAccessor &KvStoreAppAccessor::GetInstance()
{
    static KvStoreAppAccessor appAccessor;
    return appAccessor;
}

void KvStoreAppAccessor::EnableKvStoreAutoLaunch(const AppAccessorParam &param)
{
    if (!PermissionValidator::IsAutoLaunchEnabled(param.appId)) {
        ZLOGI("AppId:%s is not allowed.", param.appId.c_str());
        return;
    }

    auto callback = std::bind(&KvStoreAppAccessor::OnCallback, this, std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3, std::placeholders::_4);
    auto dbStatus = DistributedDB::KvStoreDelegateManager::EnableKvStoreAutoLaunch(param.userId, param.appId,
                                                                                   param.storeId, param.launchOption,
                                                                                   callback);
    if (dbStatus == DistributedDB::DBStatus::OK || dbStatus == DistributedDB::DBStatus::ALREADY_SET) {
        ZLOGI("AppId:%s enable auto launch success.", param.appId.c_str());
        return;
    }
    ZLOGW("AppId:%s enable auto launch failed.", param.appId.c_str());
}

void KvStoreAppAccessor::EnableKvStoreAutoLaunch()
{
    ZLOGI("start");
    std::map<std::string, MetaData> entries;
    if (KvStoreMetaManager::GetInstance().GetFullMetaData(entries)) {
        for (auto &meta : entries) {
            KvStoreMetaData &metaData = meta.second.kvStoreMetaData;
            ZLOGI("meta appId:%s", metaData.appId.c_str());
            if (!PermissionValidator::IsAutoLaunchEnabled(metaData.appId)) {
                continue;
            }
            DistributedDB::CipherPassword password;
            const std::vector<uint8_t> &secretKey = meta.second.secretKeyMetaData.secretKey;
            if (password.SetValue(secretKey.data(), secretKey.size()) != DistributedDB::CipherPassword::OK) {
                ZLOGE("Get secret key failed.");
                return;
            }
            auto pathType = KvStoreAppManager::ConvertPathType(metaData.bundleName, metaData.securityLevel);
            std::string appPath = KvStoreAppManager::GetDataStoragePath(
                metaData.deviceAccountId, metaData.bundleName, pathType);
            DistributedDB::AutoLaunchOption dbLaunchOption;
            dbLaunchOption.createIfNecessary = true;
            dbLaunchOption.createDirByStoreIdOnly = true;
            dbLaunchOption.dataDir = Constant::Concatenate({appPath, "/", metaData.bundleName});
            dbLaunchOption.observer = nullptr;
            if (password.GetSize() > 0) {
                dbLaunchOption.isEncryptedDb = true;
                dbLaunchOption.cipher = DistributedDB::CipherType::AES_256_GCM;
                dbLaunchOption.passwd = password;
            }
            dbLaunchOption.secOption = KvStoreAppManager::ConvertSecurity(metaData.securityLevel);
            EnableKvStoreAutoLaunch({metaData.userId, metaData.appId, metaData.storeId, dbLaunchOption});
        }
    } else {
        ZLOGW("Init Service start enable failed.");
    }
}

void KvStoreAppAccessor::DisableKvStoreAutoLaunch(const AppAccessorParam &param)
{
}

void KvStoreAppAccessor::OnCallback(const std::string &userId, const std::string &appId, const std::string &storeId,
                                    const DistributedDB::AutoLaunchStatus status)
{
    ZLOGI("start");
    if (status == DistributedDB::AutoLaunchStatus::WRITE_OPENED) {
        KvStoreMetaData kvStoreMetaData;
        if (KvStoreMetaManager::GetInstance().GetKvStoreMetaDataByAppId(appId, kvStoreMetaData)) {
            BroadcastSender::GetInstance()->SendEvent({userId, kvStoreMetaData.bundleName, storeId});
            ZLOGW("AppId:%s is opened, bundle:%s.", appId.c_str(), kvStoreMetaData.bundleName.c_str());
        } else {
            ZLOGW("AppId:%s open failed.", appId.c_str());
        }
    } else {
        ZLOGD("AppId:%s is closed.", appId.c_str());
    }
}
}
