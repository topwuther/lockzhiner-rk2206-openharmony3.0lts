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

#ifndef OMIT_JSON
#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"
#include "distributeddb_schema_test_tools.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbPredicateQueryExpand {
KvStoreNbDelegate *g_nbQueryDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;
const int CURSOR_POSITION_NEGATIVE1 = -1;
const int CURSOR_POSITION_1 = 1;

DistributedDB::CipherPassword g_passwd1;

class DistributeddbNbPredicateQueryExpandTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbPredicateQueryExpandTest::SetUpTestCase(void)
{
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
}

void DistributeddbNbPredicateQueryExpandTest::TearDownTestCase(void)
{
}

void DistributeddbNbPredicateQueryExpandTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    string validSchema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_COMBINATION_DEFINE, VALID_INDEX_1);
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = validSchema;
    g_nbQueryDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, option);
    ASSERT_TRUE(g_nbQueryDelegate != nullptr && g_manager != nullptr);
}

void DistributeddbNbPredicateQueryExpandTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_nbQueryDelegate, STORE_ID_1, false));
}

vector<string> GenerateCombinationSchemaValue(const vector<vector<string>> &fieldValue)
{
    vector<string> schemasValue;
    int valueNum = fieldValue.size();
    for (int index = 0; index < valueNum; index++) {
        string valueStr;
        string field1Val = fieldValue[index][INDEX_ZEROTH];
        if (field1Val != "null") {
            valueStr = valueStr + "{\"field1\":\"" + field1Val + "\"";
        } else {
            valueStr = valueStr + "{\"field1\":" + field1Val;
        }
        // 1,2,3,4 is the index of fieldValue
        valueStr = valueStr + ",\"field2\":{\"field3\":" + fieldValue[index][INDEX_FIRST] + ",\"field4\":{\"field5\":" +
        fieldValue[index][INDEX_SECOND] + ",\"field6\":{\"field7\":" + fieldValue[index][INDEX_THIRD] + ",\"field8\":" +
        fieldValue[index][INDEX_FORTH] + "}}}}";
        schemasValue.push_back(valueStr);
    }
    return schemasValue;
}

void PresetDatasToDB(KvStoreNbDelegate *&delegate, std::vector<uint8_t> keyPrefix, vector<string> &schemasValue,
    vector<Entry> &entries)
{
    int num = schemasValue.size();
    for (int index = 1; index <= num; index++) {
        DistributedDB::Entry entry;
        std::string ind = std::to_string(index);
        entry.key.push_back(keyPrefix[0]);
        for (auto ch = ind.begin(); ch != ind.end(); ch++) {
            entry.key.push_back(*ch);
        }
        Value value(schemasValue[index - 1].begin(), schemasValue[index - 1].end());
        entry.value = value;
        EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, entry.key, entry.value), OK);
        entries.push_back(entry);
    }
}

bool CheckQueryResult(KvStoreNbDelegate &delegate, const Query &query, vector<Entry> &expectEntry,
    const DBStatus status, bool canGetCount)
{
    vector<Entry> entries;
    EXPECT_EQ(delegate.GetEntries(query, entries), status);
    if (!expectEntry.empty()) {
        if (entries.size() != expectEntry.size()) {
            MST_LOG("The entries from query is %zd, The expectEntry is %zd, they are not equal",
                entries.size(), expectEntry.size());
            return false;
        }
        for (vector<Entry>::size_type index = 0; index < entries.size(); index++) {
            if (entries[index].key != expectEntry[index].key || entries[index].value != expectEntry[index].value) {
                string keyGot(entries[index].key.begin(), entries[index].key.end());
                string keyExpect(expectEntry[index].key.begin(), expectEntry[index].key.end());
                MST_LOG("entry key compare failed, expectKey:%s, gotKey:%s, line:%d", keyExpect.c_str(), keyGot.c_str(),
                    __LINE__);
                return false;
            }
        }
    }
    KvStoreResultSet *resultSet = nullptr;
    if (status != NOT_FOUND) {
        EXPECT_EQ(delegate.GetEntries(query, resultSet), status);
    } else {
        EXPECT_EQ(delegate.GetEntries(query, resultSet), OK);
        if (resultSet != nullptr) {
            Entry entry;
            EXPECT_EQ(resultSet->GetEntry(entry), status);
        }
    }
    int expectCnt = expectEntry.size();
    if (resultSet != nullptr) {
        EXPECT_EQ(resultSet->GetCount(), expectCnt);
        EXPECT_EQ(delegate.CloseResultSet(resultSet), DBStatus::OK);
    }
    if (canGetCount) {
        int cnt = 0;
        EXPECT_EQ(delegate.GetCount(query, cnt), status);
        EXPECT_EQ(cnt, expectCnt);
    }
    return true;
}

/**
 * @tc.name: BracketsTest 001
 * @tc.desc: Verify the Brackets can change priority of query methods if brackets appear in pairs.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, BracketsTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 7 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},   {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},   {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}, {"abxxx", "true", "12", "120", "-79"},
        {"ab", "true", "20", "82", "150.999"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. test the Query without brackets and check the result with GetEntries.
     * @tc.expected: step2. GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field2.field3", true).And().LessThanOrEqualTo("$.field2.field4.field5",
        33).Or().LessThanOrEqualTo("$.field2.field4.field6.field7", 1000).And().NotLike("$.field1", "%c"). \
        OrderBy("$.field2.field4.field6.field8", true);
    vector<Entry> expectEntry1 = {entries[INDEX_FIRST], entries[INDEX_FIFTH], entries[INDEX_ZEROTH],
        entries[INDEX_THIRD], entries[INDEX_SECOND], entries[INDEX_FORTH], entries[INDEX_SIXTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, expectEntry1, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with brackets appear in pairs and check the result with GetEntries.
     * @tc.expected: step3. GetEntries return right result.
     */
    Query query2 = Query::Select().BeginGroup().EqualTo("$.field2.field3", true).And(). \
        LessThanOrEqualTo("$.field2.field4.field5", 33).Or().LessThanOrEqualTo("$.field2.field4.field6.field7", 1000). \
        EndGroup().And().NotLike("$.field1", "%c").OrderBy("$.field2.field4.field6.field8", true);
    vector<Entry> expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_FIFTH], entries[INDEX_THIRD],
        entries[INDEX_SECOND], entries[INDEX_FORTH], entries[INDEX_SIXTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, expectEntry2, DBStatus::OK, false));

    /**
     * @tc.steps: step4. test the Query with brackets appear in pairs and check the result with GetCount.
     * @tc.expected: step4. GetCount return 7.
     */
    Query query3 = Query::Select().BeginGroup().EqualTo("$.field2.field3", true).And(). \
        LessThanOrEqualTo("$.field2.field4.field5", 33).Or().LessThanOrEqualTo("$.field2.field4.field6.field7", 1000). \
        EndGroup().And().NotLike("$.field1", "%c");
    int cnt = 0;
    int expectCnt = expectEntry2.size();
    EXPECT_EQ(g_nbQueryDelegate->GetCount(query3, cnt), OK);
    EXPECT_EQ(cnt, expectCnt);
}

/**
 * @tc.name: BracketsTest 002
 * @tc.desc: Verify the Brackets can change priority of query methods if brackets nested appear.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, BracketsTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 7 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},   {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},   {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}, {"abxxx", "true", "12", "120", "-79"},
        {"ab", "true", "20", "82", "150.999"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. test the Query without brackets and check the result with GetEntries.
     * @tc.expected: step2. GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field2.field3", true).And().LessThanOrEqualTo("$.field2.field4.field5",
        33).Or().LessThanOrEqualTo("$.field2.field4.field6.field7", 1000).And().NotLike("$.field1", "%c"). \
        OrderBy("$.field2.field4.field6.field8", true);
    vector<Entry> expectEntry1 = {entries[INDEX_FIRST], entries[INDEX_FIFTH], entries[INDEX_ZEROTH],
        entries[INDEX_THIRD], entries[INDEX_SECOND], entries[INDEX_FORTH], entries[INDEX_SIXTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, expectEntry1, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with brackets nested appeared and check the result with GetEntries.
     * @tc.expected: step3. GetEntries return right result.
     */
    Query query2 = Query::Select().EqualTo("$.field2.field3", true).And().BeginGroup(). \
        LessThanOrEqualTo("$.field2.field4.field5", 33).Or().BeginGroup(). \
        LessThanOrEqualTo("$.field2.field4.field6.field7", 1000).And().NotLike("$.field1", "%c"). \
        EndGroup().EndGroup().OrderBy("$.field2.field4.field6.field8", true);
    vector<Entry> expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_FIFTH], entries[INDEX_ZEROTH],
        entries[INDEX_SECOND], entries[INDEX_FORTH], entries[INDEX_SIXTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, expectEntry2, DBStatus::OK, false));

    /**
     * @tc.steps: step4. test the Query with brackets nested appeared and check the result with GetCount.
     * @tc.expected: step4. GetCount return 6.
     */
    Query query3 = Query::Select().EqualTo("$.field2.field3", true).And().BeginGroup(). \
        LessThanOrEqualTo("$.field2.field4.field5", 33).Or().BeginGroup(). \
        LessThanOrEqualTo("$.field2.field4.field6.field7", 1000).And().NotLike("$.field1", "%c").EndGroup().EndGroup();
    int cnt = 0;
    int expectCnt = expectEntry2.size();
    EXPECT_EQ(g_nbQueryDelegate->GetCount(query3, cnt), OK);
    EXPECT_EQ(cnt, expectCnt);
}
// insert query condition between brackets.
void SpliceQueryMethod(int flag, Query &query)
{
    switch (flag) {
        case 0: // query condition 0
            query.LessThan("$.field2.field4.field5", "100").And().NotEqualTo("$.field2.field4.field6.field7", "-100");
            break;
        case 1: // query condition 1
            query.EqualTo("$.field2.field3", "true").Or().NotLike("$.field1", "%c");
            break;
        case 2: // query condition 2
            query.Like("$.field1", "ab%");
            break;
        case 3: // query condition 3
            query.GreaterThanOrEqualTo("$.field2.field4.field6.field8", "0");
            break;
        default: // other query condition
            break;
    }
}
// Generate rand query by the number of brackets.
void GenerateRandQuery(Query &query, int beginNum, int endNum)
{
    int left = 0;
    int right = 0;
    for (int cnt = 0; cnt < beginNum + endNum; cnt++) {
        int brachetTest = GetRandInt(0, 1);
        if (brachetTest == 0 && left < beginNum) {
            left++;
            if (cnt != 0) {
                query.Or();
            }
            query.BeginGroup();
        } else if (brachetTest == 1 && right < endNum) {
            right++;
            query.EndGroup();
        } else {
            cnt--;
            continue;
        }
        int flag = GetRandInt(0, 4); // add query condition 0-4 to brackets.
        if ((brachetTest == 0 && cnt < beginNum + endNum - 1) || (brachetTest == 1 && cnt == 0)) {
            SpliceQueryMethod(flag, query);
        } else if (brachetTest == 1 && cnt < beginNum + endNum - 1) {
            query.And();
            SpliceQueryMethod(flag, query);
        }
    }
}
/**
 * @tc.name: BracketsTest 003
 * @tc.desc: Verify the Brackets appear abnormally then call predicate query interface will return INVALID_QUERY_FORMAT.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, BracketsTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 7 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},   {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},   {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}, {"abxxx", "true", "12", "120", "-79"},
        {"ab", "true", "20", "82", "150.999"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. generate query that the right bracket is more than left bracket,then call GetEntries()/
     *     GetCount().
     * @tc.expected: step2. call GetEntries()/GetCount() return INVALID_QUERY_FORMAT.
     */
    Query query1 = Query::Select();
    int bracketNum1 = GetRandInt(0, 5); // generate the number of bracket from 0 to 5 randomly.
    GenerateRandQuery(query1, bracketNum1, bracketNum1 + 1);
    vector<Entry> expectEntry;
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));

    /**
     * @tc.steps: step3. generate query that the right bracket is less than left bracket,then call GetEntries()/
     *     GetCount().
     * @tc.expected: step3. call GetEntries()/GetCount() return INVALID_QUERY_FORMAT.
     */
    Query query2 = Query::Select();
    int bracketNum2 = GetRandInt(1, 5); // generate the number of bracket from 1 to 5 randomly.
    GenerateRandQuery(query2, bracketNum2, bracketNum2 - 1);
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));
}

/**
 * @tc.name: BracketsTest 004
 * @tc.desc: Verify test the Query with brackets that having And() after BeginGroup() or having Or() before EndGroup()
 *     will return INVALID_QUERY_FORMAT.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, BracketsTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 7 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},   {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},   {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}, {"abxxx", "true", "12", "120", "-79"},
        {"ab", "true", "20", "82", "150.999"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. test the Query with brackets that having And() after BeginGroup() and check the result
     *     with GetEntries/GetCount.
     * @tc.expected: step2. GetEntries and GetCount return INVALID_QUERY_FORMAT.
     */
    Query query1 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().BeginGroup().And(). \
        EqualTo("$.field2.field3", true).Or().Like("$.field1", "ab%").EndGroup();
    vector<Entry> expectEntry;
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));

    /**
     * @tc.steps: step3. test the Query with brackets that having or having Or() before EndGroup() and check the result
     *     with GetEntries/GetCount.
     * @tc.expected: step3. GetEntries and GetCount return INVALID_QUERY_FORMAT.
     */
    Query query2 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().BeginGroup(). \
        EqualTo("$.field2.field3", true).Or().Like("$.field1", "ab%").Or().EndGroup();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));

    /**
     * @tc.steps: step4. test the Query with brackets that right bracket before left bracket.
     * @tc.expected: step4. GetEntries and GetCount return INVALID_QUERY_FORMAT.
     */
    Query query3 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().EndGroup(). \
        EqualTo("$.field2.field3", true).Or().Like("$.field1", "ab%").Or().BeginGroup();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));
}

/**
 * @tc.name: BracketsTest 005
 * @tc.desc: Verify query with OrderBy/Limit in brackets will return INVALID_QUERY_FORMAT.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, BracketsTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 7 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},   {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},   {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}, {"abxxx", "true", "12", "120", "-79"},
        {"ab", "true", "20", "82", "150.999"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. test the Query with OrderBy in the brackets and check the result.
     *     with GetEntries/GetCount.
     * @tc.expected: step2. GetEntries and GetCount return INVALID_QUERY_FORMAT.
     */
    Query query1 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().BeginGroup(). \
        EqualTo("$.field2.field3", true).Or().Like("$.field1", "ab%").OrderBy("$.field2.field4.field6.field8", true). \
        EndGroup();
    vector<Entry> expectEntry;
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, expectEntry, DBStatus::INVALID_QUERY_FORMAT, true));

    /**
     * @tc.steps: step3. test the Query with Limit in the brackets and check the result
     * @tc.expected: step3. GetEntries and GetCount return INVALID_QUERY_FORMAT.
     */
    Query query2 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().BeginGroup(). \
        EqualTo("$.field2.field3", true).Or().Like("$.field1", "ab%").Limit(5, 0).EndGroup();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, expectEntry, DBStatus::INVALID_QUERY_FORMAT, false));
}

/**
 * @tc.name: NotNullTest 001
 * @tc.desc: Verify query IsNotNull can get right result when put right param.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, NotNullTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"null", "true", "9", "1000", "12"},  {"abc123", "null", "88", "-100", "-99"},
        {"abfxy", "true", "null", "0", "38"}, {"ab789", "false", "999", "null", "15.8"},
        {"ab000", "true", "33", "30", "null"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);
    vector<vector<Entry>> expectEntries = {entries, entries, entries, entries, entries};
    expectEntries[0].erase(expectEntries[0].begin());
    expectEntries[INDEX_FIRST].erase(expectEntries[INDEX_FIRST].begin() + INDEX_FIRST);
    expectEntries[INDEX_SECOND].erase(expectEntries[INDEX_SECOND].begin() + INDEX_SECOND);
    expectEntries[INDEX_THIRD].erase(expectEntries[INDEX_THIRD].begin() + INDEX_THIRD);
    expectEntries[INDEX_FORTH].pop_back();

    /**
     * @tc.steps: step2. test the Query with IsNotNull interface to test each field.
     * @tc.expected: step2. construct success.
     */
    vector<Query> queries;
    queries.push_back(Query::Select().IsNotNull("$.field1"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field3"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field5"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field7"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field8"));
    /**
     * @tc.steps: step3. call GetEntries(query, Entries), GetEntries(query, resultSet), and GetCount()
     *    to check the query
     * @tc.expected: step3. GetEntries success and each query can return right result.
     */
    for (vector<Entry>::size_type index = 0; index < queries.size(); index++) {
        EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, queries[index], expectEntries[index], DBStatus::OK, true));
    }
}

/**
 * @tc.name: NotNullTest 002
 * @tc.desc: Verify query IsNotNull can get right result when put right param.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, NotNullTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entries, entries2;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entries);

    /**
     * @tc.steps: step2. test the Query with IsNotNull interface to test each field.
     * @tc.expected: step2. construct success.
     */
    vector<Query> queries;
    queries.push_back(Query::Select().IsNotNull("$.field1"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field3"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field5"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field7"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field8"));
    /**
     * @tc.steps: step3. call GetEntries(query, Entries), GetEntries(query, resultSet), and GetCount()
     *    to check the query
     * @tc.expected: step3. GetEntries success and each query can return right result.
     */
    for (vector<Entry>::size_type index = 0; index < queries.size(); index++) {
        EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, queries[index], entries, DBStatus::OK, true));
    }

    /**
     * @tc.steps: step4. delete all the 5 entries and insert new 5 records that include null element.
     * @tc.expected: step4. delete and insert success.
     */
    vector<Key> keys;
    for (vector<Entry>::iterator iter = entries.begin(); iter != entries.end(); iter++) {
        keys.push_back(iter->key);
    }
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbQueryDelegate, keys), OK);

    vector<vector<string>> fieldValue2 = {
        {"null", "null", "null", "null", "null"}, {"null", "null", "null", "null", "null"},
        {"null", "null", "null", "null", "null"}, {"null", "null", "null", "null", "null"},
        {"null", "null", "null", "null", "null"}};
    vector<string> schemasValue2 = GenerateCombinationSchemaValue(fieldValue2);
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue2, entries2);

    /**
     * @tc.steps: step5. get the Query again to check the IsNotNull interface.
     * @tc.expected: step5. query success.
     */
    queries.clear();
    queries.push_back(Query::Select().IsNotNull("$.field1"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field3"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field5"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field7"));
    queries.push_back(Query::Select().IsNotNull("$.field2.field4.field6.field8"));
    /**
     * @tc.steps: step6. call GetEntries(query, Entries), GetEntries(query, resultSet), and GetCount()
     *    to check the query
     * @tc.expected: step6. GetEntries success and each query can return right result.
     */
    vector<vector<Entry>> expectEntries = {{}, {}, {}, {}, {}};
    for (vector<Entry>::size_type index = 0; index < queries.size(); index++) {
        EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, queries[index], expectEntries[index],
            DBStatus::NOT_FOUND, true));
    }
}

/**
 * @tc.name: PrefixKeyTest 001
 * @tc.desc: Verify that PrefixKey interface can return right records.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 10 entries 5 of which has prefixKey {'k'} and rest of them has
     *    prefixKey {'a'}.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesExpect, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey('') interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 10 records.
     */
    for (vector<Entry>::size_type index = 0; index < entriesA.size(); index++) {
        entriesExpect.push_back(entriesA[index]);
    }
    for (vector<Entry>::size_type index = 0; index < entriesK.size(); index++) {
        entriesExpect.push_back(entriesK[index]);
    }
    Query query1 = Query::Select().PrefixKey({});
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, entriesExpect, DBStatus::OK, true));

    /**
     * @tc.steps: step3. test the Query with PrefixKey({'k'}) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 5 records.
     */
    Query query2 = Query::Select().PrefixKey({'k'});
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, entriesK, DBStatus::OK, true));
    /**
     * @tc.steps: step4. test the Query with PrefixKey({'a'}) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query success and all of the 3 interface can find 5 records.
     */
    Query query3 = Query::Select().PrefixKey({'b'});
    entriesExpect.clear();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, entriesExpect, DBStatus::NOT_FOUND, true));
}

/**
 * @tc.name: PrefixKeyTest 002
 * @tc.desc: Verify that none-schema DB use PrefixKey can return right result
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest002, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(delegate != nullptr && manager != nullptr);
    /**
     * @tc.steps: step1. put 10 records to DB, 5 of which has the prefix key 'k', and rest of which has prefix key 'a';
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesExpect, entriesGot;
    PresetDatasToDB(delegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(delegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey({}) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 10 records.
     */
    for (vector<Entry>::size_type index = 0; index < entriesA.size(); index++) {
        entriesExpect.push_back(entriesA[index]);
    }
    for (vector<Entry>::size_type index = 0; index < entriesK.size(); index++) {
        entriesExpect.push_back(entriesK[index]);
    }
    Query query1 = Query::Select().PrefixKey({});
    EXPECT_TRUE(CheckQueryResult(*delegate, query1, entriesExpect, DBStatus::OK, true));

    /**
     * @tc.steps: step3. test the Query with PrefixKey({'k'}) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 5 records.
     */
    Query query2 = Query::Select().PrefixKey({'k'});
    EXPECT_TRUE(CheckQueryResult(*delegate, query2, entriesK, DBStatus::OK, true));
    /**
     * @tc.steps: step4. test the Query with PrefixKey({'b'}) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query success and all of the 3 interface can find 0 records.
     */
    entriesA.clear();
    Query query3 = Query::Select().PrefixKey({'b'});
    EXPECT_TRUE(CheckQueryResult(*delegate, query3, entriesA, DBStatus::NOT_FOUND, true));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: PrefixKeyTest 003
 * @tc.desc: Verify that use PrefixKey check schema DB can return right result, and prefixKey can't be call many time
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 10 entries 5 of which has prefixKey {'k'} and rest of them has
     *    prefixKey {'a'}.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey({}).Limit(5, 0) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 5 records.
     */
    Query query1 = Query::Select().PrefixKey({}).Limit(5, 0);
    vector<Entry> entriesCheck = {entriesA[0], entriesA[INDEX_FIRST], entriesA[INDEX_SECOND],
        entriesA[INDEX_THIRD], entriesA[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, entriesCheck, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with PrefixKey({}).And().GreaterThan("$.field2.field4.field5", 10).Limit(5, 0)
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 4 records.
     */
    Query query2 = Query::Select().PrefixKey({}).GreaterThan("$.field2.field4.field5", "10").Limit(5, 0);
    entriesCheck = {entriesA[INDEX_FIRST], entriesA[INDEX_THIRD], entriesA[INDEX_FORTH],
        entriesK[INDEX_FIRST], entriesK[INDEX_THIRD]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, entriesCheck, DBStatus::OK, false));
    /**
     * @tc.steps: step4. test the Query with PrefixKey({}).And().PrefixKey({'k'}).And().PrefixKey({'b'})
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query failed and returned INVALID_QUERY_FORMAT.
     */
    Query query3 = Query::Select().PrefixKey({}).PrefixKey({'k'}).And().PrefixKey({'b'});
    entriesCheck.clear();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, entriesCheck, DBStatus::INVALID_QUERY_FORMAT, false));
}

/**
 * @tc.name: PrefixKeyTest 004
 * @tc.desc: Verify that none-schema DB can't use query interface except PrefixKey and Limit.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest004, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(delegate != nullptr && manager != nullptr);
    /**
     * @tc.steps: step1. put 10 records to DB, 5 of which has the prefix key 'k', and rest of which has prefix key 'a';
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesExpect, entriesGot;
    PresetDatasToDB(delegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(delegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey('').Limit(5, 0) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 5 records.
     */
    Query query1 = Query::Select().PrefixKey({ }).Limit(5, 0);
    vector<Entry> entriesCheck = {entriesA[0], entriesA[INDEX_FIRST], entriesA[INDEX_SECOND],
        entriesA[INDEX_THIRD], entriesA[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*delegate, query1, entriesCheck, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with PrefixKey({}).And().GreaterThan("$.field2.field4.field5", 10).Limit(5, 0)
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 4 records.
     */
    Query query2 = Query::Select().PrefixKey({}).GreaterThan("$.field2.field4.field5", 10).Limit(5, 0);
    entriesCheck.clear();
    EXPECT_TRUE(CheckQueryResult(*delegate, query2, entriesCheck, DBStatus::NOT_SUPPORT, false));
    /**
     * @tc.steps: step4. test the Query with PrefixKey({}).And().PrefixKey({'k'}).And().PrefixKey({'b'})
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query failed and returned INVALID_QUERY_FORMAT.
     */
    Query query3 = Query::Select().PrefixKey({}).PrefixKey({'k'}).PrefixKey({'b'});
    EXPECT_TRUE(CheckQueryResult(*delegate, query3, entriesCheck, DBStatus::INVALID_QUERY_FORMAT, false));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: PrefixKeyTest 005
 * @tc.desc: Verify that schema DB can use prefixKey and other interface constructor legal query
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 10 entries 5 of which has prefixKey {'k'} and rest of them has
     *    prefixKey {'a'}.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey({}).And().EqualTo("$.field2.field3",true).OrderBy("field8")
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 8 records.
     */
    Query query1 = Query::Select().PrefixKey({}).EqualTo("$.field2.field3",
        true).OrderBy("$.field2.field4.field6.field8");
    vector<Entry> entriesExpect = {entriesA[INDEX_FIRST], entriesK[INDEX_FIRST], entriesA[0], entriesK[0],
        entriesA[INDEX_SECOND], entriesK[INDEX_SECOND], entriesA[INDEX_FORTH], entriesK[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, entriesExpect, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with NotEqualTo("$.field2.field3",false).And().PrefixKey({}).OrderBy("field8")
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 8 records.
     */
    Query query2 = Query::Select().NotEqualTo("$.field2.field3", false).PrefixKey({}). \
        OrderBy("$.field2.field4.field6.field8");
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, entriesExpect, DBStatus::OK, false));

    /**
     * @tc.steps: step4. test the Query with LessThan("$.field2.field4.field6.field8",
     *    149).And().PrefixKey({}).Limit(3, 0) interface and then check the query use GetEntries(query, Entries),
     *    GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query successfully and returned {entriesA[0], entriesA[1], entriesA[2]}.
     */
    Query query3 = Query::Select().LessThan("$.field2.field4.field6.field8", 149).PrefixKey({}).Limit(3, 0);
    entriesExpect = {entriesA[0], entriesA[INDEX_FIRST], entriesA[INDEX_SECOND]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step5. test the Query with GreaterThanOrEqualTo("$.field2.field4.field5", 9).And().PrefixKey({'a'}).
     *    LessThanOrEqualTo("field7", 50).OrderBy("$.field2.field4.field6.field8").Limit(4, 0) interface and then
     *    check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step5. query failed and can't find any records.
     */
    Query query4 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 9).PrefixKey({'a'}).And(). \
        LessThanOrEqualTo("$.field2.field4.field6.field7", 50).OrderBy("$.field2.field4.field6.field8").Limit(4, 0);
    entriesExpect = {entriesA[INDEX_FIRST], entriesA[INDEX_THIRD], entriesA[INDEX_SECOND], entriesA[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query4, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step6. test the Query with GreaterThanOrEqualTo("$.field2.field4.field5", 9).And().PrefixKey({'b'}).
     *    LessThanOrEqualTo("field7", 50).OrderBy("$.field2.field4.field6.field8").Limit(4, 0) interface and then
     *    check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step6. query failed and can't find any records.
     */
    Query query5 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 9).PrefixKey({'b'}).And(). \
        LessThanOrEqualTo("$.field2.field4.field6.field7", 50).OrderBy("$.field2.field4.field6.field8"). \
        OrderBy("$.field1");
    entriesExpect = {};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query5, entriesExpect, DBStatus::NOT_FOUND, false));
}

/**
 * @tc.name: PrefixKeyTest 006
 * @tc.desc: Verify that schema DB can use prefixKey and other interface constructor legal query
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 10 entries 5 of which has prefixKey {'k'} and rest of them has
     *    prefixKey {'a'}.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"null", "true", "9", "1000", "12"},  {"abc123", "null", "88", "-100", "-99"},
        {"abfxy", "true", "null", "0", "38"}, {"ab789", "false", "999", "null", "15.8"},
        {"ab000", "true", "33", "30", "null"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with PrefixKey({'b'}).And().Like("$.field1", ab%).OrderBy("field8")
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 8 records.
     */
    Query query1 = Query::Select().PrefixKey({'b'}).Like("$.field1", "ab%"). \
        OrderBy("$.field2.field4.field6.field8");
    vector<Entry> entriesExpect;
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, entriesExpect, DBStatus::NOT_FOUND, false));

    /**
     * @tc.steps: step3. test the Query with NotLike("$.field1", ab%).And().PrefixKey({}).OrderBy("field8")
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 8 records.
     */
    Query query2 = Query::Select().NotLike("$.field1", "ab%").PrefixKey({}). \
        OrderBy("$.field2.field4.field6.field8");
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, entriesExpect, DBStatus::NOT_FOUND, false));
    /**
     * @tc.steps: step4. test the Query with In("$.field2.field4.field6.field7",
     *    [0, 30, 50, 1000]).Limit(3, 3) interface and then check the query use GetEntries(query, Entries),
     *    GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query successfully and returned {entriesA[0], entriesA[1], entriesA[2]}.
     */
    vector<string> scope = {"0", "30", "50", "1000"};
    Query query3 = Query::Select().PrefixKey({}).In("$.field2.field4.field6.field7", scope).Limit(3, 3);
    entriesExpect = {entriesK[0], entriesK[INDEX_SECOND], entriesK[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step5. test the Query with NotIn("$.field2.field4.field6.field7", [-100]).And().PrefixKey({}).
     *    Limit(3, 3) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step5. query failed and can't find any records.
     */
    scope = {"-100"};
    Query query4 = Query::Select().NotIn("$.field2.field4.field6.field7", scope).PrefixKey({}).Limit(3, 3);
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query4, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step6. test the Query with IsNull("$.field2.field4.field5").And().PrefixKey({}).
        IsNotNull("$.field2.field3").OrderBy("$.field2.field4.field6.field8") interface
     *    and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step6. query failed and can't find any records.
     */
    Query query5 = Query::Select().IsNull("$.field2.field4.field5").And().PrefixKey({}). \
        IsNotNull("$.field2.field3").OrderBy("$.field2.field4.field6.field8");
    entriesExpect = {entriesA[INDEX_SECOND], entriesK[INDEX_SECOND]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query5, entriesExpect, DBStatus::OK, false));
}

/**
 * @tc.name: PrefixKeyTest 007
 * @tc.desc: the query interface that without any condition can get all the records.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest007, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(delegate != nullptr && manager != nullptr);
    /**
     * @tc.steps: step1. create schema db and non-schema DB and put 10 entries to them.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"},      {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"},      {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"},    {"abc", "true", "9", "1000", "12"},
        {"abc123", "true", "88", "-100", "-99"}, {"abfxy", "true", "10", "0", "38"},
        {"ab789", "false", "999", "50", "15.8"}, {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesSchema, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesSchema);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*delegate, entriesSchema), OK);

    /**
     * @tc.steps: step2. test the Query Select() from the schema DB and then check the query
     *    use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 10 records.
     */
    Query query = Query::Select();
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query, entriesSchema, DBStatus::OK, true));

    /**
     * @tc.steps: step3. test the Query Select() from the non-schema DB and then check the query
     *    use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 10 records.
     */
    EXPECT_TRUE(CheckQueryResult(*delegate, query, entriesSchema, DBStatus::OK, true));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: PrefixKeyTest 008
 * @tc.desc: the query that the position of prefixKey do not effect the result.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db and put 10 entries 5 of which has prefixKey {'k'} and rest of them has
     *    prefixKey {'a'}.
     * @tc.expected: step1. create and put successfully.
     */
    vector<vector<string>> fieldValue = {
        {"abc", "true", "9", "1000", "12"}, {"abc123", "true", "88", "-100", "-99"},
        {"abfxy", "true", "10", "0", "38"}, {"ab789", "false", "999", "50", "15.8"},
        {"ab000", "true", "33", "30", "149"}};
    vector<string> schemasValue = GenerateCombinationSchemaValue(fieldValue);
    vector<Entry> entriesK, entriesA, entriesGot;
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);

    /**
     * @tc.steps: step2. test the Query with Like("$.field1", "ab%").OrderBy("$.field2.field4.field6.field8").
     *     PrefixKey({'a'}).Limit(5, 1) interface and then check the query use GetEntries(query, Entries),
     *     GetEntries(query, resultSet), GetCount().
     * @tc.expected: step2. query success and all of the 3 interface can find 4 records.
     */
    Query query1 = Query::Select().Like("$.field1", "ab%").OrderBy("$.field2.field4.field6.field8").
        PrefixKey({'a'}).Limit(5, 1);
    vector<Entry> entriesExpect = {entriesA[INDEX_ZEROTH], entriesA[INDEX_THIRD], entriesA[INDEX_SECOND],
        entriesA[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query1, entriesExpect, DBStatus::OK, false));

    /**
     * @tc.steps: step3. test the Query with NotLike("$.field1", ab%).And().PrefixKey({}).OrderBy("field8")
     *    interface and then check the query use GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step3. query success and all of the 3 interface can find 8 records.
     */
    Query query2 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 9).And().
        LessThanOrEqualTo("$.field2.field4.field6.field7", 50).OrderBy("$.field2.field4.field6.field8").
        PrefixKey({'a'}).Limit(4, 0);
    entriesExpect = {entriesA[INDEX_FIRST], entriesA[INDEX_THIRD], entriesA[INDEX_SECOND], entriesA[INDEX_FORTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query2, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step4. test the Query with GreaterThanOrEqualTo("$.field2.field4.field5", 9).And().
     *    LessThanOrEqualTo("$.field2.field4.field6.field7", 50).OrderBy("$.field1").PrefixKey({'a'}).
     *    OrderBy("$.field2.field4.field6.field8") interface and then check the query use GetEntries(query, Entries),
     *    GetEntries(query, resultSet), GetCount().
     * @tc.expected: step4. query successfully and returned {entriesA[0], entriesA[1], entriesA[2]}.
     */
    Query query3 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 9).And().
        LessThanOrEqualTo("$.field2.field4.field6.field7", 50).OrderBy("$.field1").PrefixKey({'a'}).
        OrderBy("$.field2.field4.field6.field8");
    entriesExpect = {entriesA[INDEX_FORTH], entriesA[INDEX_THIRD], entriesA[INDEX_FIRST], entriesA[INDEX_SECOND]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query3, entriesExpect, DBStatus::OK, false));
    /**
     * @tc.steps: step5. test the Query with NotIn("$.field2.field4.field6.field7", [-100]).And().PrefixKey({}).
     *    Limit(3, 3) interface and then check the query use
     *    GetEntries(query, Entries), GetEntries(query, resultSet), GetCount().
     * @tc.expected: step5. query failed and can't find any records.
     */
    fieldValue = {
        {"abc", "true", "33", "1000", "12"}, {"abc", "true", "9", "-100", "-99"},
        {"ab789", "true", "100", "0", "38"}, {"ab789", "false", "99", "50", "15.8"},
        {"abc", "true", "9", "30", "149"}};
    schemasValue = GenerateCombinationSchemaValue(fieldValue);
    entriesA.clear();
    entriesK.clear();
    PresetDatasToDB(g_nbQueryDelegate, KEY_A, schemasValue, entriesA);
    PresetDatasToDB(g_nbQueryDelegate, KEY_K, schemasValue, entriesK);

    Query query4 = Query::Select().OrderBy("$.field1").OrderBy("$.field2.field4.field5").
        OrderBy("$.field2.field4.field6.field8").PrefixKey({'a'}).Limit(3, 3);
    entriesExpect = {entriesA[INDEX_FORTH], entriesA[INDEX_ZEROTH]};
    EXPECT_TRUE(CheckQueryResult(*g_nbQueryDelegate, query4, entriesExpect, DBStatus::OK, false));
}

void MoveCursor(KvStoreResultSet &resultSet, const vector<Entry> &entriesBatch)
{
    int currentPosition = CURSOR_POSITION_NEGATIVE1;
    Entry entry;
    bool result;
    for (int position = CURSOR_POSITION_NEGATIVE1; position < TEN_RECORDS; ++position) {
        result = resultSet.MoveToNext();
        if (position < (TEN_RECORDS - CURSOR_POSITION_1)) {
            EXPECT_TRUE(result);
        } else {
            EXPECT_TRUE(result == false);
        }
        currentPosition = resultSet.GetPosition();
        EXPECT_TRUE(currentPosition == (position + CURSOR_POSITION_1));
        if (position < (TEN_RECORDS - CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet.GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entriesBatch[position + CURSOR_POSITION_1].key));
            EXPECT_TRUE(CompareVector(entry.value, entriesBatch[position + CURSOR_POSITION_1].value));
        } else {
            EXPECT_TRUE(resultSet.GetEntry(entry) == NOT_FOUND);
        }
    }
    /**
     * @tc.steps: step3. set the current position is 10, call MoveToPrevious, GetPostion, GetEntry looply.
     * @tc.expected: step3. return values are all right.
     */
    for (int position = TEN_RECORDS; position > CURSOR_POSITION_NEGATIVE1; --position) {
        result = resultSet.MoveToPrevious();
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(result);
        } else {
            EXPECT_TRUE(result == false);
        }
        currentPosition = resultSet.GetPosition();
        EXPECT_TRUE(currentPosition == (position - CURSOR_POSITION_1));
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet.GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entriesBatch[position - CURSOR_POSITION_1].key));
            EXPECT_TRUE(CompareVector(entry.value, entriesBatch[position - CURSOR_POSITION_1].value));
        } else {
            EXPECT_TRUE(resultSet.GetEntry(entry) == NOT_FOUND);
        }
    }
}
/*
 * @tc.name: PrefixKeyTest 009
 * @tc.desc: test MoveToNext, MoveToPrevious interface with 10 2M data by query= prefixkey.
 * @tc.type: FUNC
 * @tc.require: SR000EPA23
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryExpandTest, PrefixKeyTest009, TestSize.Level2)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, PERF_SCHEMA_DEFINE,
        PERF_SCHEMA_SIX_INDEXES, SKIP_SIZE);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_NE(manager, nullptr);
    ASSERT_NE(delegate, nullptr);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    EntrySize entrySize = {KEY_EIGHT_BYTE, TWO_M_LONG_STRING};
    entriesBatch = DistributedDBSchemaTestTools::GenerateFixedSchemaRecords(allKeys, TEN_RECORDS, entrySize, 'k', '0');
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*delegate, entriesBatch), OK);

    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet by query.
     * @tc.expected: step1. get success.
     */
    Query query = Query::Select().GreaterThanOrEqualTo("$.field1", 0).PrefixKey({'k'});
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(query, resultSet) == OK);
    sort(entriesBatch.begin(), entriesBatch.end(), DistributedTestTools::CompareKey);
    /**
     * @tc.steps: step2. set the current position is -1, call MoveToNext, GetPostion and GetEntry interface looply.
     * @tc.expected: step2. return values are all right.
     */
    ASSERT_NE(resultSet, nullptr);
    MoveCursor(*resultSet, entriesBatch);
    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, false));
}
}
#endif