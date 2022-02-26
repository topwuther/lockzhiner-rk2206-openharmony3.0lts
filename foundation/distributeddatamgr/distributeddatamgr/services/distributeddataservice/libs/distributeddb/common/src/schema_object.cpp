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
#include "schema_utils.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
namespace {
const std::string JSON_EXTRACT_FUNC_NAME = "json_extract_by_path";
const std::string FLATBUFFER_EXTRACT_FUNC_NAME = "flatbuffer_extract_by_path";

// For Json-Schema, display its original content before parse. For FlatBuffer-Schema, only display its parsed content.
void DisplaySchemaLineByLine(SchemaType inType, const std::string &inSchema)
{
    constexpr uint32_t lengthPerLine = 400; // 400 char per line
    constexpr uint32_t usualMaxLine = 25; // For normal schema, 25 line for 10k length is quite enough
    LOGD("[Schema][Display] IS %s, LENGTH=%zu.", SchemaUtils::SchemaTypeString(inType).c_str(), inSchema.size());
    uint32_t totalLine = (inSchema.size() + lengthPerLine - 1) / lengthPerLine;
    for (uint32_t line = 0; line < totalLine; line++) {
        if (line >= usualMaxLine) {
            LOGD("......(UNCOMPLETED SCHEMA)");
            break;
        }
        std::string lineStr = inSchema.substr(line * lengthPerLine, lengthPerLine);
        LOGD("%s", lineStr.c_str());
    }
}
}

std::string SchemaObject::GetExtractFuncName(SchemaType inSchemaType)
{
    if (inSchemaType == SchemaType::JSON) {
        return JSON_EXTRACT_FUNC_NAME;
    } else {
        return FLATBUFFER_EXTRACT_FUNC_NAME;
    }
}

std::string SchemaObject::GenerateExtractSQL(SchemaType inSchemaType, const FieldPath &inFieldpath,
    FieldType inFieldType, uint32_t skipSize)
{
    static std::map<FieldType, std::string> fieldTypeMapSQLiteType {
        {FieldType::LEAF_FIELD_BOOL, "INT"},
        {FieldType::LEAF_FIELD_INTEGER, "INT"},
        {FieldType::LEAF_FIELD_LONG, "INT"},
        {FieldType::LEAF_FIELD_DOUBLE, "REAL"},
        {FieldType::LEAF_FIELD_STRING, "TEXT"},
    };
    if (inFieldpath.empty()) {
        LOGE("[Schema][GenExtract] Path empty.");
        return "";
    }
    if (fieldTypeMapSQLiteType.count(inFieldType) == 0) {
        LOGE("[Schema][GenExtract] FieldType not support.");
        return "";
    }
    std::string resultSql = " CAST("; // Reserve blank at begin for convenience.
    resultSql += GetExtractFuncName(inSchemaType);
    resultSql += "(value, '";
    resultSql += SchemaUtils::FieldPathString(inFieldpath);
    resultSql += "', ";
    resultSql += std::to_string(skipSize);
    resultSql += ") AS ";
    resultSql += fieldTypeMapSQLiteType[inFieldType];
    resultSql += ") "; // Reserve blank at end for convenience.
    return resultSql;
}

namespace {
inline SchemaType ReadSchemaType(uint8_t inType)
{
    if (inType >= static_cast<uint8_t>(SchemaType::UNRECOGNIZED)) {
        return SchemaType::UNRECOGNIZED;
    }
    return static_cast<SchemaType>(inType);
}
}

// Some principle in current version describe below. (Relative-type will be introduced in future but not involved now)
// 1.   PermitSync: Be false may because schemaType-unrecognized, schemaType-different, schema-unparsable,
//      schemaVersion-unrecognized, schema-incompatible, and so on.
// 2.   RequirePeerConvert: Be true normally when permitSync false, for future possible sync and convert(by remote).
// 3.   checkOnReceive: Be false when local is KV-DB, or when local is not KV-DB only if schema type equal as well as
//      define equal or remote is the upgradation of local.
SyncOpinion SchemaObject::MakeLocalSyncOpinion(const SchemaObject &localSchema, const std::string &remoteSchema,
    uint8_t remoteSchemaType)
{
    SchemaType localType = localSchema.GetSchemaType(); // An invalid schemaObject will return SchemaType::NONE
    SchemaType remoteType = ReadSchemaType(remoteSchemaType);
    // Logic below only be correct in current version, should be redesigned if new type added in the future
    // 1. If remote-type unrecognized(Include Relative-type), Do not permit sync.
    if (remoteType == SchemaType::UNRECOGNIZED) {
        LOGE("[Schema][Opinion] Remote-type=%u unrecognized.", remoteSchemaType);
        return SyncOpinion{false, true, true};
    }
    // 2. If local-type is KV(Here remote-type is within recognized), Always permit sync.
    if (localType == SchemaType::NONE) {
        LOGI("[Schema][Opinion] Local-type KV.");
        return SyncOpinion{true, false, false};
    }
    // 3. If remote-type is KV(Here local-type can only be JSON or FLATBUFFER), Always permit sync but need check.
    if (remoteType == SchemaType::NONE) {
        LOGI("[Schema][Opinion] Remote-type KV.");
        return SyncOpinion{true, false, true};
    }
    // 4. If local-type differ with remote-type(Here both type can only be JSON or FLATBUFFER), Do not permit sync.
    if (localType != remoteType) {
        LOGE("[Schema][Opinion] Local-type=%s differ remote-type=%s.", SchemaUtils::SchemaTypeString(localType).c_str(),
            SchemaUtils::SchemaTypeString(remoteType).c_str());
        return SyncOpinion{false, true, true};
    }
    // 5. If schema parse fail, Do not permit sync.
    SchemaObject remoteSchemaObj;
    int errCode = remoteSchemaObj.ParseFromSchemaString(remoteSchema);
    if (errCode != E_OK) {
        LOGE("[Schema][Opinion] Parse remote-schema fail, errCode=%d, remote-type=%s.", errCode,
            SchemaUtils::SchemaTypeString(remoteType).c_str());
        return SyncOpinion{false, true, true};
    }
    // 6. If remote-schema is not incompatible based on local-schema(SchemaDefine Equal), Permit sync and don't check.
    errCode = localSchema.CompareAgainstSchemaObject(remoteSchemaObj);
    if (errCode != -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return SyncOpinion{true, false, false};
    }
    // 7. If local-schema is not incompatible based on remote-schema(Can only be COMPATIBLE_UPGRADE), Sync and check.
    errCode = remoteSchemaObj.CompareAgainstSchemaObject(localSchema);
    if (errCode != -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return SyncOpinion{true, false, true};
    }
    // 8. Local-schema incompatible with remote-schema mutually.
    LOGE("[Schema][Opinion] Local-schema incompatible with remote-schema mutually.");
    return SyncOpinion{false, true, true};
}

SyncStrategy SchemaObject::ConcludeSyncStrategy(const SyncOpinion &localOpinion, const SyncOpinion &remoteOpinion)
{
    SyncStrategy outStrategy;
    // Any side permit sync, the final conclusion is permit sync.
    outStrategy.permitSync = (localOpinion.permitSync || remoteOpinion.permitSync);
    bool convertConflict = (localOpinion.requirePeerConvert && remoteOpinion.requirePeerConvert);
    if (convertConflict) {
        outStrategy.permitSync = false;
    }
    // Responsible for conversion on send now that local do not require remote to do conversion
    outStrategy.convertOnSend = (!localOpinion.requirePeerConvert);
    // Responsible for conversion on receive since remote will not do conversion on send and require local to convert
    outStrategy.convertOnReceive = remoteOpinion.requirePeerConvert;
    // Only depend on local opinion
    outStrategy.checkOnReceive = localOpinion.checkOnReceive;
    LOGI("[Schema][Strategy] PermitSync=%d, SendConvert=%d, ReceiveConvert=%d, ReceiveCheck=%d.",
        outStrategy.permitSync, outStrategy.convertOnSend, outStrategy.convertOnReceive, outStrategy.checkOnReceive);
    return outStrategy;
}

SchemaObject::SchemaObject() : flatbufferSchema_(*this) {};

SchemaObject::SchemaObject(const SchemaObject &other)
    : flatbufferSchema_(*this)
{
    isValid_ = other.isValid_;
    schemaType_ = other.schemaType_;
    schemaString_ = other.schemaString_;
    schemaVersion_ = other.schemaVersion_;
    schemaMode_ = other.schemaMode_;
    schemaSkipSize_ = other.schemaSkipSize_;
    schemaIndexes_ = other.schemaIndexes_;
    schemaDefine_ = other.schemaDefine_;
}

SchemaObject& SchemaObject::operator=(const SchemaObject &other)
{
    if (&other != this) {
        isValid_ = other.isValid_;
        schemaType_ = other.schemaType_;
        flatbufferSchema_.CopyFrom(other.flatbufferSchema_);
        schemaString_ = other.schemaString_;
        schemaVersion_ = other.schemaVersion_;
        schemaMode_ = other.schemaMode_;
        schemaSkipSize_ = other.schemaSkipSize_;
        schemaIndexes_ = other.schemaIndexes_;
        schemaDefine_ = other.schemaDefine_;
    }
    return *this;
}

int SchemaObject::ParseFromSchemaString(const std::string &inSchemaString)
{
    if (isValid_) {
        return -E_NOT_PERMIT;
    }

    // Judge whether it is FlatBuffer-Schema then check the schema-size first
    SchemaType estimateType = SchemaType::JSON; // Estimate as JSON type firstly
    std::string decoded;
    if (FlatBufferSchema::IsFlatBufferSchema(inSchemaString, decoded)) {
        estimateType = SchemaType::FLATBUFFER;
        LOGD("[Schema][Parse] FlatBuffer-Type, Decode before=%zu, after=%zu.", inSchemaString.size(), decoded.size());
    }
    const std::string &oriSchema = ((estimateType == SchemaType::FLATBUFFER) ? decoded : inSchemaString);
    if (oriSchema.size() > SCHEMA_STRING_SIZE_LIMIT) {
        LOGE("[Schema][Parse] SchemaSize=%zu Too Large.", oriSchema.size());
        return -E_INVALID_ARGS;
    }

    // Parse the corresponding type schema
    if (estimateType == SchemaType::FLATBUFFER) {
        int errCode = flatbufferSchema_.ParseFlatBufferSchema(oriSchema);
        if (errCode != E_OK) {
            return errCode;
        }
        DisplaySchemaLineByLine(SchemaType::FLATBUFFER, flatbufferSchema_.GetDescription());
        schemaType_ = SchemaType::FLATBUFFER;
        schemaString_ = oriSchema;
    } else {
        DisplaySchemaLineByLine(SchemaType::JSON, oriSchema);
        JsonObject schemaJson;
        int errCode = schemaJson.Parse(oriSchema);
        if (errCode != E_OK) {
            LOGE("[Schema][Parse] Json parse schema fail, errCode=%d, Not FlatBuffer Not Json.", errCode);
            return errCode;
        }
        errCode = ParseJsonSchema(schemaJson);
        if (errCode != E_OK) {
            return errCode;
        }
        schemaType_ = SchemaType::JSON;
        schemaString_ = schemaJson.ToString(); // Save the minify type of version string
    }

    isValid_ = true;
    return E_OK;
}

bool SchemaObject::IsSchemaValid() const
{
    return isValid_;
}

SchemaType SchemaObject::GetSchemaType() const
{
    return schemaType_;
}

std::string SchemaObject::ToSchemaString() const
{
    return schemaString_;
}

uint32_t SchemaObject::GetSkipSize() const
{
    return schemaSkipSize_;
}

std::map<IndexName, IndexInfo> SchemaObject::GetIndexInfo() const
{
    if (!isValid_) {
        // An invalid SchemaObject may contain some dirty info produced by failed parse.
        return std::map<IndexName, IndexInfo>();
    }
    return schemaIndexes_;
}

bool SchemaObject::IsIndexExist(const IndexName &indexName) const
{
    if (!isValid_) {
        return false;
    }
    return (schemaIndexes_.count(indexName) != 0);
}

int SchemaObject::CheckQueryableAndGetFieldType(const FieldPath &inPath, FieldType &outType) const
{
    if (inPath.empty()) {
        return -E_INVALID_ARGS;
    }
    if (schemaDefine_.count(inPath.size() - 1) == 0) {
        return -E_NOT_FOUND;
    }
    if (schemaDefine_.at(inPath.size() - 1).count(inPath) == 0) {
        return -E_NOT_FOUND;
    }
    const SchemaAttribute &targetAttr = schemaDefine_.at(inPath.size() - 1).at(inPath);
    outType = targetAttr.type;
    return (targetAttr.isIndexable ? E_OK : -E_NOT_SUPPORT);
}

int SchemaObject::CompareAgainstSchemaString(const std::string &inSchemaString) const
{
    IndexDifference indexDiffer;
    return CompareAgainstSchemaString(inSchemaString, indexDiffer);
}

int SchemaObject::CompareAgainstSchemaString(const std::string &inSchemaString, IndexDifference &indexDiffer) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    SchemaObject newSchema;
    int errCode = newSchema.ParseFromSchemaString(inSchemaString);
    if (errCode != E_OK) {
        return errCode;
    }
    return CompareAgainstSchemaObject(newSchema, indexDiffer);
}

int SchemaObject::CompareAgainstSchemaObject(const SchemaObject &inSchemaObject) const
{
    IndexDifference indexDiffer;
    return CompareAgainstSchemaObject(inSchemaObject, indexDiffer);
}

int SchemaObject::CompareAgainstSchemaObject(const SchemaObject &inSchemaObject, IndexDifference &indexDiffer) const
{
    if (!isValid_ || !inSchemaObject.isValid_) {
        return -E_NOT_PERMIT;
    }
    if (schemaType_ != inSchemaObject.schemaType_) {
        LOGE("[Schema][Compare] Self is %s, other is %s.", SchemaUtils::SchemaTypeString(schemaType_).c_str(),
            SchemaUtils::SchemaTypeString(inSchemaObject.schemaType_).c_str());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }

    // Return E_SCHEMA_EQUAL_EXACTLY or E_SCHEMA_UNEQUAL_INCOMPATIBLE
    int verModeResult = CompareSchemaVersionMode(inSchemaObject);
    if (verModeResult == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return verModeResult;
    }

    // Return E_SCHEMA_EQUAL_EXACTLY or E_SCHEMA_UNEQUAL_INCOMPATIBLE
    int skipSizeResult = CompareSchemaSkipSize(inSchemaObject);
    if (skipSizeResult == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return skipSizeResult;
    }

    // Return E_SCHEMA_EQUAL_EXACTLY or E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE or E_SCHEMA_UNEQUAL_INCOMPATIBLE
    int defineResult;
    if (schemaType_ == SchemaType::JSON) {
        defineResult = CompareSchemaDefine(inSchemaObject);
    } else {
        defineResult = flatbufferSchema_.CompareFlatBufferDefine(inSchemaObject.flatbufferSchema_);
    }
    if (defineResult == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        return defineResult;
    }

    // Return E_SCHEMA_EQUAL_EXACTLY or E_SCHEMA_UNEQUAL_COMPATIBLE
    int indexResult = CompareSchemaIndexes(inSchemaObject, indexDiffer);
    return ((defineResult == -E_SCHEMA_EQUAL_EXACTLY) ? indexResult : defineResult);
}

int SchemaObject::CheckValueAndAmendIfNeed(ValueSource sourceType, ValueObject &inValue) const
{
    if (!isValid_ || schemaType_ != SchemaType::JSON) { // Currently this methed only support Json-Schema
        return -E_NOT_PERMIT;
    }

    std::set<FieldPath> lackingPaths;
    int errCode = CheckValue(inValue, lackingPaths);
    if (errCode != -E_VALUE_MATCH) {
        return errCode;
    }

    bool amended = false;
    errCode = AmendValueIfNeed(inValue, lackingPaths, amended);
    if (errCode != E_OK) { // Unlikely
        LOGE("[Schema][CheckAmend] Amend fail, errCode=%d, srcType=%d.", errCode, static_cast<int>(sourceType));
        return -E_INTERNAL_ERROR;
    }
    return (amended ? -E_VALUE_MATCH_AMENDED : -E_VALUE_MATCH);
}

int SchemaObject::VerifyValue(ValueSource sourceType, const Value &inValue) const
{
    return VerifyValue(sourceType, RawValue{inValue.data(), inValue.size()});
}

int SchemaObject::VerifyValue(ValueSource sourceType, const RawValue &inValue) const
{
    if (inValue.first == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (!isValid_ || schemaType_ != SchemaType::FLATBUFFER) {
        return -E_NOT_PERMIT;
    }
    if (inValue.second <= schemaSkipSize_) {
        LOGE("[Schema][Verify] Value length=%zu invalid, skipsize=%u.", inValue.second, schemaSkipSize_);
        return -E_FLATBUFFER_VERIFY_FAIL;
    }

    RawValue rawValue;
    std::vector<uint8_t> cache;
    if (schemaSkipSize_ % SECURE_BYTE_ALIGN == 0) {
        rawValue = {inValue.first + schemaSkipSize_, inValue.second - schemaSkipSize_};
    } else {
        cache.assign(inValue.first + schemaSkipSize_, inValue.first + inValue.second);
        rawValue = {cache.data(), cache.size()};
    }

    // Currently do not try no sizePrefix, future may depend on sourceType
    int errCode = flatbufferSchema_.VerifyFlatBufferValue(rawValue, false);
    if (errCode != E_OK) {
        LOGE("[Schema][Verify] Value verify fail, srcType=%d.", static_cast<int>(sourceType));
        return errCode;
    }
    return E_OK;
}

int SchemaObject::ExtractValue(ValueSource sourceType, RawString inPath, const RawValue &inValue,
    TypeValue &outExtract, std::vector<uint8_t> *cache) const
{
    // NOTE!!! This function is performance sensitive !!! Carefully not to allocate memory often!!!
    if (!isValid_ || schemaType_ != SchemaType::FLATBUFFER) {
        return -E_NOT_PERMIT;
    }
    if (inPath == nullptr || inValue.first == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (inValue.second <= schemaSkipSize_) {
        LOGE("[Schema][Extract] Value length=%u invalid, skipsize=%u.", inValue.second, schemaSkipSize_);
        return -E_FLATBUFFER_VERIFY_FAIL;
    }

    RawValue rawValue;
    std::vector<uint8_t> *tempCache = nullptr; // A temporary cache for use when input cache can not hold.
    if (schemaSkipSize_ % SECURE_BYTE_ALIGN == 0) {
        rawValue = {inValue.first + schemaSkipSize_, inValue.second - schemaSkipSize_};
    } else if ((cache != nullptr) && (cache->size() >= (inValue.second - schemaSkipSize_))) {
        // Do not expand the cache if it can not hold
        cache->assign(inValue.first + schemaSkipSize_, inValue.first + inValue.second);
        rawValue = {cache->data(), inValue.second - schemaSkipSize_}; // Attention: Do not use cache.size() as second.
    } else {
        // Use a temporary cache, which will release its memory quickly
        tempCache = new (std::nothrow) std::vector<uint8_t>;
        if (tempCache == nullptr) {
            LOGE("[Schema][Extract] OOM.");
            return -E_OUT_OF_MEMORY;
        }
        tempCache->resize(inValue.second - schemaSkipSize_);
        tempCache->assign(inValue.first + schemaSkipSize_, inValue.first + inValue.second);
        rawValue = {tempCache->data(), tempCache->size()};
    }

    // Currently do not try no sizePrefix, future may depend on sourceType
    int errCode = flatbufferSchema_.ExtractFlatBufferValue(inPath, rawValue, outExtract, false);
    if (errCode != E_OK) {
        LOGE("[Schema][Extract] Fail, path=%s, srcType=%d.", inPath, static_cast<int>(sourceType));
    }
    delete tempCache; // delete nullptr is safe
    tempCache = nullptr;
    return errCode;
}

int SchemaObject::ParseJsonSchema(const JsonObject &inJsonObject)
{
    // Parse and check mandatory metaField below
    int errCode = CheckMetaFieldCountAndType(inJsonObject);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = ParseCheckSchemaVersionMode(inJsonObject);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = ParseCheckSchemaDefine(inJsonObject);
    if (errCode != E_OK) {
        return errCode;
    }
    // Parse and check optional metaField below
    errCode = ParseCheckSchemaIndexes(inJsonObject);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = ParseCheckSchemaSkipSize(inJsonObject);
    if (errCode != E_OK) {
        return errCode;
    }
    return E_OK;
}

namespace {
int CheckOptionalMetaFieldCountAndType(const std::map<FieldPath, FieldType> &metaFieldPathType)
{
    uint32_t indexMetaFieldCount = 0;
    uint32_t skipSizeMetaFieldCount = 0;
    if (metaFieldPathType.count(FieldPath{KEYWORD_SCHEMA_INDEXES}) != 0) {
        indexMetaFieldCount++;
        FieldType type = metaFieldPathType.at(FieldPath{KEYWORD_SCHEMA_INDEXES});
        if (type != FieldType::LEAF_FIELD_ARRAY) {
            LOGE("[Schema][CheckMeta] Expect SCHEMA_INDEXES type ARRAY but %s.",
                SchemaUtils::FieldTypeString(type).c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    if (metaFieldPathType.count(FieldPath{KEYWORD_SCHEMA_SKIPSIZE}) != 0) {
        skipSizeMetaFieldCount++;
        FieldType type = metaFieldPathType.at(FieldPath{KEYWORD_SCHEMA_SKIPSIZE});
        if (type != FieldType::LEAF_FIELD_INTEGER) {
            LOGE("[Schema][CheckMeta] Expect SCHEMA_SKIPSIZE type INTEGER but %s.",
                SchemaUtils::FieldTypeString(type).c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    if (metaFieldPathType.size() != (SCHEMA_META_FEILD_COUNT_MIN + indexMetaFieldCount + skipSizeMetaFieldCount)) {
        LOGE("[Schema][CheckMeta] Unrecognized metaField exist: total=%u, indexField=%u, skipSizeField=%u.",
            metaFieldPathType.size(), indexMetaFieldCount, skipSizeMetaFieldCount);
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}
}

int SchemaObject::CheckMetaFieldCountAndType(const JsonObject& inJsonObject) const
{
    std::map<FieldPath, FieldType> metaFieldPathType;
    int errCode = inJsonObject.GetSubFieldPathAndType(FieldPath(), metaFieldPathType);
    if (errCode != E_OK) {
        LOGE("[Schema][CheckMeta] GetSubFieldPathAndType fail, errCode=%d.", errCode);
        return errCode;
    }
    if (metaFieldPathType.size() < SCHEMA_META_FEILD_COUNT_MIN ||
        metaFieldPathType.size() > SCHEMA_META_FEILD_COUNT_MAX) {
        LOGE("[Schema][CheckMeta] Unexpected metafield count=%zu.", metaFieldPathType.size());
        return -E_SCHEMA_PARSE_FAIL;
    }
    // Check KeyWord SCHEMA_VERSION
    if (metaFieldPathType.count(FieldPath{KEYWORD_SCHEMA_VERSION}) == 0) {
        LOGE("[Schema][CheckMeta] Expect metafield SCHEMA_VERSION but not find.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    FieldType type = metaFieldPathType.at(FieldPath{KEYWORD_SCHEMA_VERSION});
    if (type != FieldType::LEAF_FIELD_STRING) {
        LOGE("[Schema][CheckMeta] Expect SCHEMA_VERSION type STRING but %s.",
            SchemaUtils::FieldTypeString(type).c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    // Check KeyWord SCHEMA_MODE
    if (metaFieldPathType.count(FieldPath{KEYWORD_SCHEMA_MODE}) == 0) {
        LOGE("[Schema][CheckMeta] Expect metafield SCHEMA_MODE but not find.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    type = metaFieldPathType.at(FieldPath{KEYWORD_SCHEMA_MODE});
    if (type != FieldType::LEAF_FIELD_STRING) {
        LOGE("[Schema][CheckMeta] Expect SCHEMA_MODE type STRING but %s.", SchemaUtils::FieldTypeString(type).c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    // Check KeyWord SCHEMA_DEFINE
    if (metaFieldPathType.count(FieldPath{KEYWORD_SCHEMA_DEFINE}) == 0) {
        LOGE("[Schema][CheckMeta] Expect metafield SCHEMA_DEFINE but not find.");
        return -E_SCHEMA_PARSE_FAIL;
    }
    type = metaFieldPathType.at(FieldPath{KEYWORD_SCHEMA_DEFINE});
    if (type != FieldType::INTERNAL_FIELD_OBJECT) { // LEAF_FIELD_OBJECT indicate an empty object which is not allowed
        LOGE("[Schema][CheckMeta] Expect SCHEMA_DEFINE type INTERNAL_OBJECT but %s.",
            SchemaUtils::FieldTypeString(type).c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    // Check KeyWord SCHEMA_INDEXES If Need
    return CheckOptionalMetaFieldCountAndType(metaFieldPathType);
}

int SchemaObject::ParseCheckSchemaVersionMode(const JsonObject& inJsonObject)
{
    // Note: it has been checked in CheckMetaFieldCountAndType that SCHEMA_VERSION field exists and its type is string.
    FieldValue versionValue;
    int errCode = inJsonObject.GetFieldValueByFieldPath(FieldPath{KEYWORD_SCHEMA_VERSION}, versionValue);
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }
    if (SchemaUtils::Strip(versionValue.stringValue) != SCHEMA_SUPPORT_VERSION) {
        LOGE("[Schema][ParseVerMode] Unexpected SCHEMA_VERSION=%s.", versionValue.stringValue.c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    schemaVersion_ = SCHEMA_SUPPORT_VERSION;

    // Note: it has been checked in CheckMetaFieldCountAndType that SCHEMA_MODE field exists and its type is string.
    FieldValue modeValue;
    errCode = inJsonObject.GetFieldValueByFieldPath(FieldPath{KEYWORD_SCHEMA_MODE}, modeValue);
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }
    std::string modeStripped = SchemaUtils::Strip(modeValue.stringValue);
    if (modeStripped != KEYWORD_MODE_STRICT && modeStripped != KEYWORD_MODE_COMPATIBLE) {
        LOGE("[Schema][ParseVerMode] Unexpected SCHEMA_MODE=%s.", modeValue.stringValue.c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    schemaMode_ = ((modeStripped == KEYWORD_MODE_STRICT) ? SchemaMode::STRICT : SchemaMode::COMPATIBLE);
    return E_OK;
}

int SchemaObject::ParseCheckSchemaDefine(const JsonObject& inJsonObject)
{
    // Clear schemaDefine_ to recover from a fail parse
    schemaDefine_.clear();
    // Note: it has been checked in CheckMetaFieldCountAndType that SCHEMA_DEFINE field exists and its type is
    // internal-object. Nest path refer to those field with type internal object that has sub field.
    std::set<FieldPath> nestPathCurDepth{FieldPath{KEYWORD_SCHEMA_DEFINE}};
    uint32_t fieldNameCount = 0;
    for (uint32_t depth = 0; depth < SCHEMA_FEILD_PATH_DEPTH_MAX; depth++) {
        std::map<FieldPath, FieldType> subPathType;
        int errCode = inJsonObject.GetSubFieldPathAndType(nestPathCurDepth, subPathType);
        if (errCode != E_OK) { // Unlikely
            LOGE("[Schema][ParseDefine] Internal Error: GetSubFieldPathAndType Fail, Depth=%u.", depth);
            return -E_INTERNAL_ERROR;
        }
        fieldNameCount += subPathType.size();
        nestPathCurDepth.clear(); // Clear it for collecting new nestPath
        for (const auto &subField : subPathType) {
            SchemaAttribute attribute;
            errCode = CheckSchemaDefineItemDecideAttribute(inJsonObject, subField.first, subField.second, attribute);
            if (errCode != E_OK) {
                LOGE("[Schema][ParseDefine] CheckSchemaDefineItemDecideAttribute Fail, Path=%s.",
                    SchemaUtils::FieldPathString(subField.first).c_str());
                return -E_SCHEMA_PARSE_FAIL;
            }
            // If everything ok, insert this schema item into schema define
            // Remember to remove SCHEMA_DEFINE in the front of the fieldpath
            schemaDefine_[depth][FieldPath(++(subField.first.begin()), subField.first.end())] = attribute;
            // Deal with the nestpath and check depth limitation
            if (subField.second == FieldType::INTERNAL_FIELD_OBJECT) {
                if (depth == SCHEMA_FEILD_PATH_DEPTH_MAX - 1) { // Minus 1 to be the boundary
                    LOGE("[Schema][ParseDefine] Path=%s is INTERNAL_FIELD_OBJECT but reach schema depth limitation.",
                        SchemaUtils::FieldPathString(subField.first).c_str());
                    return -E_SCHEMA_PARSE_FAIL;
                }
                nestPathCurDepth.insert(subField.first);
            }
        }
        // If no deeper schema define, quit loop in advance
        if (nestPathCurDepth.empty()) {
            break;
        }
    }
    if (fieldNameCount > SCHEMA_FEILD_NAME_COUNT_MAX) {
        // Check Field Count Here
        LOGE("[Schema][ParseDefine] FieldName count=%u exceed the limitation.", fieldNameCount);
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}

int SchemaObject::CheckSchemaDefineItemDecideAttribute(const JsonObject& inJsonObject, const FieldPath &inPath,
    FieldType inType, SchemaAttribute &outAttr) const
{
    // Note: inPath will never be an empty vector, internal logic guarantee it, see the caller logic
    if (inPath.empty()) { // Not Possible. Just For Clear CodeDEX.
        return -E_INTERNAL_ERROR;
    }
    int errCode = SchemaUtils::CheckFieldName(inPath.back());
    if (errCode != E_OK) {
        LOGE("[Schema][CheckItemDecideAttr] Invalid fieldName=%s, errCode=%d.", inPath.back().c_str(), errCode);
        return -E_SCHEMA_PARSE_FAIL;
    }
    if (inType == FieldType::LEAF_FIELD_STRING) {
        FieldValue subFieldValue;
        errCode = inJsonObject.GetFieldValueByFieldPath(inPath, subFieldValue);
        if (errCode != E_OK) { // Unlikely
            LOGE("[Schema][CheckItemDecideAttr] Internal Error: GetFieldValueByFieldPath Fail.");
            return -E_INTERNAL_ERROR;
        }
        errCode = SchemaUtils::ParseAndCheckSchemaAttribute(subFieldValue.stringValue, outAttr);
        if (errCode != E_OK) {
            LOGE("[Schema][CheckItemDecideAttr] ParseAndCheckSchemaAttribute Fail, errCode=%d.", errCode);
            return -E_SCHEMA_PARSE_FAIL;
        }
        // The ParseAndCheckSchemaAttribute do not cope with isIndexable field. Need to set it true here
        outAttr.isIndexable = true;
    } else if (inType == FieldType::LEAF_FIELD_ARRAY) {
        uint32_t arraySize = 0;
        errCode = inJsonObject.GetArraySize(inPath, arraySize);
        if (errCode != E_OK) {
            LOGE("[Schema][CheckItemDecideAttr] Internal Error: GetArraySize Fail.");
            return -E_INTERNAL_ERROR;
        }
        if (arraySize != 0) {
            LOGE("[Schema][CheckItemDecideAttr] Expect array empty but size=%u.", arraySize);
            return -E_SCHEMA_PARSE_FAIL;
        }
        outAttr = SchemaAttribute{inType, false, false, false, FieldValue()};
    } else if (inType == FieldType::LEAF_FIELD_OBJECT) {
        outAttr = SchemaAttribute{inType, false, false, false, FieldValue()};
    } else if (inType == FieldType::INTERNAL_FIELD_OBJECT) {
        outAttr = SchemaAttribute{inType, false, false, false, FieldValue()}; // hasNotNull set false is OK for this
    } else {
        LOGE("[Schema][CheckItemDecideAttr] Unexpected FieldType=%s.", SchemaUtils::FieldTypeString(inType).c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}

int SchemaObject::ParseCheckSchemaIndexes(const JsonObject& inJsonObject)
{
    // Clear schemaIndexes_ to recover from a fail parse
    schemaIndexes_.clear();
    // No SCHEMA_INDEXES field is allowed
    if (!inJsonObject.IsFieldPathExist(FieldPath{KEYWORD_SCHEMA_INDEXES})) {
        LOGD("[Schema][ParseIndex] No SCHEMA_INDEXES Field.");
        return E_OK;
    }
    // The type of SCHEMA_INDEXES field has been checked in CheckMetaFieldCountAndType to be an array
    // If not all members of the array are string type or string-array, this call will return error
    std::vector<std::vector<std::string>> oriIndexArray;
    int errCode = inJsonObject.GetArrayContentOfStringOrStringArray(FieldPath{KEYWORD_SCHEMA_INDEXES}, oriIndexArray);
    if (errCode != E_OK) {
        LOGE("[Schema][ParseIndex] GetArrayContent Fail, errCode=%d.", errCode);
        return -E_SCHEMA_PARSE_FAIL;
    }
    if (oriIndexArray.size() > SCHEMA_INDEX_COUNT_MAX) {
        LOGE("[Schema][ParseIndex] Index(Ori) count=%zu exceed limitation.", oriIndexArray.size());
        return -E_SCHEMA_PARSE_FAIL;
    }
    for (const auto &entry : oriIndexArray) {
        errCode = ParseCheckEachIndexFromStringArray(entry);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int SchemaObject::ParseCheckSchemaSkipSize(const JsonObject& inJsonObject)
{
    // No SCHEMA_SKIPSIZE field is allowed
    if (!inJsonObject.IsFieldPathExist(FieldPath{KEYWORD_SCHEMA_SKIPSIZE})) {
        LOGD("[Schema][ParseSkipSize] No SCHEMA_SKIPSIZE Field.");
        return E_OK;
    }
    // The type of SCHEMA_SKIPSIZE field has been checked in CheckMetaFieldCountAndType to be an INTEGER
    FieldValue skipSizeValue;
    int errCode = inJsonObject.GetFieldValueByFieldPath(FieldPath{KEYWORD_SCHEMA_SKIPSIZE}, skipSizeValue);
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }
    if (skipSizeValue.integerValue < 0 || static_cast<uint32_t>(skipSizeValue.integerValue) > SCHEMA_SKIPSIZE_MAX) {
        LOGE("[Schema][ParseSkipSize] Unexpected SCHEMA_SKIPSIZE=%d.", skipSizeValue.integerValue);
        return -E_SCHEMA_PARSE_FAIL;
    }
    schemaSkipSize_ = static_cast<uint32_t>(skipSizeValue.integerValue);
    return E_OK;
}

int SchemaObject::ParseCheckEachIndexFromStringArray(const std::vector<std::string> &inStrArray)
{
    std::vector<FieldPath> indexPathVec;
    std::set<FieldPath> indexPathSet;
    // Parse each indexFieldPathString and check duplication
    for (const auto &eachPathStr : inStrArray) {
        FieldPath eachPath;
        int errCode = SchemaUtils::ParseAndCheckFieldPath(eachPathStr, eachPath);
        if (errCode != E_OK) {
            LOGE("[Schema][ParseEachIndex] IndexPath=%s Invalid.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (eachPath.size() == 0 || eachPath.size() > SCHEMA_FEILD_PATH_DEPTH_MAX) {
            LOGE("[Schema][ParseEachIndex] Root not indexable or path=%s depth exceed limit.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (indexPathSet.count(eachPath) != 0) {
            LOGE("[Schema][ParseEachIndex] IndexPath=%s Duplicated.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        indexPathVec.push_back(eachPath);
        indexPathSet.insert(eachPath);
    }
    if (indexPathVec.empty()) { // Unlikely, empty JsonArray had been eliminated by GetArrayContent Method
        return -E_INTERNAL_ERROR;
    }
    // Check indexDefine duplication, Use Sort-Column(the first fieldPath in index) as the indexName.
    const IndexName &indexName = indexPathVec.front();
    if (schemaIndexes_.count(indexName) != 0) {
        LOGE("[Schema][ParseEachIndex] IndexName=%s Already Defined.", SchemaUtils::FieldPathString(indexName).c_str());
        return -E_SCHEMA_PARSE_FAIL;
    }
    // Create new indexInfo entry, then check indexable for each indexFieldPath against schemaDefine
    return CheckFieldPathIndexableThenSave(indexPathVec, schemaIndexes_[indexName]);
}

int SchemaObject::CheckFieldPathIndexableThenSave(const std::vector<FieldPath> &inPathVec, IndexInfo &infoToSave)
{
    for (const auto &eachPath : inPathVec) {
        // Previous logic guarantee eachPath.size greater than zero
        uint32_t depth = eachPath.size() - 1; // minus 1 to change depth count from zero
        std::string eachPathStr = SchemaUtils::FieldPathString(eachPath);
        if (schemaDefine_.count(depth) == 0) {
            LOGE("[Schema][CheckIndexable] No schema define of this depth, path=%s.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (schemaDefine_[depth].count(eachPath) == 0) {
            LOGE("[Schema][CheckIndexable] No such path in schema define, path=%s.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (!schemaDefine_[depth][eachPath].isIndexable) {
            LOGE("[Schema][CheckIndexable] Path=%s is not indexable.", eachPathStr.c_str());
            return -E_SCHEMA_PARSE_FAIL;
        }
        // Save this indexField to indexInfo
        infoToSave.push_back({eachPath, schemaDefine_[depth][eachPath].type});
    }
    return E_OK;
}

int SchemaObject::CompareSchemaVersionMode(const SchemaObject &newSchema) const
{
    static std::map<SchemaMode, std::string> modeMapString = {
        {SchemaMode::STRICT, "STRICT"},
        {SchemaMode::COMPATIBLE, "COMPATIBLE"},
    };
    if (schemaVersion_ != newSchema.schemaVersion_) {
        LOGE("[Schema][CompareVerMode] OldVer=%s mismatch newVer=%s.", schemaVersion_.c_str(),
            newSchema.schemaVersion_.c_str());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Only Json-Schema need to compare mode
    if (schemaType_ == SchemaType::JSON && schemaMode_ != newSchema.schemaMode_) {
        LOGE("[Schema][CompareVerMode] OldMode=%s mismatch newMode=%s.", modeMapString[schemaMode_].c_str(),
            modeMapString[newSchema.schemaMode_].c_str());
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Do not return E_OK here, E_OK is ambiguous.
    return -E_SCHEMA_EQUAL_EXACTLY;
}

int SchemaObject::CompareSchemaSkipSize(const SchemaObject &newSchema) const
{
    if (schemaSkipSize_ != newSchema.schemaSkipSize_) {
        LOGE("[Schema][CompareSkipSize] OldSkip=%u mismatch newSkip=%u.", schemaSkipSize_, newSchema.schemaSkipSize_);
        return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
    }
    // Do not return E_OK here, E_OK is ambiguous.
    return -E_SCHEMA_EQUAL_EXACTLY;
}

int SchemaObject::CompareSchemaDefine(const SchemaObject &newSchema) const
{
    bool isEqualExactly = true;
    for (uint32_t depth = 0; depth < SCHEMA_FEILD_PATH_DEPTH_MAX; depth++) {
        SchemaDefine emptyDefine;
        const SchemaDefine &defineInOldSchema =
            (schemaDefine_.count(depth) == 0 ? emptyDefine : schemaDefine_.at(depth));
        const SchemaDefine &defineInNewSchema =
            (newSchema.schemaDefine_.count(depth) == 0 ? emptyDefine : newSchema.schemaDefine_.at(depth));

        // No define at this depth for both schema
        if (defineInNewSchema.empty() && defineInOldSchema.empty()) {
            break;
        }
        // No matter strict or compatible mode, newSchema can't have less field than oldSchema
        if (defineInNewSchema.size() < defineInOldSchema.size()) {
            LOGE("[Schema][CompareDefine] newSize=%u less than oldSize=%u at depth=%u.", defineInNewSchema.size(),
                defineInOldSchema.size(), depth);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
        if (defineInNewSchema.size() > defineInOldSchema.size()) {
            // Strict mode not support increase fieldDefine
            if (schemaMode_ == SchemaMode::STRICT) {
                LOGE("[Schema][CompareDefine] newSize=%u more than oldSize=%u at depth=%u in STRICT mode.",
                    defineInNewSchema.size(), defineInOldSchema.size(), depth);
                return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
            }
            isEqualExactly = false;
        }

        // Compare schema define of this depth, looking for incompatible
        int errCode = CompareSchemaDefineByDepth(defineInOldSchema, defineInNewSchema);
        if (errCode == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
            return errCode;
        }
    }
    // Do not return E_OK here, E_OK is ambiguous.
    return (isEqualExactly ? -E_SCHEMA_EQUAL_EXACTLY : -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE);
}

namespace {
inline bool IsExtraFieldConformToCompatibility(const SchemaAttribute &inAttr)
{
    return (!inAttr.hasNotNullConstraint || inAttr.hasDefaultValue);
}
}

int SchemaObject::CompareSchemaDefineByDepth(const SchemaDefine &oldDefine, const SchemaDefine &newDefine) const
{
    // Looking for incompatible : new define should at least contain all field the old define hold
    for (auto &entry : oldDefine) {
        if (newDefine.count(entry.first) == 0) {
            LOGE("[Schema][CompareDefineDepth] fieldpath=%s not found in new schema.",
                SchemaUtils::FieldPathString(entry.first).c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
        // SchemaAttribute require to be equal exactly
        int errCode = CompareSchemaAttribute(entry.second, newDefine.at(entry.first));
        if (errCode != -E_SCHEMA_EQUAL_EXACTLY) {
            LOGE("[Schema][CompareDefineDepth] Attribute mismatch at fieldpath=%s.",
                SchemaUtils::FieldPathString(entry.first).c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    // Looking for incompatible : the extra field in new schema should has default or can be null
    for (auto &entry : newDefine) {
        if (oldDefine.count(entry.first) != 0) {
            continue;
        }
        if (!IsExtraFieldConformToCompatibility(entry.second)) {
            LOGE("[Schema][CompareDefineDepth] ExtraField=%s, {notnull=%d, default=%d}, not conform compatibility.",
                SchemaUtils::FieldPathString(entry.first).c_str(), entry.second.hasNotNullConstraint,
                entry.second.hasDefaultValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    return -E_SCHEMA_EQUAL_EXACTLY;
}

int SchemaObject::CompareSchemaAttribute(const SchemaAttribute &oldAttr, const SchemaAttribute &newAttr) const
{
    if (oldAttr.type != newAttr.type) {
        // The exceptional case is that the field type changed from the leaf_object to internal_object,
        // which indicate that sub fields are added to it. Changed from internal_object to leaf_object will
        // sooner or later cause an incompatible detection in next depth, we discern this situation here in advance.
        if (!(oldAttr.type == FieldType::LEAF_FIELD_OBJECT && newAttr.type == FieldType::INTERNAL_FIELD_OBJECT)) {
            LOGE("[Schema][CompareAttr] OldType=%s mismatch newType=%s.",
                SchemaUtils::FieldTypeString(oldAttr.type).c_str(), SchemaUtils::FieldTypeString(newAttr.type).c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    // Here we use isIndexable info to distinguish two categories of type.
    // "BOOL, INTEGER, LONG, DOUBLE, STRING" are all indexable type, NULL type will not appear in Schema
    // "ARRAY, LEAF_OBJECT, INTERNAL_OBJECT" are all not indexable type.
    // They have been checked same type just above. No need to check more for not indexable type
    if (oldAttr.isIndexable) {
        if (oldAttr.hasNotNullConstraint != newAttr.hasNotNullConstraint) {
            LOGE("[Schema][CompareAttr] OldNotNull=%d mismatch newNotNull=%d.", oldAttr.hasNotNullConstraint,
                newAttr.hasNotNullConstraint);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
        if (oldAttr.hasDefaultValue != newAttr.hasDefaultValue) {
            LOGE("[Schema][CompareAttr] OldHasDefault=%d mismatch newHasDefault=%d.", oldAttr.hasDefaultValue,
                newAttr.hasDefaultValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
        if (oldAttr.hasDefaultValue) {
            // DefaultValue require to be equal exactly
            int errCode = CompareSchemaDefaultValue(oldAttr, newAttr);
            if (errCode != -E_SCHEMA_EQUAL_EXACTLY) {
                return errCode;
            }
        }
    }
    return -E_SCHEMA_EQUAL_EXACTLY;
}

namespace {
inline bool IsDoubleBinaryEqual(double left, double right)
{
    return *(reinterpret_cast<uint64_t *>(&left)) == *(reinterpret_cast<uint64_t *>(&right));
}
}

int SchemaObject::CompareSchemaDefaultValue(const SchemaAttribute &oldAttr, const SchemaAttribute &newAttr) const
{
    // Value type has been check equal for both attribute in the caller
    if (oldAttr.type == FieldType::LEAF_FIELD_BOOL) {
        if (oldAttr.defaultValue.boolValue != newAttr.defaultValue.boolValue) {
            LOGE("[Schema][CompareDefault] OldDefault=%d mismatch newDefault=%d.", oldAttr.defaultValue.boolValue,
                newAttr.defaultValue.boolValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    } else if (oldAttr.type == FieldType::LEAF_FIELD_INTEGER) {
        if (oldAttr.defaultValue.integerValue != newAttr.defaultValue.integerValue) {
            LOGE("[Schema][CompareDefault] OldDefault=%d mismatch newDefault=%d.", oldAttr.defaultValue.integerValue,
                newAttr.defaultValue.integerValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    } else if (oldAttr.type == FieldType::LEAF_FIELD_LONG) {
        if (oldAttr.defaultValue.longValue != newAttr.defaultValue.longValue) {
            LOGE("[Schema][CompareDefault] OldDefault=%lld mismatch newDefault=%lld.", oldAttr.defaultValue.longValue,
                newAttr.defaultValue.longValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    } else if (oldAttr.type == FieldType::LEAF_FIELD_DOUBLE) {
        // ATTENTION: Here we should compare two double by their binary layout. We should not judge them equal when
        // difference is small enough, since two different default double value may diff so little. The binary
        // layout of the double value will be the same if the original string is the same, so we directly compare them.
        if (!IsDoubleBinaryEqual(oldAttr.defaultValue.doubleValue, newAttr.defaultValue.doubleValue)) {
            LOGE("[Schema][CompareDefault] OldDefault=%f mismatch newDefault=%f.", oldAttr.defaultValue.doubleValue,
                newAttr.defaultValue.doubleValue);
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    } else if (oldAttr.type == FieldType::LEAF_FIELD_STRING) {
        if (oldAttr.defaultValue.stringValue != newAttr.defaultValue.stringValue) {
            LOGE("[Schema][CompareDefault] OldDefault=%s mismatch newDefault=%s.",
                oldAttr.defaultValue.stringValue.c_str(), newAttr.defaultValue.stringValue.c_str());
            return -E_SCHEMA_UNEQUAL_INCOMPATIBLE;
        }
    }
    // The caller logic guarantee that both attribute type will not be null, array, object
    return -E_SCHEMA_EQUAL_EXACTLY;
}

namespace {
inline void ClearIndexDifference(IndexDifference &indexDiffer)
{
    indexDiffer.change.clear();
    indexDiffer.increase.clear();
    indexDiffer.decrease.clear();
}

inline bool IsIndexInfoExactlyEqual(const IndexInfo &leftInfo, const IndexInfo &rightInfo)
{
    // Exactly equal require count, order and type of each indexField in the index be the same
    return leftInfo == rightInfo;
}

inline bool IsSchemaIndexesExactlyEqual(const IndexDifference &indexDiffer)
{
    return (indexDiffer.change.empty() && indexDiffer.increase.empty() && indexDiffer.decrease.empty());
}
}

int SchemaObject::CompareSchemaIndexes(const SchemaObject &newSchema, IndexDifference &indexDiffer) const
{
    ClearIndexDifference(indexDiffer);
    // Find the increase and change index
    for (const auto &entry : newSchema.schemaIndexes_) {
        if (schemaIndexes_.count(entry.first) == 0) {
            LOGD("[Schema][CompareIndex] Increase indexName=%s.", SchemaUtils::FieldPathString(entry.first).c_str());
            indexDiffer.increase[entry.first] = entry.second;
        } else {
            // Both schema have same IndexName, Check whether indexInfo differs
            if (!IsIndexInfoExactlyEqual(entry.second, schemaIndexes_.at(entry.first))) {
                LOGD("[Schema][CompareIndex] Change indexName=%s.", SchemaUtils::FieldPathString(entry.first).c_str());
                indexDiffer.change[entry.first] = entry.second;
            }
        }
    }
    // Find the decrease index
    for (const auto &entry : schemaIndexes_) {
        if (newSchema.schemaIndexes_.count(entry.first) == 0) {
            LOGD("[Schema][CompareIndex] Decrease indexName=%s.", SchemaUtils::FieldPathString(entry.first).c_str());
            indexDiffer.decrease.insert(entry.first);
        }
    }
    // Do not return E_OK here, E_OK is ambiguous.
    return IsSchemaIndexesExactlyEqual(indexDiffer) ? -E_SCHEMA_EQUAL_EXACTLY : -E_SCHEMA_UNEQUAL_COMPATIBLE;
}

namespace {
int CheckValueItemNumericType(FieldType typeInValue, FieldType typeInSchema)
{
    if (typeInValue == FieldType::LEAF_FIELD_DOUBLE) {
        if (typeInSchema != FieldType::LEAF_FIELD_DOUBLE) {
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
    } else if (typeInValue == FieldType::LEAF_FIELD_LONG) {
        if (typeInSchema != FieldType::LEAF_FIELD_LONG &&
            typeInSchema != FieldType::LEAF_FIELD_DOUBLE) {
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
    } else {
        // LEAF_FIELD_INTEGER
        if (typeInSchema != FieldType::LEAF_FIELD_INTEGER &&
            typeInSchema != FieldType::LEAF_FIELD_LONG &&
            typeInSchema != FieldType::LEAF_FIELD_DOUBLE) {
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
    }
    return -E_VALUE_MATCH;
}

inline bool IsTypeMustBeExactlyEqualBetweenSchemaAndValue(FieldType inType)
{
    return (inType == FieldType::LEAF_FIELD_BOOL ||
            inType == FieldType::LEAF_FIELD_STRING ||
            inType == FieldType::LEAF_FIELD_ARRAY);
}

inline bool IsObjectType(FieldType inType)
{
    return (inType == FieldType::LEAF_FIELD_OBJECT || inType == FieldType::INTERNAL_FIELD_OBJECT);
}

// Check in the value-view for convenience
int CheckValueItem(const SchemaAttribute &refAttr, FieldType typeInValue)
{
    FieldType typeInSchema = refAttr.type;
    if (typeInSchema == FieldType::LEAF_FIELD_NULL) { // Unlikely
        return -E_INTERNAL_ERROR;
    }
    // Check NotNull-Constraint first
    if (typeInValue == FieldType::LEAF_FIELD_NULL) {
        if (refAttr.hasNotNullConstraint) {
            return -E_VALUE_MISMATCH_CONSTRAINT;
        }
        return -E_VALUE_MATCH;
    }
    // If typeInValue not NULL, check against schema. First check type that must be equal.
    if (IsTypeMustBeExactlyEqualBetweenSchemaAndValue(typeInValue)) {
        if (typeInValue != typeInSchema) {
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
        return -E_VALUE_MATCH;
    }
    // Check Object related type, lack or more field will be deal with at next depth
    // typeInSchema/typeInValue     LEAF_OBJECT                         INTERNAL_OBJECT
    //              LEAF_OBJECT     MATCH                               MATCH(More field at next depth)
    //          INTERNAL_OBJECT     MATCH(Lack field at next depth)     MATCH
    //           ELSE(POSSIBLE)     TYPE_MISMATCH                       TYPE_MISMATCH
    if (IsObjectType(typeInValue)) {
        if (!IsObjectType(typeInSchema)) {
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
        return -E_VALUE_MATCH;
    }
    // Check Numeric related type, at last
    return CheckValueItemNumericType(typeInValue, typeInSchema);
}

inline bool IsLackingFieldViolateNotNullConstraint(const SchemaAttribute &refAttr)
{
    return (refAttr.hasNotNullConstraint && !refAttr.hasDefaultValue);
}

// Function only for split big function
int CheckValueBySchemaItem(const std::pair<FieldPath, SchemaAttribute> &schemaItem,
    const std::map<FieldPath, FieldType> &subPathType, std::set<FieldPath> &lackingPaths)
{
    if (subPathType.count(schemaItem.first) == 0) { // Value do not contain this field
        if (IsLackingFieldViolateNotNullConstraint(schemaItem.second)) {
            return -E_VALUE_MISMATCH_CONSTRAINT;
        }
        lackingPaths.insert(schemaItem.first);
        return -E_VALUE_MATCH;
    }
    // Value contain this field, check its type
    return CheckValueItem(schemaItem.second, subPathType.at(schemaItem.first));
}

inline std::string ValueFieldType(const std::map<FieldPath, FieldType> &subPathType, const FieldPath &inPath)
{
    if (subPathType.count(inPath) == 0) {
        return "NotExist";
    }
    return SchemaUtils::FieldTypeString(subPathType.at(inPath));
}
}

int SchemaObject::CheckValue(const ValueObject &inValue, std::set<FieldPath> &lackingPaths) const
{
    std::set<FieldPath> nestPathCurDepth{FieldPath()}; // Empty path represent root path
    for (uint32_t depth = 0; depth < SCHEMA_FEILD_PATH_DEPTH_MAX; depth++) {
        if (schemaDefine_.count(depth) == 0 || schemaDefine_.at(depth).empty()) { // No schema define in this depth
            break;
        }

        std::map<FieldPath, FieldType> subPathType;
        int errCode = inValue.GetSubFieldPathAndType(nestPathCurDepth, subPathType); // Value field of current depth
        if (errCode != E_OK && errCode != -E_INVALID_PATH) { // E_INVALID_PATH for path not exist
            LOGE("[Schema][CheckValue] GetSubFieldPathAndType Fail=%d, Depth=%u.", errCode, depth);
            return -E_VALUE_MISMATCH_FEILD_TYPE;
        }
        nestPathCurDepth.clear(); // Clear it for collecting new nestPath

        if ((schemaMode_ == SchemaMode::STRICT) && (subPathType.size() > schemaDefine_.at(depth).size())) {
            LOGE("[Schema][CheckValue] ValueFieldCount=%zu more than SchemaFieldCount=%zu at depth=%u",
                subPathType.size(), schemaDefine_.at(depth).size(), depth);
            return -E_VALUE_MISMATCH_FEILD_COUNT; // Value contain more field than schema
        }

        for (const auto &schemaItem : schemaDefine_.at(depth)) { // Check each field define in schema
            if (schemaItem.second.type == FieldType::INTERNAL_FIELD_OBJECT) {
                nestPathCurDepth.insert(schemaItem.first); // This field has subfield in schema
            }
            errCode = CheckValueBySchemaItem(schemaItem, subPathType, lackingPaths);
            if (errCode != -E_VALUE_MATCH) {
                LOGE("[Schema][CheckValue] Path=%s, schema{NotNull=%d,Default=%d,Type=%s}, Value{Type=%s}, errCode=%d.",
                    SchemaUtils::FieldPathString(schemaItem.first).c_str(), schemaItem.second.hasNotNullConstraint,
                    schemaItem.second.hasDefaultValue, SchemaUtils::FieldTypeString(schemaItem.second.type).c_str(),
                    ValueFieldType(subPathType, schemaItem.first).c_str(), errCode);
                return errCode;
            }
        }
    }
    return -E_VALUE_MATCH;
}

int SchemaObject::AmendValueIfNeed(ValueObject &inValue, const std::set<FieldPath> &lackingPaths, bool &amended) const
{
    for (const auto &eachLackingPath : lackingPaths) {
        // Note: The upper code logic guarantee that eachLackingPath won't be empty and must exist in schemaDefine_
        uint32_t depth = eachLackingPath.size() - 1; // Depth count from zero
        const SchemaAttribute &lackingPathAttr = schemaDefine_.at(depth).at(eachLackingPath);
        // If no default value, just ignore this lackingPath
        if (!lackingPathAttr.hasDefaultValue) {
            continue;
        }
        // If has default value, the ParseSchema logic guarantee that fieldType won't be NULL, ARRAY or OBJECT
        // The lacking intermediate field will be automatically insert for this lackingPath
        int errCode = inValue.InsertField(eachLackingPath, lackingPathAttr.type, lackingPathAttr.defaultValue);
        if (errCode != E_OK) { // Unlikely
            LOGE("[Schema][AmendValue] InsertField fail, errCode=%d, Path=%s, Type=%s.", errCode,
                SchemaUtils::FieldPathString(eachLackingPath).c_str(),
                SchemaUtils::FieldTypeString(lackingPathAttr.type).c_str());
            return -E_INTERNAL_ERROR;
        }
        amended = true;
    }
    return E_OK;
}
} // namespace DistributedDB
