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

#include "value_object.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
ValueObject::ValueObject(const ValueObject &other)
{
    isValid_ = other.isValid_;
    value_ = other.value_;
    dataBeforeOffset_ = other.dataBeforeOffset_;
}

ValueObject& ValueObject::operator=(const ValueObject &other)
{
    if (&other != this) {
        isValid_ = other.isValid_;
        value_ = other.value_;
        dataBeforeOffset_ = other.dataBeforeOffset_;
    }
    return *this;
}

int ValueObject::Parse(const std::string &inString)
{
    if (isValid_) {
        return -E_NOT_PERMIT;
    }
    int errCode = value_.Parse(inString);
    isValid_ = ((errCode == E_OK) ? true : false);
    return errCode;
}

int ValueObject::Parse(const std::vector<uint8_t> &inData)
{
    if (isValid_) {
        return -E_NOT_PERMIT;
    }
    int errCode = value_.Parse(inData);
    isValid_ = ((errCode == E_OK) ? true : false);
    return errCode;
}

int ValueObject::Parse(const uint8_t *dataBegin, const uint8_t *dataEnd, uint32_t offset)
{
    if (isValid_) {
        return -E_NOT_PERMIT;
    }
    if (dataBegin == nullptr || dataBegin >= dataEnd || offset >= static_cast<uint64_t>(dataEnd - dataBegin)) {
        LOGE("[Value][Parse] Data range invalid: dataEnd - dataBegin=%lld, offset=%u",
            static_cast<int64_t>(dataEnd - dataBegin), offset);
        return -E_INVALID_ARGS;
    }
    int errCode = value_.Parse(dataBegin + offset, dataEnd);
    if (errCode != E_OK) {
        return errCode;
    }
    dataBeforeOffset_.assign(dataBegin, dataBegin + offset);
    isValid_ = true;
    return E_OK;
}

bool ValueObject::IsValid() const
{
    return isValid_;
}

std::string ValueObject::ToString() const
{
    if (dataBeforeOffset_.empty()) {
        return value_.ToString();
    }
    // It is OK if '\0' exist in dataBeforeOffset_, when call string.size, '\0' will not disturb
    std::string outString(dataBeforeOffset_.begin(), dataBeforeOffset_.end());
    outString += value_.ToString();
    return outString;
}

void ValueObject::WriteIntoVector(std::vector<uint8_t> &outData) const
{
    // If not valid, valueStr and dataBeforeOffset_ will be empty
    std::string valueStr = value_.ToString();
    // If valid, dataBeforeOffset_ may be empty
    outData.insert(outData.end(), dataBeforeOffset_.begin(), dataBeforeOffset_.end());
    outData.insert(outData.end(), valueStr.begin(), valueStr.end());
}

bool ValueObject::IsFieldPathExist(const FieldPath &inPath) const
{
    return value_.IsFieldPathExist(inPath);
}

int ValueObject::GetFieldTypeByFieldPath(const FieldPath &inPath, FieldType &outType) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetFieldTypeByFieldPath(inPath, outType);
}

int ValueObject::GetFieldValueByFieldPath(const FieldPath &inPath, FieldValue &outValue) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetFieldValueByFieldPath(inPath, outValue);
}

int ValueObject::GetSubFieldPath(const FieldPath &inPath, std::set<FieldPath> &outSubPath) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetSubFieldPath(inPath, outSubPath);
}

int ValueObject::GetSubFieldPath(const std::set<FieldPath> &inPath, std::set<FieldPath> &outSubPath) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetSubFieldPath(inPath, outSubPath);
}

int ValueObject::GetSubFieldPathAndType(const FieldPath &inPath, std::map<FieldPath, FieldType> &outSubPathType) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetSubFieldPathAndType(inPath, outSubPathType);
}

int ValueObject::GetSubFieldPathAndType(const std::set<FieldPath> &inPath,
    std::map<FieldPath, FieldType> &outSubPathType) const
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.GetSubFieldPathAndType(inPath, outSubPathType);
}

int ValueObject::InsertField(const FieldPath &inPath, FieldType inType, const FieldValue &inValue)
{
    int errCode = value_.InsertField(inPath, inType, inValue);
    if (errCode == E_OK) {
        isValid_ = true;
    }
    return errCode;
}

int ValueObject::DeleteField(const FieldPath &inPath)
{
    if (!isValid_) {
        return -E_NOT_PERMIT;
    }
    return value_.DeleteField(inPath);
}
} // namespace DistributedDB
