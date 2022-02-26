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

#ifndef SCHEMA_OBJECT_H
#define SCHEMA_OBJECT_H

#include <set>
#include <map>
#ifndef OMIT_FLATBUFFER
#include <flatbuffers/idl.h>
#endif // OMIT_FLATBUFFER
#include "db_types.h"
#include "macro_utils.h"
#include "value_object.h"

namespace DistributedDB {
// SchemaType::NONE represent for KV database which do not have schema. Only invalid SchemaObject is NONE type.
// Enum value must not be changed except SchemaType::UNRECOGNIZED.
enum class SchemaType : uint8_t {
    NONE = 0,
    JSON = 1,
    FLATBUFFER = 2,
    UNRECOGNIZED = 3,
};

struct SchemaAttribute {
    FieldType type = FieldType::LEAF_FIELD_NULL;
    bool isIndexable = false;
    bool hasNotNullConstraint = false;
    bool hasDefaultValue = false;
    FieldValue defaultValue; // Has default value in union part and default construction in string part
};

using IndexName = FieldPath;
using IndexFieldInfo = std::pair<FieldPath, FieldType>;
using IndexInfo = std::vector<IndexFieldInfo>;
template<typename T> using PairConstPointer = std::pair<const T *, const T *>;

struct IndexDifference {
    std::map<IndexName, IndexInfo> change;
    std::map<IndexName, IndexInfo> increase;
    std::set<IndexName> decrease;
};

struct SyncOpinion {
    bool permitSync = false;
    bool requirePeerConvert = false;
    bool checkOnReceive = false;
};

struct SyncStrategy {
    bool permitSync = false;
    bool convertOnSend = false;
    bool convertOnReceive = false;
    bool checkOnReceive = false;
};

class SchemaObject {
public:
    static std::string GetExtractFuncName(SchemaType inSchemaType);
    static std::string GenerateExtractSQL(SchemaType inSchemaType, const FieldPath &inFieldpath, FieldType inFieldType,
        uint32_t skipSize);

    // The remoteSchemaType may beyond local SchemaType definition
    static SyncOpinion MakeLocalSyncOpinion(const SchemaObject &localSchema, const std::string &remoteSchema,
        uint8_t remoteSchemaType);
    // The remoteOpinion.checkOnReceive is ignored
    static SyncStrategy ConcludeSyncStrategy(const SyncOpinion &localOpinion, const SyncOpinion &remoteOpinion);

    // Support default constructor, copy constructor and copy assignment
    SchemaObject();
    ~SchemaObject() = default;
    SchemaObject(const SchemaObject &);
    SchemaObject& operator=(const SchemaObject &);
    // Move constructor and move assignment is not need currently
    SchemaObject(SchemaObject &&) = delete;
    SchemaObject& operator=(SchemaObject &&) = delete;

    // Should be called on an invalid SchemaObject, create new SchemaObject if need to reparse
    int ParseFromSchemaString(const std::string &inSchemaString);

    bool IsSchemaValid() const;
    SchemaType GetSchemaType() const;
    // For Json-Schema : Unnecessary spacing will be removed and fieldname resorted by lexicographical order
    // For FlatBuffer-Schema : Original binary schema(Base64 decoded if need)
    std::string ToSchemaString() const;

    uint32_t GetSkipSize() const;
    std::map<IndexName, IndexInfo> GetIndexInfo() const;
    bool IsIndexExist(const IndexName &indexName) const;
    // Return E_OK if queryale. outType will be set if path exist no matter binary or not
    int CheckQueryableAndGetFieldType(const FieldPath &inPath, FieldType &outType) const;

    // Attention: it doesn't return E_OK. instead:
    // E_JSON_PARSE_FAIL : the inSchemaString is not an valid json
    // E_SCHEMA_PARSE_FAIL : the inSchemaString is not an valid schema
    // E_SCHEMA_EQUAL_EXACTLY : the inSchema is exactly equal to this SchemaObject
    // E_SCHEMA_UNEQUAL_COMPATIBLE : the inSchema is not equal to but only index differ with this SchemaObject
    // E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE : the inSchema is not equal to but can upgrade from this SchemaObject
    // E_SCHEMA_UNEQUAL_INCOMPATIBLE : the inSchema is not equal to and can not upgrade from this SchemaObject
    int CompareAgainstSchemaString(const std::string &inSchemaString) const;
    int CompareAgainstSchemaString(const std::string &inSchemaString, IndexDifference &indexDiffer) const;
    int CompareAgainstSchemaObject(const SchemaObject &inSchemaObject) const;
    int CompareAgainstSchemaObject(const SchemaObject &inSchemaObject, IndexDifference &indexDiffer) const;

    // Attention: it doesn't return E_OK. instead:
    // E_VALUE_MATCH : Value match schema(no matter strict or compatible mode) without any change
    // E_VALUE_MATCH_AMENDED : Value match schema(no matter strict or compatible mode) with some amendment
    // E_VALUE_MISMATCH_FEILD_COUNT : Value contain more field then schema when in strict mode
    // E_VALUE_MISMATCH_FEILD_TYPE : Type of some fields of value mismatch schema
    // E_VALUE_MISMATCH_CONSTRAINT : Some fields of value violate the NotNull constraint against schema
    // E_VALUE_MISMATCH_OTHER_REASON : Value mismatch schema because of other reason unmentioned
    int CheckValueAndAmendIfNeed(ValueSource sourceType, ValueObject &inValue) const;

    // Currently only for flatBuffer-type schema and value.
    // Accept the original entry-value, return E_OK or E_FLATBUFFER_VERIFY_FAIL.
    int VerifyValue(ValueSource sourceType, const Value &inValue) const;
    int VerifyValue(ValueSource sourceType, const RawValue &inValue) const;
    // Accept the original value from database. The cache will not be expanded. Return E_OK if nothing error.
    // The ExtractValue is with nice performance by carefully not use std-class to avoid memory allocation.
    // But currently it can only deal with path with $. prefix and only one depth. However, meet current demand.
    int ExtractValue(ValueSource sourceType, RawString inPath, const RawValue &inValue, TypeValue &outExtract,
        std::vector<uint8_t> *cache) const;
private:
    enum class SchemaMode {
        STRICT,
        COMPATIBLE,
    };
    using SchemaDefine = std::map<FieldPath, SchemaAttribute>;

    // For Json-Schema : Parsing related methods.
    int ParseJsonSchema(const JsonObject &inJsonObject);
    int CheckMetaFieldCountAndType(const JsonObject &inJsonObject) const;
    int ParseCheckSchemaVersionMode(const JsonObject &inJsonObject);
    int ParseCheckSchemaDefine(const JsonObject &inJsonObject);
    int CheckSchemaDefineItemDecideAttribute(const JsonObject &inJsonObject, const FieldPath &inPath, FieldType inType,
        SchemaAttribute &outAttr) const;
    int ParseCheckSchemaIndexes(const JsonObject &inJsonObject);
    int ParseCheckSchemaSkipSize(const JsonObject &inJsonObject);

    // For both Json-Schema and FlatBuffer-Schema.
    int ParseCheckEachIndexFromStringArray(const std::vector<std::string> &inStrArray);
    int CheckFieldPathIndexableThenSave(const std::vector<FieldPath> &inPathVec, IndexInfo &infoToSave);

    // CompareAgainstSchemaObject related sub methods
    int CompareSchemaVersionMode(const SchemaObject &newSchema) const;
    int CompareSchemaSkipSize(const SchemaObject &newSchema) const;
    int CompareSchemaDefine(const SchemaObject &newSchema) const;
    int CompareSchemaDefineByDepth(const SchemaDefine &oldDefine, const SchemaDefine &newDefine) const;
    int CompareSchemaAttribute(const SchemaAttribute &oldAttr, const SchemaAttribute &newAttr) const;
    int CompareSchemaDefaultValue(const SchemaAttribute &oldAttr, const SchemaAttribute &newAttr) const;
    int CompareSchemaIndexes(const SchemaObject &newSchema, IndexDifference &indexDiffer) const;

    // CheckValueAndAmendIfNeed related sub methods
    int CheckValue(const ValueObject &inValue, std::set<FieldPath> &lackingPaths) const;
    int AmendValueIfNeed(ValueObject &inValue, const std::set<FieldPath> &lackingPaths, bool &amended) const;

    // It is better using a class to represent flatBuffer-Schema related other than more private method(As well as for
    // Json-Schema in the future refactor). Delegation is chosen other than inheritance for accessing SchemaObject.
    // Choose inner-class other than friend-class to avoid forward declaration and using pointer.
    class FlatBufferSchema {
    public:
        FlatBufferSchema(SchemaObject &owner) : owner_(owner) {};
        ~FlatBufferSchema() = default;
        DISABLE_COPY_ASSIGN_MOVE(FlatBufferSchema);
        // Copy-Constructor can not define due to Const-Ref member. Code standard require copy assignment be deleted.
        void CopyFrom(const FlatBufferSchema &other);

        std::string GetDescription() const;

        // Judge whether it's flatbuffer type schema, no matter whether it is Base64 encoded, provide a decoded one.
        static bool IsFlatBufferSchema(const std::string &inOriginal, std::string &outDecoded);

        // Accept a decoded and verified flatbuffer-schema, then parse its content
        int ParseFlatBufferSchema(const std::string &inDecoded);

        // Compare based on self.
        // return E_SCHEMA_EQUAL_EXACTLY or E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE or E_SCHEMA_UNEQUAL_INCOMPATIBLE
        int CompareFlatBufferDefine(const FlatBufferSchema &other) const;
        // Accept a no-skipsize(so byte-aligned) value, return E_OK or E_FLATBUFFER_VERIFY_FAIL.
        int VerifyFlatBufferValue(const RawValue &inValue, bool tryNoSizePrefix) const;
        // Accept a no-skipsize(so byte-aligned) value.
        int ExtractFlatBufferValue(RawString inPath, const RawValue &inValue, TypeValue &outExtract,
            bool tryNoSizePrefix) const;
    private:
#ifndef OMIT_FLATBUFFER
        using RawIndexInfos = std::map<std::string, std::string>; // First the fieldName, second the index-attr value.

        const reflection::Schema *GetSchema() const;

        int ParseCheckRootTableAttribute(const reflection::Object &rootTable);
        int ParseCheckRootTableDefine(const reflection::Schema &schema, const reflection::Object &rootTable,
            RawIndexInfos &indexCollect);
        int ParseCheckFieldInfo(const reflection::Schema &schema, const reflection::Field &field,
            const FieldPath &path, RawIndexInfos &indexCollect);
        void CollectRawIndexInfos(const reflection::Field &field, RawIndexInfos &indexCollect) const;
        int ParseCheckStructDefine(const reflection::Schema &schema, const reflection::Field &field,
            const FieldPath &path);
        int ParseCheckIndexes(const RawIndexInfos &indexCollect);

        int CompareTableOrStructDefine(const PairConstPointer<reflection::Schema> &bothSchema,
            const PairConstPointer<reflection::Object> &bothObject, bool isRoot, std::set<std::string> &compared) const;
        int CompareStruct(const PairConstPointer<reflection::Schema> &bothSchema,
            const PairConstPointer<reflection::Field> &bothField, std::set<std::string> &compared) const;
#endif
        SchemaObject &owner_;
        std::string description_;
    };

    bool isValid_ = false;
    SchemaType schemaType_ = SchemaType::NONE; // Default NONE
    FlatBufferSchema flatbufferSchema_;
    std::string schemaString_; // The minified and valid schemaString

    std::string schemaVersion_;
    SchemaMode schemaMode_ = SchemaMode::STRICT; // Only for Json-Schema, Consider refactor into JsonSchema class
    uint32_t schemaSkipSize_ = 0;
    std::map<IndexName, IndexInfo> schemaIndexes_;
    std::map<uint32_t, SchemaDefine> schemaDefine_; // SchemaDefine classified by the depth of fieldpath
};
} // namespace DistributedDB

#endif // SCHEMA_OBJECT_H

