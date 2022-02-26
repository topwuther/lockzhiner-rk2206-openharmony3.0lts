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

#ifndef VALUE_OBJECT_H
#define VALUE_OBJECT_H

#include "json_object.h"

namespace DistributedDB {
// ValueObject is the abstraction of value of KvEntry, a value is not always an json in different solutions.
// Thus, ValueObject can't just inherit JsonObject, although their methods are nearly the same.
class ValueObject {
public:
    // Support default constructor, copy constructor and copy assignment
    ValueObject() = default;
    ~ValueObject() = default;
    ValueObject(const ValueObject &);
    ValueObject& operator=(const ValueObject &);
    // Move constructor and move assignment is not need currently
    ValueObject(ValueObject &&) = delete;
    ValueObject& operator=(ValueObject &&) = delete;

    // Should be called on an invalid ValueObject, create new ValueObject if need to reparse
    int Parse(const std::string &inString);
    int Parse(const std::vector<uint8_t> &inData); // Whether ends with '\0' in vector is OK
    // The end refer to the byte after the last valid byte
    int Parse(const uint8_t *dataBegin, const uint8_t *dataEnd, uint32_t offset = 0);

    bool IsValid() const;
    // Unnecessary spacing will be removed and fieldname resorted by lexicographical order
    std::string ToString() const;
    void WriteIntoVector(std::vector<uint8_t> &outData) const; // An vector version ToString

    bool IsFieldPathExist(const FieldPath &inPath) const;
    int GetFieldTypeByFieldPath(const FieldPath &inPath, FieldType &outType) const;
    int GetFieldValueByFieldPath(const FieldPath &inPath, FieldValue &outValue) const;
    // An empty fieldpath indicate the root, the outSubPath should be empty before call, we will not empty it at first.
    // If inPath is of multiple path, then outSubPath is combination of result of each inPath.
    int GetSubFieldPath(const FieldPath &inPath, std::set<FieldPath> &outSubPath) const;
    int GetSubFieldPath(const std::set<FieldPath> &inPath, std::set<FieldPath> &outSubPath) const;
    int GetSubFieldPathAndType(const FieldPath &inPath, std::map<FieldPath, FieldType> &outSubPathType) const;
    int GetSubFieldPathAndType(const std::set<FieldPath> &inPath, std::map<FieldPath, FieldType> &outSubPathType) const;

    // Can be called no matter ValueObject valid or not. Invalid turn into valid after successful call. An empty inPath
    // is not allowed. LEAF_FIELD_ARRAY and INTERNAL_FIELD_OBJECT is not supported. infinite double is not support.
    // inValue is ignored for LEAF_FIELD_NULL. If inPath already exist or nearest path ends with type not object,
    // returns not E_OK. Otherwise insert field as well as filling up intermediate field, then returns E_OK;
    int InsertField(const FieldPath &inPath, FieldType inType, const FieldValue &inValue);
    // Should be called on an valid ValueObject. Never turn into invalid after call. An empty inPath is not allowed.
    // If inPath not exist, returns not E_OK. Otherwise delete field from its parent returns E_OK;
    int DeleteField(const FieldPath &inPath);
private:
    bool isValid_ = false;
    JsonObject value_;
    std::vector<uint8_t> dataBeforeOffset_;
};
} // namespace DistributedDB
#endif // VALUE_OBJECT_H