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
#include "query_expression.h"
#include "log_print.h"
#include "schema_utils.h"
#include "db_errno.h"

namespace DistributedDB {
namespace
{
    const int MAX_OPR_TIMES = 256;
} // namespace

void QueryExpression::AssemblyQueryInfo(const QueryObjType queryOperType, const std::string& field,
    const QueryValueType type, const std::vector<FieldValue> &values, bool isNeedFieldPath = true)
{
    if (queryInfo_.size() > MAX_OPR_TIMES) {
        SetErrFlag(false);
        LOGE("Operate too much times!");
        return;
    }

    if (!GetErrFlag()) {
        LOGE("Illegal data node!");
        return;
    }

    if (isNeedFieldPath) {
        FieldPath outPath;
        if (SchemaUtils::ParseAndCheckFieldPath(field, outPath) != E_OK) {
            SetErrFlag(false);
            LOGE("Field path illegal!");
            return;
        }
    }
    queryInfo_.emplace_back(QueryObjNode{queryOperType, field, type, values});
}

QueryExpression::QueryExpression()
    : errFlag_(true)
{}

void QueryExpression::EqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::EQUALTO, field, type, fieldValues);
}

void QueryExpression::NotEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::NOT_EQUALTO, field, type, fieldValues);
}

void QueryExpression::GreaterThan(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::GREATER_THAN, field, type, fieldValues);
}

void QueryExpression::LessThan(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::LESS_THAN, field, type, fieldValues);
}

void QueryExpression::GreaterThanOrEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::GREATER_THAN_OR_EQUALTO, field, type, fieldValues);
}

void QueryExpression::LessThanOrEqualTo(const std::string& field, const QueryValueType type, const FieldValue &value)
{
    if (type == QueryValueType::VALUE_TYPE_BOOL) {
        LOGD("Prohibit the use of bool for comparison!");
        SetErrFlag(false);
    }
    std::vector<FieldValue> fieldValues{value};
    AssemblyQueryInfo(QueryObjType::LESS_THAN_OR_EQUALTO, field, type, fieldValues);
}

void QueryExpression::OrderBy(const std::string& field, bool isAsc)
{
    FieldValue fieldValue;
    fieldValue.boolValue = isAsc;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::ORDERBY, field, QueryValueType::VALUE_TYPE_BOOL, fieldValues);
}

void QueryExpression::Like(const std::string& field, const std::string &value)
{
    FieldValue fieldValue;
    fieldValue.stringValue = value;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::LIKE, field, QueryValueType::VALUE_TYPE_STRING, fieldValues);
}

void QueryExpression::NotLike(const std::string& field, const std::string &value)
{
    FieldValue fieldValue;
    fieldValue.stringValue = value;
    std::vector<FieldValue> fieldValues{fieldValue};
    AssemblyQueryInfo(QueryObjType::NOT_LIKE, field, QueryValueType::VALUE_TYPE_STRING, fieldValues);
}

void QueryExpression::Limit(int number, int offset)
{
    FieldValue fieldNumber;
    fieldNumber.integerValue = number;
    FieldValue fieldOffset;
    fieldOffset.integerValue = offset;
    std::vector<FieldValue> fieldValues{fieldNumber, fieldOffset};
    AssemblyQueryInfo(QueryObjType::LIMIT, std::string(), QueryValueType::VALUE_TYPE_INTEGER, fieldValues, false);
}

void QueryExpression::IsNull(const std::string& field)
{
    AssemblyQueryInfo(QueryObjType::IS_NULL, field, QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>());
}

void QueryExpression::IsNotNull(const std::string& field)
{
    AssemblyQueryInfo(QueryObjType::IS_NOT_NULL, field, QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>());
}

void QueryExpression::In(const std::string& field, const QueryValueType type, const std::vector<FieldValue> &values)
{
    AssemblyQueryInfo(QueryObjType::IN, field, type, values);
}

void QueryExpression::NotIn(const std::string& field, const QueryValueType type, const std::vector<FieldValue> &values)
{
    AssemblyQueryInfo(QueryObjType::NOT_IN, field, type, values);
}

void QueryExpression::And()
{
    AssemblyQueryInfo(QueryObjType::AND, std::string(), QueryValueType::VALUE_TYPE_NULL,
        std::vector<FieldValue>(), false);
}

void QueryExpression::Or()
{
    AssemblyQueryInfo(QueryObjType::OR, std::string(), QueryValueType::VALUE_TYPE_NULL,
        std::vector<FieldValue>(), false);
}

void QueryExpression::QueryByPrefixKey(const std::vector<uint8_t> &key)
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::QUERY_BY_KEY_PREFIX, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
    prefixKey_ = key;
}

void QueryExpression::QueryBySuggestIndex(const std::string &indexName)
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::SUGGEST_INDEX, indexName,
        QueryValueType::VALUE_TYPE_STRING, std::vector<FieldValue>()});
    suggestIndex_ = indexName;
}

const std::list<QueryObjNode> &QueryExpression::GetQueryExpression()
{
    if (!GetErrFlag()) {
        queryInfo_.clear();
        queryInfo_.emplace_back(QueryObjNode{QueryObjType::OPER_ILLEGAL});
        LOGE("Query operate illegal!");
    }

    return queryInfo_;
}

std::vector<uint8_t> QueryExpression::GetPreFixKey()
{
    return prefixKey_;
}

std::string QueryExpression::GetSuggestIndex()
{
    return suggestIndex_;
}

void QueryExpression::BeginGroup()
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::BEGIN_GROUP, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
}

void QueryExpression::EndGroup()
{
    queryInfo_.emplace_back(QueryObjNode{QueryObjType::END_GROUP, std::string(),
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()});
}

void QueryExpression::SetErrFlag(bool flag)
{
    errFlag_ = flag;
}

bool QueryExpression::GetErrFlag()
{
    return errFlag_;
}
} // namespace DistributedDB
