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

#include "kvdb_properties.h"

#include "db_constant.h"

namespace DistributedDB {
const std::string KvDBProperties::CREATE_IF_NECESSARY = "createIfNecessary";
const std::string KvDBProperties::DATABASE_TYPE = "databaseType";
const std::string KvDBProperties::DATA_DIR = "dataDir";
const std::string KvDBProperties::USER_ID = "userId";
const std::string KvDBProperties::APP_ID = "appId";
const std::string KvDBProperties::STORE_ID = "storeId";
const std::string KvDBProperties::FILE_NAME = "fileName";
const std::string KvDBProperties::MEMORY_MODE = "memoryMode";
const std::string KvDBProperties::ENCRYPTED_MODE = "isEncryptedDb";
const std::string KvDBProperties::IDENTIFIER_DATA = "identifier";
const std::string KvDBProperties::IDENTIFIER_DIR = "identifierDir";
const std::string KvDBProperties::FIRST_OPEN_IS_READ_ONLY = "firstOpenIsReadOnly";
const std::string KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY = "createDirByStoreIdOnly";
const std::string KvDBProperties::SECURITY_LABEL = "securityLabel";
const std::string KvDBProperties::SECURITY_FLAG = "securityFlag";
const std::string KvDBProperties::CONFLICT_RESOLVE_POLICY = "conflictResolvePolicy";

KvDBProperties::KvDBProperties()
    : cipherType_(CipherType::AES_256_GCM)
{}

KvDBProperties::~KvDBProperties() {}

std::string KvDBProperties::GetStoreSubDirectory(int type)
{
    switch (type) {
        case LOCAL_TYPE:
            return DBConstant::LOCAL_SUB_DIR;
        case MULTI_VER_TYPE:
            return DBConstant::MULTI_SUB_DIR;
        case SINGLE_VER_TYPE:
            return DBConstant::SINGLE_SUB_DIR;
        default:
            return "unknown";
    }
}

std::string KvDBProperties::GetStringProp(const std::string &name, const std::string &defaultValue) const
{
    auto iter = stringProperties_.find(name);
    if (iter != stringProperties_.end()) {
        return iter->second;
    } else {
        return defaultValue;
    }
}

void KvDBProperties::SetStringProp(const std::string &name, const std::string &value)
{
    stringProperties_[name] = value;
}

bool KvDBProperties::GetBoolProp(const std::string &name, bool defaultValue) const
{
    auto iter = boolProperties_.find(name);
    if (iter != boolProperties_.end()) {
        return iter->second;
    } else {
        return defaultValue;
    }
}

void KvDBProperties::SetBoolProp(const std::string &name, bool value)
{
    boolProperties_[name] = value;
}

int KvDBProperties::GetIntProp(const std::string &name, int defaultValue) const
{
    auto iter = intProperties_.find(name);
    if (iter != intProperties_.end()) {
        return iter->second;
    } else {
        return defaultValue;
    }
}

void KvDBProperties::SetIntProp(const std::string &name, int value)
{
    intProperties_[name] = value;
}

void KvDBProperties::GetPassword(CipherType &type, CipherPassword &password) const
{
    type = cipherType_;
    password = password_;
}

void KvDBProperties::SetPassword(CipherType type, const CipherPassword &password)
{
    cipherType_ = type;
    password_ = password;
}

bool KvDBProperties::IsSchemaExist() const
{
    return schema_.IsSchemaValid();
}

void KvDBProperties::SetSchema(const SchemaObject &schema)
{
    schema_ = schema;
}

SchemaObject KvDBProperties::GetSchema() const
{
    return schema_;
}

int KvDBProperties::GetSecLabel() const
{
    return GetIntProp(KvDBProperties::SECURITY_LABEL, 0);
}

int KvDBProperties::GetSecFlag() const
{
    return GetIntProp(KvDBProperties::SECURITY_FLAG, 0);
}

const SchemaObject &KvDBProperties::GetSchemaConstRef() const
{
    return schema_;
}
} // namespace DistributedDB
