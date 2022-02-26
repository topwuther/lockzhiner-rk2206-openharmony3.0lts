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

#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include <map>
#include <set>
#include <string>
#include <vector>
#ifndef OMIT_JSON
#include <json/json.h>
#endif
#include "db_types.h"

namespace DistributedDB {
// JsonObject is the abstraction of JsonString, it hides the JsonLib that we use and other messy details.
// JsonObject do not support concurrence inherently, use it locally or under mutex protection.
class JsonObject {
public:
    // Set max allowed nest depth and return the value before set.
    static uint32_t SetMaxNestDepth(uint32_t nestDepth);
    // Calculate nest depth when json string is legal or estimate depth by legal part from illegal json.
    static uint32_t CalculateNestDepth(const std::string &inString);
    static uint32_t CalculateNestDepth(const uint8_t *dataBegin, const uint8_t *dataEnd);

    // Support default constructor, copy constructor and copy assignment
    JsonObject() = default;
    ~JsonObject() = default;
    JsonObject(const JsonObject &);
    JsonObject& operator=(const JsonObject &);
    // Move constructor and move assignment is not need currently
    JsonObject(JsonObject &&) = delete;
    JsonObject& operator=(JsonObject &&) = delete;

    // Should be called on an invalid JsonObject, create new JsonObject if need to reparse
    // Require the type of the root to be JsonObject, otherwise parse fail
    int Parse(const std::string &inString);
    int Parse(const std::vector<uint8_t> &inData); // Whether ends with '\0' in vector is OK
    // The end refer to the byte after the last valid byte
    int Parse(const uint8_t *dataBegin, const uint8_t *dataEnd);

    bool IsValid() const;
    // Unnecessary spacing will be removed and fieldname resorted by lexicographical order
    std::string ToString() const;

    bool IsFieldPathExist(const FieldPath &inPath) const;
    int GetFieldTypeByFieldPath(const FieldPath &inPath, FieldType &outType) const;
    int GetFieldValueByFieldPath(const FieldPath &inPath, FieldValue &outValue) const;
    // An empty fieldpath indicate the root, the outSubPath should be empty before call, we will not empty it at first.
    // If inPath is of multiple path, then outSubPath is combination of result of each inPath.
    int GetSubFieldPath(const FieldPath &inPath, std::set<FieldPath> &outSubPath) const;
    int GetSubFieldPath(const std::set<FieldPath> &inPath, std::set<FieldPath> &outSubPath) const;
    int GetSubFieldPathAndType(const FieldPath &inPath, std::map<FieldPath, FieldType> &outSubPathType) const;
    int GetSubFieldPathAndType(const std::set<FieldPath> &inPath, std::map<FieldPath, FieldType> &outSubPathType) const;
    // If inPath not refer to an array, return error.
    int GetArraySize(const FieldPath &inPath, uint32_t &outSize) const;
    // If inPath not refer to an array, return error. If not all members are string or array type, return error.
    // If array-type member is empty, ignore. If not all members of the array-type member are string, return error.
    int GetArrayContentOfStringOrStringArray(const FieldPath &inPath,
        std::vector<std::vector<std::string>> &outContent) const;

    // Can be called no matter JsonObject valid or not. Invalid turn into valid after call(insert on invalid never fail
    // if parameter is valid). An empty inPath is not allowed. LEAF_FIELD_ARRAY and INTERNAL_FIELD_OBJECT is not
    // supported. infinite double is not support. inValue is ignored for LEAF_FIELD_NULL. If inPath already exist,
    // returns -E_JSON_INSERT_PATH_EXIST, if nearest path ends with type not object, rets -E_JSON_INSERT_PATH_CONFLICT.
    // Otherwise insert field as well as filling up intermediate field, then returns E_OK;
    int InsertField(const FieldPath &inPath, FieldType inType, const FieldValue &inValue);
    // Should be called on an valid JsonObject. Never turn into invalid after call. An empty inPath is not allowed.
    // If inPath not exist, returns -E_JSON_DELETE_PATH_NOT_FOUND. Otherwise delete field from its parent returns E_OK;
    int DeleteField(const FieldPath &inPath);
private:
#ifndef OMIT_JSON
    // Auxiliary Method: If inPath not refer to an array, return error. If not all members are string, return error.
    int GetStringArrayContentByJsonValue(const Json::Value &value, std::vector<std::string> &outStringArray) const;
    // Common Type Judgement Logic
    int GetFieldTypeByJsonValue(const Json::Value &value, FieldType &outType) const;
    // Return E_OK if JsonValueNode found at exact the path, otherwise not E_OK
    const Json::Value &GetJsonValueByFieldPath(const FieldPath &inPath, int &errCode) const;
    // REQUIRE: JsonObject is valid(Root value is object type).
    // If inPath empty(means root), set exact and nearest to root value and nearDepth to 0, then ret E_OK;
    // If JsonValue exist at exact path, set exact to this JsonValue, set nearest to its parent JsonValue, set nearDepth
    // to the depth of this parent JsonValue, then ret E_OK;
    // If exact path no exist, set exact to nullptr, set nearest to nearest JsonValue that can be found, set nearDepth
    // to the depth of this nearest JsonValue, then ret -E_NOT_FOUND;
    int LocateJsonValueByFieldPath(const FieldPath &inPath, Json::Value *&exact,
        Json::Value *&nearest, uint32_t &nearDepth);

    static uint32_t maxNestDepth_;

    bool isValid_ = false;
    Json::Value value_;
#endif
};
} // namespace DistributedDB
#endif // JSON_OBJECT_H