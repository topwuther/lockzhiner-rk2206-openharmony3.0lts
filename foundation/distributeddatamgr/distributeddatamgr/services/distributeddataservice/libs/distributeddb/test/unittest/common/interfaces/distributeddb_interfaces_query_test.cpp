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

#include <gtest/gtest.h>

#include "get_query_info.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace std;

namespace {
    const std::string TEST_FIELD_NAME = "$.test";

    bool CheckQueryContainer(Query &query, std::list<QueryObjNode> &checkList)
    {
        const std::list<QueryObjNode> queryList = GetQueryInfo::GetQueryExpression(query).GetQueryExpression();
        if (queryList.size() != checkList.size()) {
            return false;
        }
        auto queryIter = queryList.begin();

        for (auto checkIter = checkList.begin(); checkIter != checkList.end(); checkIter++, queryIter++) {
            EXPECT_EQ(checkIter->operFlag, queryIter->operFlag);
            EXPECT_EQ(checkIter->type, queryIter->type);
            EXPECT_EQ(checkIter->fieldName, queryIter->fieldName);
            if (checkIter->fieldValue.size() != queryIter->fieldValue.size()) {
                return false;
            }
            for (size_t i = 0; i < checkIter->fieldValue.size(); i++) {
                EXPECT_EQ(memcmp(&(checkIter->fieldValue[i]), &(queryIter->fieldValue[i]), 8), 0); // only need check 8
                EXPECT_EQ(checkIter->fieldValue[i].stringValue, queryIter->fieldValue[i].stringValue);
            }
        }
        return true;
    }

    template<typename T>
    std::list<QueryObjNode> CraetCheckList(QueryObjType operFlag, const std::string &fieldName, const T &queryValue)
    {
        FieldValue fieldValue;
        QueryValueType type = GetQueryValueType::GetFieldTypeAndValue(queryValue, fieldValue);
        std::vector<FieldValue> values{fieldValue};
        if (type == QueryValueType::VALUE_TYPE_BOOL) {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_BOOL, values}};
            return result;
        } else if (type == QueryValueType::VALUE_TYPE_INTEGER) {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_INTEGER, values}};
            return result;
        } else if (type == QueryValueType::VALUE_TYPE_LONG) {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_LONG, values}};
            return result;
        } else if (type == QueryValueType::VALUE_TYPE_DOUBLE) {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_DOUBLE, values}};
            return result;
        } else if (type == QueryValueType::VALUE_TYPE_STRING) {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_STRING, values}};
            return result;
        } else {
            std::list<QueryObjNode> result{{operFlag, fieldName, QueryValueType::VALUE_TYPE_INVALID, values}};
            return result;
        }
    }

    void CheckQueryCompareOper()
    {
        Query query1 = Query::Select().NotEqualTo(TEST_FIELD_NAME, 123); // random test data
        std::list<QueryObjNode> result = CraetCheckList(QueryObjType::NOT_EQUALTO, TEST_FIELD_NAME, 123);
        EXPECT_TRUE(CheckQueryContainer(query1, result));

        Query query2 = Query::Select().EqualTo(TEST_FIELD_NAME, true);
        result.clear();
        result = CraetCheckList(QueryObjType::EQUALTO, TEST_FIELD_NAME, true);
        EXPECT_TRUE(CheckQueryContainer(query2, result));

        Query query3 = Query::Select().GreaterThan(TEST_FIELD_NAME, 0);
        result.clear();
        result = CraetCheckList(QueryObjType::GREATER_THAN, TEST_FIELD_NAME, 0);
        EXPECT_TRUE(CheckQueryContainer(query3, result));

        Query query4 = Query::Select().LessThan(TEST_FIELD_NAME, INT_MAX);
        result.clear();
        result = CraetCheckList(QueryObjType::LESS_THAN, TEST_FIELD_NAME, INT_MAX);
        EXPECT_TRUE(CheckQueryContainer(query4, result));

        Query query5 = Query::Select().GreaterThanOrEqualTo(TEST_FIELD_NAME, 1.56); // random test data
        result.clear();
        result = CraetCheckList(QueryObjType::GREATER_THAN_OR_EQUALTO, TEST_FIELD_NAME, 1.56);
        EXPECT_TRUE(CheckQueryContainer(query5, result));

        Query query6 = Query::Select().LessThanOrEqualTo(TEST_FIELD_NAME, 100); // random test data
        result.clear();
        result = CraetCheckList(QueryObjType::LESS_THAN_OR_EQUALTO, TEST_FIELD_NAME, 100);
        EXPECT_TRUE(CheckQueryContainer(query6, result));
    }
}

class DistributedDBInterfacesQueryDBTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesQueryDBTest::SetUpTestCase(void)
{
}

void DistributedDBInterfacesQueryDBTest::TearDownTestCase(void)
{
}

void DistributedDBInterfacesQueryDBTest::SetUp(void)
{
}

void DistributedDBInterfacesQueryDBTest::TearDown(void)
{
}

/**
  * @tc.name: Query001
  * @tc.desc: Check the legal single query operation to see if the generated container is correct
  * @tc.type: FUNC
  * @tc.require: AR000DR9K6
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesQueryDBTest, Query001, TestSize.Level0)
{
    Query query = Query::Select();
    Query queryCopy = query;
    Query queryMove = std::move(query);

    CheckQueryCompareOper();

    std::string testValue = "testValue";
    Query query7 = Query::Select().Like(TEST_FIELD_NAME, testValue);
    std::list<QueryObjNode> result = CraetCheckList(QueryObjType::LIKE, TEST_FIELD_NAME, testValue);
    EXPECT_TRUE(CheckQueryContainer(query7, result));

    Query query8 = Query::Select().NotLike(TEST_FIELD_NAME, "testValue");
    result.clear();
    result = CraetCheckList(QueryObjType::NOT_LIKE, TEST_FIELD_NAME, testValue);
    EXPECT_TRUE(CheckQueryContainer(query8, result));

    vector<int> fieldValues{1, 1, 1};
    Query query9 = Query::Select().In(TEST_FIELD_NAME, fieldValues);
    FieldValue fieldValue;
    fieldValue.integerValue = 1;
    std::vector<FieldValue> values{fieldValue, fieldValue, fieldValue};
    std::list<QueryObjNode> result1{{QueryObjType::IN, TEST_FIELD_NAME, QueryValueType::VALUE_TYPE_INTEGER, values}};
    EXPECT_TRUE(CheckQueryContainer(query9, result1));

    Query query10 = Query::Select().NotIn(TEST_FIELD_NAME, fieldValues);
    std::list<QueryObjNode> result2{{QueryObjType::NOT_IN, TEST_FIELD_NAME,
        QueryValueType::VALUE_TYPE_INTEGER, values}};
    EXPECT_TRUE(CheckQueryContainer(query10, result2));

    Query query11 = Query::Select().OrderBy(TEST_FIELD_NAME, false);
    result.clear();
    result = CraetCheckList(QueryObjType::ORDERBY, TEST_FIELD_NAME, false);
    EXPECT_TRUE(CheckQueryContainer(query11, result));

    Query query12 = Query::Select().Limit(1, 2);
    values.pop_back();
    values.back().integerValue = 2;
    std::list<QueryObjNode> result3{{QueryObjType::LIMIT, string(), QueryValueType::VALUE_TYPE_INTEGER, values}};
    EXPECT_TRUE(CheckQueryContainer(query12, result3));

    Query query13 = Query::Select().IsNull(TEST_FIELD_NAME);
    std::list<QueryObjNode> result4{{QueryObjType::IS_NULL, TEST_FIELD_NAME,
        QueryValueType::VALUE_TYPE_NULL, std::vector<FieldValue>()}};
    EXPECT_TRUE(CheckQueryContainer(query13, result4));
}

/**
  * @tc.name: Query002
  * @tc.desc: Check for illegal query conditions
  * @tc.type: FUNC
  * @tc.require: AR000DR9K6
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesQueryDBTest, Query002, TestSize.Level0)
{
    float testValue = 1.1;
    Query query = Query::Select().NotEqualTo(".test", testValue);
    EXPECT_FALSE(GetQueryInfo::GetQueryExpression(query).GetErrFlag());

    EXPECT_FALSE(GetQueryInfo::GetQueryExpression(Query::Select().GreaterThan(TEST_FIELD_NAME, true)).GetErrFlag());

    EXPECT_FALSE(GetQueryInfo::GetQueryExpression(Query::Select().LessThan("$.test.12test", true)).GetErrFlag());
}

/**
  * @tc.name: Query003
  * @tc.desc: Check combination condition
  * @tc.type: FUNC
  * @tc.require: AR000DR9K6
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesQueryDBTest, Query003, TestSize.Level0)
{
    Query query = Query::Select().EqualTo(TEST_FIELD_NAME, true).And().GreaterThan(TEST_FIELD_NAME, 1);
    QueryExpression queryExpression = GetQueryInfo::GetQueryExpression(query);
    EXPECT_TRUE(queryExpression.GetErrFlag());
    EXPECT_EQ(queryExpression.GetQueryExpression().size(), 3UL);

    Query query1 = Query::Select().GreaterThan(TEST_FIELD_NAME, 1).OrderBy(TEST_FIELD_NAME);
    QueryExpression queryExpression1 = GetQueryInfo::GetQueryExpression(query1);
    EXPECT_TRUE(queryExpression1.GetErrFlag());
    EXPECT_EQ(queryExpression1.GetQueryExpression().size(), 2UL);
}