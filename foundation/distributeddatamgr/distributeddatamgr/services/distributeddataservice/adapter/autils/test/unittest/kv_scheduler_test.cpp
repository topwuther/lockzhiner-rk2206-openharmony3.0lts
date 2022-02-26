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
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <cstdio>
#include <iostream>
#include "kv_scheduler.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class KvSchedulerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KvSchedulerTest::SetUpTestCase(void)
{}

void KvSchedulerTest::TearDownTestCase(void)
{}

void KvSchedulerTest::SetUp(void)
{}

void KvSchedulerTest::TearDown(void)
{}

/**
  * @tc.name: KvSchedulerTest001
  * @tc.desc: test schedule at some time.
  * @tc.type: FUNC
  * @tc.require: SR000DR9J0 AR000DR9J1
  * @tc.author: hongbo
  */
HWTEST_F(KvSchedulerTest, KvSchedulerTest001, TestSize.Level1)
{
    auto sch = std::make_unique<KvScheduler>();
    ASSERT_TRUE(nullptr != sch) << "sch is nullptr";
    std::chrono::system_clock::time_point tp3 = std::chrono::system_clock::now() + std::chrono::duration<int>(1);
    int a = 1;
    auto task = sch->At(tp3, [&]() {
        a = 2;
    });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_EQ(a, 2) << "KvSchedulerTest KvSchedulerTest001 failed";
    ASSERT_TRUE(nullptr != sch) << "sch is nullptr";
    sch->Remove(task);
}

/**
  * @tc.name: KvSchedulerTest002
  * @tc.desc: test schedule at some time.
  * @tc.type: FUNC
  * @tc.require: SR000DR9J0 AR000DR9J1
  * @tc.author: hongbo
  */
HWTEST_F(KvSchedulerTest, Test002, TestSize.Level1)
{
    auto sch = std::make_unique<KvScheduler>();
    ASSERT_TRUE(nullptr != sch) << "sch is nullptr";
    std::chrono::duration<int> d(1);
    int a = 1;
    sch->Every(d, d, [&]() {
        a = 2;
    });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_EQ(a, 2) << "KvSchedulerTest KvSchedulerTest002 failed";
}

/**
  * @tc.name: KvSchedulerTest003
  * @tc.desc: test schedule at some time.
  * @tc.type: FUNC
  * @tc.require: SR000DR9J0 AR000DR9J1
  * @tc.author: hongbo
  */
HWTEST_F(KvSchedulerTest, KvSchedulerTest003, TestSize.Level1)
{
    auto sch = std::make_unique<KvScheduler>();
    ASSERT_TRUE(nullptr != sch) << "sch is nullptr";
    std::chrono::duration<int> d(1);
    int a = 1;
    sch->Every(2, d, d, [&]() {
        a = a + 1;
    });
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_EQ(a, 3) << "KvSchedulerTest KvSchedulerTest003 failed";
}