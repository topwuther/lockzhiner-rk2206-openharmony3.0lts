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

#include "param_check_utils.h"

#include "db_errno.h"
#include "platform_specific.h"
#include "log_print.h"

namespace DistributedDB {
bool ParamCheckUtils::CheckDataDir(const std::string &dataDir, std::string &canonicalDir)
{
    if (dataDir.empty() || (dataDir.length() > DBConstant::MAX_DATA_DIR_LENGTH)) {
        LOGE("Invalid data directory[%zu]", dataDir.length());
        return false;
    }

    if (OS::GetRealPath(dataDir, canonicalDir) != E_OK) {
        LOGE("Failed to canonicalize the data dir.");
        return false;
    }
    // After normalizing the path, determine whether the path is a legal path considered by the program.
    // There has been guaranteed by the upper layer, So there is no need trustlist set here.
    return true;
}

bool ParamCheckUtils::IsStoreIdSafe(const std::string &storeId)
{
    if (storeId.empty() || (storeId.length() > DBConstant::MAX_STORE_ID_LENGTH)) {
        LOGE("Invalid store id[%zu]", storeId.length());
        return false;
    }

    auto iter = std::find_if_not(storeId.begin(), storeId.end(),
        [](char value) { return (std::isalnum(value) || value == '_'); });
    if (iter != storeId.end()) {
        LOGE("Invalid store id format");
        return false;
    }
    return true;
}

bool ParamCheckUtils::CheckStoreParameter(const std::string &storeId, const std::string &appId,
    const std::string &userId)
{
    if (!IsStoreIdSafe(storeId)) {
        return false;
    }
    if (userId.empty() || userId.length() > DBConstant::MAX_USER_ID_LENGTH ||
        appId.empty() || appId.length() > DBConstant::MAX_APP_ID_LENGTH) {
        LOGE("Invalid user or app info[%zu][%zu]", userId.length(), appId.length());
        return false;
    }

    if ((userId.find(DBConstant::ID_CONNECTOR) != std::string::npos) ||
        (appId.find(DBConstant::ID_CONNECTOR) != std::string::npos) ||
        (storeId.find(DBConstant::ID_CONNECTOR) != std::string::npos)) {
        LOGE("Invalid character in the store para info.");
        return false;
    }
    return true;
}

bool ParamCheckUtils::CheckEncryptedParameter(CipherType cipher, const CipherPassword &passwd)
{
    if (cipher != CipherType::DEFAULT && cipher != CipherType::AES_256_GCM) {
        LOGE("Invalid cipher type!");
        return false;
    }

    return (passwd.GetSize() != 0);
}

bool ParamCheckUtils::CheckConflictNotifierType(int conflictType)
{
    if (conflictType <= 0) {
        return false;
    }
    // Divide the type into different types.
    if (conflictType >= CONFLICT_NATIVE_ALL) {
        conflictType -= CONFLICT_NATIVE_ALL;
    }
    if (conflictType >= CONFLICT_FOREIGN_KEY_ORIG) {
        conflictType -= CONFLICT_FOREIGN_KEY_ORIG;
    }
    if (conflictType >= CONFLICT_FOREIGN_KEY_ONLY) {
        conflictType -= CONFLICT_FOREIGN_KEY_ONLY;
    }
    if (conflictType != 0) {
        return false;
    }
    return true;
}

bool ParamCheckUtils::CheckSecOption(const SecurityOption &secOption)
{
    if (secOption.securityLabel > S4 || secOption.securityLabel < NOT_SET) {
        LOGE("[DBCommon] SecurityLabel is invalid, label is [%d].", secOption.securityLabel);
        return false;
    }
    if (secOption.securityFlag != 0) {
        if ((secOption.securityLabel != S3 && secOption.securityLabel != S4) || secOption.securityFlag != SECE) {
            LOGE("[DBCommon] SecurityFlag is invalid.");
            return false;
        }
    }
    return true;
}

bool ParamCheckUtils::CheckObserver(const Key &key, unsigned int mode)
{
    if (key.size() > DBConstant::MAX_KEY_SIZE) {
        return false;
    }

    if (mode > OBSERVER_CHANGES_LOCAL_ONLY || mode < OBSERVER_CHANGES_NATIVE) {
        return false;
    }
    return true;
}

bool ParamCheckUtils::IsS3SECEOpt(const SecurityOption &secOpt)
{
    SecurityOption S3SeceOpt = {SecurityLabel::S3, SecurityFlag::SECE};
    return (secOpt == S3SeceOpt);
}

int ParamCheckUtils::CheckAndTransferAutoLaunchParam(const AutoLaunchParam &param,
    SchemaObject &schemaObject, std::string &canonicalDir)
{
    if ((param.option.notifier && !ParamCheckUtils::CheckConflictNotifierType(param.option.conflictType)) ||
        (!param.option.notifier && param.option.conflictType != 0)) {
        LOGE("[AutoLaunch] CheckConflictNotifierType is invalid.");
        return -E_INVALID_ARGS;
    }
    if (!ParamCheckUtils::CheckStoreParameter(param.storeId, param.appId, param.userId)) {
        LOGE("[AutoLaunch] CheckStoreParameter is invalid.");
        return -E_INVALID_ARGS;
    }

    const AutoLaunchOption &option = param.option;
    if (!ParamCheckUtils::CheckSecOption(option.secOption)) {
        LOGE("[AutoLaunch] CheckSecOption is invalid.");
        return -E_INVALID_ARGS;
    }

    if (option.isEncryptedDb) {
        if (!ParamCheckUtils::CheckEncryptedParameter(option.cipher, option.passwd)) {
            LOGE("[AutoLaunch] CheckEncryptedParameter is invalid.");
            return -E_INVALID_ARGS;
        }
    }

    if (!param.option.schema.empty()) {
        schemaObject.ParseFromSchemaString(param.option.schema);
        if (!schemaObject.IsSchemaValid()) {
            LOGE("[AutoLaunch] ParseFromSchemaString is invalid.");
            return -E_INVALID_SCHEMA;
        }
    }

    if (!ParamCheckUtils::CheckDataDir(param.option.dataDir, canonicalDir)) {
        LOGE("[AutoLaunch] CheckDataDir is invalid.");
        return -E_INVALID_ARGS;
    }
    return E_OK;
}
} // namespace DistributedDB