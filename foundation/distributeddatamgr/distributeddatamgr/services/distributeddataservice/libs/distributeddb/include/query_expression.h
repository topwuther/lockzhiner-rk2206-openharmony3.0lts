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

#ifndef DISTRIBUTEDDB_QUERY_EXPRESSION_H
#define DISTRIBUTEDDB_QUERY_EXPRESSION_H

#include <string>
#include <vector>
#include <list>

#include "types_export.h"

namespace DistributedDB {
enum class QueryValueType {
    VALUE_TYPE_INVALID = -1,
    VALUE_TYPE_NULL,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_LONG,
    VALUE_TYPE_DOUBLE,
    VALUE_TYPE_STRING,
};

enum class QueryObjType {
    OPER_ILLEGAL = -1,
    QUERY_VALUE,
    EQUALTO,
    NOT_EQUALTO,
    GREATER_THAN,
    LESS_THAN,
    GREATER_THAN_OR_EQUALTO,
    LESS_THAN_OR_EQUALTO,
    LIKE,
    NOT_LIKE,
    IS_NULL,
    IS_NOT_NULL,
    IN,
    NOT_IN,
    QUERY_BY_KEY_PREFIX,
    BEGIN_GROUP,
    END_GROUP,
    AND,
    OR,
    LIMIT,
    ORDERBY,
    SUGGEST_INDEX,
};

struct QueryObjNode {
    QueryObjType operFlag = QueryObjType::OPER_ILLEGAL;
    std::string fieldName = "";
    QueryValueType type = QueryValueType::VALUE_TYPE_INVALID;
    std::vector<FieldValue> fieldValue = {};
};

class QueryExpression final {
public:
    DB_SYMBOL QueryExpression();
    DB_SYMBOL ~QueryExpression() {};

    void EqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void NotEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void GreaterThan(const std::string &field, const QueryValueType type, const FieldValue &value);

    void LessThan(const std::string &field, const QueryValueType type, const FieldValue &value);

    void GreaterThanOrEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void LessThanOrEqualTo(const std::string &field, const QueryValueType type, const FieldValue &value);

    void OrderBy(const std::string &field, bool isAsc);

    void Limit(int number, int offset);

    void Like(const std::string &field, const std::string &value);
    void NotLike(const std::string &field, const std::string &value);

    void In(const std::string &field, const QueryValueType type, const std::vector<FieldValue> &values);
    void NotIn(const std::string &field, const QueryValueType type, const std::vector<FieldValue> &values);

    void IsNull(const std::string &field);
    void IsNotNull(const std::string &field);

    void And();

    void Or();

    void BeginGroup();

    void EndGroup();

    void Reset();

    void QueryByPrefixKey(const std::vector<uint8_t> &key);

    void QueryBySuggestIndex(const std::string &indexName);

    std::vector<uint8_t> GetPreFixKey();

    std::string GetSuggestIndex();

    const std::list<QueryObjNode> &GetQueryExpression();

    void SetErrFlag(bool flag);
    bool GetErrFlag();

private:
    void AssemblyQueryInfo(const QueryObjType querrOperType, const std::string &field,
        const QueryValueType type, const std::vector<FieldValue> &value, bool isNeedFieldPath);

    std::list<QueryObjNode> queryInfo_;
    bool errFlag_ = true;
    std::vector<uint8_t> prefixKey_;
    std::string suggestIndex_;
};

// specialize for double
class GetQueryValueType {
public:
    static QueryValueType GetFieldTypeAndValue(const double &queryValue, FieldValue &fieldValue)
    {
        fieldValue.doubleValue = queryValue;
        return QueryValueType::VALUE_TYPE_DOUBLE;
    }
    static QueryValueType GetFieldTypeAndValue(const int &queryValue, FieldValue &fieldValue)
    {
        fieldValue.integerValue = queryValue;
        return QueryValueType::VALUE_TYPE_INTEGER;
    }
    static QueryValueType GetFieldTypeAndValue(const int64_t &queryValue, FieldValue &fieldValue)
    {
        fieldValue.longValue = queryValue;
        return QueryValueType::VALUE_TYPE_LONG;
    }
    static QueryValueType GetFieldTypeAndValue(const bool &queryValue, FieldValue &fieldValue)
    {
        fieldValue.boolValue = queryValue;
        return QueryValueType::VALUE_TYPE_BOOL;
    }
    static QueryValueType GetFieldTypeAndValue(const std::string &queryValue, FieldValue &fieldValue)
    {
        fieldValue.stringValue = queryValue;
        return QueryValueType::VALUE_TYPE_STRING;
    }
    static QueryValueType GetFieldTypeAndValue(const char *queryValue, FieldValue &fieldValue)
    {
        if (queryValue == nullptr) {
            return QueryValueType::VALUE_TYPE_STRING;
        }
        fieldValue.stringValue = queryValue;
        return QueryValueType::VALUE_TYPE_STRING;
    }
};
}
#endif