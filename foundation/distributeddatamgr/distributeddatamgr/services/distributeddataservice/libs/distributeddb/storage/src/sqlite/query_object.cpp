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

#include "query_object.h"
#include "db_errno.h"
#include "schema_utils.h"
#include "get_query_info.h"
#include "log_print.h"
#include "sqlite_utils.h"
#include "db_constant.h"
#include "macro_utils.h"

namespace DistributedDB {
namespace {
    const std::string PRE_QUERY_KV_SQL = "SELECT key, value FROM sync_data ";
    const std::string PRE_QUERY_ROWID_SQL = "SELECT rowid FROM sync_data ";
    const std::string PRE_GET_COUNT_SQL = "SELECT count(*) FROM sync_data ";
    const std::string FILTER_NATIVE_DATA_SQL = "WHERE (flag&0x01=0) ";
    const std::string USING_INDEX = "INDEXED BY ";
    const int MAX_SQL_LEN = 1024 * 1024; // 1M bytes
    const int LIMIT_FIELD_VALUE_SIZE = 2;
    const int SINGLE_FIELD_VALUE_SIZE = 1;
    const int INVALID_LIMIT = INT_MAX;
    const int MAX_CONDITIONS_SIZE = 128;
    const int MAX_SQLITE_BIND_SIZE = 50000;

    enum SymbolType {
        INVALID_SYMBOL = -1,
        COMPARE_SYMBOL, // relation symbol use to compare
        RELATIONAL_SYMBOL,
        RANGE_SYMBOL,
        LOGIC_SYMBOL,
        LINK_SYMBOL, // use to link relatonal symbol
        SPECIAL_SYMBOL, // need special precess and need at the last
        PREFIXKEY_SYMBOL,
        SUGGEST_INDEX_SYMBOL,
    };

    bool IsNeedCheckEqualFormat(SymbolType type)
    {
        return type == COMPARE_SYMBOL || type == RELATIONAL_SYMBOL || type == RANGE_SYMBOL;
    }

    // Considering our company's style, give up hexadecimal expressions state machines to avoid unreadable
    const std::map<QueryObjType, SymbolType> SYMBOL_TYPE_DIC {
        {QueryObjType::EQUALTO, COMPARE_SYMBOL},
        {QueryObjType::NOT_EQUALTO, COMPARE_SYMBOL},
        {QueryObjType::GREATER_THAN, COMPARE_SYMBOL},
        {QueryObjType::LESS_THAN, COMPARE_SYMBOL},
        {QueryObjType::GREATER_THAN_OR_EQUALTO, COMPARE_SYMBOL},
        {QueryObjType::LESS_THAN_OR_EQUALTO, COMPARE_SYMBOL},
        {QueryObjType::LIKE, RELATIONAL_SYMBOL},
        {QueryObjType::NOT_LIKE, RELATIONAL_SYMBOL},
        {QueryObjType::IS_NULL, RELATIONAL_SYMBOL},
        {QueryObjType::IS_NOT_NULL, RELATIONAL_SYMBOL},
        {QueryObjType::IN, RANGE_SYMBOL},
        {QueryObjType::NOT_IN, RANGE_SYMBOL},
        {QueryObjType::BEGIN_GROUP, LOGIC_SYMBOL},
        {QueryObjType::END_GROUP, LOGIC_SYMBOL},
        {QueryObjType::AND, LINK_SYMBOL},
        {QueryObjType::OR, LINK_SYMBOL},
        {QueryObjType::LIMIT, SPECIAL_SYMBOL},
        {QueryObjType::ORDERBY, SPECIAL_SYMBOL},
        {QueryObjType::QUERY_BY_KEY_PREFIX, PREFIXKEY_SYMBOL},
        {QueryObjType::SUGGEST_INDEX, SUGGEST_INDEX_SYMBOL}
    };

    SymbolType GetSymbolType(const QueryObjType &queryObjType)
    {
        if (SYMBOL_TYPE_DIC.find(queryObjType) == SYMBOL_TYPE_DIC.end()) {
            return INVALID_SYMBOL;
        }
        return SYMBOL_TYPE_DIC.at(queryObjType);
    }

    const std::map<QueryObjType, std::string> RELATIONAL_SYMBOL_TO_SQL {
        {QueryObjType::EQUALTO, "= "},
        {QueryObjType::NOT_EQUALTO, "!= "},
        {QueryObjType::GREATER_THAN, "> "},
        {QueryObjType::LESS_THAN, "< "},
        {QueryObjType::GREATER_THAN_OR_EQUALTO, ">= "},
        {QueryObjType::LESS_THAN_OR_EQUALTO, "<= "},
        {QueryObjType::LIKE, " LIKE "},
        {QueryObjType::NOT_LIKE, " NOT LIKE "},
        {QueryObjType::IS_NULL, " IS NULL "},
        {QueryObjType::IS_NOT_NULL, " IS NOT NULL "},
        {QueryObjType::IN, " IN ("},
        {QueryObjType::NOT_IN, " NOT IN ("},
    };

    const std::map<QueryObjType, std::string> LOGIC_SYMBOL_TO_SQL {
        {QueryObjType::AND, " AND "},
        {QueryObjType::OR, " OR "},
        {QueryObjType::BEGIN_GROUP, "("},
        {QueryObjType::END_GROUP, ")"},
    };

    int CheckLinkerBefor(std::list<QueryObjNode>::iterator &iter)
    {
        auto preIter = std::prev(iter, 1);
        SymbolType symbolType = GetSymbolType(preIter->operFlag);
        if (symbolType != COMPARE_SYMBOL && symbolType != RELATIONAL_SYMBOL && symbolType != LOGIC_SYMBOL &&
            symbolType != RANGE_SYMBOL && symbolType != PREFIXKEY_SYMBOL) {
            LOGE("Must be a comparison operation before the connective! operFage = %s", VNAME(preIter->operFlag));
            return -E_INVALID_QUERY_FORMAT;
        }
        return E_OK;
    }
}

QueryObject::QueryObject()
    : limit_(INVALID_LIMIT),
      offset_(0),
      orderByCounts_(0),
      isValid_(true),
      transformed_(false),
      hasOrderBy_(false),
      hasLimit_(false),
      isOrderByAppeared_(false),
      hasPrefixKey_(false),
      isNeedOrderbyKey_(true)
{}

QueryObject::QueryObject(const Query &query)
    : limit_(INVALID_LIMIT), offset_(0), orderByCounts_(0), transformed_(false), hasOrderBy_(false),
      hasLimit_(false), isOrderByAppeared_(false), hasPrefixKey_(false), isNeedOrderbyKey_(true)
{
    QueryExpression queryExpressions = GetQueryInfo::GetQueryExpression(query);
    queryObjNodes_ = queryExpressions.GetQueryExpression();
    isValid_ = queryExpressions.GetErrFlag();
    prefixKey_ = queryExpressions.GetPreFixKey();
    suggestIndex_ = queryExpressions.GetSuggestIndex();
}

int QueryObject::GetQuerySql(std::string &sql, bool onlyRowid)
{
    int errCode = E_OK;
    if (!IsValid(errCode)) {
        return errCode;
    }

    const std::string &querySqlForUse = (onlyRowid ? PRE_QUERY_ROWID_SQL : PRE_QUERY_KV_SQL);
    sql = AssembleSqlForSuggestIndex(querySqlForUse);
    sql = !hasPrefixKey_ ? sql : (sql + " AND (key>=? AND key<=?) ");
    if (transformed_) {
        LOGD("This query object has been parsed.");
        sql += querySql_;
        return E_OK;
    }
    errCode = ToQuerySql();
    if (errCode != E_OK) {
        LOGE("To query sql fail! errCode[%d]", errCode);
        return errCode;
    }
    transformed_ = true;
    sql += querySql_;
    return errCode;
}

int QueryObject::GetCountQuerySql(std::string &sql)
{
    int errCode = E_OK;
    if (!IsValid(errCode)) {
        return errCode;
    }

    errCode = ToGetCountSql();
    if (errCode != E_OK) {
        return errCode;
    }
    sql = AssembleSqlForSuggestIndex(PRE_GET_COUNT_SQL);
    sql = !hasPrefixKey_ ? sql : (sql + " AND (key>=? AND key<=?) ");
    sql += countSql_;
    return E_OK;
}

void QueryObject::SetSchema(const SchemaObject &schema)
{
    schema_ = schema;
}

bool QueryObject::IsValid(int &errCode)
{
    if (!isValid_) {
        errCode = -E_INVALID_QUERY_FORMAT;
        LOGE("Invalid query object!");
        return isValid_;
    }
    errCode = CheckQueryLegality();
    if (errCode != E_OK) {
        LOGE("Check query object illegal!");
        isValid_ = false;
    }
    return isValid_;
}

bool QueryObject::IsCountValid() const
{
    if (hasLimit_ || hasOrderBy_) {
        LOGI("It is invalid for limit and orderby!");
        return false;
    }
    return true;
}

bool QueryObject::HasLimit() const
{
    return hasLimit_;
}

void QueryObject::GetLimitVal(int &limit, int &offset) const
{
    limit = limit_;
    offset = offset_;
}

void QueryObject::FilterSymbolToAddBracketLink(bool &isNeedEndBracket, std::string &querySql) const
{
    for (const auto &iter : queryObjNodes_) {
        SymbolType symbolType = GetSymbolType(iter.operFlag);
        if (symbolType == COMPARE_SYMBOL || symbolType == RELATIONAL_SYMBOL || symbolType == RANGE_SYMBOL) {
            querySql += " AND (";
            isNeedEndBracket = true;
            break;
        } else if (symbolType == LOGIC_SYMBOL || symbolType == PREFIXKEY_SYMBOL) {
            continue;
        } else {
            break;
        }
    }
}

int QueryObject::ToQuerySql()
{
    if (queryObjNodes_.empty()) {
        querySql_ += ";";
        return E_OK;
    }

    bool isNeedEndBracket = false;
    QueryObject::FilterSymbolToAddBracketLink(isNeedEndBracket, querySql_);

    int errCode = E_OK;
    for (const QueryObjNode &objNode : queryObjNodes_) {
        SymbolType symbolType = GetSymbolType(objNode.operFlag);
        if (symbolType == SPECIAL_SYMBOL && isNeedEndBracket) {
            querySql_ += ") ";
            isNeedEndBracket = false;
        }
        errCode = ParseQueryExpression(objNode, querySql_);
        if (errCode != E_OK) {
            querySql_.clear();
            return errCode;
        }
    }

    if (isNeedEndBracket) {
        querySql_ += ") ";
    }

    // Limit needs to be placed after orderby and processed separately in the limit branch
    if (hasPrefixKey_ && !hasOrderBy_ && !hasLimit_ && isNeedOrderbyKey_) {
        LOGD("Need add order by key at last when has prefixkey no need order by value and limit!");
        querySql_ += "ORDER BY key ASC";
    }

    querySql_ += ";";
    return errCode;
}

int QueryObject::ToGetCountSql()
{
    countSql_.clear();
    if (queryObjNodes_.empty()) {
        countSql_ += ";";
        return E_OK;
    }
    bool isNeedEndBracket = false;
    QueryObject::FilterSymbolToAddBracketLink(isNeedEndBracket, countSql_);

    int errCode = E_OK;
    for (const QueryObjNode &objNode : queryObjNodes_) {
        SymbolType symbolType = GetSymbolType(objNode.operFlag);
        if (symbolType == SPECIAL_SYMBOL && isNeedEndBracket) {
            countSql_ += ") ";
            isNeedEndBracket = false;
        }

        if (objNode.operFlag == QueryObjType::LIMIT) {
            hasLimit_ = true;
            if (objNode.fieldValue.size() == LIMIT_FIELD_VALUE_SIZE) {
                limit_ = objNode.fieldValue[0].integerValue;
                offset_ = objNode.fieldValue[1].integerValue;
            }
            continue;
        }
        if (objNode.operFlag == QueryObjType::ORDERBY) {
            hasOrderBy_ = true;
            continue;
        }
        errCode = ParseQueryExpression(objNode, countSql_);
        if (errCode != E_OK) {
            countSql_.clear();
            return errCode;
        }
    }

    if (isNeedEndBracket) {
        countSql_ += ") ";
    }

    // Limit needs to be placed after orderby and processed separately in the limit branch
    if (hasPrefixKey_ && !hasOrderBy_ && !hasLimit_ && isNeedOrderbyKey_) {
        LOGD("Need add order by key at last when has prefixkey no need order by value and limit!");
        countSql_ += "ORDER BY key ASC";
    }
    countSql_ += ";";
    return errCode;
}

int QueryObject::GetQuerySqlStatement(sqlite3 *dbHandle, const std::string &sql, sqlite3_stmt *&statement)
{
    int errCode = SQLiteUtils::GetStatement(dbHandle, sql, statement);
    if (errCode != E_OK) {
        LOGE("[Query] Get statement fail!");
        return -E_INVALID_QUERY_FORMAT;
    }
    int index = 1;
    if (hasPrefixKey_) {
        // bind the prefix key for the first and second args.
        errCode = SQLiteUtils::BindPrefixKey(statement, 1, prefixKey_);
        if (errCode != E_OK) {
            LOGE("[Query] Get statement when bind prefix key, errCode = %d", errCode);
            return errCode;
        }
        index = 3; // begin from 3rd args
    }

    for (const QueryObjNode &objNode : queryObjNodes_) {
        errCode = BindFieldValue(statement, objNode, index);
        if (errCode != E_OK) {
            LOGE("[Query] Get statement fail when bind field value, errCode = %d", errCode);
            return errCode;
        }
    }
    return errCode;
}

int QueryObject::GetCountSqlStatement(sqlite3 *dbHandle, const std::string &countSql, sqlite3_stmt *&countStmt)
{
    // bind statement for count
    int errCode  = SQLiteUtils::GetStatement(dbHandle, countSql, countStmt);
    if (errCode != E_OK) {
        LOGE("Get count statement error:%d", errCode);
        return -E_INVALID_QUERY_FORMAT;
    }
    int index = 1;
    if (hasPrefixKey_) {
        // bind the prefix key for the first and second args.
        errCode = SQLiteUtils::BindPrefixKey(countStmt, 1, prefixKey_);
        if (errCode != E_OK) {
            LOGE("[Query] Get count statement fail when bind prefix key, errCode = %d", errCode);
            return errCode;
        }
        index = 3; // begin from 3rd args
    }

    for (const QueryObjNode &objNode : queryObjNodes_) {
        if (GetSymbolType(objNode.operFlag) == SPECIAL_SYMBOL) {
            continue;
        }
        errCode = BindFieldValue(countStmt, objNode, index);
        if (errCode != E_OK) {
            LOGE("[Query] Get count statement fail when bind field value, errCode = %d", errCode);
            return errCode;
        }
    }
    return errCode;
}

int QueryObject::CheckEqualFormat(std::list<QueryObjNode>::iterator &iter) const
{
    if (!schema_.IsSchemaValid()) {
        LOGE("Schema is invalid!");
        return -E_NOT_SUPPORT;
    }

    FieldPath fieldPath;
    int errCode = SchemaUtils::ParseAndCheckFieldPath(iter->fieldName, fieldPath);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }

    FieldType schemaFieldType = FieldType::LEAF_FIELD_BOOL;
    errCode = schema_.CheckQueryableAndGetFieldType(fieldPath, schemaFieldType);
    if (errCode != E_OK) {
        LOGE("Get field type fail when check compare format! errCode = %d, fieldType = %d", errCode, schemaFieldType);
        return -E_INVALID_QUERY_FIELD;
    }

    if (schemaFieldType == FieldType::LEAF_FIELD_BOOL && GetSymbolType(iter->operFlag) == COMPARE_SYMBOL &&
        iter->operFlag != QueryObjType::EQUALTO && iter->operFlag != QueryObjType::NOT_EQUALTO) { // bool can == or !=
        LOGE("Bool forbid compare!!!");
        return -E_INVALID_QUERY_FORMAT;
    }
    auto nextIter = std::next(iter, 1);
    if (nextIter != queryObjNodes_.end()) {
        SymbolType symbolType = GetSymbolType(nextIter->operFlag);
        if (symbolType == RELATIONAL_SYMBOL || symbolType == COMPARE_SYMBOL || symbolType == RANGE_SYMBOL) {
            LOGE("After Compare you need, You need the conjunction like and or for connecting!");
            return -E_INVALID_QUERY_FORMAT;
        }
    }
    return E_OK;
}

int QueryObject::CheckLinkerFormat(std::list<QueryObjNode>::iterator &iter) const
{
    if (iter == queryObjNodes_.begin()) {
        LOGE("Connectives are not allowed in the first place!");
        return -E_INVALID_QUERY_FORMAT;
    }
    auto nextIter = std::next(iter, 1);
    if (nextIter == queryObjNodes_.end()) {
        LOGE("Connectives are not allowed in the last place!");
        return -E_INVALID_QUERY_FORMAT;
    }
    SymbolType symbolType = GetSymbolType(nextIter->operFlag);
    if (symbolType == INVALID_SYMBOL || symbolType == LINK_SYMBOL || symbolType == SPECIAL_SYMBOL) {
        LOGE("Must be followed by comparison operation! operflag[%d], symbolType[%d]", nextIter->operFlag, symbolType);
        return -E_INVALID_QUERY_FORMAT;
    }

    return CheckLinkerBefor(iter);
}

int QueryObject::CheckOrderByFormat(std::list<QueryObjNode>::iterator &iter)
{
    if (!schema_.IsSchemaValid()) {
        return -E_NOT_SUPPORT;
    }

    FieldType schemaFieldType;
    FieldPath fieldPath;
    int errCode = SchemaUtils::ParseAndCheckFieldPath(iter->fieldName, fieldPath);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }
    errCode = schema_.CheckQueryableAndGetFieldType(fieldPath, schemaFieldType);
    if (errCode != E_OK) {
        return -E_INVALID_QUERY_FIELD;
    }
    if (schemaFieldType == FieldType::LEAF_FIELD_BOOL) {
        return -E_INVALID_QUERY_FORMAT;
    }
    hasOrderBy_ = true;
    orderByCounts_++;
    LOGD("Need order by %d filed value!", orderByCounts_);
    return E_OK;
}

int QueryObject::CheckLimitFormat(std::list<QueryObjNode>::iterator &iter) const
{
    std::list<QueryObjNode>::iterator next = std::next(iter, 1);
    if (next != queryObjNodes_.end() && GetSymbolType(next->operFlag) != SUGGEST_INDEX_SYMBOL) {
        LOGE("Limit should be last node or just before suggest-index nod!");
        return -E_INVALID_QUERY_FORMAT;
    }
    return E_OK;
}

int QueryObject::CheckSuggestIndexFormat(std::list<QueryObjNode>::iterator &iter) const
{
    std::list<QueryObjNode>::iterator next = std::next(iter, 1);
    if (next != queryObjNodes_.end()) {
        LOGE("SuggestIndex only allowed once, and must appear at the end!");
        return -E_INVALID_QUERY_FORMAT;
    }
    return E_OK;
}

int QueryObject::CheckExpressionFormat(std::list<QueryObjNode>::iterator &iter)
{
    SymbolType symbolType = GetSymbolType(iter->operFlag);
    // The object is newly instantiated in the connection, and there is no reentrancy problem.
    if (symbolType == PREFIXKEY_SYMBOL && hasPrefixKey_) {
        LOGE("Only filt by prefix key once!!");
        return -E_INVALID_QUERY_FORMAT;
    }

    if (iter->operFlag == QueryObjType::OPER_ILLEGAL || iter->type == QueryValueType::VALUE_TYPE_INVALID) {
        return -E_INVALID_QUERY_FORMAT;
    } else if (IsNeedCheckEqualFormat(symbolType)) {
        return CheckEqualFormat(iter);
    } else if (symbolType == LINK_SYMBOL) {
        return CheckLinkerFormat(iter);
    } else if (iter->operFlag == QueryObjType::LIMIT) {
        hasLimit_ = true;
        return CheckLimitFormat(iter);
    } else if (iter->operFlag == QueryObjType::ORDERBY) {
        return CheckOrderByFormat(iter);
    } else if (symbolType == PREFIXKEY_SYMBOL) {
        hasPrefixKey_ = true;
        if (prefixKey_.size() > DBConstant::MAX_KEY_SIZE) {
            return -E_INVALID_ARGS;
        }
    } else if (symbolType == SUGGEST_INDEX_SYMBOL) {
        return CheckSuggestIndexFormat(iter);
    }
    return E_OK;
}

int QueryObject::CheckQueryLegality()
{
    hasPrefixKey_ = false; // For Check preFixkey once
    orderByCounts_ = 0;

    auto iter = queryObjNodes_.begin();
    int errCode = E_OK;
    while (iter != queryObjNodes_.end()) {
        errCode = CheckExpressionFormat(iter);
        if (errCode != E_OK) {
            return errCode;
        }
        iter++;
    }
    return errCode;
}

std::string QueryObject::MapRelationalSymbolToSql(const QueryObjNode &queryNode)
{
    if (RELATIONAL_SYMBOL_TO_SQL.find(queryNode.operFlag) == RELATIONAL_SYMBOL_TO_SQL.end()) {
        return "";
    };
    std::string sql = RELATIONAL_SYMBOL_TO_SQL.at(queryNode.operFlag) + MapValueToSql(queryNode);
    if (GetSymbolType(queryNode.operFlag) == RANGE_SYMBOL) {
        sql += ")";
    }
    return sql;
}

std::string QueryObject::MapLogicSymbolToSql(const QueryObjNode &queryNode) const
{
    if (LOGIC_SYMBOL_TO_SQL.find(queryNode.operFlag) == LOGIC_SYMBOL_TO_SQL.end()) {
        return "";
    }
    return LOGIC_SYMBOL_TO_SQL.at(queryNode.operFlag);
}

std::string QueryObject::MapKeywordSymbolToSql(const QueryObjNode &queryNode)
{
    std::string sql;
    switch (queryNode.operFlag) {
        case QueryObjType::ORDERBY:
            if (queryNode.fieldValue.size() == SINGLE_FIELD_VALUE_SIZE) {
                if (!isOrderByAppeared_) {
                    sql += "ORDER BY ";
                }

                sql += QueryObject::MapCastFuncSql(queryNode);
                sql += queryNode.fieldValue[0].boolValue ? "ASC," : "DESC,";
                orderByCounts_--;
                if (orderByCounts_ == 0) {
                    sql.pop_back();
                }
                isOrderByAppeared_ = true;
            }
            return sql;
        case QueryObjType::LIMIT:
            if (queryNode.fieldValue.size() == LIMIT_FIELD_VALUE_SIZE) {
                if (hasPrefixKey_ && !hasOrderBy_ && isNeedOrderbyKey_) {
                    sql += "ORDER BY key ASC ";
                }
                sql += " LIMIT ";
                sql += std::to_string(queryNode.fieldValue[0].integerValue);
                sql += " OFFSET ";
                sql += std::to_string(queryNode.fieldValue[1].integerValue);
            }
            return sql;
        default:
            return "";
    }
    return sql;
}

std::string QueryObject::MapValueToSql(const QueryObjNode &queryNode)
{
    std::string resultSql;
    for (size_t i = 0; i < queryNode.fieldValue.size(); i++) {
        if (i == 0) {
            resultSql += " ? ";
        } else {
            resultSql += ", ? ";
        }
    }
    return resultSql;
}

static bool IsNeedCastWitEmptyValue(const QueryObjNode &queryNode)
{
    return (queryNode.operFlag == QueryObjType::IS_NULL || queryNode.operFlag == QueryObjType::IS_NOT_NULL ||
        queryNode.operFlag == QueryObjType::IN || queryNode.operFlag == QueryObjType::NOT_IN);
}

std::string QueryObject::MapCastFuncSql(const QueryObjNode &queryNode)
{
    std::string resultSql;
    if (queryNode.fieldValue.empty() && !IsNeedCastWitEmptyValue(queryNode)) {
        return resultSql;
    }
    // fieldPath and isQueryable had been checked ok in the previous code, So here parse path and get type won't fail.
    FieldPath fieldPath;
    SchemaUtils::ParseAndCheckFieldPath(queryNode.fieldName, fieldPath);
    FieldType fieldType = FieldType::LEAF_FIELD_INTEGER;
    schema_.CheckQueryableAndGetFieldType(fieldPath, fieldType);
    resultSql += SchemaObject::GenerateExtractSQL(schema_.GetSchemaType(), fieldPath, fieldType, schema_.GetSkipSize());
    isNeedOrderbyKey_ = false; // When index by value, No need order by key
    return resultSql;
}

int QueryObject::BindFieldValue(sqlite3_stmt *statement, const QueryObjNode &queryNode, int &index) const
{
    int errCode = E_OK;
    SymbolType symbolType = GetSymbolType(queryNode.operFlag);
    if (symbolType != COMPARE_SYMBOL && symbolType != RELATIONAL_SYMBOL && symbolType != RANGE_SYMBOL) {
        return errCode;
    }

    for (size_t i = 0; i < queryNode.fieldValue.size(); i++) {
        if (queryNode.type == QueryValueType::VALUE_TYPE_BOOL) {
            errCode = sqlite3_bind_int(statement, index, queryNode.fieldValue[i].boolValue);
        } else if (queryNode.type == QueryValueType::VALUE_TYPE_INTEGER) {
            errCode = sqlite3_bind_int(statement, index, queryNode.fieldValue[i].integerValue);
        } else if (queryNode.type == QueryValueType::VALUE_TYPE_LONG) {
            errCode = sqlite3_bind_int64(statement, index, queryNode.fieldValue[i].longValue);
        } else if (queryNode.type == QueryValueType::VALUE_TYPE_DOUBLE) {
            errCode = sqlite3_bind_double(statement, index, queryNode.fieldValue[i].doubleValue);
        } else {
            if (queryNode.fieldValue[i].stringValue.size() > MAX_SQLITE_BIND_SIZE) {
                return -E_MAX_LIMITS;
            }
            errCode = sqlite3_bind_text(statement, index, queryNode.fieldValue[i].stringValue.c_str(),
                queryNode.fieldValue[i].stringValue.size(), SQLITE_TRANSIENT);
        }
        if (errCode != SQLITE_OK) {
            break;
        }
        index++;
    }
    return SQLiteUtils::MapSQLiteErrno(errCode);
}

std::string QueryObject::MapCastTypeSql(const FieldType &type)
{
    switch (type) {
        case FieldType::LEAF_FIELD_BOOL:
        case FieldType::LEAF_FIELD_INTEGER:
        case FieldType::LEAF_FIELD_LONG:
            return "INT";
        case FieldType::LEAF_FIELD_DOUBLE:
            return "REAL";
        case FieldType::LEAF_FIELD_STRING:
            return "TEXT";
        case FieldType::LEAF_FIELD_NULL:
            return "NULL";
        default:
            return "";
    }
}

int QueryObject::ParseQueryExpression(const QueryObjNode &queryNode, std::string &querySql)
{
    SymbolType symbolType = GetSymbolType(queryNode.operFlag);
    if (symbolType == RANGE_SYMBOL && queryNode.fieldValue.size() > MAX_CONDITIONS_SIZE) {
        LOGE("[Query][Parse][Expression] conditions is too many!");
        return -E_MAX_LIMITS;
    }

    if (symbolType == COMPARE_SYMBOL || symbolType == RELATIONAL_SYMBOL || symbolType == RANGE_SYMBOL) {
        querySql += MapCastFuncSql(queryNode);
        querySql += MapRelationalSymbolToSql(queryNode);
    } else if (symbolType == LOGIC_SYMBOL || symbolType == LINK_SYMBOL) {
        querySql += MapLogicSymbolToSql(queryNode);
    } else {
        querySql += MapKeywordSymbolToSql(queryNode);
    }

    if (querySql.size() > MAX_SQL_LEN) {
        LOGE("[Query][Parse][Expression] Sql is too long!");
        return -E_MAX_LIMITS;
    }
    return E_OK;
}

std::string QueryObject::AssembleSqlForSuggestIndex(const std::string &baseSql) const
{
    std::string formatIndex = CheckAndFormatSuggestIndex();
    if (formatIndex.empty()) {
        return baseSql + FILTER_NATIVE_DATA_SQL;
    }

    return baseSql + USING_INDEX + "'" + formatIndex + "' " + FILTER_NATIVE_DATA_SQL;
}

std::string QueryObject::CheckAndFormatSuggestIndex() const
{
    if (suggestIndex_.empty()) {
        return "";
    }
    IndexName indexName;
    int errCode = SchemaUtils::ParseAndCheckFieldPath(suggestIndex_, indexName);
    if (errCode != E_OK) {
        LOGW("Check and format suggest index failed! %d", errCode);
        return "";
    }

    if (!schema_.IsIndexExist(indexName)) {
        LOGW("The suggest index not exist!");
        return "";
    }

    return SchemaUtils::FieldPathString(indexName);
}
}
