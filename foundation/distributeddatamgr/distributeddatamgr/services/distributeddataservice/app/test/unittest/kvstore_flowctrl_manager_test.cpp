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

#include "flowctrl_manager/kvstore_flowctrl_manager.h"
#include <memory>
#include <thread>
#include "time_utils.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS;

class KvStoreFlowCtrlManagerTest : public testing::Test {
public:
    static inline const int MANAGER_BURST_CAPACITY = 50;
    static inline const int MANAGER_SUSTAINED_CAPACITY = 500;
    static inline const int OPERATION_BURST_CAPACITY = 1000;
    static inline const int OPERATION_SUSTAINED_CAPACITY = 10000;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KvStoreFlowCtrlManagerTest::SetUpTestCase(void)
{}

void KvStoreFlowCtrlManagerTest::TearDownTestCase(void)
{}

void KvStoreFlowCtrlManagerTest::SetUp(void)
{}

void KvStoreFlowCtrlManagerTest::TearDown(void)
{}

/**
* @tc.name: KvStoreFlowCtrlManagerTest001
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest001, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 1001; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(1000, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest002
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest002, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 1000; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(1000, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest003
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest003, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 999; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(999, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest004
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: SR000F3H5U AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest004, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 9999; i++) {
        arr[ptr->IsTokenEnough()]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(9999, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest005
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest005, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 10000; i++) {
        arr[ptr->IsTokenEnough()]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(10000, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest006
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest006, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(OPERATION_BURST_CAPACITY, OPERATION_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    uint64_t curTime = 0;
    uint64_t lastTime = TimeUtils::CurrentTimeMicros();
    for (int i = 0; i < 10001; i++) {
        arr[ptr->IsTokenEnough()]++;
        while (true) {
            curTime = TimeUtils::CurrentTimeMicros();
            if ((curTime - lastTime) > 1000) {
                lastTime = curTime;
                break;
            }
        }
    }
    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(10000, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest007
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest007, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 51; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(50, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest008
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest008, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 50; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(50, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest009
* @tc.desc: burst flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP7
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest009, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 49; i++) {
        arr[ptr->IsTokenEnough()]++;
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(49, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest010
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest010, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 499; i++) {
        arr[ptr->IsTokenEnough()]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(499, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest011
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest011, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 500; i++) {
        arr[ptr->IsTokenEnough()]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    EXPECT_EQ(0, arr[0]);
    EXPECT_EQ(500, arr[1]);
}

/**
* @tc.name: KvStoreFlowCtrlManagerTest012
* @tc.desc: sustained flow control
* @tc.type: FUNC
* @tc.require: AR000F3OP8
* @tc.author: jishengwu
*/
HWTEST_F(KvStoreFlowCtrlManagerTest, KvStoreFlowCtrlManagerTest012, TestSize.Level1)
{
    auto ptr = std::make_shared<KvStoreFlowCtrlManager>(MANAGER_BURST_CAPACITY, MANAGER_SUSTAINED_CAPACITY);
    int arr[2] = {0, 0};
    for (int i = 0; i < 501; i++) {
        arr[ptr->IsTokenEnough()]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    EXPECT_EQ(1, arr[0]);
    EXPECT_EQ(500, arr[1]);
}

