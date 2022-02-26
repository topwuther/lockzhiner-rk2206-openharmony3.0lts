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

#include "json_object.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
#ifndef OMIT_JSON
namespace {
    const uint32_t MAX_NEST_DEPTH = 100;
}
uint32_t JsonObject::maxNestDepth_ = MAX_NEST_DEPTH;

uint32_t JsonObject::SetMaxNestDepth(uint32_t nestDepth)
{
    uint32_t preValue = maxNestDepth_;
    // No need to check the reasonability, only test code will use this method
    maxNestDepth_ = nestDepth;
    return preValue;
}

uint32_t JsonObject::CalculateNestDepth(const std::string &inString)
{
    auto begin = reinterpret_cast<const uint8_t *>(inString.c_str());
    auto end = begin + inString.size();
    return CalculateNestDepth(begin, end);
}

uint32_t JsonObject::CalculateNestDepth(const uint8_t *dataBegin, const uint8_t *dataEnd)
{
    if (dataBegin == nullptr || dataEnd == nullptr || dataBegin >= dataEnd) {
        return -E_INVALID_ARGS;
    }
    bool isInString = false;
    uint32_t maxDepth = 0;
    uint32_t objectDepth = 0;
    uint32_t arrayDepth = 0;
    uint32_t numOfEscape = 0;

    for (auto ptr = dataBegin; ptr < dataEnd; ptr++) {
        if (*ptr == '"' && numOfEscape % 2 == 0) { // 2 used to detect parity
            isInString = !isInString;
            continue;
        }
        if (!isInString) {
            if (*ptr == '{') {
                objectDepth++;
                maxDepth = std::max(maxDepth, objectDepth + arrayDepth);
            }
            if (*ptr == '}') {
                objectDepth = ((objectDepth > 0) ? (objectDepth - 1) : 0);
            }
            if (*ptr == '[') {
                arrayDepth++;
                maxDepth = std::max(maxDepth, objectDepth + arrayDepth);
            }
            if (*ptr == ']') {
                arrayDepth = ((arrayDepth > 0) ? (arrayDepth - 1) : 0);
            }
        }
        numOfEscape = ((*ptr == '\\') ? (numOfEscape + 1) : 0);
    }
    return maxDepth;
}

JsonObject::JsonObject(const JsonObject &other)
{
    isValid_ = other.isValid_;
    value_ = other.value_;
}

JsonObject& JsonObject::operator=(const JsonObject &other)
{
    if (&other != this) {
        isValid_ = other.isValid_;
        value_ = other.value_;
    }
    return *this;
}

int JsonObject::Parse(const std::string &inString)
{
    // The jsoncpp lib parser in strict mode will still regard root type jsonarray as valid, but we require jsonobject
    if (isValid_) {
        LOGE("[Json][Parse] Already Valid.");
        return -E_NOT_PERMIT;
    }
    uint32_t nestDepth = CalculateNestDepth(inString);
    if (nestDepth > maxNestDepth_) {
        LOGE("[Json][Parse] Json nest depth=%u exceed max allowed=%u.", nestDepth, maxNestDepth_);
        return -E_JSON_PARSE_FAIL;
    }
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(inString, value_, false)) {
        value_ = Json::Value();
        LOGE("[Json][Parse] Parse string to JsonValue fail, reason=%s.", reader.getFormattedErrorMessages().c_str());
        return -E_JSON_PARSE_FAIL;
    }
    // The jsoncpp lib parser in strict mode will still regard root type jsonarray as valid, but we require jsonobject
    if (value_.type() != Json::ValueType::objectValue) {
        value_ = Json::Value();
        LOGE("[Json][Parse] Not an object at root.");
        return -E_JSON_PARSE_FAIL;
    }
    isValid_ = true;
    return E_OK;
}

int JsonObject::Parse(const std::vector<uint8_t> &inData)
{
    if (inData.empty()) {
        return -E_INVALID_ARGS;
    }
    return Parse(inData.data(), inData.data() + inData.size());
}

int JsonObject::Parse(const uint8_t *dataBegin, const uint8_t *dataEnd)
{
    if (isValid_) {
        LOGE("[Json][Parse] Already Valid.");
        return -E_NOT_PERMIT;
    }
    if (dataBegin == nullptr || dataEnd == nullptr || dataBegin >= dataEnd) {
        return -E_INVALID_ARGS;
    }
    uint32_t nestDepth = CalculateNestDepth(dataBegin, dataEnd);
    if (nestDepth > maxNestDepth_) {
        LOGE("[Json][Parse] Json nest depth=%u exceed max allowed=%u.", nestDepth, maxNestDepth_);
        return -E_JSON_PARSE_FAIL;
    }
    Json::Reader reader(Json::Features::strictMode());
    auto begin = reinterpret_cast<const std::string::value_type *>(dataBegin);
    auto end = reinterpret_cast<const std::string::value_type *>(dataEnd);
    // The endDoc parameter of reader::parse refer to the byte after the string itself
    if (!reader.parse(begin, end, value_, false)) {
        value_ = Json::Value();
        LOGE("[Json][Parse] Parse dataRange to JsonValue fail, reason=%s.", reader.getFormattedErrorMessages().c_str());
        return -E_JSON_PARSE_FAIL;
    }
    // The jsoncpp lib parser in strict mode will still regard root type jsonarray as valid, but we require jsonobject
    if (value_.type() != Json::ValueType::objectValue) {
        value_ = Json::Value();
        LOGE("[Json][Parse] Not an object at root.");
        return -E_JSON_PARSE_FAIL;
    }
    isValid_ = true;
    return E_OK;
}

bool JsonObject::IsValid() const
{
    return isValid_;
}

std::string JsonObject::ToString() const
{
    if (!isValid_) {
        LOGE("[Json][ToString] Not Valid Yet.");
        return std::string();
    }
    Json::FastWriter fastWriter;
    // Call omitEndingLineFeed to let JsonCpp not append an \n at the end of string. If not doing so, when passing a
    // minified jsonString, the result of this function will be one byte longer then the original, which may cause the
    // result checked as length invalid by upper logic when the original length is just at the limitation boundary.
    fastWriter.omitEndingLineFeed();
    return fastWriter.write(value_);
}

bool JsonObject::IsFieldPathExist(const FieldPath &inPath) const
{
    if (!isValid_) {
        LOGE("[Json][isExisted] Not Valid Yet.");
        return false;
    }
    int errCode = E_OK;
    (void)GetJsonValueByFieldPath(inPath, errCode); // Ignore return const reference
    return (errCode == E_OK) ? true : false;
}

int JsonObject::GetFieldTypeByFieldPath(const FieldPath &inPath, FieldType &outType) const
{
    if (!isValid_) {
        LOGE("[Json][GetType] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    return GetFieldTypeByJsonValue(valueNode, outType);
}

int JsonObject::GetFieldValueByFieldPath(const FieldPath &inPath, FieldValue &outValue) const
{
    if (!isValid_) {
        LOGE("[Json][GetValue] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    FieldType valueType;
    errCode = GetFieldTypeByJsonValue(valueNode, valueType);
    if (errCode != E_OK) {
        return errCode;
    }
    switch (valueType) {
        case FieldType::LEAF_FIELD_BOOL:
            outValue.boolValue = valueNode.asBool();
            break;
        case FieldType::LEAF_FIELD_INTEGER:
            outValue.integerValue = valueNode.asInt();
            break;
        case FieldType::LEAF_FIELD_LONG:
            outValue.longValue = valueNode.asInt64();
            break;
        case FieldType::LEAF_FIELD_DOUBLE:
            outValue.doubleValue = valueNode.asDouble();
            break;
        case FieldType::LEAF_FIELD_STRING:
            outValue.stringValue = valueNode.asString();
            break;
        default:
            return -E_NOT_SUPPORT;
    }
    return E_OK;
}

int JsonObject::GetSubFieldPath(const FieldPath &inPath, std::set<FieldPath> &outSubPath) const
{
    if (!isValid_) {
        LOGE("[Json][GetSubPath] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    if (valueNode.type() != Json::ValueType::objectValue) {
        return -E_NOT_SUPPORT;
    }
    // Note: the subFields JsonCpp returnout will be different from each other
    std::vector<std::string> subFields = valueNode.getMemberNames();
    for (const auto &eachSubField : subFields) {
        FieldPath eachSubPath = inPath;
        eachSubPath.push_back(eachSubField);
        outSubPath.insert(eachSubPath);
    }
    return E_OK;
}

int JsonObject::GetSubFieldPath(const std::set<FieldPath> &inPath, std::set<FieldPath> &outSubPath) const
{
    for (const auto &eachPath : inPath) {
        int errCode = GetSubFieldPath(eachPath, outSubPath);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int JsonObject::GetSubFieldPathAndType(const FieldPath &inPath, std::map<FieldPath, FieldType> &outSubPathType) const
{
    if (!isValid_) {
        LOGE("[Json][GetSubPathType] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    if (valueNode.type() != Json::ValueType::objectValue) {
        return -E_NOT_SUPPORT;
    }
    // Note: the subFields JsonCpp returnout will be different from each other
    std::vector<std::string> subFields = valueNode.getMemberNames();
    for (const auto &eachSubField : subFields) {
        FieldPath eachSubPath = inPath;
        eachSubPath.push_back(eachSubField);
        FieldType eachSubType;
        errCode = GetFieldTypeByJsonValue(valueNode[eachSubField], eachSubType);
        if (errCode != E_OK) {
            return errCode;
        }
        outSubPathType[eachSubPath] = eachSubType;
    }
    return E_OK;
}

int JsonObject::GetSubFieldPathAndType(const std::set<FieldPath> &inPath,
    std::map<FieldPath, FieldType> &outSubPathType) const
{
    for (const auto &eachPath : inPath) {
        int errCode = GetSubFieldPathAndType(eachPath, outSubPathType);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int JsonObject::GetArraySize(const FieldPath &inPath, uint32_t &outSize) const
{
    if (!isValid_) {
        LOGE("[Json][GetArraySize] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    if (valueNode.type() != Json::ValueType::arrayValue) {
        return -E_NOT_SUPPORT;
    }
    outSize = valueNode.size();
    return E_OK;
}

int JsonObject::GetArrayContentOfStringOrStringArray(const FieldPath &inPath,
    std::vector<std::vector<std::string>> &outContent) const
{
    if (!isValid_) {
        LOGE("[Json][GetArrayContent] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    int errCode = E_OK;
    const Json::Value &valueNode = GetJsonValueByFieldPath(inPath, errCode);
    if (errCode != E_OK) {
        LOGE("[Json][GetArrayContent] Get JsonValue Fail=%d.", errCode);
        return errCode;
    }
    if (valueNode.type() != Json::ValueType::arrayValue) {
        LOGE("[Json][GetArrayContent] Not an array.");
        return -E_NOT_SUPPORT;
    }
    for (uint32_t index = 0; index < valueNode.size(); index++) {
        const Json::Value &eachArrayItem = valueNode[index];
        if (eachArrayItem.isString()) {
            outContent.emplace_back(std::vector<std::string>({eachArrayItem.asString()}));
            continue;
        }
        if (eachArrayItem.isArray()) {
            if (eachArrayItem.empty()) {
                continue; // Ignore empty array-type member
            }
            outContent.emplace_back(std::vector<std::string>());
            errCode = GetStringArrayContentByJsonValue(eachArrayItem, outContent.back());
            if (errCode == E_OK) {
                continue; // Everything ok
            }
        }
        // If reach here, then something is not ok
        outContent.clear();
        LOGE("[Json][GetArrayContent] Not string or array or GetStringArray fail=%d at index=%u.", errCode, index);
        return -E_NOT_SUPPORT;
    }
    return E_OK;
}

namespace {
bool InsertFieldCheckParameter(const FieldPath &inPath, FieldType inType, const FieldValue &inValue,
    uint32_t maxNestDepth)
{
    if (inPath.empty() || inPath.size() > maxNestDepth || inType == FieldType::LEAF_FIELD_ARRAY ||
        inType == FieldType::INTERNAL_FIELD_OBJECT) {
        return false;
    }
    // Infinite double not support
    if (inType == FieldType::LEAF_FIELD_DOUBLE && !std::isfinite(inValue.doubleValue)) {
        return false;
    }
    return true;
}

// Function design for InsertField call on an null-type Json::Value
void LeafJsonNodeAssignValue(Json::Value &leafNode, FieldType inType, const FieldValue &inValue)
{
    switch (inType) {
        case FieldType::LEAF_FIELD_BOOL:
            leafNode = Json::Value(inValue.boolValue);
            break;
        case FieldType::LEAF_FIELD_INTEGER:
            // Cast to Json::Int to avoid "ambiguous call of overloaded function"
            leafNode = Json::Value(static_cast<Json::Int>(inValue.integerValue));
            break;
        case FieldType::LEAF_FIELD_LONG:
            // Cast to Json::Int64 to avoid "ambiguous call of overloaded function"
            leafNode = Json::Value(static_cast<Json::Int64>(inValue.longValue));
            break;
        case FieldType::LEAF_FIELD_DOUBLE:
            leafNode = Json::Value(inValue.doubleValue);
            break;
        case FieldType::LEAF_FIELD_STRING:
            leafNode = Json::Value(inValue.stringValue);
            break;
        case FieldType::LEAF_FIELD_OBJECT:
            leafNode = Json::Value(Json::ValueType::objectValue);
            break;
        default:
            // For LEAF_FIELD_NULL, Do nothing.
            // For LEAF_FIELD_ARRAY and INTERNAL_FIELD_OBJECT, Not Support, had been excluded by InsertField
            return;
    }
}
}

int JsonObject::InsertField(const FieldPath &inPath, FieldType inType, const FieldValue &inValue)
{
    if (!InsertFieldCheckParameter(inPath, inType, inValue, maxNestDepth_)) {
        return -E_INVALID_ARGS;
    }
    if (!isValid_) {
        // Insert on invalid object never fail after parameter check ok, so here no need concern rollback.
        value_ = Json::Value(Json::ValueType::objectValue);
        isValid_ = true;
    }
    Json::Value *exact = nullptr;
    Json::Value *nearest = nullptr;
    uint32_t nearDepth = 0;
    int errCode = LocateJsonValueByFieldPath(inPath, exact, nearest, nearDepth);
    if (errCode != -E_NOT_FOUND) { // Path already exist
        return -E_JSON_INSERT_PATH_EXIST;
    }
    // nearDepth 0 represent for root value. nearDepth equal to inPath.size indicate an exact path match
    if (nearest == nullptr || nearDepth >= inPath.size()) { // Impossible
        return -E_INTERNAL_ERROR;
    }
    if (nearest->type() != Json::ValueType::objectValue) { // path ends with type not object
        return -E_JSON_INSERT_PATH_CONFLICT;
    }
    // Use nearDepth as startIndex pointing to the first field that lacked
    for (uint32_t lackFieldIndex = nearDepth; lackFieldIndex < inPath.size(); lackFieldIndex++) {
        // The new JsonValue is null-type, we can safely add members to an null-type JsonValue which will turn into
        // object-type after member adding. Then move "nearest" to point to the new JsonValue.
        nearest = &((*nearest)[inPath[lackFieldIndex]]);
    }
    // Here "nearest" points to the JsonValue(null-type now) corresponding to the last field
    LeafJsonNodeAssignValue(*nearest, inType, inValue);
    return E_OK;
}

int JsonObject::DeleteField(const FieldPath &inPath)
{
    if (!isValid_) {
        LOGE("[Json][DeleteField] Not Valid Yet.");
        return -E_NOT_PERMIT;
    }
    if (inPath.empty()) {
        return -E_INVALID_ARGS;
    }
    Json::Value *exact = nullptr;
    Json::Value *nearest = nullptr;
    uint32_t nearDepth = 0;
    int errCode = LocateJsonValueByFieldPath(inPath, exact, nearest, nearDepth);
    if (errCode != E_OK) { // Path not exist
        return -E_JSON_DELETE_PATH_NOT_FOUND;
    }
    // nearDepth should be equal to inPath.size() - 1, because nearest is at the parent path of inPath
    if (nearest == nullptr || nearest->type() != Json::ValueType::objectValue || nearDepth != inPath.size() - 1) {
        return -E_INTERNAL_ERROR; // Impossible
    }
    // Remove member from nearest, ignore returned removed Value, use nearDepth as index pointing to last field of path.
    (void)nearest->removeMember(inPath[nearDepth]);
    return E_OK;
}

int JsonObject::GetStringArrayContentByJsonValue(const Json::Value &value,
    std::vector<std::string> &outStringArray) const
{
    if (value.type() != Json::ValueType::arrayValue) {
        LOGE("[Json][GetStringArrayByValue] Not an array.");
        return -E_NOT_SUPPORT;
    }
    for (uint32_t index = 0; index < value.size(); index++) {
        const Json::Value &eachArrayItem = value[index];
        if (!eachArrayItem.isString()) {
            LOGE("[Json][GetStringArrayByValue] Index=%u in Array is not string.", index);
            outStringArray.clear();
            return -E_NOT_SUPPORT;
        }
        outStringArray.push_back(eachArrayItem.asString());
    }
    return E_OK;
}

int JsonObject::GetFieldTypeByJsonValue(const Json::Value &value, FieldType &outType) const
{
    Json::ValueType valueType = value.type();
    switch (valueType) {
        case Json::ValueType::nullValue:
            outType = FieldType::LEAF_FIELD_NULL;
            break;
        case Json::ValueType::booleanValue:
            outType = FieldType::LEAF_FIELD_BOOL;
            break;
        // The case intValue and uintValue cover from INT64_MIN to UINT64_MAX. Inside this range, isInt() take range
        // from INT32_MIN to INT32_MAX, which should be regard as LEAF_FIELD_INTEGER; isInt64() take range from
        // INT64_MIN to INT64_MAX, which should be regard as LEAF_FIELD_LONG if it is not LEAF_FIELD_INTEGER;
        // INT64_MAX + 1 to UINT64_MAX will be regard as LEAF_FIELD_DOUBLE, therefore lose its precision when read out
        // as double value.
        case Json::ValueType::intValue:
        case Json::ValueType::uintValue:
            if (value.isInt()) {
                outType = FieldType::LEAF_FIELD_INTEGER;
            } else if (value.isInt64()) {
                outType = FieldType::LEAF_FIELD_LONG;
            } else {
                outType = FieldType::LEAF_FIELD_DOUBLE; // The isDouble() judge is always true in this case.
            }
            break;
        // Integral value beyond range INT64_MIN to UINT64_MAX will be recognized as realValue and lose its precision.
        // Value in scientific notation or has decimal point will be recognized as realValue without exception,
        // no matter whether the value is large or small, no matter with or without non-zero decimal part.
        // In a word, when regard as DOUBLE type, a value can not guarantee its presision
        case Json::ValueType::realValue:
            // The isDouble() judge is always true in this case. A value exceed double range is not support.
            outType = FieldType::LEAF_FIELD_DOUBLE;
            if (!std::isfinite(value.asDouble())) {
                LOGE("[Json][GetTypeByJson] Infinite double not support.");
                return -E_NOT_SUPPORT;
            }
            break;
        case Json::ValueType::stringValue:
            outType = FieldType::LEAF_FIELD_STRING;
            break;
        case Json::ValueType::arrayValue:
            outType = FieldType::LEAF_FIELD_ARRAY;
            break;
        case Json::ValueType::objectValue:
            if (value.getMemberNames().empty()) {
                outType = FieldType::LEAF_FIELD_OBJECT;
                break;
            }
            outType = FieldType::INTERNAL_FIELD_OBJECT;
            break;
        default:
            LOGE("[Json][GetTypeByJson] no such type.");
            return -E_NOT_SUPPORT;
    }
    return E_OK;
}

const Json::Value &JsonObject::GetJsonValueByFieldPath(const FieldPath &inPath, int &errCode) const
{
    // Root path always exist
    if (inPath.empty()) {
        errCode = E_OK;
        return value_;
    }
    const Json::Value *valueNode = &value_;
    for (const auto &eachPathSegment : inPath) {
        if ((valueNode->type() != Json::ValueType::objectValue) || (!valueNode->isMember(eachPathSegment))) {
            // Current JsonValue is not an object, or no such member field
            errCode = -E_INVALID_PATH;
            return value_;
        }
        valueNode = &((*valueNode)[eachPathSegment]);
    }
    errCode = E_OK;
    return *valueNode;
}

int JsonObject::LocateJsonValueByFieldPath(const FieldPath &inPath, Json::Value *&exact,
    Json::Value *&nearest, uint32_t &nearDepth)
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    exact = &value_;
    nearest = &value_;
    nearDepth = 0;
    if (inPath.empty()) {
        return E_OK;
    }
    for (const auto &eachPathSegment : inPath) {
        nearest = exact; // Let "nearest" trace "exact" before "exact" go deeper
        if (nearest != &value_) {
            nearDepth++; // For each "nearest" trace up "exact", increase nearDepth to indicate where it is.
        }
        if ((exact->type() != Json::ValueType::objectValue) || (!exact->isMember(eachPathSegment))) {
            // "exact" is not an object, or no such member field
            exact = nullptr; // Set "exact" to nullptr indicate exact path not exist
            return -E_NOT_FOUND;
        }
        exact = &((*exact)[eachPathSegment]); // "exact" go deeper
    }
    // Here, JsonValue exist at exact path, "nearest" is "exact" parent.
    return E_OK;
}
#else // OMIT_JSON
uint32_t JsonObject::SetMaxNestDepth(uint32_t nestDepth)
{
    return 0;
}

uint32_t JsonObject::CalculateNestDepth(const std::string &inString)
{
    return 0;
}

uint32_t JsonObject::CalculateNestDepth(const uint8_t *dataBegin, const uint8_t *dataEnd)
{
    return 0;
}

JsonObject::JsonObject(const JsonObject &other) = default;

JsonObject& JsonObject::operator=(const JsonObject &other) = default;

int JsonObject::Parse(const std::string &inString)
{
    LOGW("[Json][Parse] Json Omit From Compile.");
    return -E_NOT_PERMIT;
}

int JsonObject::Parse(const std::vector<uint8_t> &inData)
{
    LOGW("[Json][Parse] Json Omit From Compile.");
    return -E_NOT_PERMIT;
}

int JsonObject::Parse(const uint8_t *dataBegin, const uint8_t *dataEnd)
{
    LOGW("[Json][Parse] Json Omit From Compile.");
    return -E_NOT_PERMIT;
}

bool JsonObject::IsValid() const
{
    return false;
}

std::string JsonObject::ToString() const
{
    return std::string();
}

bool JsonObject::IsFieldPathExist(const FieldPath &inPath) const
{
    return false;
}

int JsonObject::GetFieldTypeByFieldPath(const FieldPath &inPath, FieldType &outType) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetFieldValueByFieldPath(const FieldPath &inPath, FieldValue &outValue) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetSubFieldPath(const FieldPath &inPath, std::set<FieldPath> &outSubPath) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetSubFieldPath(const std::set<FieldPath> &inPath, std::set<FieldPath> &outSubPath) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetSubFieldPathAndType(const FieldPath &inPath, std::map<FieldPath, FieldType> &outSubPathType) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetSubFieldPathAndType(const std::set<FieldPath> &inPath,
    std::map<FieldPath, FieldType> &outSubPathType) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetArraySize(const FieldPath &inPath, uint32_t &outSize) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::GetArrayContentOfStringOrStringArray(const FieldPath &inPath,
    std::vector<std::vector<std::string>> &outContent) const
{
    return -E_NOT_PERMIT;
}

int JsonObject::InsertField(const FieldPath &inPath, FieldType inType, const FieldValue &inValue)
{
    return -E_NOT_PERMIT;
}

int JsonObject::DeleteField(const FieldPath &inPath)
{
    return -E_NOT_PERMIT;
}
#endif // OMIT_JSON
} // namespace DistributedDB
