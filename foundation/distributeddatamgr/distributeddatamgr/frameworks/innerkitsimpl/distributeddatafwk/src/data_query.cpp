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

#define LOG_TAG "DataQuery"

#include "data_query.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
const std::string DataQuery::EQUAL_TO = "^EQUAL";
const std::string DataQuery::NOT_EQUAL_TO = "^NOT_EQUAL";
const std::string DataQuery::GREATER_THAN = "^GREATER";
const std::string DataQuery::LESS_THAN = "^LESS";
const std::string DataQuery::GREATER_THAN_OR_EQUAL_TO = "^GREATER_EQUAL";
const std::string DataQuery::LESS_THAN_OR_EQUAL_TO = "^LESS_EQUAL";
const std::string DataQuery::IS_NULL = "^IS_NULL";
const std::string DataQuery::IN = "^IN";
const std::string DataQuery::NOT_IN = "^NOT_IN";
const std::string DataQuery::LIKE = "^LIKE";
const std::string DataQuery::NOT_LIKE = "^NOT_LIKE";
const std::string DataQuery::AND = "^AND";
const std::string DataQuery::OR = "^OR";
const std::string DataQuery::ORDER_BY_ASC = "^ASC";
const std::string DataQuery::ORDER_BY_DESC = "^DESC";
const std::string DataQuery::LIMIT = "^LIMIT";
const std::string DataQuery::SPACE = " ";
const std::string DataQuery::SPECIAL = "^";
const std::string DataQuery::SPECIAL_ESCAPE = "(^)";
const std::string DataQuery::SPACE_ESCAPE = "^^";
const std::string DataQuery::EMPTY_STRING = "^EMPTY_STRING";
const std::string DataQuery::START_IN = "^START";
const std::string DataQuery::END_IN = "^END";
const std::string DataQuery::BEGIN_GROUP = "^BEGIN_GROUP";
const std::string DataQuery::END_GROUP = "^END_GROUP";
const std::string DataQuery::KEY_PREFIX = "^KEY_PREFIX";
const std::string DataQuery::DEVICE_ID = "^DEVICE_ID";
const std::string DataQuery::IS_NOT_NULL = "^IS_NOT_NULL";
const std::string DataQuery::TYPE_STRING = "STRING";
const std::string DataQuery::TYPE_INTEGER = "INTEGER";
const std::string DataQuery::TYPE_LONG = "LONG";
const std::string DataQuery::TYPE_DOUBLE = "DOUBLE";
const std::string DataQuery::TYPE_BOOLEAN = "BOOL";
const std::string DataQuery::VALUE_TRUE = "true";
const std::string DataQuery::VALUE_FALSE = "false";
const std::string DataQuery::SUGGEST_INDEX = "^SUGGEST_INDEX";
constexpr int MAX_QUERY_LENGTH = 5 * 1024; // Max query string length 5k

DataQuery::DataQuery()
{}

DataQuery& DataQuery::Reset()
{
    str_ = "";
    return *this;
}

DataQuery& DataQuery::EqualTo(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(EQUAL_TO, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::EqualTo(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(EQUAL_TO, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::EqualTo(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(EQUAL_TO, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::EqualTo(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(EQUAL_TO, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::EqualTo(const std::string &field, const bool value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonBoolean(EQUAL_TO, TYPE_BOOLEAN, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::NotEqualTo(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(NOT_EQUAL_TO, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::NotEqualTo(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(NOT_EQUAL_TO, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::NotEqualTo(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(NOT_EQUAL_TO, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::NotEqualTo(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(NOT_EQUAL_TO, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::NotEqualTo(const std::string &field, const bool value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonBoolean(NOT_EQUAL_TO, TYPE_BOOLEAN, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThan(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThan(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThan(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThan(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(GREATER_THAN, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::LessThan(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThan(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThan(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThan(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(LESS_THAN, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThanOrEqualTo(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN_OR_EQUAL_TO, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThanOrEqualTo(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN_OR_EQUAL_TO, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThanOrEqualTo(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(GREATER_THAN_OR_EQUAL_TO, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::GreaterThanOrEqualTo(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(GREATER_THAN_OR_EQUAL_TO, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::LessThanOrEqualTo(const std::string &field, const int value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN_OR_EQUAL_TO, TYPE_INTEGER, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThanOrEqualTo(const std::string &field, const int64_t value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN_OR_EQUAL_TO, TYPE_LONG, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThanOrEqualTo(const std::string &field, const double value)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommon(LESS_THAN_OR_EQUAL_TO, TYPE_DOUBLE, myField, value);
    }
    return *this;
}

DataQuery& DataQuery::LessThanOrEqualTo(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(LESS_THAN_OR_EQUAL_TO, TYPE_STRING, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::IsNull(const std::string &field)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        str_.append(SPACE);
        str_.append(IS_NULL);
        str_.append(SPACE);
        EscapeSpace(myField);
        str_.append(myField);
    }
    return *this;
}

DataQuery& DataQuery::IsNotNull(const std::string &field)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        str_.append(SPACE);
        str_.append(IS_NOT_NULL);
        str_.append(SPACE);
        EscapeSpace(myField);
        str_.append(myField);
    }
    return *this;
}

DataQuery& DataQuery::InInt(const std::string &field, const std::vector<int> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(IN, TYPE_INTEGER, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::InLong(const std::string &field, const std::vector<int64_t> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(IN, TYPE_LONG, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::InDouble(const std::string &field, const std::vector<double> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(IN, TYPE_DOUBLE, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::InString(const std::string &field, const std::vector<std::string> &valueList)
{
    std::string myField = field;
    std::vector<std::string> myValueList(valueList);
    if (ValidateField(myField)) {
        AppendCommonListString(IN, TYPE_STRING, myField, myValueList);
    }
    return *this;
}

DataQuery& DataQuery::NotInInt(const std::string &field, const std::vector<int> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(NOT_IN, TYPE_INTEGER, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::NotInLong(const std::string &field, const std::vector<int64_t> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(NOT_IN, TYPE_LONG, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::NotInDouble(const std::string &field, const std::vector<double> &valueList)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        AppendCommonList(NOT_IN, TYPE_DOUBLE, myField, valueList);
    }
    return *this;
}

DataQuery& DataQuery::NotInString(const std::string &field, const std::vector<std::string> &valueList)
{
    std::string myField = field;
    std::vector<std::string> myValueList(valueList);
    if (ValidateField(myField)) {
        AppendCommonListString(NOT_IN, TYPE_STRING, myField, myValueList);
    }
    return *this;
}

DataQuery& DataQuery::Like(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(LIKE, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::Unlike(const std::string &field, const std::string &value)
{
    std::string myField = field;
    std::string myValue = value;
    if (ValidateField(myField)) {
        AppendCommonString(NOT_LIKE, myField, myValue);
    }
    return *this;
}

DataQuery& DataQuery::And()
{
    str_.append(SPACE);
    str_.append(AND);
    return *this;
}

DataQuery& DataQuery::Or()
{
    str_.append(SPACE);
    str_.append(OR);
    return *this;
}

DataQuery& DataQuery::OrderByAsc(const std::string &field)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        str_.append(SPACE);
        str_.append(ORDER_BY_ASC);
        str_.append(SPACE);
        EscapeSpace(myField);
        str_.append(myField);
    }
    return *this;
}

DataQuery& DataQuery::OrderByDesc(const std::string &field)
{
    std::string myField = field;
    if (ValidateField(myField)) {
        str_.append(SPACE);
        str_.append(ORDER_BY_DESC);
        str_.append(SPACE);
        EscapeSpace(myField);
        str_.append(myField);
    }
    return *this;
}

DataQuery& DataQuery::Limit(const int number, const int offset)
{
    if (number < 0 || offset < 0) {
        ZLOGE("Invalid number param");
        return *this;
    }
    str_.append(SPACE);
    str_.append(LIMIT);
    str_.append(SPACE);
    str_.append(BasicToString(number));
    str_.append(SPACE);
    str_.append(BasicToString(offset));
    return *this;
}

DataQuery& DataQuery::BeginGroup()
{
    str_.append(SPACE);
    str_.append(BEGIN_GROUP);
    return *this;
}

DataQuery& DataQuery::EndGroup()
{
    str_.append(SPACE);
    str_.append(END_GROUP);
    return *this;
}

DataQuery& DataQuery::KeyPrefix(const std::string &prefix)
{
    std::string myPrefix = prefix;
    if (ValidateField(myPrefix)) {
        str_.append(SPACE);
        str_.append(KEY_PREFIX);
        str_.append(SPACE);
        EscapeSpace(myPrefix);
        str_.append(myPrefix);
    }
    return *this;
}

DataQuery& DataQuery::SetSuggestIndex(const std::string &index)
{
    std::string suggestIndex = index;
    if (ValidateField(suggestIndex)) {
        str_.append(SPACE);
        str_.append(SUGGEST_INDEX);
        str_.append(SPACE);
        EscapeSpace(suggestIndex);
        str_.append(suggestIndex);
    }
    return *this;
}

std::string DataQuery::ToString() const
{
    if (str_.length() > MAX_QUERY_LENGTH) {
        ZLOGE("Query is too long");
        return std::string();
    }
    std::string str(str_.begin(), str_.end());
    return str;
}

template<typename T>
void DataQuery::AppendCommon(const std::string &keyword, const std::string &fieldType,
                             std::string &field, const T &value)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    str_.append(fieldType);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    str_.append(BasicToString(value));
}

void DataQuery::AppendCommonString(const std::string &keyword, const std::string &fieldType,
                                   std::string &field, std::string &value)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    str_.append(fieldType);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    EscapeSpace(value);
    str_.append(value);
}

void DataQuery::AppendCommonBoolean(const std::string &keyword, const std::string &fieldType,
                                    std::string &field, const bool &value)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    str_.append(fieldType);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    if (value) {
        str_.append(VALUE_TRUE);
    } else {
        str_.append(VALUE_FALSE);
    }
}

void DataQuery::AppendCommonString(const std::string &keyword, std::string &field, std::string &value)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    EscapeSpace(value);
    str_.append(value);
}

template<typename T>
void DataQuery::AppendCommonList(const std::string &keyword, const std::string &fieldType,
                                 std::string &field, const std::vector<T> &valueList)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    str_.append(fieldType);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    str_.append(START_IN);
    str_.append(SPACE);
    for (T object : valueList) {
        str_.append(BasicToString(object));
        str_.append(SPACE);
    }
    str_.append(END_IN);
}

void DataQuery::AppendCommonListString(const std::string &keyword, const std::string &fieldType,
                                       std::string &field, std::vector<std::string> &valueList)
{
    str_.append(SPACE);
    str_.append(keyword);
    str_.append(SPACE);
    str_.append(fieldType);
    str_.append(SPACE);
    EscapeSpace(field);
    str_.append(field);
    str_.append(SPACE);
    str_.append(START_IN);
    str_.append(SPACE);
    for (std::string str : valueList) {
        EscapeSpace(str);
        str_.append(str);
        str_.append(SPACE);
    }
    str_.append(END_IN);
}

void DataQuery::EscapeSpace(std::string &input)
{
    if (input.length() == 0) {
        input = EMPTY_STRING;
    }
    size_t index = 0; // search from the beginning of the string
    while (true) {
        index = input.find(DataQuery::SPECIAL, index);
        if (index == std::string::npos) {
            break;
        }
        input.replace(index, 1, DataQuery::SPECIAL_ESCAPE); // 1 char to be replaced
        index += 3; // replaced with 3 chars, keep searching the remaining string
    }
    index = 0; // search from the beginning of the string
    while (true) {
        index = input.find(DataQuery::SPACE, index);
        if (index == std::string::npos) {
            break;
        }
        input.replace(index, 1, DataQuery::SPACE_ESCAPE); // 1 char to be replaced
        index += 2; // replaced with 2 chars, keep searching the remaining string
    }
}

bool DataQuery::ValidateField(const std::string &field)
{
    if (field.empty() || field.find(DataQuery::SPECIAL) != std::string::npos) {
        ZLOGE("invalid string argument");
        return false;
    }
    return true;
}

template<typename T>
std::string DataQuery::BasicToString(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
}  // namespace DistributedKv
}  // namespace OHOS
