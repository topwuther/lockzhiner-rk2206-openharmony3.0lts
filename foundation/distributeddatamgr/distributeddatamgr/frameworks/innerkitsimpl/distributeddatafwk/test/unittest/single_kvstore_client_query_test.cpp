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

#define LOG_TAG "SingleKvStoreClientQueryTest"

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "distributed_kv_data_manager.h"
#include "types.h"
#include "log_print.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class SingleKvStoreClientQueryTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();

    static std::unique_ptr<SingleKvStore> singleKvStorePtr;
    static Status statusGetKvStore;
};

const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_SKIPSIZE\":0,"
        "\"SCHEMA_DEFINE\":{"
            "\"name\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.name\"]}";
std::unique_ptr<SingleKvStore> SingleKvStoreClientQueryTest::singleKvStorePtr = nullptr;
Status SingleKvStoreClientQueryTest::statusGetKvStore = Status::ERROR;

void SingleKvStoreClientQueryTest::SetUpTestCase(void)
{}

void SingleKvStoreClientQueryTest::TearDownTestCase(void)
{}

void SingleKvStoreClientQueryTest::SetUp(void)
{}

void SingleKvStoreClientQueryTest::TearDown(void)
{}

/**
* @tc.name: TestQueryC001
* @tc.desc: Query reset.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC001, TestSize.Level1)
{
    ZLOGD("TestQueryC001 start");
    DataQuery query;
    EXPECT_TRUE(query.ToString().length() == 0);
    std::string str = "test value";
    query.EqualTo("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    EXPECT_TRUE(query.ToString().length() == 0);
    ZLOGD("TestQueryC001 end");
}

/**
* @tc.name: TestQueryC002
* @tc.desc: Query equalTo.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC002, TestSize.Level1)
{
    ZLOGD("TestQueryC002 start");
    DataQuery query;
    query.EqualTo("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.EqualTo("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.EqualTo("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.EqualTo("$.test_field_name", false);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "";
    query.EqualTo("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC002 end");
}

/**
* @tc.name: TestQueryC003
* @tc.desc: Query notEqualTo.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC003, TestSize.Level1)
{
    ZLOGD("TestQueryC003 start");
    DataQuery query;
    query.NotEqualTo("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.NotEqualTo("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.NotEqualTo("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.NotEqualTo("$.test_field_name", false);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "test value";
    query.NotEqualTo("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC003 end");
}

/**
* @tc.name: TestQueryC004
* @tc.desc: Query greaterThan.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC004, TestSize.Level1)
{
    ZLOGD("TestQueryC004 start");
    DataQuery query;
    query.GreaterThan("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.GreaterThan("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.GreaterThan("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "test value";
    query.GreaterThan("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC004 end");
}

/**
* @tc.name: TestQueryC005
* @tc.desc: Query lessThan.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC005, TestSize.Level1)
{
    ZLOGD("TestQueryC005 start");
    DataQuery query;
    query.LessThan("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.LessThan("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.LessThan("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "test value";
    query.LessThan("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC005 end");
}

/**
* @tc.name: TestQueryC006
* @tc.desc: Query greaterThanOrEqualTo.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC006, TestSize.Level1)
{
    ZLOGD("TestQueryC006 start");
    DataQuery query;
    query.GreaterThanOrEqualTo("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.GreaterThanOrEqualTo("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.GreaterThanOrEqualTo("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "test value";
    query.GreaterThanOrEqualTo("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC006 end");
}

/**
* @tc.name: TestQueryC007
* @tc.desc: Query lessThanOrEqualTo.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC007, TestSize.Level1)
{
    ZLOGD("TestQueryC007 start");
    DataQuery query;
    query.LessThanOrEqualTo("$.test_field_name", 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.LessThanOrEqualTo("$.test_field_name", (int64_t) 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    query.LessThanOrEqualTo("$.test_field_name", 1.23);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::string str = "test value";
    query.LessThanOrEqualTo("$.test_field_name", str);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC007 end");
}

/**
* @tc.name: TestQueryC008
* @tc.desc: Query isNull.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC008, TestSize.Level1)
{
    ZLOGD("TestQueryC008 start");
    DataQuery query;
    query.IsNull("$.test_field_name");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC008 end");
}

/**
* @tc.name: TestQueryC009
* @tc.desc: Query in.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC009, TestSize.Level1)
{
    ZLOGD("TestQueryC009 start");
    DataQuery query;
    std::vector<int> vectInt{ 10, 20, 30 };
    query.InInt("$.test_field_name", vectInt);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<int64_t> vectLong{ (int64_t) 100, (int64_t) 200, (int64_t) 300 };
    query.InLong("$.test_field_name", vectLong);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<double> vectDouble{ 1.23, 2.23, 3.23 };
    query.InDouble("$.test_field_name", vectDouble);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<std::string> vectString{ "value 1", "value 2", "value 3" };
    query.InString("$.test_field_name", vectString);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC009 end");
}

/**
* @tc.name: TestQueryC010
* @tc.desc: Query in.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC010, TestSize.Level1)
{
    ZLOGD("TestQueryC010 start");
    DataQuery query;
    std::vector<int> vectInt{ 10, 20, 30 };
    query.NotInInt("$.test_field_name", vectInt);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<int64_t> vectLong{ (int64_t) 100, (int64_t) 200, (int64_t) 300 };
    query.NotInLong("$.test_field_name", vectLong);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<double> vectDouble{ 1.23, 2.23, 3.23 };
    query.NotInDouble("$.test_field_name", vectDouble);
    EXPECT_TRUE(query.ToString().length() > 0);
    query.Reset();
    std::vector<std::string> vectString{ "value 1", "value 2", "value 3" };
    query.NotInString("$.test_field_name", vectString);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC010 end");
}

/**
* @tc.name: TestQueryC011
* @tc.desc: Query like.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC011, TestSize.Level1)
{
    ZLOGD("TestQueryC011 start");
    DataQuery query;
    query.Like("$.test_field_name", "test value");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC011 end");
}

/**
* @tc.name: TestQueryC012
* @tc.desc: Query unlike.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC012, TestSize.Level1)
{
    ZLOGD("TestQueryC012 start");
    DataQuery query;
    query.Unlike("$.test_field_name", "test value");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC012 end");
}

/**
* @tc.name: TestQueryC013
* @tc.desc: Query and.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC013, TestSize.Level1)
{
    ZLOGD("TestQueryC013 start");
    DataQuery query;
    query.Like("$.test_field_name1", "test value1");
    query.And();
    query.Like("$.test_field_name2", "test value2");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC013 end");
}

/**
* @tc.name: TestQueryC014
* @tc.desc: Query or.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC014, TestSize.Level1)
{
    ZLOGD("TestQueryC014 start");
    DataQuery query;
    query.Like("$.test_field_name1", "test value1");
    query.Or();
    query.Like("$.test_field_name2", "test value2");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC014 end");
}

/**
* @tc.name: TestQueryC015
* @tc.desc: Query orderBy.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC015, TestSize.Level1)
{
    ZLOGD("TestQueryC015 start");
    DataQuery query;
    query.OrderByAsc("$.test_field_name1");
    query.Reset();
    query.OrderByDesc("$.test_field_name1");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC015 end");
}

/**
* @tc.name: TestQueryC016
* @tc.desc: Query orderBy.
* @tc.type: FUNC
* @tc.require: AR000DPSF5
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC016, TestSize.Level1)
{
    ZLOGD("TestQueryC016 start");
    DataQuery query;
    query.Limit(10, 100);
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC016 end");
}

/**
* @tc.name: TestSingleKvStoreQueryC001
* @tc.desc: query single KvStore.
* @tc.type: FUNC
* @tc.require: SR000DPCO9
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestSingleKvStoreQueryC001, TestSize.Level1)
{
    ZLOGD("TestSingleKvStoreQueryC001 start");

    DistributedKvDataManager manager;
    Options options = { .createIfMissing = true, .encrypt = true, .autoSync = true,
                        .kvStoreType = KvStoreType::SINGLE_VERSION };
    options.schema = VALID_SCHEMA_STRICT_DEFINE;
    AppId appId = { "SingleKvStoreClientQueryTestAppId1" };
    StoreId storeId = { "SingleKvStoreClientQueryTestStoreId1" };
    manager.GetSingleKvStore(options, appId, storeId, [&](Status status, std::unique_ptr<SingleKvStore> kvStore) {
        statusGetKvStore = status;
        singleKvStorePtr = std::move(kvStore);
    });
    EXPECT_NE(singleKvStorePtr, nullptr) << "kvStorePtr is null.";
    singleKvStorePtr->Put("test_key_1", "{\"name\":1}");
    singleKvStorePtr->Put("test_key_2", "{\"name\":2}");
    singleKvStorePtr->Put("test_key_3", "{\"name\":3}");

    DataQuery query;
    query.NotEqualTo("$.name", 3);
    std::vector<Entry> results1;
    Status status1 = singleKvStorePtr->GetEntriesWithQuery(query.ToString(), results1);
    ASSERT_EQ(status1, Status::SUCCESS);
    EXPECT_TRUE(results1.size() == 2);
    std::vector<Entry> results2;
    Status status2 = singleKvStorePtr->GetEntriesWithQuery(query, results2);
    ASSERT_EQ(status2, Status::SUCCESS);
    EXPECT_TRUE(results2.size() == 2);

    std::unique_ptr<KvStoreResultSet> callback1;
    singleKvStorePtr->GetResultSetWithQuery(query.ToString(), [&](Status status3, std::unique_ptr<KvStoreResultSet> call) {
        ASSERT_EQ(status3, Status::SUCCESS);
        callback1 = std::move(call);
        EXPECT_TRUE(callback1->GetCount() == 2);
    });
    auto closeResultSetStatus = singleKvStorePtr->CloseResultSet(std::move(callback1));
    ASSERT_EQ(closeResultSetStatus, Status::SUCCESS);
    std::unique_ptr<KvStoreResultSet> callback2;
    singleKvStorePtr->GetResultSetWithQuery(query, [&](Status status4, std::unique_ptr<KvStoreResultSet> call) {
        ASSERT_EQ(status4, Status::SUCCESS);
        callback2 = std::move(call);
        EXPECT_TRUE(callback2->GetCount() == 2);
    });
    closeResultSetStatus = singleKvStorePtr->CloseResultSet(std::move(callback2));
    ASSERT_EQ(closeResultSetStatus, Status::SUCCESS);

    int resultSize1;
    Status status5 = singleKvStorePtr->GetCountWithQuery(query.ToString(), resultSize1);
    ASSERT_EQ(status5, Status::SUCCESS);
    EXPECT_TRUE(resultSize1 == 2);
    int resultSize2;
    Status status6 = singleKvStorePtr->GetCountWithQuery(query, resultSize2);
    ASSERT_EQ(status6, Status::SUCCESS);
    EXPECT_TRUE(resultSize2 == 2);

    singleKvStorePtr->Delete("test_key_1");
    singleKvStorePtr->Delete("test_key_2");
    singleKvStorePtr->Delete("test_key_3");
    Status status = manager.CloseAllKvStore(appId);
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager.DeleteAllKvStore(appId);
    EXPECT_EQ(status, Status::SUCCESS);

    ZLOGD("TestSingleKvStoreQueryC001 end");
}


/**
* @tc.name: TestSingleKvStoreQueryC002
* @tc.desc: query single KvStore.
* @tc.type: FUNC
* @tc.require: SR000DPCO9
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestSingleKvStoreQueryC002, TestSize.Level1)
{
    ZLOGD("TestSingleKvStoreQueryC002 start");

    DistributedKvDataManager manager;
    Options options = { .createIfMissing = true, .encrypt = true, .autoSync = true,
            .kvStoreType = KvStoreType::SINGLE_VERSION };
    options.schema = VALID_SCHEMA_STRICT_DEFINE;
    AppId appId = { "SingleKvStoreClientQueryTestAppId2" };
    StoreId storeId = { "SingleKvStoreClientQueryTestStoreId2" };
    manager.GetSingleKvStore(options, appId, storeId, [&](Status status, std::unique_ptr<SingleKvStore> kvStore) {
        statusGetKvStore = status;
        singleKvStorePtr = std::move(kvStore);
    });
    EXPECT_NE(singleKvStorePtr, nullptr) << "kvStorePtr is null.";
    singleKvStorePtr->Put("test_key_1", "{\"name\":1}");
    singleKvStorePtr->Put("test_key_2", "{\"name\":2}");
    singleKvStorePtr->Put("test_key_3", "{\"name\":3}");

    DataQuery query;
    query.NotEqualTo("$.name", 3);
    query.And();
    query.EqualTo("$.name", 1);
    std::vector<Entry> results1;
    Status status1 = singleKvStorePtr->GetEntriesWithQuery(query.ToString(), results1);
    ASSERT_EQ(status1, Status::SUCCESS);
    EXPECT_TRUE(results1.size() == 1);
    std::vector<Entry> results2;
    Status status2 = singleKvStorePtr->GetEntriesWithQuery(query, results2);
    ASSERT_EQ(status2, Status::SUCCESS);
    EXPECT_TRUE(results2.size() == 1);

    std::unique_ptr<KvStoreResultSet> callback1;
    singleKvStorePtr->GetResultSetWithQuery(query.ToString(), [&](Status status3, std::unique_ptr<KvStoreResultSet> call) {
        ASSERT_EQ(status3, Status::SUCCESS);
        callback1 = std::move(call);
        EXPECT_TRUE(callback1->GetCount() == 1);
    });
    auto closeResultSetStatus = singleKvStorePtr->CloseResultSet(std::move(callback1));
    ASSERT_EQ(closeResultSetStatus, Status::SUCCESS);
    std::unique_ptr<KvStoreResultSet> callback2;
    singleKvStorePtr->GetResultSetWithQuery(query, [&](Status status4, std::unique_ptr<KvStoreResultSet> call) {
        ASSERT_EQ(status4, Status::SUCCESS);
        callback2 = std::move(call);
        EXPECT_TRUE(callback2->GetCount() == 1);
    });
    closeResultSetStatus = singleKvStorePtr->CloseResultSet(std::move(callback2));
    ASSERT_EQ(closeResultSetStatus, Status::SUCCESS);

    int resultSize1;
    Status status5 = singleKvStorePtr->GetCountWithQuery(query.ToString(), resultSize1);
    ZLOGD("this is it %ul", status5);
    ASSERT_EQ(status5, Status::SUCCESS);
    EXPECT_TRUE(resultSize1 == 1);
    int resultSize2;
    Status status6 = singleKvStorePtr->GetCountWithQuery(query, resultSize2);
    ASSERT_EQ(status6, Status::SUCCESS);
    EXPECT_TRUE(resultSize2 == 1);

    singleKvStorePtr->Delete("test_key_1");
    singleKvStorePtr->Delete("test_key_2");
    singleKvStorePtr->Delete("test_key_3");
    Status status = manager.CloseAllKvStore(appId);
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager.DeleteAllKvStore(appId);
    EXPECT_EQ(status, Status::SUCCESS);

    ZLOGD("TestSingleKvStoreQueryC002 end");
}

/**
* @tc.name: TestQueryC017
* @tc.desc: Query group prefix isNotNull.
* @tc.type: FUNC
* @tc.require: AR000EPAMV
* @tc.author: YangLeda
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC017, TestSize.Level1)
{
    ZLOGD("TestQueryC017 start");
    DataQuery query;
    query.KeyPrefix("prefix");
    query.BeginGroup();
    query.IsNotNull("$.name");
    query.EndGroup();
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC017 end");
}

/**
* @tc.name: TestQueryC018
* @tc.desc: Query SetSuggestIndex.
* @tc.type: FUNC
* @tc.require: AR000F3PBJ
* @tc.author: liuwenhui
*/
HWTEST_F(SingleKvStoreClientQueryTest, TestQueryC018, TestSize.Level1)
{
    ZLOGD("TestQueryC018 start");
    DataQuery query;
    query.SetSuggestIndex("test_field_name");
    EXPECT_TRUE(query.ToString().length() > 0);
    ZLOGD("TestQueryC018 end");
}
