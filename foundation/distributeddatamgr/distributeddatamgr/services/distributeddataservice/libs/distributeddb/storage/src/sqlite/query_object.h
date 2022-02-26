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
#ifndef QUERY_OBJECT_H
#define QUERY_OBJECT_H

#include <string>
#include "schema_object.h"
#include "query.h"
#include "sqlite_import.h"

namespace DistributedDB {
class QueryObject {
public:
    QueryObject();
    explicit QueryObject(const Query &query);
    ~QueryObject() = default;
    int GetQuerySql(std::string &sql, bool onlyRowid = false);
    int GetCountQuerySql(std::string &sql);
    bool IsValid(int &errCode);
    bool IsCountValid() const;
    bool HasLimit() const;
    void GetLimitVal(int &limit, int &offset) const;
    void SetSchema(const SchemaObject &schema);
    int GetQuerySqlStatement(sqlite3 *dbHandle, const std::string &sql, sqlite3_stmt *&statement);
    int GetCountSqlStatement(sqlite3 *dbHandle, const std::string &countSql, sqlite3_stmt *&countStmt);
private:
    int ToQuerySql();
    int ToGetCountSql();
    int CheckEqualFormat(std::list<QueryObjNode>::iterator &iter) const;
    int CheckLinkerFormat(std::list<QueryObjNode>::iterator &iter) const;
    int CheckOrderByFormat(std::list<QueryObjNode>::iterator &iter);
    int CheckExpressionFormat(std::list<QueryObjNode>::iterator &iter);
    int CheckLimitFormat(std::list<QueryObjNode>::iterator &iter) const;
    int CheckSuggestIndexFormat(std::list<QueryObjNode>::iterator &iter) const;
    int CheckQueryLegality();
    int ParseQueryExpression(const QueryObjNode &queryNode, std::string &querySql);
    static std::string MapRelationalSymbolToSql(const QueryObjNode &queryNode);
    std::string MapKeywordSymbolToSql(const QueryObjNode &queryNode);
    std::string MapLogicSymbolToSql(const QueryObjNode &queryNode) const;
    static std::string MapValueToSql(const QueryObjNode &queryNode);
    std::string MapCastFuncSql(const QueryObjNode &queryNode);
    static std::string MapCastTypeSql(const FieldType &type);
    int BindFieldValue(sqlite3_stmt *statement, const QueryObjNode &queryNode, int &index) const;
    void FilterSymbolToAddBracketLink(bool &isNeedEndBracket, std::string &querySql) const;
    std::string AssembleSqlForSuggestIndex(const std::string &baseSql) const;
    std::string CheckAndFormatSuggestIndex() const;
    SchemaObject schema_;
    std::list<QueryObjNode> queryObjNodes_;
    std::vector<uint8_t> prefixKey_;
    std::string querySql_;
    std::string countSql_;
    std::string suggestIndex_;

    int limit_;
    int offset_;
    int orderByCounts_; // Record processing to which orderBy node
    bool isValid_;
    bool transformed_;
    bool hasOrderBy_;
    bool hasLimit_;
    bool isOrderByAppeared_;
    bool hasPrefixKey_;
    bool isNeedOrderbyKey_;  // The tag field is used for prefix query filtering key sorting
};
}
#endif
