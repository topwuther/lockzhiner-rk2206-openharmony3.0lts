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

#ifndef KV_DB_PROPERTIES_H
#define KV_DB_PROPERTIES_H

#include <map>
#include <string>
#include <vector>

#include "schema_object.h"

namespace DistributedDB {
class KvDBProperties final {
public:
    KvDBProperties();
    ~KvDBProperties();

    // Get the sub directory for different type database.
    static std::string GetStoreSubDirectory(int type);

    // Get the string property according the name
    std::string GetStringProp(const std::string &name, const std::string &defaultValue) const;

    // Set the string property for the name
    void SetStringProp(const std::string &name, const std::string &value);

    // Get the bool property according the name
    bool GetBoolProp(const std::string &name, bool defaultValue) const;

    // Set the bool property for the name
    void SetBoolProp(const std::string &name, bool value);

    // Get the bool property according the name
    int GetIntProp(const std::string &name, int defaultValue) const;

    // Set the integer property for the name
    void SetIntProp(const std::string &name, int value);

    // Get the password
    void GetPassword(CipherType &type, CipherPassword &password) const;

    // Set the password
    void SetPassword(CipherType type, const CipherPassword &password);

    // is schema exist
    bool IsSchemaExist() const;

    // set schema
    void SetSchema(const SchemaObject &schema);

    // get schema
    SchemaObject GetSchema() const;

    // If it does not exist, use the int map default value 0
    int GetSecLabel() const;

    int GetSecFlag() const;
    // Get schema const reference if you can guarantee the lifecycle of this KvDBProperties
    // The upper code will not change the schema if it is already set
    const SchemaObject &GetSchemaConstRef() const;

    static const std::string CREATE_IF_NECESSARY;
    static const std::string DATABASE_TYPE;
    static const std::string DATA_DIR;
    static const std::string USER_ID;
    static const std::string APP_ID;
    static const std::string STORE_ID;
    static const std::string FILE_NAME;
    static const std::string SYNC_MODE;
    static const std::string MEMORY_MODE;
    static const std::string ENCRYPTED_MODE;
    static const std::string IDENTIFIER_DATA;
    static const std::string IDENTIFIER_DIR;
    static const std::string FIRST_OPEN_IS_READ_ONLY;
    static const std::string CREATE_DIR_BY_STORE_ID_ONLY;
    static const std::string SECURITY_LABEL;
    static const std::string SECURITY_FLAG;
    static const std::string CONFLICT_RESOLVE_POLICY;

    static const int LOCAL_TYPE = 1;
    static const int MULTI_VER_TYPE = 2;
    static const int SINGLE_VER_TYPE = 3;

private:
    std::map<std::string, std::string> stringProperties_;
    std::map<std::string, bool> boolProperties_;
    std::map<std::string, int> intProperties_;
    CipherType cipherType_;
    CipherPassword password_;
    SchemaObject schema_;
};
} // namespace DistributedDB

#endif // KV_DB_PROPERTIES_H
