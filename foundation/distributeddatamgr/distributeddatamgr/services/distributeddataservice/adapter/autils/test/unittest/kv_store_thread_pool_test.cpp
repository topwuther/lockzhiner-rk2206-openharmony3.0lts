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

#include <thread>
#include <chrono>
#include <mutex>
#include <gtest/gtest.h>
#include "kv_store_thread_pool.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class KvStoreThreadPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KvStoreThreadPoolTest::SetUpTestCase(void)
{}

void KvStoreThreadPoolTest::TearDownTestCase(void)
{}

void KvStoreThreadPoolTest::SetUp(void)
{}

void KvStoreThreadPoolTest::TearDown(void)
{}

/**
  * @tc.name: TestApplyTask001
  * @tc.desc: test if task can be done asynchronous.
  * @tc.type: FUNC
  * @tc.require: AR000CQS31
  * @tc.author: liqiao
  */
HWTEST_F(KvStoreThreadPoolTest, TestApplyTask001, TestSize.Level1)
{
    auto pool = KvStoreThreadPool::GetPool(8, true);
    int var = 0;
    auto start = std::chrono::system_clock::now();
    pool->AddTask(KvStoreTask([&var](){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        var++;
    }));
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> returnTime = end - start;
    EXPECT_LT(returnTime.count(), 0.05);
    EXPECT_EQ(var, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_EQ(var, 1);
}

/**
  * @tc.name: TestApplyTask002
  * @tc.desc: test if task can be done in different thread.
  * @tc.type: FUNC
  * @tc.require: AR000CQS31
  * @tc.author: liqiao
  */
HWTEST_F(KvStoreThreadPoolTest, TestApplyTask002, TestSize.Level2)
{
    auto pool = KvStoreThreadPool::GetPool(2, false);
    int var = 0;
    std::mutex varMutex;
    auto start = std::chrono::system_clock::now();
    KvStoreTask task([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::lock_guard<std::mutex> lock(varMutex);
        var++;
    });
    for (int i = 0; i < 8; i++) {
        pool->AddTask(KvStoreTask(task));
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> returnTime = end - start;
    EXPECT_LT(returnTime.count(), 0.1);
    EXPECT_EQ(var, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    EXPECT_EQ(var, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(var, 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(var, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(var, 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(var, 8);
    pool->Stop();
}

/**
  * @tc.name: TestApplyTask003
  * @tc.desc: test whether task can be done if they are not scheduled when calling Stop().
  * @tc.type: FUNC
  * @tc.require: AR000CQS31
  * @tc.author: liqiao
  */
HWTEST_F(KvStoreThreadPoolTest, TestApplyTask003, TestSize.Level1)
{
    auto pool = KvStoreThreadPool::GetPool(2, false);
    int var = 0;
    std::mutex varMutex;
    KvStoreTask task([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(varMutex);
        var++;
    });
    for (int i = 0; i < 8; i++) {
        pool->AddTask(KvStoreTask(task));
    }
    EXPECT_EQ(var, 0);
    pool->Stop();
    EXPECT_EQ(var, 8);
}