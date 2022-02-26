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
#include <cmath>
#include <random>
#include <chrono>
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
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbPredicateQuery {
KvStoreNbDelegate *g_nbQueryDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;
Option g_predicateOption;
const int IN_AND_NOTIN_MAX_LENGTH = 128;
const int LIKE_AND_NOTLIKE_MAX_LENGTH = 50000;

DistributedDB::CipherPassword g_passwd1;
const std::string VALID_DEFINE_STRING = "{\"field1\":\"STRING ,DEFAULT null\",\"field2\":"
    "{\"field3\":\"STRING ,DEFAULT null\",\"field4\":[]}}";
const std::string VALID_DEFINE_BOOL = "{\"field1\": \"BOOL ,DEFAULT null\",\"field2\":"
    "{\"field3\": \"BOOL ,DEFAULT null\",\"field4\":[]}}";
const std::string VALID_DEFINE_INT = "{\"field1\": \"INTEGER ,DEFAULT null\",\"field2\":"
    "{\"field3\": \"INTEGER ,DEFAULT null\",\"field4\":[]}}";
const std::string VALID_DEFINE_LONG = "{\"field1\": \"LONG ,DEFAULT null\",\"field2\":"
    "{\"field3\": \"LONG ,DEFAULT null\",\"field4\":[]}}";
const std::string VALID_DEFINE_DOUBLE = "{\"field1\": \"DOUBLE ,DEFAULT null\",\"field2\":"
    "{\"field3\": \"DOUBLE ,DEFAULT null\",\"field4\": []}}";

const int SCHEMA_GOT_COUNT_1 = 1;
const int SCHEMA_GOT_COUNT_2 = 2;
const int SCHEMA_GOT_COUNT_3 = 3;
const int SCHEMA_GOT_COUNT_4 = 4;
const int SCHEMA_GOT_COUNT_5 = 5;
const int CURSOR_OFFSET_2 = 2;
const int NO_RECORD = 0;

const int CURSOR_POSITION_NEGATIVE1 = -1;
const int INTERFACE_QTY_2 = 2;
const int INTERFACE_QTY_6 = 6;
class DistributeddbNbPredicateQueryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbPredicateQueryTest::SetUpTestCase(void)
{
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
}

void DistributeddbNbPredicateQueryTest::TearDownTestCase(void)
{
}

void DistributeddbNbPredicateQueryTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbNbPredicateQueryTest::TearDown(void)
{
    RemoveDir(NB_DIRECTOR);
}

void PrepareSchemaDBAndData(KvStoreNbDelegate *&delegate, KvStoreDelegateManager *&manager,
    vector<string> &schemasValue, vector<Entry> &entries, const std::string schemaDefine)
{
    string validSchema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, VALID_INDEX_1);

    g_predicateOption = g_option;
    g_predicateOption.isMemoryDb = false;
    g_predicateOption.schema = validSchema;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_predicateOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    vector<Key> keys;
    if (schemasValue.size() == 5) { // 5 elements
        keys = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5};
    } else if (schemasValue.size() == 7) { // 7 elements
        keys = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7};
    } else if (schemasValue.size() == 10) { // 10 elements
        keys = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_10};
    }
    for (unsigned long index = 0; index < schemasValue.size(); index++) {
        Value value(schemasValue[index].begin(), schemasValue[index].end());
        entries.push_back({keys[index], value});
    }

    for (unsigned long index = 0; index < schemasValue.size(); index++) {
        EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, entries[index].key, entries[index].value), OK);
    }
}

bool CheckSchemaQuery(KvStoreNbDelegate *&delegate, Query &queryNeedCheck, vector<Entry> &expectEntry,
    const int &expectCount, DBStatus status)
{
    bool result = true;
    vector<Entry> entries;
    int count = 0;
    DBStatus statusGot = delegate->GetEntries(queryNeedCheck, entries);
    result = (statusGot == status) && result;
    MST_LOG("GetEntries entries result: %d, statusGot: %d", result, statusGot);
    for (unsigned long index = 0; index < entries.size(); index++) {
        if (!expectEntry.empty()) {
            result = (entries[index].key == expectEntry[index].key) && result;
            MST_LOG("check entries[%ld] key result:%d", index, result);
            result = (entries[index].value == expectEntry[index].value) && result;
        } else {
            MST_LOG("expect value is empty, but Got has: %zu entries", entries.size());
            return false;
        }
    }
    statusGot = delegate->GetCount(queryNeedCheck, count);
    result = (statusGot == status) && result;
    MST_LOG("GetCount result: %d, statusGot: %d", result, statusGot);
    result = (count == expectCount) && result;
    MST_LOG("Is GotCount equal to Expect result: %d", result);
    KvStoreResultSet *resultSet = nullptr;
    if (status != DBStatus::NOT_FOUND) {
        statusGot = delegate->GetEntries(queryNeedCheck, resultSet);
        result = (statusGot == status) && result;
    } else {
        statusGot = delegate->GetEntries(queryNeedCheck, resultSet);
        result = (statusGot == DBStatus::OK) && result;
    }
    MST_LOG("GetEntries resultSet result: %d, statusGot: %d", result, statusGot);
    if (resultSet != nullptr) {
        int quatity = resultSet->GetCount();
        result = (quatity == expectCount) && result;
        MST_LOG("ResultSet GetCount result: %d, quatity: %d", result, quatity);
        result = (delegate->CloseResultSet(resultSet) == DBStatus::OK) && result;
    }
    return result;
}

bool CheckSchemaQueryForOrderBy(KvStoreNbDelegate *&delegate, Query &queryNeedCheck, vector<Entry> &expectEntry,
    const int &expectCount, DBStatus status)
{
    bool result = true;
    vector<Entry> entries;
    DBStatus statusGot = delegate->GetEntries(queryNeedCheck, entries);
    result = (statusGot == status) && result;
    MST_LOG("GetEntries entries result: %d, statusGot: %d", result, statusGot);
    for (unsigned long index = 0; index < entries.size(); index++) {
        if (!expectEntry.empty()) {
            result = (entries[index].key == expectEntry[index].key) && result;
            MST_LOG("check entries[%ld] key result:%d", index, result);
            result = (entries[index].value == expectEntry[index].value) && result;
        } else {
            MST_LOG("expect value is empty, but Got: %zu entries", entries.size());
        }
    }
    KvStoreResultSet *resultSet = nullptr;
    if (status != DBStatus::NOT_FOUND) {
        statusGot = delegate->GetEntries(queryNeedCheck, resultSet);
        result = (statusGot == status) && result;
    } else {
        statusGot = delegate->GetEntries(queryNeedCheck, resultSet);
        result = (statusGot == DBStatus::OK) && result;
    }
    MST_LOG("GetEntries resultSet result: %d, statusGot: %d", result, statusGot);
    if (resultSet != nullptr) {
        int quatity = resultSet->GetCount();
        result = (quatity == expectCount) && result;
        MST_LOG("ResultSet GetCount result: %d, quatity: %d", result, quatity);
        result = (delegate->CloseResultSet(resultSet) == DBStatus::OK) && result;
    }
    return result;
}

bool CheckSchemaNotExist(KvStoreNbDelegate *&delegate)
{
    bool result = true;
    vector<Entry> entries;
    bool valueBool = true; // boolean true
    int valueInt = 10; // int value 10
    int64_t valueLong = 15; // long value 15
    double valueDouble = 10.5; // double value 10.5

    string fieldStr = "$.field1";
    vector<Query> queries1, queries2, queries3;
    bool functionChoiceComp1[INTERFACE_QTY_6] = {true, false, false, false, false, false};
    QueryGenerate<bool>::Instance().GenerateQueryComp(queries1, functionChoiceComp1, fieldStr, valueBool);
    bool functionChoiceComp2[INTERFACE_QTY_6] = {false, true, false, false, false, false};
    QueryGenerate<bool>::Instance().GenerateQueryComp(queries2, functionChoiceComp2, fieldStr, valueBool);
    bool functionChoiceComp3[INTERFACE_QTY_6] = {false, false, true, true, true, true};
    QueryGenerate<bool>::Instance().GenerateQueryComp(queries3, functionChoiceComp3, fieldStr, valueBool);

    bool functionChoiceComp4[INTERFACE_QTY_6] = {true, false, false, false, true, true};
    QueryGenerate<int>::Instance().GenerateQueryComp(queries1, functionChoiceComp4, fieldStr, valueInt);
    bool functionChoiceComp5[INTERFACE_QTY_6] = {false, true, true, true, false, false};
    QueryGenerate<int>::Instance().GenerateQueryComp(queries2, functionChoiceComp5, fieldStr, valueInt);

    QueryGenerate<int64_t>::Instance().GenerateQueryComp(queries1, functionChoiceComp4, fieldStr, valueLong);
    QueryGenerate<int64_t>::Instance().GenerateQueryComp(queries2, functionChoiceComp5, fieldStr, valueLong);

    QueryGenerate<double>::Instance().GenerateQueryComp(queries1, functionChoiceComp4, fieldStr, valueDouble);
    QueryGenerate<double>::Instance().GenerateQueryComp(queries2, functionChoiceComp5, fieldStr, valueDouble);

    vector<bool> valuesBool = {true};
    vector<int> valuesInt = {10, 11};
    vector<int64_t> valuesLong = {15, 16};
    vector<double> valuesDouble = {10.5, 11.5};

    bool functionChoiceIn1[INTERFACE_QTY_2] = {true, false};
    QueryGenerate<bool>::Instance().GenerateQueryIn(queries1, functionChoiceIn1, fieldStr, valuesBool);
    bool functionChoiceIn2[INTERFACE_QTY_2] = {false, true};
    QueryGenerate<bool>::Instance().GenerateQueryIn(queries2, functionChoiceIn2, fieldStr, valuesBool);

    QueryGenerate<int>::Instance().GenerateQueryIn(queries1, functionChoiceIn1, fieldStr, valuesInt);
    QueryGenerate<int>::Instance().GenerateQueryIn(queries2, functionChoiceIn2, fieldStr, valuesInt);

    QueryGenerate<int64_t>::Instance().GenerateQueryIn(queries1, functionChoiceIn1, fieldStr, valuesLong);
    QueryGenerate<int64_t>::Instance().GenerateQueryIn(queries2, functionChoiceIn2, fieldStr, valuesLong);

    QueryGenerate<double>::Instance().GenerateQueryIn(queries1, functionChoiceIn1, fieldStr, valuesDouble);
    QueryGenerate<double>::Instance().GenerateQueryIn(queries2, functionChoiceIn2, fieldStr, valuesDouble);

    for (auto const &it : queries1) {
        result = (delegate->GetEntries(it, entries) == NOT_FOUND) && result;
    }
    for (auto const &it : queries2) {
        result = (delegate->GetEntries(it, entries) == OK) && result;
        result = (entries.size() == SCHEMA_GOT_COUNT_4) && result;
    }
    for (auto const &it : queries3) {
        result = (delegate->GetEntries(it, entries) == INVALID_QUERY_FORMAT) && result;
    }
    return result;
}

/**
 * @tc.name: Query 001
 * @tc.desc: check valid schema of String type and construct valid query object and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query001, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\":\"fxy\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"abc\",\"field2\":{\"field3\":\"fxz\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\": null ,\"field2\":{\"field3\":\"fxw\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"TRUE\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_STRING);

    /**
     * @tc.steps: step2. test the Query interface of EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and check the result with GetEntries.
     * @tc.expected: step2. each interface called ok and GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field1", "bxz");
    vector<Entry> expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query1, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));
    Query query5 = Query::Select().GreaterThan("$.field1", "abc");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query5, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query2 = Query::Select().NotEqualTo("$.field1", "bxz");
    expectEntry = {entries[INDEX_FIRST], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query2, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));
    Query query8 = Query::Select().LessThanOrEqualTo("$.field1", "abc");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query8, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    vector<string> scope = {"bxz", "abc"};
    Query query3 = Query::Select().In("$.field1", scope);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_FIRST], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query3, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));
    Query query7 = Query::Select().GreaterThanOrEqualTo("$.field1", "abc");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query7, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    Query query4 = Query::Select().NotIn("$.field1", scope);
    expectEntry = {entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query4, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));

    Query query6 = Query::Select().LessThan("$.field1", "abc");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query6, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));

    EXPECT_TRUE(CheckSchemaNotExist(delegate));

    /**
     * @tc.steps: step3. test Fuzzy match interface of Like and NotLike where field3 = "fx?" and check.
     * @tc.expected: step3. check the result rightly.
     */
    Query query10 = Query::Select().Like("$.field2.field3", "fx_");
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_FIRST], entries[INDEX_SECOND]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query10, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));
    Query query11 = Query::Select().NotLike("$.field2.field3", "fx_");
    expectEntry = {};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query11, expectEntry, 0, DBStatus::NOT_FOUND));

    /**
     * @tc.steps: step4. test IsNull interface where field = field3.
     * @tc.expected: step4. check the result rightly.
     */
    Query query12 = Query::Select().IsNull("$.field2.field3");
    expectEntry = {entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query12, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));
    /**
     * @tc.steps: step5. test OrderBy interface where field = field1.
     * @tc.expected: step5. check the result rightly.
     */
    Query query13 = Query::Select().OrderBy("$.field2.field3");
    expectEntry = {entries[INDEX_THIRD], entries[INDEX_FORTH], entries[INDEX_SECOND], entries[INDEX_ZEROTH],
        entries[INDEX_FIRST]};
    EXPECT_TRUE(CheckSchemaQueryForOrderBy(delegate, query13, expectEntry, SCHEMA_GOT_COUNT_5, DBStatus::OK));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

bool InvalidFieldCheck(KvStoreNbDelegate *&delegate, string field, DBStatus status)
{
    bool result = true;
    Query query1 = Query::Select().EqualTo(field, "bxz");
    vector<Entry> expectEntry = {};
    result = (CheckSchemaQuery(delegate, query1, expectEntry, 0, status)) && result;
    MST_LOG("EqualTo result: %d", result);
    Query query2 = Query::Select().NotEqualTo(field, "bxz");
    result = (CheckSchemaQuery(delegate, query2, expectEntry, 0, status)) && result;
    MST_LOG("NotEqualTo result: %d", result);

    vector<string> scope = {"bxz", "abc"};
    Query query3 = Query::Select().In(field, scope);
    result = (CheckSchemaQuery(delegate, query3, expectEntry, 0, status)) && result;
    MST_LOG("In result: %d", result);

    Query query4 = Query::Select().NotIn(field, scope);
    result = (CheckSchemaQuery(delegate, query4, expectEntry, 0, status)) && result;
    MST_LOG("NoIn result: %d", result);

    Query query5 = Query::Select().GreaterThan(field, "abc");
    result = (CheckSchemaQuery(delegate, query5, expectEntry, 0, status)) && result;
    MST_LOG("GreaterThan result: %d", result);

    Query query6 = Query::Select().LessThan(field, "abc");
    result = (CheckSchemaQuery(delegate, query6, expectEntry, 0, status)) && result;
    MST_LOG("LessThan result: %d", result);

    Query query7 = Query::Select().GreaterThanOrEqualTo(field, "abc");
    result = (CheckSchemaQuery(delegate, query7, expectEntry, 0, status)) && result;
    MST_LOG("GreaterThanOrEqualTo result: %d", result);

    Query query8 = Query::Select().LessThanOrEqualTo(field, "abc");
    result = (CheckSchemaQuery(delegate, query8, expectEntry, 0, status)) && result;
    MST_LOG("LessThanOrEqualTo result: %d", result);
    return result;
}

/**
 * @tc.name: Query 002
 * @tc.desc: check valid schema of String type and construct invalid fields and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query002, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\":\"fxy\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"abc\",\"field2\":{\"field3\":\"fxz\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\": null ,\"field2\":{\"field3\":\"fxw\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"TRUE\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_STRING);

    /**
     * @tc.steps: step2. use notexist field5 to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo and GetEntries with Query.
     * @tc.expected: step2. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field5", DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step3. use not Leaf node field to test EqualTo/NotEqualTo/In/NotIn/
     *    GreaterThan/LessThan/GreaterThanOrEqualTo/LessThanOrEqualTo and call GetEntries and check.
     * @tc.expected: step3. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field2.field4", DBStatus::INVALID_QUERY_FIELD));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field3.field2", DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step4. use invalid format field to test EqualTo/NotEqualTo/In/NotIn/
     *    GreaterThan/LessThan/GreaterThanOrEqualTo/LessThanOrEqualTo and call GetEntries and check.
     * @tc.expected: step4. GetEntries return INVALID_QUERY_FORMAT.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, ".field2.field3", DBStatus::INVALID_QUERY_FORMAT));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$$field1", DBStatus::INVALID_QUERY_FORMAT));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

bool CheckSchemaBoolNotExist(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueStringTrue = "true";
    string valueString = "abc";
    int64_t valueLong = 15; // long value 15
    double valueDouble = 10.5; // double value 10.5
    float valueFloat = 5.0; // float value 5.0

    vector<Query> queries1, queries2;
    queries1.push_back(Query::Select().EqualTo("$.field1", valueStringTrue));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueString));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueLong));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueDouble));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueFloat));

    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueStringTrue));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueString));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueLong));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueDouble));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueFloat));

    vector<string> valuesStringTrue = {"true"};
    vector<string> valuesString = {"abc"};
    vector<int64_t> valuesLong = {15};
    vector<double> valuesDouble = {10.5};
    vector<float> valuesFloat = {5.0};

    queries1.push_back(Query::Select().In("$.field1", valuesStringTrue));
    queries1.push_back(Query::Select().In("$.field1", valuesString));
    queries1.push_back(Query::Select().In("$.field1", valuesLong));
    queries1.push_back(Query::Select().In("$.field1", valuesDouble));
    queries1.push_back(Query::Select().In("$.field1", valuesFloat));

    queries2.push_back(Query::Select().NotIn("$.field1", valuesStringTrue));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesString));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesLong));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesDouble));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesFloat));

    vector<Entry> entries1;
    for (auto const &it : queries1) {
        result = (delegate->GetEntries(it, entries1) == NOT_FOUND) && result;
    }
    vector<Entry> expectEntry
        = {entries[INDEX_ZEROTH], entries[INDEX_FIRST], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    for (auto &it : queries2) {
        result = CheckSchemaQuery(delegate, it, expectEntry, SCHEMA_GOT_COUNT_4, DBStatus::OK) && result;
    }
    return result;
}

bool CheckSchemaBoolExist(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueStringZero = "0";
    int valueInt = 1; // int value 1
    Query query = Query::Select().EqualTo("$.field1", valueStringZero);
    vector<Entry> expectEntry = {entries[INDEX_FIRST]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK)) && result;
    query = Query::Select().EqualTo("$.field1", valueInt);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK)) && result;
    query = Query::Select().NotEqualTo("$.field1", valueStringZero);
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK)) && result;
    query = Query::Select().NotEqualTo("$.field1", valueInt);
    expectEntry = {entries[INDEX_FIRST]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK)) && result;

    vector<string> valuesStringZero = {"0"};
    vector<int> valuesInt = {1};
    query = Query::Select().In("$.field1", valuesStringZero);
    expectEntry = {entries[INDEX_FIRST]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK)) && result;
    query = Query::Select().In("$.field1", valuesInt);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK)) && result;

    query = Query::Select().NotIn("$.field1", valuesStringZero);
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK)) && result;
    query = Query::Select().NotIn("$.field1", valuesInt);
    expectEntry = {entries[INDEX_FIRST]};
    result = (CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK)) && result;

    return result;
}

/**
 * @tc.name: Query 003
 * @tc.desc: check valid schema of Bool type and construct valid query object and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query003, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":false,\"field2\":{\"field3\":false,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null ,\"field2\":{\"field3\":false,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_BOOL);

    /**
     * @tc.steps: step2. test the Query interface of EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo and call .
     * @tc.expected: step2. each interface called ok and GetEntries return right result.
     */
    Query query = Query::Select().EqualTo("$.field1", true);
    vector<Entry> expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    query = Query::Select().NotEqualTo("$.field1", true);
    expectEntry = {entries[INDEX_FIRST]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));

    vector<bool> scope = {true};
    query = Query::Select().In("$.field1", scope);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    query = Query::Select().NotIn("$.field1", scope);
    expectEntry = {entries[INDEX_FIRST]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));
    /**
     * @tc.steps: step3. query the illegal value of field and check with the GetEntries.
     * @tc.expected: step3. if the value is "0" or 1, can get valid query, or it will return invalid query
     *    and GetEntries will return NOT_FOUND.
     */
    EXPECT_TRUE(CheckSchemaBoolNotExist(delegate, entries));
    EXPECT_TRUE(CheckSchemaBoolExist(delegate, entries));

    /**
     * @tc.steps: step4. test IsNull interface where field = field3.
     * @tc.expected: step4. there are entries[0], entries[3], entries[4] in the query.
     */
    query = Query::Select().IsNull("$.field2.field3");
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

bool InvalidBoolFieldCheck(KvStoreNbDelegate *&delegate, string field, DBStatus status)
{
    bool result = true;
    Query query1 = Query::Select().EqualTo(field, true);
    vector<Entry> expectEntry = {};
    result = (CheckSchemaQuery(delegate, query1, expectEntry, 0, status)) && result;
    MST_LOG("Equal To result: %d", result);

    Query query2 = Query::Select().NotEqualTo(field, true);
    result = (CheckSchemaQuery(delegate, query2, expectEntry, 0, status)) && result;
    MST_LOG("NotEqual To result: %d", result);

    vector<bool> scope = {true};
    Query query3 = Query::Select().In(field, scope);
    result = (CheckSchemaQuery(delegate, query3, expectEntry, 0, status)) && result;
    MST_LOG("In result: %d", result);

    Query query4 = Query::Select().NotIn(field, scope);
    result = (CheckSchemaQuery(delegate, query4, expectEntry, 0, status)) && result;
    MST_LOG("Not in result: %d", result);

    return result;
}

/**
 * @tc.name: Query 004
 * @tc.desc: check valid schema of Bool type and construct invalid field and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query004, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":false,\"field2\":{\"field3\":false,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null ,\"field2\":{\"field3\":false,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":true,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_BOOL);

    /**
     * @tc.steps: step2. use notexist field5 to test EqualTo/NotEqualTo/In/NotIn and call GetEntries with Query.
     * @tc.expected: step2. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidBoolFieldCheck(delegate, "$.field5", DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step3. use not Leaf node field to test EqualTo/NotEqualTo/In/NotIn and call GetEntries to check.
     * @tc.expected: step3. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidBoolFieldCheck(delegate, "$.field2.field4", DBStatus::INVALID_QUERY_FIELD));
    EXPECT_TRUE(InvalidBoolFieldCheck(delegate, "$.field3.field2", DBStatus::INVALID_QUERY_FIELD));
    /**
     * @tc.steps: step4. use invalid format field to test EqualTo/NotEqualTo/In/NotIn and call GetEntries with Query.
     * @tc.expected: step4. GetEntries return INVALID_QUERY_FORMAT.
     */
    EXPECT_TRUE(InvalidBoolFieldCheck(delegate, ".field2.field3", DBStatus::INVALID_QUERY_FORMAT));
    EXPECT_TRUE(InvalidBoolFieldCheck(delegate, "$$field1", DBStatus::INVALID_QUERY_FORMAT));

    /**
     * @tc.steps: step5. use invalid format field to test
     *    GreaterThan/LessThan/GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step5. GetEntries return INVALID_QUERY_FORMAT.
     */
    Query query = Query::Select().GreaterThan("$.field1", "1");
    vector<Entry> expectEntry = {};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, 0, DBStatus::INVALID_QUERY_FORMAT));
    query = Query::Select().GreaterThanOrEqualTo("$.field1", "1");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, 0, DBStatus::INVALID_QUERY_FORMAT));
    query = Query::Select().LessThan("$.field1", "1");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, 0, DBStatus::INVALID_QUERY_FORMAT));
    query = Query::Select().LessThanOrEqualTo("$.field1", "1");
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, 0, DBStatus::INVALID_QUERY_FORMAT));
    query = Query::Select().OrderBy("$.field1", true);
    EXPECT_TRUE(CheckSchemaQuery(delegate, query, expectEntry, 0, DBStatus::INVALID_QUERY_FORMAT));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

bool CheckSchemaExceptionValue(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueStringTen = "10"; // string 10
    string valueString = "abc";
    float valueFloat = 10.0; // float 10.0
    bool valueBool = false; // boolean false
    vector<Query> queries1, queries2, queries3;
    queries1.push_back(Query::Select().EqualTo("$.field1", valueString));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueBool));

    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueString));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueBool));

    vector<string> valuesString = {"abc"};
    vector<bool> valuesBool = {false};
    vector<double> valuesDouble = {10.5};
    queries1.push_back(Query::Select().In("$.field1", valuesString));
    queries1.push_back(Query::Select().In("$.field1", valuesBool));
    queries1.push_back(Query::Select().In("$.field1", valuesDouble));

    queries2.push_back(Query::Select().NotIn("$.field1", valuesString));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesBool));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesDouble));

    queries1.push_back(Query::Select().GreaterThan("$.field1", valueString));
    queries3.push_back(Query::Select().GreaterThan("$.field1", valueBool));

    queries2.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueStringTen));
    queries2.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueFloat));
    queries1.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueString));
    queries3.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueBool));

    vector<Entry> expectEntry3 = {};
    Query query = Query::Select().LessThan("$.field1", valueStringTen);
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::NOT_FOUND) && result;
    query = Query::Select().LessThan("$.field1", valueFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::NOT_FOUND) && result;

    queries2.push_back(Query::Select().LessThan("$.field1", valueString));
    queries3.push_back(Query::Select().LessThan("$.field1", valueBool));

    queries2.push_back(Query::Select().LessThanOrEqualTo("$.field1", valueString));
    queries3.push_back(Query::Select().LessThanOrEqualTo("$.field1", valueBool));

    vector<Entry> entries1;
    for (auto const &it : queries1) {
        result = (delegate->GetEntries(it, entries1) == NOT_FOUND) && result;
    }
    vector<Entry> expectEntry2 =
        {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    for (auto &it : queries2) {
        result = CheckSchemaQuery(delegate, it, expectEntry2, SCHEMA_GOT_COUNT_4, DBStatus::OK) && result;
    }
    for (auto &it : queries3) {
        result = CheckSchemaQuery(delegate, it, expectEntry3, 0, DBStatus::INVALID_QUERY_FORMAT) && result;
    }
    return result;
}

bool CheckSchemaIntValue(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueStringTen = "10"; // string 10
    float valueFloat = 10.0; // float 10.0
    double valueDouble = 10.5; // double value 10.5
    Query query = Query::Select().EqualTo("$.field1", valueStringTen);
    vector<Entry> expectEntry1 = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().EqualTo("$.field1", valueFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().EqualTo("$.field1", valueDouble);
    vector<Entry> entries1;
    result = (delegate->GetEntries(query, entries1) == NOT_FOUND) && result;

    query = Query::Select().NotEqualTo("$.field1", valueStringTen);
    vector<Entry> expectEntry2 = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().NotEqualTo("$.field1", valueFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().GreaterThan("$.field1", valueStringTen);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().GreaterThan("$.field1", valueFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().LessThanOrEqualTo("$.field1", valueStringTen);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().LessThanOrEqualTo("$.field1", valueFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    vector<string> valuesStringTen = {"10"};
    vector<float> valuesFloat = {10.0};
    query = Query::Select().In("$.field1", valuesStringTen);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().In("$.field1", valuesFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().NotIn("$.field1", valuesStringTen);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().NotIn("$.field1", valuesFloat);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().GreaterThan("$.field1", valueDouble);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().GreaterThanOrEqualTo("$.field1", valueDouble);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().LessThan("$.field1", valueDouble);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().LessThanOrEqualTo("$.field1", valueDouble);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;

    query = Query::Select().NotEqualTo("$.field1", valueDouble);
    expectEntry2 = {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_4, DBStatus::OK) && result;

    return result;
}

/**
 * @tc.name: Query 005
 * @tc.desc: check valid schema of Int type and construct valid query object and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query005, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":15,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":10,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":20,\"field2\":{\"field3\":null,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_INT);

    /**
     * @tc.steps: step2. test the Query interface of EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo and check the return query with GetEntries.
     * @tc.expected: step2. each interface called ok and GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field1", 10);
    vector<Entry> expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query1, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query2 = Query::Select().NotEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query2, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    vector<int> scope = {10};
    Query query3 = Query::Select().In("$.field1", scope);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query3, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query4 = Query::Select().NotIn("$.field1", scope);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query4, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query5 = Query::Select().GreaterThan("$.field1", 10);
    EXPECT_TRUE(CheckSchemaQuery(delegate, query5, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query6 = Query::Select().LessThan("$.field1", 10);
    expectEntry = {};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query6, expectEntry, 0, DBStatus::NOT_FOUND));

    Query query7 = Query::Select().GreaterThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query7, expectEntry, SCHEMA_GOT_COUNT_4, DBStatus::OK));

    Query query8 = Query::Select().LessThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query8, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));
    /**
     * @tc.steps: step3. query the illegal value of field and check with the GetEntries.
     * @tc.expected: step3. if the value is "0" or 1, can get valid query, or it will return invalid query
     *    and GetEntries will return NOT_FOUND.
     */
    EXPECT_TRUE(CheckSchemaExceptionValue(delegate, entries));
    EXPECT_TRUE(CheckSchemaIntValue(delegate, entries));

    /**
     * @tc.steps: step4. test IsNull interface where field = field3.
     * @tc.expected: step4. create and put successfully.
     */
    Query query9 = Query::Select().IsNull("$.field2.field3");
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query9, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

template<typename T>
bool InvalidFieldCheck(KvStoreNbDelegate *&delegate, string field, T value, DBStatus status)
{
    bool result = true;
    Query query1 = Query::Select().EqualTo(field, value);
    vector<Entry> expectEntry = {};
    result = (CheckSchemaQuery(delegate, query1, expectEntry, 0, status)) && result;
    MST_LOG("Equal to result: %d", result);

    Query query2 = Query::Select().NotEqualTo(field, value);
    result = (CheckSchemaQuery(delegate, query2, expectEntry, 0, status)) && result;
    MST_LOG("NotEqual to result: %d", result);

    vector<T> scope = {value};
    Query query3 = Query::Select().In(field, scope);
    result = (CheckSchemaQuery(delegate, query3, expectEntry, 0, status)) && result;
    MST_LOG("In result: %d", result);

    Query query4 = Query::Select().NotIn(field, scope);
    result = (CheckSchemaQuery(delegate, query4, expectEntry, 0, status)) && result;
    MST_LOG("NotIn result: %d", result);

    Query query5 = Query::Select().GreaterThan(field, value);
    result = (CheckSchemaQuery(delegate, query5, expectEntry, 0, status)) && result;
    MST_LOG("GreaterThan result: %d", result);

    Query query6 = Query::Select().LessThan(field, value);
    result = (CheckSchemaQuery(delegate, query6, expectEntry, 0, status)) && result;
    MST_LOG("LessThan result: %d", result);

    Query query7 = Query::Select().GreaterThanOrEqualTo(field, value);
    result = (CheckSchemaQuery(delegate, query7, expectEntry, 0, status)) && result;
    MST_LOG("GreaterThanOrEqualTo result: %d", result);

    Query query8 = Query::Select().LessThanOrEqualTo(field, value);
    result = (CheckSchemaQuery(delegate, query8, expectEntry, 0, status)) && result;
    MST_LOG("LessThanOrEqualTo result: %d", result);

    Query query9 = Query::Select().OrderBy(field);
    result = (CheckSchemaQueryForOrderBy(delegate, query9, expectEntry, 0, status)) && result;
    MST_LOG("OrderBy result: %d", result);
    return result;
}

/**
 * @tc.name: Query 006
 * @tc.desc: check valid schema of Int type and construct invalid field and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query006, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":15 ,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":10 ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":20,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_INT);

    /**
     * @tc.steps: step2. use notexist field5 to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step2. GetEntries return INVALID_QUERY_FIELD.
     */
    int valueIntTen = 10;
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field5", valueIntTen, DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step3. use not Leaf node field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries to check.
     * @tc.expected: step3. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field2.field4", valueIntTen, DBStatus::INVALID_QUERY_FIELD));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field3.field2", valueIntTen, DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step4. use invalid format field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step4. GetEntries return INVALID_QUERY_FORMAT.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, ".field2.field3", valueIntTen, DBStatus::INVALID_QUERY_FORMAT));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$$field1", valueIntTen, DBStatus::INVALID_QUERY_FORMAT));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 007
 * @tc.desc: check valid schema of Long type and construct valid query object and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query007, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":18 ,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":-25 ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":20,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_LONG);

    /**
     * @tc.steps: step2. test the Query interface of EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo and check the return query with GetEntries.
     * @tc.expected: step2. each interface called ok and GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field1", 10);
    vector<Entry> expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query1, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query2 = Query::Select().NotEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query2, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    vector<int> scope = {10, -10};
    Query query3 = Query::Select().In("$.field1", scope);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query3, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query4 = Query::Select().NotIn("$.field1", scope);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query4, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query5 = Query::Select().GreaterThan("$.field1", 10);
    EXPECT_TRUE(CheckSchemaQuery(delegate, query5, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query6 = Query::Select().LessThan("$.field1", 10);
    expectEntry = {};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query6, expectEntry, 0, DBStatus::NOT_FOUND));

    Query query7 = Query::Select().GreaterThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query7, expectEntry, SCHEMA_GOT_COUNT_4, DBStatus::OK));

    Query query8 = Query::Select().LessThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query8, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));
    /**
     * @tc.steps: step3. query the exception value of field and check with the GetEntries.
     * @tc.expected: step3. if the value is "10" or 10.0, can get valid query, or it will return invalid query
     *    and GetEntries will return NOT_FOUND.
     */
    EXPECT_TRUE(CheckSchemaExceptionValue(delegate, entries));
    EXPECT_TRUE(CheckSchemaIntValue(delegate, entries));

    /**
     * @tc.steps: step4. test IsNull interface where field = field3.
     * @tc.expected: step4. create and put successfully.
     */
    Query query9 = Query::Select().IsNull("$.field2.field3");
    expectEntry = {entries[INDEX_ZEROTH], entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query9, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 008
 * @tc.desc: check valid schema of Long type and construct invalid field and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query008, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":18 ,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10,\"field2\":{\"field3\":-25 ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":20,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_LONG);

    /**
     * @tc.steps: step2. use notexist field5 to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step2. GetEntries return INVALID_QUERY_FIELD.
     */
    int64_t valueLongTen = 10;
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field5", valueLongTen, DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step3. use not Leaf node field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries to check.
     * @tc.expected: step3. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field2.field4", valueLongTen, DBStatus::INVALID_QUERY_FIELD));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field3.field2", valueLongTen, DBStatus::INVALID_QUERY_FIELD));
    /**
     * @tc.steps: step4. use invalid format field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step4. GetEntries return INVALID_QUERY_FORMAT.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, ".field2.field3", valueLongTen, DBStatus::INVALID_QUERY_FORMAT));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$$field1", valueLongTen, DBStatus::INVALID_QUERY_FORMAT));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

bool CheckSchemaDoubleExceptionValue(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueString = "abc";
    int64_t valueLong = 15; // long value 15
    vector<Query> queries1, queries2;
    queries1.push_back(Query::Select().EqualTo("$.field1", valueString));
    queries1.push_back(Query::Select().EqualTo("$.field1", valueLong));

    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueString));
    queries2.push_back(Query::Select().NotEqualTo("$.field1", valueLong));

    vector<string> valuesStringZeroPoint = {"0.0"};
    vector<bool> valuesBool = {false};
    vector<string> valuesString = {"abc"};
    vector<int64_t> valuesLong = {15};
    Query query = Query::Select().In("$.field1", valuesStringZeroPoint);
    vector<Entry> expectEntry1 = {entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_1, DBStatus::OK) && result;
    query = Query::Select().In("$.field1", valuesBool);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_1, DBStatus::OK) && result;
    queries1.push_back(Query::Select().In("$.field1", valuesString));
    queries1.push_back(Query::Select().In("$.field1", valuesLong));

    query = Query::Select().NotIn("$.field1", valuesStringZeroPoint);
    vector<Entry> expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_SECOND], entries[INDEX_THIRD]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_3, DBStatus::OK) && result;
    query = Query::Select().NotIn("$.field1", valuesBool);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_3, DBStatus::OK) && result;
    queries2.push_back(Query::Select().NotIn("$.field1", valuesString));
    queries2.push_back(Query::Select().NotIn("$.field1", valuesLong));

    queries1.push_back(Query::Select().GreaterThan("$.field1", valueString));
    queries1.push_back(Query::Select().GreaterThan("$.field1", valueLong));

    queries1.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueString));
    queries1.push_back(Query::Select().GreaterThanOrEqualTo("$.field1", valueLong));

    queries2.push_back(Query::Select().LessThan("$.field1", valueString));
    queries2.push_back(Query::Select().LessThan("$.field1", valueLong));

    queries2.push_back(Query::Select().LessThanOrEqualTo("$.field1", valueString));
    queries2.push_back(Query::Select().LessThanOrEqualTo("$.field1", valueLong));

    vector<Entry> entries1;
    for (auto const &it : queries1) {
        result = (delegate->GetEntries(it, entries1) == NOT_FOUND) && result;
    }
    expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    for (auto &it : queries2) {
        result = CheckSchemaQuery(delegate, it, expectEntry2, SCHEMA_GOT_COUNT_4, DBStatus::OK) && result;
    }
    return result;
}

bool CheckSchemaDoubleValue(KvStoreNbDelegate *&delegate, vector<Entry> &entries)
{
    bool result = true;
    string valueStringZeroPoint = "0.0"; // string 0.0
    bool valueBool = false; // boolean false
    Query query = Query::Select().EqualTo("$.field1", valueStringZeroPoint);
    vector<Entry> expectEntry1 = {entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_1, DBStatus::OK) && result;
    query = Query::Select().EqualTo("$.field1", valueBool);
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_1, DBStatus::OK) && result;

    query = Query::Select().NotEqualTo("$.field1", valueStringZeroPoint);
    vector<Entry> expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_SECOND], entries[INDEX_THIRD]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_3, DBStatus::OK) && result;
    query = Query::Select().NotEqualTo("$.field1", valueBool);
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_3, DBStatus::OK) && result;

    query = Query::Select().GreaterThan("$.field1", valueStringZeroPoint);
    vector<Entry> expectEntry3 = {entries[INDEX_FIRST], entries[INDEX_THIRD]};
    result = CheckSchemaQuery(delegate, query, expectEntry3, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().GreaterThan("$.field1", valueBool);
    expectEntry3 = {};
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::INVALID_QUERY_FORMAT) && result;

    query = Query::Select().GreaterThanOrEqualTo("$.field1", valueStringZeroPoint);
    expectEntry2 = {entries[INDEX_FIRST], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_3, DBStatus::OK) && result;
    query = Query::Select().GreaterThanOrEqualTo("$.field1", valueBool);
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::INVALID_QUERY_FORMAT) && result;

    query = Query::Select().LessThan("$.field1", valueStringZeroPoint);
    expectEntry2 = {entries[INDEX_SECOND]};
    result = CheckSchemaQuery(delegate, query, expectEntry2, SCHEMA_GOT_COUNT_1, DBStatus::OK) && result;
    query = Query::Select().LessThan("$.field1", valueBool);
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::INVALID_QUERY_FORMAT) && result;

    query = Query::Select().LessThanOrEqualTo("$.field1", valueStringZeroPoint);
    expectEntry1 = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    result = CheckSchemaQuery(delegate, query, expectEntry1, SCHEMA_GOT_COUNT_2, DBStatus::OK) && result;
    query = Query::Select().LessThanOrEqualTo("$.field1", valueBool);
    result = CheckSchemaQuery(delegate, query, expectEntry3, 0, DBStatus::INVALID_QUERY_FORMAT) && result;

    return result;
}
/**
 * @tc.name: Query 009
 * @tc.desc: check valid schema of Double type and construct valid query object and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query009, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10.0,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10.0,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":-10.0 ,\"field2\":{\"field3\":30,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10.5,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":-0.0,\"field2\":{\"field3\":12.5 ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_DOUBLE);

    /**
     * @tc.steps: step2. test the Query interface of EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo and check the return query with GetEntries.
     * @tc.expected: step2. each interface called ok and GetEntries return right result.
     */
    Query query1 = Query::Select().EqualTo("$.field1", 10);
    vector<Entry> expectEntry = {entries[INDEX_FIRST]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query1, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));

    Query query2 = Query::Select().NotEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query2, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));

    vector<int> scope = {10, -10};
    Query query3 = Query::Select().In("$.field1", scope);
    expectEntry = {entries[INDEX_FIRST], entries[INDEX_SECOND]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query3, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query4 = Query::Select().NotIn("$.field1", scope);
    expectEntry = {entries[INDEX_THIRD], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query4, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query5 = Query::Select().GreaterThan("$.field1", 10);
    expectEntry = {entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query5, expectEntry, SCHEMA_GOT_COUNT_1, DBStatus::OK));

    Query query6 = Query::Select().LessThan("$.field1", 10);
    expectEntry = {entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query6, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query7 = Query::Select().GreaterThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_FIRST], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query7, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    Query query8 = Query::Select().LessThanOrEqualTo("$.field1", 10);
    expectEntry = {entries[INDEX_FIRST], entries[INDEX_SECOND], entries[INDEX_FORTH]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query8, expectEntry, SCHEMA_GOT_COUNT_3, DBStatus::OK));
    /**
     * @tc.steps: step3. query the exception value of field and check with the GetEntries.
     * @tc.expected: step3. if the value is "0.0" or false, can get valid query, or it will return invalid query
     *    and GetEntries will return NOT_FOUND.
     */
    EXPECT_TRUE(CheckSchemaDoubleExceptionValue(delegate, entries));
    EXPECT_TRUE(CheckSchemaDoubleValue(delegate, entries));

    /**
     * @tc.steps: step4. test IsNull interface where field = field3.
     * @tc.expected: step4. create and put successfully.
     */
    Query query9 = Query::Select().IsNull("$.field2.field3");
    expectEntry = {entries[INDEX_FIRST], entries[INDEX_THIRD]};
    EXPECT_TRUE(CheckSchemaQuery(delegate, query9, expectEntry, SCHEMA_GOT_COUNT_2, DBStatus::OK));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 010
 * @tc.desc: check valid schema of Double type and construct invalid field and traverse all function of Query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query010, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;

    schemasValue.push_back("{\"field1\":null,\"field2\":{\"field3\":10.0,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10.0,\"field2\":{\"field3\":null,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":-10.0 ,\"field2\":{\"field3\":30,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":10.5,\"field2\":{\"field3\":null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":-0.0,\"field2\":{\"field3\":12.5 ,\"field4\":[]}}");
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_DOUBLE);

    /**
     * @tc.steps: step2. use notexist field5 to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step2. GetEntries return INVALID_QUERY_FIELD.
     */
    double valueDoubleTen = 10.0;
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field5", valueDoubleTen, DBStatus::INVALID_QUERY_FIELD));

    /**
     * @tc.steps: step3. use not Leaf node field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries to check.
     * @tc.expected: step3. GetEntries return INVALID_QUERY_FIELD.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field2.field4", valueDoubleTen, DBStatus::INVALID_QUERY_FIELD));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$.field3.field2", valueDoubleTen, DBStatus::INVALID_QUERY_FIELD));
    /**
     * @tc.steps: step4. use invalid format field to test EqualTo/NotEqualTo/In/NotIn/GreaterThan/LessThan/
     *    GreaterThanOrEqualTo/LessThanOrEqualTo/OrderBy and call GetEntries with Query.
     * @tc.expected: step4. GetEntries return INVALID_QUERY_FORMAT.
     */
    EXPECT_TRUE(InvalidFieldCheck(delegate, ".field2.field3", valueDoubleTen, DBStatus::INVALID_QUERY_FORMAT));
    EXPECT_TRUE(InvalidFieldCheck(delegate, "$$field1", valueDoubleTen, DBStatus::INVALID_QUERY_FORMAT));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

vector<string> GenerateCombinationSchemaValue()
{
    vector<string> schemasValue;
    schemasValue.push_back("{\"field1\":\"abc\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":9,"
        "\"field6\":{\"field7\":1000,\"field8\":12}}}}"); // value1

    schemasValue.push_back("{\"field1\":\"ab123\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":88,"
        "\"field6\":{\"field7\":-100,\"field8\":-99}}}}"); // value2

    schemasValue.push_back("{\"field1\":\"abfxy\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":10,"
        "\"field6\":{\"field7\":0,\"field8\":38}}}}"); // value3

    schemasValue.push_back("{\"field1\":\"ab789\",\"field2\":{\"field3\":false,\"field4\":{\"field5\":999,"
        "\"field6\":{\"field7\":50,\"field8\":15.8}}}}"); // value4

    schemasValue.push_back("{\"field1\":\"ab000\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":33,"
        "\"field6\":{\"field7\":30,\"field8\":149}}}}"); // value5

    schemasValue.push_back("{\"field1\":\"abxxx\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":12,"
        "\"field6\":{\"field7\":120,\"field8\":-79}}}}"); // value6

    schemasValue.push_back("{\"field1\":\"ab\",\"field2\":{\"field3\":true,\"field4\":{\"field5\":20,"
        "\"field6\":{\"field7\":82,\"field8\":150.999}}}}"); // value7

    return schemasValue;
}

/**
 * @tc.name: Query 011
 * @tc.desc: verify that GetCount interface can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query011, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create common db and close it, and then open it with schema mode and put some schema data to
     *    the new mode db;
     * @tc.expected: step1. operate successfully.
     */
    Option option = g_option;
    option.isMemoryDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;

    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of LessThan on field5 = 100,
     *    and NotEqualTo on field7 = -100 and NotLike on field1 = "%c".
     * @tc.expected: step2. construct successfully.
     */
    Query query1 = Query::Select().LessThan("$.field2.field4.field5", 100).And().
        NotEqualTo("$.field2.field4.field6.field7", -100).And().NotLike("$.field1", "%c");

    /**
     * @tc.steps: step3. use GetCount interface to check the count of records in the query object.
     * @tc.expected: step3. the count is 4.
     */
    int count = 0;
    EXPECT_TRUE(delegate->GetCount(query1, count) == DBStatus::OK);
    EXPECT_EQ(count, SCHEMA_GOT_COUNT_4);

    /**
     * @tc.steps: step4. construct query object that test combination interface of LessThan on field5 = 100,
     *    and NotEqualTo on field7 = -100 or NotLike on field1 = "%c".
     * @tc.expected: step4. construct successfully.
     */
    Query query2 = Query::Select().LessThan("$.field2.field4.field5", 100).And().
        NotEqualTo("$.field2.field4.field6.field7", -100).Or().NotLike("$.field1", "%c");

    /**
     * @tc.steps: step5. use GetCount interface to check the count of records in the query object.
     * @tc.expected: step5. the count is 7.
     */
    EXPECT_TRUE(delegate->GetCount(query2, count) == DBStatus::OK);
    EXPECT_EQ(count, (SCHEMA_GOT_COUNT_4 + SCHEMA_GOT_COUNT_3));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 012
 * @tc.desc: verify that OrderBy and Limit interface and the offset of limit is smaller than the count of the records
 *    got can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query012, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of GreaterThanOrEqualTo on
     *    field7 = 0, and EqualTo on field3 = true and then OrderBy on field5 by asec when get 3 records begin from 1.
     * @tc.expected: step2. construct successfully.
     */
    Query query = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field6.field7", 0).And().
        EqualTo("$.field2.field3", true).OrderBy("$.field2.field4.field5", false).Limit(3, 1);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query object.
     * @tc.expected: step3. the count is 4.
     */
    KvStoreResultSet *resultSet = nullptr;
    Entry entry;
    EXPECT_EQ(delegate->GetEntries(query, resultSet), DBStatus::OK);
    EXPECT_EQ(resultSet->GetCount(), SCHEMA_GOT_COUNT_3);
    EXPECT_EQ(resultSet->GetPosition(), CURSOR_POSITION_NEGATIVE1);
    EXPECT_EQ(resultSet->MoveToPrevious(), false);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SIXTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_SIXTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FIFTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FIFTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SECOND].key);
    EXPECT_EQ(entry.value, entries[INDEX_SECOND].value);
    EXPECT_EQ(resultSet->IsLast(), true);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps: step4. use GetEntries interface to check the records in the query object.
     * @tc.expected: step4. the result and the order in the vector is: entries[6], entries[5], entries[2].
     */
    vector<Entry> entriesGot, entriesExpect;
    EXPECT_TRUE(delegate->GetEntries(query, entriesGot) == DBStatus::OK);
    entriesExpect = {entries[INDEX_SIXTH], entries[INDEX_FIFTH], entries[INDEX_SECOND]};
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 013
 * @tc.desc: verify that OrderBy and Limit interface but the offset of limit is greater than the count of the records
 *     got can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query013, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of GreaterThanOrEqualTo on
     *    field7 = 0, and EqualTo on field3 = true and then OrderBy on field5 by asec when get 5 records begin from 1.
     * @tc.expected: step2. construct successfully.
     */
    Query query = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field6.field7", 0).And().
        EqualTo("$.field2.field3", true).OrderBy("$.field2.field4.field5", false).Limit(5, 1);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query object.
     * @tc.expected: step3. the count is 4.
     */
    KvStoreResultSet *resultSet = nullptr;
    Entry entry;
    EXPECT_EQ(delegate->GetEntries(query, resultSet), DBStatus::OK);
    EXPECT_EQ(resultSet->GetCount(), FOUR_RECORDS);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SIXTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_SIXTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FIFTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FIFTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SECOND].key);
    EXPECT_EQ(entry.value, entries[INDEX_SECOND].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_ZEROTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_ZEROTH].value);
    EXPECT_EQ(resultSet->IsLast(), true);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps: step4. use GetEntries interface to check the records in the query object.
     * @tc.expected: step4. the result and the order in the vector is: entries[6], entries[5], entries[2], entries[0].
     */
    vector<Entry> entriesGot, entriesExpect;
    EXPECT_TRUE(delegate->GetEntries(query, entriesGot) == DBStatus::OK);
    entriesExpect = {entries[INDEX_SIXTH], entries[INDEX_FIFTH], entries[INDEX_SECOND], entries[INDEX_ZEROTH]};
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 014
 * @tc.desc: verify that OrderBy and Limit interface but the offset of limit is nagative
 *    can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query014, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of LessThanOrEqualTo on field7 = 1000,
     *    and EqualTo on field3 = true and Like field1 = ab*, and then OrderBy on field8 by asec
     *    when get 10 records begin from -1.
     * @tc.expected: step2. construct successfully.
     */
    Query query = Query::Select().LessThanOrEqualTo("$.field2.field4.field6.field7", 1000).Or().
        EqualTo("$.field2.field3", true).And().Like("$.field1", "ab%").
        OrderBy("$.field2.field4.field6.field8", true).Limit(10, -1);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query object.
     * @tc.expected: step3. the count is 6.
     */
    KvStoreResultSet *resultSet = nullptr;
    Entry entry;
    EXPECT_EQ(delegate->GetEntries(query, resultSet), DBStatus::OK);
    EXPECT_EQ(resultSet->GetCount(), 7); // 7 records
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FIRST].key);
    EXPECT_EQ(entry.value, entries[INDEX_FIRST].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FIFTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FIFTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_ZEROTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_ZEROTH].value);
    EXPECT_TRUE(resultSet->Move(CURSOR_OFFSET_2));
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SECOND].key);
    EXPECT_EQ(entry.value, entries[INDEX_SECOND].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FORTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FORTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_SIXTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_SIXTH].value);
    EXPECT_EQ(resultSet->IsLast(), true);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps: step4. use GetEntries interface to check the records in the query object.
     * @tc.expected: step4. the result and the order in the vector is: entries[1], entries[5],
     *    entries[0], entries[2], entries[4], entries[6].
     */
    vector<Entry> entriesGot, entriesExpect;
    EXPECT_TRUE(delegate->GetEntries(query, entriesGot) == OK);
    entriesExpect = {entries[INDEX_FIRST], entries[INDEX_FIFTH], entries[INDEX_ZEROTH], entries[INDEX_THIRD],
        entries[INDEX_SECOND], entries[INDEX_FORTH], entries[INDEX_SIXTH]};
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 015
 * @tc.desc: verify that OrderBy and Limit interface but the offset of limit is out of the range of the query result
 *    can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query015, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of GreaterThanOrEqualTo on
     *    field5 = 10, and NotEqualTo on field3 = true and then OrderBy on field5 by asec
     *    when get 10 records begin from 6.
     * @tc.expected: step2. construct successfully.
     */
    vector<double> doubleRange = {150.999};
    Query query = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().EqualTo("$.field2.field3",
        true).And().NotIn("$.field2.field4.field6.field8", doubleRange).Limit(10, 6);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query object.
     * @tc.expected: step3. the count is 0.
     */
    KvStoreResultSet *resultSet = nullptr;
    Entry entry;
    EXPECT_EQ(delegate->GetEntries(query, resultSet), DBStatus::OK);
    EXPECT_EQ(resultSet->GetCount(), NO_RECORD);
    EXPECT_EQ(resultSet->GetPosition(), CURSOR_POSITION_NEGATIVE1);
    EXPECT_EQ(resultSet->IsLast(), false);
    EXPECT_EQ(resultSet->IsAfterLast(), true);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps: step4. use GetEntries interface to check the records in the query object.
     * @tc.expected: step4. no records in the query.
     */
    vector<Entry> entriesGot;
    EXPECT_TRUE(delegate->GetEntries(query, entriesGot) == NOT_FOUND);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 016
 * @tc.desc: verify that OrderBy and Limit interface but the limit is not only greater than the result of the
 *    query result, but also the offset of the limit is out of the range of the query result
 *    can return right value of the valid query object.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query016, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct query object that test combination interface of Like on field1 = ab*,
     *    GeaterThan on field5 = 10, and EqualTo on field3 = true and when field in {0, 30, 50, 120, 1000},
     *    then OrderBy on field8 by asec when get 10 records begin from -2.
     * @tc.expected: step2. construct successfully.
     */
    vector<int64_t> longRange = {0, 30, 50, 120, 1000};
    Query query = Query::Select().Like("$.field1", "abc").Or().GreaterThan("$.field2.field4.field5", 10).And().
        EqualTo("$.field2.field3", true).And().In("$.field2.field4.field6.field7", longRange).
        OrderBy("$.field2.field4.field6.field8", true).Limit(10, -2);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query object.
     * @tc.expected: step3. the count is 3.
     */
    KvStoreResultSet *resultSet = nullptr;
    Entry entry;
    EXPECT_EQ(delegate->GetEntries(query, resultSet), DBStatus::OK);
    EXPECT_EQ(resultSet->GetCount(), SCHEMA_GOT_COUNT_3);
    EXPECT_EQ(resultSet->GetPosition(), CURSOR_POSITION_NEGATIVE1);
    EXPECT_EQ(resultSet->GetEntry(entry), NOT_FOUND);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FIFTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FIFTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_ZEROTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_ZEROTH].value);
    EXPECT_EQ(resultSet->MoveToNext(), true);
    EXPECT_EQ(resultSet->GetEntry(entry), OK);
    EXPECT_EQ(entry.key, entries[INDEX_FORTH].key);
    EXPECT_EQ(entry.value, entries[INDEX_FORTH].value);
    EXPECT_EQ(resultSet->IsLast(), true);
    EXPECT_EQ(resultSet->MoveToNext(), false);
    EXPECT_EQ(resultSet->IsAfterLast(), true);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps: step4. use GetEntries interface to check the records in the query object.
     * @tc.expected: step4. the result and the order in the vector is: entries[1], entries[5],
     *    entries[0], entries[2], entries[4], entries[6].
     */
    vector<Entry> entriesGot, entriesExpect;
    EXPECT_TRUE(delegate->GetEntries(query, entriesGot) == OK);
    entriesExpect = {entries[INDEX_FIFTH], entries[INDEX_ZEROTH], entries[INDEX_FORTH]};
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 017
 * @tc.desc: verify that common db can not support schema query.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query017, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    Option option = g_option;
    option.isMemoryDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    vector<Key> keys = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7};
    for (unsigned long index = 0; index < schemasValue.size(); index++) {
        Value value(schemasValue[index].begin(), schemasValue[index].end());
        entries.push_back({keys[index], value});
    }

    for (unsigned long index = 0; index < schemasValue.size(); index++) {
        EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, entries[index].key, entries[index].value), OK);
    }

    /**
     * @tc.steps: step2. construct query1 object that test combination interface of Like on field1 = ab*,
     *    GeaterThan on field5 = 10, and EqualTo on field3 = true and when field in {0, 30, 50, 120, 1000},
     *    then OrderBy on field8 by asec when get 10 records begin from -2,
     *    construct query2 object the same as query1 but has not OrderBy and Limit.
     * @tc.expected: step2. construct successfully.
     */
    vector<int64_t> longRange = {0, 30, 50, 120, 1000};
    Query query1 = Query::Select().Like("$.field1", "ab*").Or().GreaterThan("$.field2.field4.field5", 10).And().
        EqualTo("$.field2.field3", true).And().In("$.field2.field4.field6.field7", longRange).
        OrderBy("$.field2.field4.field6.field8", true).Limit(10, -2);
    Query query2 = Query::Select().Like("$.field1", "ab*").Or().GreaterThan("$.field2.field4.field5", 10).And().
        EqualTo("$.field2.field3", true).And().In("$.field2.field4.field6.field7", longRange);

    /**
     * @tc.steps: step3. use GetEntries interface to get the resultSet in the query1 and query2 objects.
     * @tc.expected: step3. both of them returns NOT_SUPPORT.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(delegate->GetEntries(query1, resultSet), DBStatus::NOT_SUPPORT);
    EXPECT_EQ(delegate->GetEntries(query2, resultSet), DBStatus::NOT_SUPPORT);

    /**
     * @tc.steps: step4. use GetEntries interface to get the records in the query1 and query2 objects.
     * @tc.expected: step4. both of them returns NOT_SUPPORT.
     */
    vector<Entry> entriesGot;
    EXPECT_TRUE(delegate->GetEntries(query1, entriesGot) == DBStatus::NOT_SUPPORT);
    EXPECT_TRUE(delegate->GetEntries(query2, entriesGot) == DBStatus::NOT_SUPPORT);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 018
 * @tc.desc: verify that constructor illegal query object and returns INVALID_QUERY_FORMAT
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query018, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct illegal query object such as:
     *    query1: which didn't combine with And() or Or().
     *    query2: OrderBy is before other condition.
     *    query3: Limit is before other condition.
     *    query4: OrderBy is after Limit.
     * @tc.expected: step2. construct successfully.
     */
    // LessThan on where field8 = 100
    Query query1 = Query::Select().LessThan("$.field2.field4.field6.field8", 100).NotLike("$.field1", "%c");
    Query query2 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field6.field7", 0).
        OrderBy("$.field2.field4.field5", true).And().EqualTo("$.field2.field3", true);
    vector<double> doubleRange = {150.999};
    // field5 = 10, get 10 records begin from 6th.
    Query query3 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field5", 10).And().EqualTo("$.field2.field3",
         true).Limit(10, 6).And().NotIn("$.field2.field4.field6.field8", doubleRange);
    // field7 = 0, get 3 records begin from 1
    Query query4 = Query::Select().GreaterThanOrEqualTo("$.field2.field4.field6.field7", 0).And().
        EqualTo("$.field2.field3", true).Limit(3, 1).OrderBy("$.field2.field4.field5", true);

    /**
     * @tc.steps: step3. use GetEntries interface to to get the resultSet in the 4 query objects.
     * @tc.expected: step3. all of them return INVALID_QUERY_FORMAT.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(delegate->GetEntries(query1, resultSet), DBStatus::INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query2, resultSet), DBStatus::INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query3, resultSet), DBStatus::INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query4, resultSet), DBStatus::INVALID_QUERY_FORMAT);

    /**
     * @tc.steps: step4. use GetEntries interface get the records in the query objects.
     * @tc.expected: step4. all of them return INVALID_QUERY_FORMAT.
     */
    vector<Entry> entriesGot;
    EXPECT_EQ(delegate->GetEntries(query1, entriesGot), INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query2, entriesGot), INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query3, entriesGot), INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetEntries(query4, entriesGot), INVALID_QUERY_FORMAT);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

/**
 * @tc.name: Query 019
 * @tc.desc: verify that GetCount interface of KvStoreNbDelegate class can't support OrderBy and
 *    Limit interface in query
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query019, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db of schema mode and put some schema data the db;
     * @tc.expected: step1. create and put successfully.
     */
    vector<Entry> entries;
    vector<string> schemasValue = GenerateCombinationSchemaValue();
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_COMBINATION_DEFINE);

    /**
     * @tc.steps: step2. construct valid query object such as:
     *    query1: EqualTo on field3 = true and Order field5 by asec.
     *    query2: EqualTo on field3 = true and Limit(5, 1).
     * @tc.expected: step2. construct successfully.
     */
    Query query1 = Query::Select().EqualTo("$.field2.field3", true).OrderBy("$.field2.field4.field5", true);
    Query query2 = Query::Select().EqualTo("$.field2.field3", true).Limit(5, 1); // get 5 records begin from 1

    /**
     * @tc.steps: step3. use GetCount interface of KvStoreNbDelegate class to check the count of query objects.
     * @tc.expected: step3. all of them return INVALID_QUERY_FORMAT.
     */
    int count = 0;
    EXPECT_EQ(delegate->GetCount(query1, count), DBStatus::INVALID_QUERY_FORMAT);
    EXPECT_EQ(delegate->GetCount(query2, count), DBStatus::INVALID_QUERY_FORMAT);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

void MakeSchemaValues(vector<string> &schemasValue)
{
    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\":\"fxy\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"abc\",\"field2\":{\"field3\":\"fxz\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\": null ,\"field2\":{\"field3\":\"fxw\",\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"bxz\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
    schemasValue.push_back("{\"field1\":\"TRUE\",\"field2\":{\"field3\": null ,\"field4\":[]}}");
}
/**
 * @tc.name: Query 020
 * @tc.desc: veriry query executed normal after delete records.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query020, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;
    MakeSchemaValues(schemasValue);
    /**
     * @tc.steps: step1. create schema db and put 5 entries which has valid schema constructor
     *    and has the value given to db.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_STRING);

    /**
     * @tc.steps: step2. call GetEntries with query=Select().NotIn(field1,{}) or query=Select().In(field1,{}) or
     *     query=Select().Like(field1,{}), and check the result with GetEntries.
     * @tc.expected: step2. return 4 entries if use NotIn, return NOT_FOUND if use In and Like.
     */
    vector<string> scope;
    vector<Entry> entriesResult;
    Query query1 = Query::Select().NotIn("$.field1", scope);
    Query query2 = Query::Select().In("$.field1", scope);
    Query query3 = Query::Select().Like("$.field2.field3", std::string());
    Query query4 = Query::Select().NotLike("$.field2.field3", std::string());
    EXPECT_EQ(delegate->GetEntries(query1, entriesResult), OK);
    EXPECT_EQ(delegate->GetEntries(query2, entriesResult), NOT_FOUND);
    EXPECT_EQ(delegate->GetEntries(query3, entriesResult), NOT_FOUND);
    EXPECT_EQ(delegate->GetEntries(query4, entriesResult), OK);

    /**
     * @tc.steps: step3. call GetEntries with query=Select().Like(field1,scope) that scope.size()=50000B, 50001B,
     *     4M and check the result with GetEntries.
     * @tc.expected: step3. return error code correspondingly.
     */
    string queryValidStr(LIKE_AND_NOTLIKE_MAX_LENGTH, 'a');
    Query queryLike = Query::Select().Like("$.field2.field3", queryValidStr);
    EXPECT_EQ(delegate->GetEntries(queryLike, entriesResult), NOT_FOUND);
    Query queryNotLike = Query::Select().NotLike("$.field2.field3", queryValidStr);
    EXPECT_EQ(delegate->GetEntries(queryNotLike, entriesResult), OK);
    vector<string> queryOvermaxStr;
    string queryInvalidStr1(LIKE_AND_NOTLIKE_MAX_LENGTH + 1, 'b');
    queryOvermaxStr.push_back(queryInvalidStr1);
    string queryInvalidStr2(FOUR_M_LONG_STRING, 'c');
    queryOvermaxStr.push_back(queryInvalidStr2);
    for (auto const &it : queryOvermaxStr) {
        Query queryLike = Query::Select().Like("$.field2.field3", it);
        EXPECT_EQ(delegate->GetEntries(queryLike, entriesResult), OVER_MAX_LIMITS);
        Query queryNotLike = Query::Select().NotLike("$.field2.field3", it);
        EXPECT_EQ(delegate->GetEntries(queryNotLike, entriesResult), OVER_MAX_LIMITS);
    }

    /**
     * @tc.steps: step4. call GetEntries with query=Select().NotIn(field1,scope) or query=Select().In(field1,scope)
     *     that scope.size()= 128B, 129B and check the result with GetEntries.
     * @tc.expected: step4. return error code correspondingly.
     */
    vector<string> scopeValid;
    scopeValid.assign(IN_AND_NOTIN_MAX_LENGTH, "d");
    Query query5 = Query::Select().NotIn("$.field1", scopeValid);
    Query query6 = Query::Select().In("$.field1", scopeValid);
    EXPECT_EQ(delegate->GetEntries(query5, entriesResult), OK);
    EXPECT_EQ(delegate->GetEntries(query6, entriesResult), NOT_FOUND);

    vector<string> scopeInValid;
    scopeInValid.assign(IN_AND_NOTIN_MAX_LENGTH + 1, "e");
    Query query7 = Query::Select().NotIn("$.field1", scopeInValid);
    Query query8 = Query::Select().In("$.field1", scopeInValid);
    EXPECT_EQ(delegate->GetEntries(query7, entriesResult), OVER_MAX_LIMITS);
    EXPECT_EQ(delegate->GetEntries(query8, entriesResult), OVER_MAX_LIMITS);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}

void MakeTenSchemaValues(vector<string> &schemasValue)
{
    for (int recordIndex = 0; recordIndex < TEN_RECORDS; ++recordIndex) {
        string val = "{\"field1\":" + string("\"") + string(1, 'a' + recordIndex) + "\"" +
            ",\"field2\":{\"field3\":\"fxy\",\"field4\":[]}}";
        schemasValue.push_back(val);
    }
}
/**
 * @tc.name: Query 021
 * @tc.desc: verify the record num of In query after delete several records.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbPredicateQueryTest, Query021, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    vector<Entry> entries;
    vector<string> schemasValue;
    MakeTenSchemaValues(schemasValue);
    /**
     * @tc.steps: step1. create schema db and put 10 records with valid schema.
     * @tc.expected: step1. create and put successfully.
     */
    PrepareSchemaDBAndData(delegate, manager, schemasValue, entries, VALID_DEFINE_STRING);

    /**
     * @tc.steps: step2. call GetEntries with query=Select().GreaterThanOrEqualTo("$.field1",{}) or
     *  query=Select().EqualTo("$.field3", "fxy").
     * @tc.expected: step2. return 5 records.
     */
    for (int delRecordNum = ONE_RECORD; delRecordNum <= FIVE_RECORDS; delRecordNum++) {
        Entry entry = entries.back();
        EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, entry.key), OK);
        entries.pop_back();
    }
    vector<Entry> entriesResult;
    KvStoreResultSet *resultSet = nullptr;
    Query query = Query::Select().GreaterThanOrEqualTo("$.field1", "f").Or().EqualTo("$.field2.field3", "fxy");
    EXPECT_EQ(delegate->GetEntries(query, entriesResult), OK);
    EXPECT_EQ(delegate->GetEntries(query, resultSet), OK);
    EXPECT_TRUE(entriesResult.size() == FIVE_RECORDS);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_predicateOption.isMemoryDb));
}
}
#endif