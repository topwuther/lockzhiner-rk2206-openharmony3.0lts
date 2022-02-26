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

#ifndef DISTRIBUTED_DATA_QUERY_H
#define DISTRIBUTED_DATA_QUERY_H

#include <string>
#include <vector>
#include <sstream>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class DataQuery {
public:
    KVSTORE_API DataQuery();

    KVSTORE_API ~DataQuery() = default;

    // Reset the query.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& Reset();

    // Equal to int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EqualTo(const std::string &field, const int value);

    // Equal to long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EqualTo(const std::string &field, const int64_t value);

    // Equal to double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EqualTo(const std::string &field, const double value);

    // Equal to String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EqualTo(const std::string &field, const std::string &value);

    // Equal to boolean value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EqualTo(const std::string &field, const bool value);

    // Not equal to int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotEqualTo(const std::string &field, const int value);

    // Not equal to long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotEqualTo(const std::string &field, const int64_t value);

    // Not equal to double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotEqualTo(const std::string &field, const double value);

    // Not equal to String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotEqualTo(const std::string &field, const std::string &value);

    // Not equal to boolean value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotEqualTo(const std::string &field, const bool value);

    // Greater than int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThan(const std::string &field, const int value);

    // Greater than long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThan(const std::string &field, const int64_t value);

    // Greater than double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThan(const std::string &field, const double value);

    // Greater than String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThan(const std::string &field, const std::string &value);

    // Less than int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThan(const std::string &field, const int value);

    // Less than long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThan(const std::string &field, const int64_t value);

    // Less than double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThan(const std::string &field, const double value);

    // Less than String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThan(const std::string &field, const std::string &value);

    // Greater than or equal to int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThanOrEqualTo(const std::string &field, const int value);

    // Greater than or equal to long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThanOrEqualTo(const std::string &field, const int64_t value);

    // Greater than or equal to double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThanOrEqualTo(const std::string &field, const double value);

    // Greater than or equal to String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& GreaterThanOrEqualTo(const std::string &field, const std::string &value);

    // Less than or equal to int value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThanOrEqualTo(const std::string &field, const int value);

    // Less than or equal to long value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThanOrEqualTo(const std::string &field, const int64_t value);

    // Less than or equal to double value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThanOrEqualTo(const std::string &field, const double value);

    // Less than or equal to String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& LessThanOrEqualTo(const std::string &field, const std::string &value);

    // Is null field value.
    // Parameters:
    //     field: the field name.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& IsNull(const std::string &field);

    // Is not null field value.
    // Parameters:
    //     field: the field name.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& IsNotNull(const std::string &field);

    // In int value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& InInt(const std::string &field, const std::vector<int> &valueList);

    // In long value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& InLong(const std::string &field, const std::vector<int64_t> &valueList);

    // In Double value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& InDouble(const std::string &field, const std::vector<double> &valueList);

    // In String value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& InString(const std::string &field, const std::vector<std::string> &valueList);

    // Not in int value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotInInt(const std::string &field, const std::vector<int> &valueList);

    // Not in long value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotInLong(const std::string &field, const std::vector<int64_t> &valueList);

    // Not in Double value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotInDouble(const std::string &field, const std::vector<double> &valueList);

    // Not in String value list.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& NotInString(const std::string &field, const std::vector<std::string> &valueList);

    // Like String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& Like(const std::string &field, const std::string &value);

    // Unlike String value.
    // Parameters:
    //     field: the field name.
    //     value: the field value list.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& Unlike(const std::string &field, const std::string &value);

    // And operator.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& And();

    // Or operator.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& Or();

    // Order by ascent.
    // Parameters:
    //     field: the field name.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& OrderByAsc(const std::string &field);

    // Order by descent.
    // Parameters:
    //     field: the field name.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& OrderByDesc(const std::string &field);

    // Limit result size.
    // Parameters:
    //     number: the number of results.
    //     offset: the start position.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& Limit(const int number, const int offset);

    // Begin group.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& BeginGroup();

    // End group.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& EndGroup();

    // Select results with specified key prefix.
    // Parameters:
    //     prefix: key prefix.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& KeyPrefix(const std::string &prefix);

    // Select results with suggested index.
    // Parameters:
    //     index: suggested index.
    // Return:
    //     This Query.
    KVSTORE_API DataQuery& SetSuggestIndex(const std::string &index);

    // Get string representation
    // Return:
    //     String representation of this query.
    KVSTORE_API std::string ToString() const;

    // equal to
    static const std::string EQUAL_TO;

    // not equal to
    static const std::string NOT_EQUAL_TO;

    // greater than
    static const std::string GREATER_THAN;

    // less than
    static const std::string LESS_THAN;

    // greater than or equal to
    static const std::string GREATER_THAN_OR_EQUAL_TO;

    // less than or equal to
    static const std::string LESS_THAN_OR_EQUAL_TO;

    // is null
    static const std::string IS_NULL;

    // in
    static const std::string IN;

    // not in
    static const std::string NOT_IN;

    // like
    static const std::string LIKE;

    // not like
    static const std::string NOT_LIKE;

    // and
    static const std::string AND;

    // or
    static const std::string OR;

    // order by asc
    static const std::string ORDER_BY_ASC;

    // order by desc
    static const std::string ORDER_BY_DESC;

    // limit
    static const std::string LIMIT;

    // space
    static const std::string SPACE;

    // '^'
    static const std::string SPECIAL;

    // '^' escape
    static const std::string SPECIAL_ESCAPE;

    // space escape
    static const std::string SPACE_ESCAPE;

    // empty string
    static const std::string EMPTY_STRING;

    // start in
    static const std::string START_IN;

    // end in
    static const std::string END_IN;

    // begin group
    static const std::string BEGIN_GROUP;

    // end group
    static const std::string END_GROUP;

    // key prefix
    static const std::string KEY_PREFIX;

    // device id
    static const std::string DEVICE_ID;

    // is not null
    static const std::string IS_NOT_NULL;

    // type string
    static const std::string TYPE_STRING;

    // type integer
    static const std::string TYPE_INTEGER;

    // type long
    static const std::string TYPE_LONG;

    // type double
    static const std::string TYPE_DOUBLE;

    // type boolean
    static const std::string TYPE_BOOLEAN;

    // value true
    static const std::string VALUE_TRUE;

    // value false
    static const std::string VALUE_FALSE;

    // suggested index
    static const std::string SUGGEST_INDEX;
private:
    std::string str_;

    template<typename T>
    void AppendCommon(const std::string &keyword, const std::string &fieldType, std::string &field, const T &value);

    void AppendCommonString(const std::string &keyword, const std::string &fieldType,
                            std::string &field, std::string &value);

    void AppendCommonBoolean(const std::string &keyword, const std::string &fieldType,
                             std::string &field, const bool &value);

    void AppendCommonString(const std::string &keyword, std::string &field, std::string &value);

    template<typename T>
    void AppendCommonList(const std::string &keyword, const std::string &fieldType,
                          std::string &field, const std::vector<T> &valueList);

    void AppendCommonListString(const std::string &keyword, const std::string &fieldType,
                                std::string &field, std::vector<std::string> &valueList);

    void EscapeSpace(std::string &input);

    bool ValidateField(const std::string &field);

    bool ValidateValue(const std::string &value);

    bool ValidateStringValueList(const std::vector<std::string> &valueList);

    template<typename T>
    std::string BasicToString(const T &value);
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // DISTRIBUTED_DATA_QUERY_H
