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

#include "schema_object.h"
#include <cmath>
#include <algorithm>
#include "schema_utils.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
namespace {
#ifndef OMIT_FLATBUFFER
constexpr double EPSILON = 0.000001; // 0.000001 for tolerance
inline bool IsDoubleNearlyEqual(double left, double right)
{
    if (std::fabs(left - right) < EPSILON) {
        return true;
    }
    double absBigger = std::max(std::fabs(left), std::fabs(right));
    double relativeDiff = ((absBigger == 0.0) ? 0.0 : (std::fabs(left - right) / absBigger)); // 0.0 for double 0
    return relativeDiff < EPSILON;
}
#endif
}

void SchemaObject::FlatBufferSchema::CopyFrom(const FlatBufferSchema &other)
{
    // The SchemaObject guarantee not CopyFrom "Self"; owner_ can only be set at construction.
    description_ = other.description_;
}

std::string SchemaObject::FlatBufferSchema::GetDescription() const
{
    return description_;
}

#ifndef OMIT_FLATBUFFER
bool SchemaObject::FlatBufferSchema::IsFlatBufferSchema(const std::string &inOriginal, std::string &outDecoded)
{
    if (inOriginal.empty()) {
        LOGE("[FBSchema][Is] OriSchema empty.");
        return false;
    }
    if (inOriginal.size() >= SCHEMA_STRING_SIZE_LIMIT * 2) { // Base64 encode will not exceed 2 times original binary
        LOGE("[FBSchema][Is] OriSchemaSize=%zu too large even after base64 encode.", inOriginal.size());
        return false;
    }
    auto oriSchemaBuf = reinterpret_cast<const uint8_t *>(inOriginal.c_str());
    flatbuffers::Verifier oriVerifier(oriSchemaBuf, inOriginal.size());
    if (reflection::VerifySizePrefixedSchemaBuffer(oriVerifier)) {
        outDecoded = inOriginal; // The original one is the decoded one
        return true;
    }

    outDecoded.clear();
    return false;
}

// A macro check pointer get from flatbuffer that won't be nullptr(required field) in fact after verified by flatbuffer
#define CHECK_NULL_UNLIKELY_RETURN_ERROR(pointer) \
    if (pointer == nullptr) { \
        return -E_INTERNAL_ERROR; \
    }

namespace {
constexpr uint32_t ROOT_DEFINE_DEPTH = 0;
constexpr uint32_t SIZE_PREFIX_SIZE = sizeof(flatbuffers::uoffset_t);
constexpr int32_t INDEX_OF_NOT_ENUM = -1;

inline bool AttributeExistAndHasValue(const reflection::KeyValue *inAttr)
{
    return (inAttr != nullptr) && (inAttr->value() != nullptr) && (inAttr->value()->size() > 0);
}

inline bool IsIntegerType(reflection::BaseType inType)
{
    return (inType >= reflection::BaseType::Bool) && (inType <= reflection::BaseType::ULong);
}

inline bool IsRealType(reflection::BaseType inType)
{
    return (inType >= reflection::BaseType::Float) && (inType <= reflection::BaseType::Double);
}

inline bool IsScalarType(reflection::BaseType inType)
{
    return IsIntegerType(inType) || IsRealType(inType);
}

inline bool IsStringType(reflection::BaseType inType)
{
    return inType == reflection::BaseType::String;
}

inline bool IsIndexableType(reflection::BaseType inType)
{
    return IsScalarType(inType) || IsStringType(inType);
}

inline bool IsVectorType(reflection::BaseType inType)
{
    return inType == reflection::BaseType::Vector;
}

inline bool IsStringOrVectorType(reflection::BaseType inType)
{
    return IsStringType(inType) || IsVectorType(inType);
}

inline bool IsObjectType(reflection::BaseType inType)
{
    return inType == reflection::BaseType::Obj;
}

inline bool IsSupportTypeAtRoot(reflection::BaseType inType)
{
    return IsIndexableType(inType) || IsVectorType(inType) || IsObjectType(inType);
}

inline bool IsRequiredSupportType(reflection::BaseType inType)
{
    return IsStringOrVectorType(inType) || IsObjectType(inType);
}

inline bool IsConflict(bool deprecated, bool required)
{
    return deprecated && required;
}
}

int SchemaObject::FlatBufferSchema::ParseFlatBufferSchema(const std::string &inDecoded)
{
    description_.clear(); // For recovering from a fail parse
    // The upper logic had guaranteed that the inDecoded be verified OK
    auto schema = reflection::GetSizePrefixedSchema(inDecoded.c_str());
    CHECK_NULL_UNLIKELY_RETURN_ERROR(schema);
    auto rootTable = schema->root_table();
    if (rootTable == nullptr || rootTable->is_struct()) {
        LOGE("[FBSchema][Parse] Root table nullptr or is struct.");
        return -E_SCHEMA_PARSE_FAIL;
    }

    auto rootTableName = rootTable->name();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(rootTableName);
    description_ += ("RootTableName=" + SchemaUtils::StripNameSpace(rootTableName->str()) + ";");

    int errCode = ParseCheckRootTableAttribute(*rootTable);
    if (errCode != E_OK) {
        return errCode;
    }

    RawIndexInfos indexCollect;
    errCode = ParseCheckRootTableDefine(*schema, *rootTable, indexCollect);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = ParseCheckIndexes(indexCollect);
    if (errCode != E_OK) {
        return errCode;
    }
    return E_OK;
}

int SchemaObject::FlatBufferSchema::CompareFlatBufferDefine(const FlatBufferSchema &other) const
{
    // Schema had been parsed and constraint checked before this function is called, so as we assumed.
    // Here in the compare procedure, we only check null-point, do not check or suspect of constraint any more.
    auto selfSchema = GetSchema();
    auto otherSchema = other.GetSchema();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfSchema);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherSchema);
    auto selfRootTable = selfSchema->root_table();
    auto otherRootTable = otherSchema->root_table();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfRootTable);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherRootTable);
    auto selfRootTableName = selfRootTable->name();
    auto otherRootTableName = otherRootTable->name();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfRootTableName);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherRootTableName);

    std::string selfRootName = SchemaUtils::StripNameSpace(selfRootTableName->str());
    std::string otherRootName = SchemaUtils::StripNameSpace(otherRootTableName->str());
    if (selfRootName != otherRootName) {
        LOGE("[FBSchema][Compare] RootName differ, self=%s, other=%s.", selfRootName.c_str(), otherRootName.c_str());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // We don't have to compare rootTableAttribute or index here, they are done by SchemaObject
    std::set<std::string> comparedTypeNameSet;
    return CompareTableOrStructDefine({selfSchema, otherSchema}, {selfRootTable, otherRootTable}, true,
        comparedTypeNameSet);
}

namespace {
int CheckSizePrefixRawValue(const RawValue &inValue)
{
    if (inValue.first == nullptr) { // Unlikely
        return -E_INVALID_ARGS;
    }
    if (inValue.second <= SIZE_PREFIX_SIZE) {
        LOGE("[FBSchema][CheckSizePreValue] ValueSize=%u too short.", inValue.second);
        return -E_INVALID_ARGS;
    }
    auto realSize = flatbuffers::ReadScalar<flatbuffers::uoffset_t>(inValue.first);
    if (realSize != inValue.second - SIZE_PREFIX_SIZE) {
        LOGE("[FBSchema][CheckSizePreValue] RealSize=%u mismatch valueSize=(%u-4).", realSize, inValue.second);
        return -E_INVALID_ARGS;
    }
    return E_OK;
}
}

int SchemaObject::FlatBufferSchema::VerifyFlatBufferValue(const RawValue &inValue, bool tryNoSizePrefix) const
{
    (void)tryNoSizePrefix; // Use it in the future, currently we demand value is sizePrefixed
    int errCode = CheckSizePrefixRawValue(inValue);
    if (errCode != E_OK) {
        return errCode;
    }
    auto schema = GetSchema();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(schema);
    auto rootTable = schema->root_table();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(rootTable);
    if (!flatbuffers::Verify(*schema, *rootTable, inValue.first + SIZE_PREFIX_SIZE,
        inValue.second - SIZE_PREFIX_SIZE)) {
        return -E_FLATBUFFER_VERIFY_FAIL;
    }
    return E_OK;
}

namespace {
FieldType MapFieldType(reflection::BaseType inType)
{
    static std::map<reflection::BaseType, FieldType> fieldTypeMap{
        {reflection::BaseType::Bool, FieldType::LEAF_FIELD_BOOL},
        {reflection::BaseType::Byte, FieldType::LEAF_FIELD_INTEGER},
        {reflection::BaseType::UByte, FieldType::LEAF_FIELD_INTEGER},
        {reflection::BaseType::Short, FieldType::LEAF_FIELD_INTEGER},
        {reflection::BaseType::UShort, FieldType::LEAF_FIELD_INTEGER},
        {reflection::BaseType::Int, FieldType::LEAF_FIELD_INTEGER},
        {reflection::BaseType::UInt, FieldType::LEAF_FIELD_LONG},
        {reflection::BaseType::Long, FieldType::LEAF_FIELD_LONG},
        {reflection::BaseType::ULong, FieldType::LEAF_FIELD_DOUBLE},
        {reflection::BaseType::Float, FieldType::LEAF_FIELD_DOUBLE},
        {reflection::BaseType::Double, FieldType::LEAF_FIELD_DOUBLE},
        {reflection::BaseType::String, FieldType::LEAF_FIELD_STRING},
        {reflection::BaseType::Vector, FieldType::LEAF_FIELD_ARRAY},
        {reflection::BaseType::Obj, FieldType::INTERNAL_FIELD_OBJECT},
    };
    if (fieldTypeMap.count(inType) == 0) {
        return FieldType::LEAF_FIELD_NULL;
    }
    return fieldTypeMap[inType];
}

RawString CheckDollarDotAndSkipIt(RawString inPath)
{
    if (inPath == nullptr) {
        return nullptr;
    }
    auto pathStr = inPath;
    if (*pathStr++ != '$') {
        return nullptr;
    }
    if (*pathStr++ != '.') {
        return nullptr;
    }
    if (*pathStr == 0) {
        return nullptr;
    }
    return pathStr;
}

const reflection::Field *GetFieldInfoFromSchemaByPath(const reflection::Schema &schema, RawString pathStr)
{
    auto rootTable = schema.root_table();
    if (rootTable == nullptr) { // Unlikely
        return nullptr;
    }
    auto rootFields = rootTable->fields();
    if (rootFields == nullptr) { // Unlikely
        return nullptr;
    }
    // Unlikely to return nullptr, except internal-error happened
    return rootFields->LookupByKey(pathStr);
}

int DoVerifyBeforeExtract(const flatbuffers::Table &rootValue, const reflection::Field &fieldInfo,
    const flatbuffers::Verifier &verifier)
{
    auto type = fieldInfo.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(type);
    bool verifyResult = true;
    switch (type->base_type()) {
        case reflection::Bool:
        case reflection::Byte:
        case reflection::UByte:
            verifyResult = rootValue.VerifyField<int8_t>(verifier, fieldInfo.offset());
            break;
        case reflection::Short:
        case reflection::UShort:
            verifyResult = rootValue.VerifyField<int16_t>(verifier, fieldInfo.offset());
            break;
        case reflection::Int:
        case reflection::UInt:
            verifyResult = rootValue.VerifyField<int32_t>(verifier, fieldInfo.offset());
            break;
        case reflection::Long:
        case reflection::ULong:
            verifyResult = rootValue.VerifyField<uint64_t>(verifier, fieldInfo.offset());
            break;
        case reflection::Float:
            verifyResult = rootValue.VerifyField<float>(verifier, fieldInfo.offset());
            break;
        case reflection::Double:
            verifyResult = rootValue.VerifyField<double>(verifier, fieldInfo.offset());
            break;
        case reflection::String:
            verifyResult = rootValue.VerifyField<flatbuffers::uoffset_t>(verifier, fieldInfo.offset()) &&
                verifier.VerifyString(flatbuffers::GetFieldS(rootValue, fieldInfo)); // VerifyString can accept null
            break;
        default:
            return -E_NOT_SUPPORT;
    }
    return (verifyResult ? E_OK : -E_FLATBUFFER_VERIFY_FAIL);
}

inline std::string DoExtractString(const flatbuffers::Table &rootValue, const reflection::Field &fieldInfo)
{
    auto strVal = flatbuffers::GetFieldS(rootValue, fieldInfo);
    if (strVal == nullptr) {
        return "";
    }
    return strVal->str();
}

int DoExtractValue(const flatbuffers::Table &rootValue, const reflection::Field &fieldInfo, TypeValue &outExtract)
{
    auto type = fieldInfo.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(type);
    switch (type->base_type()) {
        case reflection::Bool:
            outExtract.second.boolValue = (flatbuffers::GetFieldI<int8_t>(rootValue, fieldInfo) != 0);
            break;
        case reflection::Byte:
            outExtract.second.integerValue = flatbuffers::GetFieldI<int8_t>(rootValue, fieldInfo);
            break;
        case reflection::UByte:
            outExtract.second.integerValue = flatbuffers::GetFieldI<uint8_t>(rootValue, fieldInfo);
            break;
        case reflection::Short:
            outExtract.second.integerValue = flatbuffers::GetFieldI<int16_t>(rootValue, fieldInfo);
            break;
        case reflection::UShort:
            outExtract.second.integerValue = flatbuffers::GetFieldI<uint16_t>(rootValue, fieldInfo);
            break;
        case reflection::Int:
            outExtract.second.integerValue = flatbuffers::GetFieldI<int32_t>(rootValue, fieldInfo);
            break;
        case reflection::UInt:
            outExtract.second.longValue = flatbuffers::GetFieldI<uint32_t>(rootValue, fieldInfo);
            break;
        case reflection::Long:
            outExtract.second.longValue = flatbuffers::GetFieldI<int64_t>(rootValue, fieldInfo);
            break;
        case reflection::ULong:
            outExtract.second.doubleValue = flatbuffers::GetFieldI<uint64_t>(rootValue, fieldInfo);
            break;
        case reflection::Float:
            outExtract.second.doubleValue = flatbuffers::GetFieldF<float>(rootValue, fieldInfo);
            break;
        case reflection::Double:
            outExtract.second.doubleValue = flatbuffers::GetFieldF<double>(rootValue, fieldInfo);
            break;
        case reflection::String:
            outExtract.second.stringValue = DoExtractString(rootValue, fieldInfo);
            break;
        default:
            return -E_NOT_SUPPORT;
    }
    return E_OK;
}

int ExtractFlatBufferValueFinal(const flatbuffers::Table &rootValue, const reflection::Field &fieldInfo,
    const flatbuffers::Verifier &verifier, TypeValue &outExtract)
{
    auto type = fieldInfo.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(type);
    auto baseType = type->base_type();
    if (!IsIndexableType(baseType)) {
        LOGE("[ExtractFinal] BaseType=%s not indexable.", reflection::EnumNameBaseType(baseType));
        return -E_NOT_SUPPORT;
    }
    outExtract.first = MapFieldType(type->base_type());
    int errCode = DoVerifyBeforeExtract(rootValue, fieldInfo, verifier);
    if (errCode != E_OK) {
        LOGE("[ExtractFinal] DoVerify fail, errCode=%d.", errCode);
        return errCode;
    }
    errCode = DoExtractValue(rootValue, fieldInfo, outExtract);
    if (errCode != E_OK) {
        LOGE("[ExtractFinal] DoExtract fail, errCode=%d.", errCode);
        return errCode;
    }
    return E_OK;
}
}

int SchemaObject::FlatBufferSchema::ExtractFlatBufferValue(RawString inPath, const RawValue &inValue,
    TypeValue &outExtract, bool tryNoSizePrefix) const
{
    // NOTE!!! This function is performance sensitive !!! Carefully not to allocate memory often!!!
    (void)tryNoSizePrefix; // Use it in the future, currently we demand value is sizePrefixed
    int errCode = CheckSizePrefixRawValue(inValue);
    if (errCode != E_OK) {
        return errCode;
    }
    auto pathStr = CheckDollarDotAndSkipIt(inPath);
    if (pathStr == nullptr) { // Unlikely
        LOGE("[FBSchema][Extract] inPath not begin with $. or nothing after it.");
        return -E_INVALID_ARGS;
    }
    auto schema = GetSchema();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(schema);
    // Currently we don't support nest-path
    auto fieldInfo = GetFieldInfoFromSchemaByPath(*schema, pathStr);
    if (fieldInfo == nullptr) {
        LOGE("[FBSchema][Extract] FieldInfo of path=%s not found.", pathStr);
        return -E_INTERNAL_ERROR;
    }
    // Begin extract, we have to minimal verify if we don't trust value from database
    auto valueRealBegin = inValue.first + SIZE_PREFIX_SIZE;
    auto valueRealSize = inValue.second - SIZE_PREFIX_SIZE;
    flatbuffers::Verifier verifier(valueRealBegin, valueRealSize);
    auto offset = verifier.VerifyOffset(0); // Attention: Verify root offset before we call GetAnyRoot
    if (offset == 0) {
        LOGE("[FBSchema][Extract] Verity root offset failed.");
        return -E_FLATBUFFER_VERIFY_FAIL;
    }
    auto rootValue = flatbuffers::GetAnyRoot(valueRealBegin);
    if (rootValue == nullptr) {
        LOGE("[FBSchema][Extract] Get rootTable from value fail.");
        return -E_INVALID_DATA;
    }
    // Verity vTable of rootTable before we extract anything from rootValue by reflection
    bool vTableOk = rootValue->VerifyTableStart(verifier);
    if (!vTableOk) {
        LOGE("[FBSchema][Extract] Verify vTable of rootTable of value fail.");
        return -E_FLATBUFFER_VERIFY_FAIL;
    }
    errCode = ExtractFlatBufferValueFinal(*rootValue, *fieldInfo, verifier, outExtract);
    if (errCode != E_OK) {
        return errCode;
    }
    verifier.EndTable();
    return E_OK;
}

const reflection::Schema *SchemaObject::FlatBufferSchema::GetSchema() const
{
    // This function is called after schemaString_ had been verified by flatbuffer
    return reflection::GetSizePrefixedSchema(owner_.schemaString_.c_str());
}

int SchemaObject::FlatBufferSchema::ParseCheckRootTableAttribute(const reflection::Object &rootTable)
{
    auto rootTableAttr = rootTable.attributes();
    if (rootTableAttr == nullptr) {
        LOGE("[FBSchema][ParseRootAttr] Root table no attribute.");
        return -E_SCHEMA_PARSE_FAIL;
    }

    auto versionAttr = rootTableAttr->LookupByKey(KEYWORD_SCHEMA_VERSION.c_str());
    if (!AttributeExistAndHasValue(versionAttr)) {
        LOGE("[FBSchema][ParseRootAttr] No SCHEMA_VERSION attribute or no value.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    if (SchemaUtils::Strip(versionAttr->value()->str()) != SCHEMA_SUPPORT_VERSION) {
        LOGE("[FBSchema][ParseRootAttr] Unexpect SCHEMA_VERSION=%s.", versionAttr->value()->c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    owner_.schemaVersion_ = SCHEMA_SUPPORT_VERSION;
    description_ += (KEYWORD_SCHEMA_VERSION + "=" + SCHEMA_SUPPORT_VERSION + ";");

    auto skipsizeAttr = rootTableAttr->LookupByKey(KEYWORD_SCHEMA_SKIPSIZE.c_str());
    if (!AttributeExistAndHasValue(skipsizeAttr)) {
        LOGI("[FBSchema][ParseRootAttr] No SCHEMA_SKIPSIZE attribute or no value.");
        owner_.schemaSkipSize_ = 0; // Default skipsize value
        return E_OK;
    }
    std::string skipsizeStr = SchemaUtils::Strip(skipsizeAttr->value()->str());
    int skipsizeInt = std::atoi(skipsizeStr.c_str()); // std::stoi will throw exception
    if (std::to_string(skipsizeInt) != skipsizeStr || skipsizeInt < 0 ||
        static_cast<uint32_t>(skipsizeInt) > SCHEMA_SKIPSIZE_MAX) {
        LOGE("[FBSchema][ParseRootAttr] Unexpect SCHEMA_SKIPSIZE value=%s.", skipsizeAttr->value()->c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    owner_.schemaSkipSize_ = static_cast<uint32_t>(skipsizeInt);
    description_ += (KEYWORD_SCHEMA_SKIPSIZE + "=" + skipsizeStr + ";");
    return E_OK;
}

int SchemaObject::FlatBufferSchema::ParseCheckRootTableDefine(const reflection::Schema &schema,
    const reflection::Object &rootTable, RawIndexInfos &indexCollect)
{
    // Clear schemaDefine_ to recover from a fail parse
    owner_.schemaDefine_.clear();
    auto fields = rootTable.fields();
    if (fields == nullptr || fields->size() == 0) {
        LOGE("[FBSchema][ParseRootDefine] Empty define.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    for (uint32_t i = 0; i < fields->size(); i++) {
        auto eachField = (*fields)[i];
        CHECK_NULL_UNLIKELY_RETURN_ERROR(eachField);

        auto name = eachField->name();
        CHECK_NULL_UNLIKELY_RETURN_ERROR(name);
        int errCode = SchemaUtils::CheckFieldName(name->str());
        if (errCode != E_OK) {
            LOGE("[FBSchema][ParseRootDefine] Invalid fieldName=%s, errCode=%d.", name->c_str(), errCode);
            return -E_SCHEMA_PARSE_FAIL;
        }
        FieldPath path{name->str()};
        if (owner_.schemaDefine_[ROOT_DEFINE_DEPTH].count(path) != 0) { // Unlikely
            LOGE("[FBSchema][ParseRootDefine] FieldPath=%s already exist at root.", name->c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }

        errCode = ParseCheckFieldInfo(schema, *eachField, path, indexCollect);
        if (errCode != E_OK) {
            LOGE("[FBSchema][ParseRootDefine] ParseFieldInfo errCode=%d, FieldPath=%s.", errCode,
                SchemaUtils::FieldPathString(path).c_str());
            return errCode;
        }
    }
    uint32_t fieldPathCount = 0;
    for (uint32_t depth = ROOT_DEFINE_DEPTH; depth < SCHEMA_FEILD_PATH_DEPTH_MAX; depth++) {
        if (owner_.schemaDefine_.count(depth) != 0) {
            fieldPathCount += owner_.schemaDefine_[depth].size();
        }
    }
    if (fieldPathCount > SCHEMA_FEILD_NAME_COUNT_MAX) {
        LOGE("[FBSchema][ParseRootDefine] FieldPath count=%u exceed the limitation.", fieldPathCount);
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}

namespace {
bool CheckFieldTypeSupport(const reflection::Type &inType, bool isRootField)
{
    auto baseType = inType.base_type();
    if (isRootField) {
        if (!IsSupportTypeAtRoot(baseType)) {
            LOGE("[FBSchema][DecideType] BaseType=%s not support at root.", reflection::EnumNameBaseType(baseType));
            return false;
        }
        if (IsIntegerType(baseType) && (inType.index() != INDEX_OF_NOT_ENUM)) {
            LOGE("[FBSchema][DecideType] BaseType=%s is enum, not support.", reflection::EnumNameBaseType(baseType));
            return false;
        }
        if (IsVectorType(baseType)) {
            auto elementType = inType.element();
            if (!IsIndexableType(elementType)) {
                LOGE("[FBSchema][DecideType] ElementType=%s not support for vector.",
                    reflection::EnumNameBaseType(elementType));
                return false;
            }
        }
    } else {
        // Currently only support nest in Struct, support only scalar and nest-struct
        if (!IsScalarType(baseType) && !IsObjectType(baseType)) {
            LOGE("[FBSchema][DecideType] BaseType=%s not support for struct.", reflection::EnumNameBaseType(baseType));
            return false;
        }
    }
    return true;
}
}

int SchemaObject::FlatBufferSchema::ParseCheckFieldInfo(const reflection::Schema &schema,
    const reflection::Field &field, const FieldPath &path, RawIndexInfos &indexCollect)
{
    if (path.empty() || path.size() > SCHEMA_FEILD_PATH_DEPTH_MAX) {
        LOGE("[FBSchema][ParseField] FieldPath size=%zu invalid.", path.size());
        return -E_SCHEMA_PARSE_FAIL;
    }
    uint32_t depth = path.size() - 1; // Depth count from zero
    bool isRootField = (depth == ROOT_DEFINE_DEPTH);
    SchemaAttribute &fieldInfo = owner_.schemaDefine_[depth][path]; // Create new entry in schemaDefine_

    auto type = field.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(type);
    if (!CheckFieldTypeSupport(*type, isRootField)) {
        return -E_SCHEMA_PARSE_FAIL;
    }
    auto baseType = type->base_type();
    // Only type and isIndexable of SchemaAttribute is necessary
    fieldInfo.type = MapFieldType(baseType);
    fieldInfo.isIndexable = (IsIndexableType(baseType) && isRootField);
    description_ += (SchemaUtils::FieldPathString(path) + "=" + reflection::EnumNameBaseType(baseType) + ";");

    if (IsRequiredSupportType(baseType)) {
        if (IsConflict(field.deprecated(), field.required())) {
            LOGE("[FBSchema][ParseField] Deprecated conflict with required.");
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    if (fieldInfo.isIndexable) {
        CollectRawIndexInfos(field, indexCollect);
    }
    if (IsObjectType(baseType)) {
        int errCode = ParseCheckStructDefine(schema, field, path);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

void SchemaObject::FlatBufferSchema::CollectRawIndexInfos(const reflection::Field &field,
    RawIndexInfos &indexCollect) const
{
    auto name = field.name();
    if (name == nullptr) { // Not possible
        return;
    }
    auto fieldAttr = field.attributes();
    if (fieldAttr == nullptr) {
        return;
    }
    auto indexAttr = fieldAttr->LookupByKey(KEYWORD_INDEX.c_str());
    if (indexAttr == nullptr) {
        return;
    }
    if (indexAttr->value() == nullptr) {
        indexCollect[name->str()] = ""; // Must be SingleField-Index
        return;
    }
    indexCollect[name->str()] = indexAttr->value()->str(); // May still be empty string
}

int SchemaObject::FlatBufferSchema::ParseCheckStructDefine(const reflection::Schema &schema,
    const reflection::Field &field, const FieldPath &path)
{
    if (path.size() >= SCHEMA_FEILD_PATH_DEPTH_MAX) {
        LOGE("[FBSchema][ParseStruct] Struct define at depth limitation.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    auto objects = schema.objects();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(objects);
    auto type = field.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(type);
    auto objIndex = type->index();
    if (objIndex < 0 || static_cast<uint32_t>(objIndex) >= objects->size()) { // Unlikely
        return -E_INTERNAL_ERROR;
    }
    auto structObj = (*objects)[objIndex];
    CHECK_NULL_UNLIKELY_RETURN_ERROR(structObj);
    auto structName = structObj->name();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(structName);
    if (!structObj->is_struct()) {
        LOGE("[FBSchema][ParseStruct] Nest table=%s not support.", structName->c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    description_ += ("StructName=" + SchemaUtils::StripNameSpace(structName->str()) + ";");

    // Parse fields
    auto structFields = structObj->fields();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(structFields);
    // Flatbuffer guarantee that struct will not be empty size, even if it is empty, we just ignore it
    for (uint32_t i = 0; i < structFields->size(); i++) {
        auto eachField = (*structFields)[i];
        CHECK_NULL_UNLIKELY_RETURN_ERROR(eachField);
        auto eachName = eachField->name();
        CHECK_NULL_UNLIKELY_RETURN_ERROR(eachName);
        int errCode = SchemaUtils::CheckFieldName(eachName->str());
        if (errCode != E_OK) {
            LOGE("[FBSchema][ParseStruct] Invalid fieldName=%s, errCode=%d.", eachName->c_str(), errCode);
            return -E_SCHEMA_PARSE_FAIL;
        }
        FieldPath eachPath = path;
        eachPath.push_back(eachName->str());
        RawIndexInfos notUsed;
        errCode = ParseCheckFieldInfo(schema, *eachField, eachPath, notUsed);
        if (errCode != E_OK) {
            LOGE("[FBSchema][ParseStruct] ParseFieldInfo errCode=%d, FieldPath=%s.", errCode,
                SchemaUtils::FieldPathString(eachPath).c_str());
            return errCode;
        }
    }
    return E_OK;
}

namespace {
inline bool IsNotCompositeIndex(const std::string &indexStr)
{
    if (indexStr.empty()) {
        return true;
    }
    if (indexStr == std::string("0")) { // In fact, test found that attrValue will be "0" if not exist
        return true;
    }
    return false;
}
}

int SchemaObject::FlatBufferSchema::ParseCheckIndexes(const RawIndexInfos &indexCollect)
{
    for (const auto &entry : indexCollect) {
        std::vector<std::string> indexStrArray{entry.first}; // Entry.first is fieldName at root that was checked valid
        const std::string &rawIndexStr = entry.second;
        if (IsNotCompositeIndex(rawIndexStr)) {
            int errCode = owner_.ParseCheckEachIndexFromStringArray(indexStrArray);
            if (errCode != E_OK) {
                LOGE("[FBSchema][ParseIndex] Create single-index=%s fail, errCode=%d.", entry.first.c_str(), errCode);
                return errCode;
            }
            description_ += ("INDEX=" + entry.first + ";");
            continue;
        }
        // Parse other indexField
        for (uint32_t curPos = 0; curPos < rawIndexStr.size();) {
            uint32_t nextCommaPos = rawIndexStr.find_first_of(',', curPos);
            std::string eachIndexField = rawIndexStr.substr(curPos, nextCommaPos - curPos);
            eachIndexField = SchemaUtils::Strip(eachIndexField);
            if (!eachIndexField.empty()) { // Continuous ',' just ignore
                indexStrArray.push_back(eachIndexField);
            }
            if (nextCommaPos >= rawIndexStr.size()) { // No ',' anymore
                break;
            }
            curPos = nextCommaPos + 1;
        }
        int errCode = owner_.ParseCheckEachIndexFromStringArray(indexStrArray);
        if (errCode != E_OK) {
            LOGE("[FBSchema][ParseIndex] Create composite-index=%s, rawStr=%s fail, errCode=%d.", entry.first.c_str(),
                rawIndexStr.c_str(), errCode);
            return errCode;
        }
        description_ += ("INDEX=" + entry.first + ";");
    }
    if (owner_.schemaIndexes_.size() > SCHEMA_INDEX_COUNT_MAX) {
        LOGE("[FBSchema][ParseIndex] Index count=%zu exceed limitation.", owner_.schemaIndexes_.size());
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}

namespace {
inline bool IsNotEqualNotCompatible(int errCode)
{
    return (errCode != -E_SCHEMA_EQUAL_EXACTLY) && (errCode != -E_SCHEMA_UNEQUAL_COMPATIBLE) &&
        (errCode != -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE);
}

int CompareFieldCount(bool isRoot, uint32_t selfCount, uint32_t otherCount)
{
    if (isRoot) {
        if (otherCount < selfCount) {
            LOGE("[FBSchema][CompareRoot] RootFieldSize: other=%u less than self=%u.", otherCount, selfCount);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    } else {
        if (selfCount != otherCount) {
            LOGE("[FBSchema][CompareRoot] StructFieldSize: self=%u differ with other=%u.", selfCount, otherCount);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    return (selfCount == otherCount) ? -E_SCHEMA_EQUAL_EXACTLY : -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE;
}

int CompareFieldInfoBesideType(const reflection::Field &selfField, const reflection::Field &otherField,
    reflection::BaseType theType)
{
    // Compare offset
    if (selfField.offset() != otherField.offset()) {
        LOGE("[FBSchema][CompareField] Offset differ: self=%u, other=%u.", selfField.offset(), otherField.offset());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Compare default value
    if (selfField.default_integer() != otherField.default_integer()) {
        LOGE("[FBSchema][CompareField] DefaultInteger differ: self=%lld, other=%lld.",
            static_cast<long long>(selfField.default_integer()), static_cast<long long>(otherField.default_integer()));
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // QUEER: for the same default_real value in fbs, flatbuffer will generate different value in binary ???
    if (!IsDoubleNearlyEqual(selfField.default_real(), otherField.default_real())) {
        LOGE("[FBSchema][CompareField] DefaultReal differ: self=%f, other=%f.", selfField.default_real(),
            otherField.default_real());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Ignore deprecated, Compare required
    if (IsRequiredSupportType(theType)) {
        if (selfField.required() != otherField.required()) {
            LOGE("[FBSchema][CompareField] Require differ: self=%d, other=%d.", selfField.required(),
                otherField.required());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    return -E_SCHEMA_EQUAL_EXACTLY;
}

int CompareFieldInfo(const reflection::Field &selfField, const reflection::Field &otherField, bool &isStruct)
{
    auto selfType = selfField.type();
    auto otherType = otherField.type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfType);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherType);
    // Compare type
    auto selfBaseType = selfType->base_type();
    auto otherBaseType = otherType->base_type();
    if (selfBaseType != otherBaseType) {
        LOGE("[FBSchema][CompareField] BaseType differ: self=%s, other=%s.", reflection::EnumNameBaseType(selfBaseType),
            reflection::EnumNameBaseType(otherBaseType));
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    if (IsVectorType(selfBaseType)) {
        auto selfElementType = selfType->element();
        auto otherElementType = otherType->element();
        if (selfElementType != otherElementType) {
            LOGE("[FBSchema][CompareField] ElementType differ: self=%u, other=%u.", selfElementType, otherElementType);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    if (IsObjectType(selfBaseType)) {
        isStruct = true;
    }
    return CompareFieldInfoBesideType(selfField, otherField, selfBaseType);
}

// Split from original functions which would be longer than 50 line
int CompareExtraField(const PairConstPointer<reflection::Object> &bothObject)
{
    // This is private function, the caller guarantee that inputParameter not nullptr
    auto selfFields = bothObject.first->fields();
    auto otherFields = bothObject.second->fields();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfFields);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherFields);
    // Each field in other not in self, should not be required
    for (uint32_t i = 0; i < otherFields->size(); i++) {
        auto eachOtherField = (*otherFields)[i];
        CHECK_NULL_UNLIKELY_RETURN_ERROR(eachOtherField);
        auto otherName = eachOtherField->name();
        CHECK_NULL_UNLIKELY_RETURN_ERROR(otherName);
        auto correspondSelfField = selfFields->LookupByKey(otherName->c_str());
        if (correspondSelfField != nullptr) {
            continue;
        }
        if (eachOtherField->required()) {
            LOGE("[FBSchema][CompareDefine] Extra field=%s should not be required.", otherName->c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    return -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE;
}
}

int SchemaObject::FlatBufferSchema::CompareTableOrStructDefine(const PairConstPointer<reflection::Schema> &bothSchema,
    const PairConstPointer<reflection::Object> &bothObject, bool isRoot, std::set<std::string> &compared) const
{
    // This is private function, the caller guarantee that inputParameter not nullptr
    auto selfFields = bothObject.first->fields();
    auto otherFields = bothObject.second->fields();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfFields);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherFields);
    int errCode = CompareFieldCount(isRoot, selfFields->size(), otherFields->size());
    if (errCode == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return errCode;
    }
    // Each field in self should be in other, and they should be same
    for (uint32_t i = 0; i < selfFields->size(); i++) {
        auto eachSelfField = (*selfFields)[i];
        CHECK_NULL_UNLIKELY_RETURN_ERROR(eachSelfField);
        auto selfName = eachSelfField->name();
        CHECK_NULL_UNLIKELY_RETURN_ERROR(selfName);
        auto correspondOtherField = otherFields->LookupByKey(selfName->c_str());
        if (correspondOtherField == nullptr) {
            LOGE("[FBSchema][CompareDefine] SelfField=%s not found in other.", selfName->c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
        bool isStruct = false;
        errCode = CompareFieldInfo(*eachSelfField, *correspondOtherField, isStruct);
        if (IsNotEqualNotCompatible(errCode)) {
            LOGE("[FBSchema][CompareDefine] Compare info of field=%s fail, errCode=%d.", selfName->c_str(), errCode);
            return errCode;
        }
        if (isStruct) {
            // Previous parse guarantee that recursion will not be unlimited, don't be afraid.
            errCode = CompareStruct(bothSchema, {eachSelfField, correspondOtherField}, compared);
            if (IsNotEqualNotCompatible(errCode)) {
                return errCode;
            }
        }
    }
    if (selfFields->size() == otherFields->size()) {
        return -E_SCHEMA_EQUAL_EXACTLY;
    }
    return CompareExtraField(bothObject);
}

int SchemaObject::FlatBufferSchema::CompareStruct(const PairConstPointer<reflection::Schema> &bothSchema,
    const PairConstPointer<reflection::Field> &bothField, std::set<std::string> &compared) const
{
    // This is private function, the caller guarantee that inputParameter not nullptr
    auto selfObjects = bothSchema.first->objects();
    auto otherObjects = bothSchema.second->objects();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfObjects);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherObjects);
    auto selfType = bothField.first->type();
    auto otherType = bothField.second->type();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfType);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherType);
    auto selfObjIndex = selfType->index();
    auto otherObjIndex = otherType->index();
    if (selfObjIndex < 0 || static_cast<uint32_t>(selfObjIndex) >= selfObjects->size()) { // Unlikely
        return -E_INTERNAL_ERROR;
    }
    if (otherObjIndex < 0 || static_cast<uint32_t>(otherObjIndex) >= otherObjects->size()) { // Unlikely
        return -E_INTERNAL_ERROR;
    }
    auto selfStructObj = (*selfObjects)[selfObjIndex];
    auto otherStructObj = (*otherObjects)[otherObjIndex];
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfStructObj);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherStructObj);
    // Previous parse can guarantee that they are both struct, no need to check again
    auto selfStructName = selfStructObj->name();
    auto otherStructName = otherStructObj->name();
    CHECK_NULL_UNLIKELY_RETURN_ERROR(selfStructName);
    CHECK_NULL_UNLIKELY_RETURN_ERROR(otherStructName);
    std::string selfName = SchemaUtils::StripNameSpace(selfStructName->str());
    std::string otherName = SchemaUtils::StripNameSpace(otherStructName->str());
    if (selfName != otherName) {
        LOGE("[FBSchema][CompareStruct] The field is not of same struct type, self=%s, other=%s.",
            selfName.c_str(), otherName.c_str());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    if (compared.count(selfName) != 0) { // This struct-type had already been compared, no need to do recurse again
        return -E_SCHEMA_EQUAL_EXACTLY;
    }
    compared.insert(selfName);
    // Compare struct detail
    if (selfStructObj->minalign() != otherStructObj->minalign()) {
        LOGE("[FBSchema][CompareStruct] The struct minalign differ, self=%d, other=%d.",
            selfStructObj->minalign(), otherStructObj->minalign());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    if (selfStructObj->bytesize() != otherStructObj->bytesize()) {
        LOGE("[FBSchema][CompareStruct] The struct bytesize differ, self=%d, other=%d.",
            selfStructObj->bytesize(), otherStructObj->bytesize());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Previous parse guarantee that recursion will not be unlimited, don't be afraid.
    return CompareTableOrStructDefine(bothSchema, {selfStructObj, otherStructObj}, false, compared);
}
#else // OMIT_FLATBUFFER
bool SchemaObject::FlatBufferSchema::IsFlatBufferSchema(const std::string &inOriginal, std::string &outDecoded)
{
    LOGW("FlatBuffer Omit From Compile.");
    return false;
}

int SchemaObject::FlatBufferSchema::ParseFlatBufferSchema(const std::string &inDecoded)
{
    owner_.schemaType_ = SchemaType::FLATBUFFER; // For fix compile warning
    return -E_NOT_PERMIT;
}

int SchemaObject::FlatBufferSchema::CompareFlatBufferDefine(const FlatBufferSchema &other) const
{
    return -E_NOT_PERMIT;
}

int SchemaObject::FlatBufferSchema::VerifyFlatBufferValue(const RawValue &inValue, bool tryNoSizePrefix) const
{
    return -E_NOT_PERMIT;
}

int SchemaObject::FlatBufferSchema::ExtractFlatBufferValue(RawString inPath, const RawValue &inValue,
    TypeValue &outExtract, bool tryNoSizePrefix) const
{
    return -E_NOT_PERMIT;
}
#endif // OMIT_FLATBUFFER
} // namespace DistributedDB
