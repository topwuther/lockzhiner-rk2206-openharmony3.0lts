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
#include <thread>
#include <atomic>

#include "db_errno.h"
#include "log_print.h"
#include "platform_specific.h"
#include "evloop/include/ievent.h"
#include "evloop/include/ievent_loop.h"

using namespace testing::ext;
using namespace DistributedDB;

namespace {
    IEventLoop *g_loop = nullptr;
    constexpr int MAX_RETRY_TIMES = 1000;
    constexpr int RETRY_TIMES_5 = 5;
    constexpr EventTime TIME_INACCURACY = 100LL;
    constexpr EventTime TIME_PIECE_1 = 1LL;
    constexpr EventTime TIME_PIECE_10 = 10LL;
    constexpr EventTime TIME_PIECE_50 = 50LL;
    constexpr EventTime TIME_PIECE_100 = 100LL;
    constexpr EventTime TIME_PIECE_1000 = 1000LL;
    constexpr EventTime TIME_PIECE_10000 = 10000LL;
}

class TimerTester {
public:
    static EventTime GetCurrentTime();
};

EventTime TimerTester::GetCurrentTime()
{
    uint64_t now;
    int errCode = OS::GetCurrentSysTimeInMicrosecond(now);
    if (errCode != E_OK) {
        LOGE("Get current time failed.");
        return 0;
    }
    return now / 1000; // microsecond to millisecond.
}

class DistributedDBEventLoopTimerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBEventLoopTimerTest::SetUpTestCase(void) {}

void DistributedDBEventLoopTimerTest::TearDownTestCase(void) {}

void DistributedDBEventLoopTimerTest::SetUp(void)
{
    /**
     * @tc.setup: Create a loop object.
     */
    if (g_loop == nullptr) {
        int errCode = E_OK;
        g_loop = IEventLoop::CreateEventLoop(errCode);
        if (g_loop == nullptr) {
            LOGE("Prepare loop in SetUp() failed.");
        }
    }
}

void DistributedDBEventLoopTimerTest::TearDown(void)
{
    /**
     * @tc.teardown: Destroy the loop object.
     */
    if (g_loop != nullptr) {
        g_loop->KillAndDecObjRef(g_loop);
        g_loop = nullptr;
    }
}

/**
 * @tc.name: EventLoopTimerTest001
 * @tc.desc: Create and destroy the event loop object.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a loop.
     * @tc.expected: step1. create successfully.
     */
    int errCode = E_OK;
    IEventLoop *loop = IEventLoop::CreateEventLoop(errCode);
    ASSERT_EQ(loop != nullptr, true);

    /**
     * @tc.steps: step2. destroy the loop.
     * @tc.expected: step2. destroy successfully.
     */
    bool finalized = false;
    loop->OnLastRef([&finalized](){ finalized = true; });
    loop->DecObjRef(loop);
    loop = nullptr;
    EXPECT_EQ(finalized, true);
}

/**
 * @tc.name: EventLoopTimerTest002
 * @tc.desc: Start and stop the loop
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest002, TestSize.Level1)
{
    // ready data
    ASSERT_EQ(g_loop != nullptr, true);

    /**
     * @tc.steps: step1. create a loop.
     * @tc.expected: step1. create successfully.
     */
    std::atomic<bool> running(false);
    EventTime delta = 0;
    std::thread loopThread([&running, &delta](){
            running = true;
            EventTime start = TimerTester::GetCurrentTime();
            g_loop->Run();
            EventTime end = TimerTester::GetCurrentTime();
            delta = end - start;
        });
    while (!running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_1));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_100));
    g_loop->KillObj();
    loopThread.join();
    EXPECT_EQ(delta > TIME_PIECE_50, true);
}

/**
 * @tc.name: EventLoopTimerTest003
 * @tc.desc: Create and destroy a timer object.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest003, TestSize.Level0)
{
     /**
     * @tc.steps: step1. create event(timer) object.
     * @tc.expected: step1. create successfully.
     */
    int errCode = E_OK;
    IEvent *timer = IEvent::CreateEvent(TIME_PIECE_1, errCode);
    ASSERT_EQ(timer != nullptr, true);

    /**
     * @tc.steps: step2. destroy the event object.
     * @tc.expected: step2. destroy successfully.
     */
    bool finalized = false;
    errCode = timer->SetAction([](EventsMask revents) -> int {
            return E_OK;
        }, [&finalized](){
            finalized = true;
        });
    EXPECT_EQ(errCode, E_OK);
    timer->KillAndDecObjRef(timer);
    timer = nullptr;
    EXPECT_EQ(finalized, true);
}

/**
 * @tc.name: EventLoopTimerTest004
 * @tc.desc: Start a timer
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest004, TestSize.Level1)
{
    // ready data
    ASSERT_EQ(g_loop != nullptr, true);

    /**
     * @tc.steps: step1. start the loop.
     * @tc.expected: step1. start successfully.
     */
    std::atomic<bool> running(false);
    std::thread loopThread([&running](){
            running = true;
            g_loop->Run();
        });

    int tryCounter = 0;
    while (!running && ++tryCounter < MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_1));
    }
    EXPECT_EQ(running, true);

    /**
     * @tc.steps: step2. create and start a timer.
     * @tc.expected: step2. start successfully.
     */
    int errCode = E_OK;
    IEvent *timer = IEvent::CreateEvent(TIME_PIECE_10, errCode);
    ASSERT_EQ(timer != nullptr, true);
    std::atomic<int> counter(0);
    errCode = timer->SetAction([&counter](EventsMask revents) -> int { ++counter; return E_OK; }, nullptr);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_loop->Add(timer);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step3. wait and check.
     * @tc.expected: step3. 'counter' increased by the timer.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_100));
    EXPECT_EQ(counter > 0, true);
    g_loop->KillObj();
    loopThread.join();
    timer->DecObjRef(timer);
}

/**
 * @tc.name: EventLoopTimerTest005
 * @tc.desc: Stop a timer
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest005, TestSize.Level1)
{
    // ready data
    ASSERT_EQ(g_loop != nullptr, true);

    /**
     * @tc.steps: step1. start the loop.
     * @tc.expected: step1. start successfully.
     */
    std::atomic<bool> running(false);
    std::thread loopThread([&running](){
            running = true;
            g_loop->Run();
        });

    int tryCounter = 0;
    while (!running && ++tryCounter <= MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(running, true);

    /**
     * @tc.steps: step2. create and start a timer.
     * @tc.expected: step2. start successfully.
     */
    int errCode = E_OK;
    IEvent *timer = IEvent::CreateEvent(10, errCode);
    ASSERT_EQ(timer != nullptr, true);
    std::atomic<int> counter(0);
    std::atomic<bool> finalize(false);
    errCode = timer->SetAction(
        [&counter](EventsMask revents) -> int {
            ++counter;
            return E_OK;
        }, [&finalize](){ finalize = true; });
    EXPECT_EQ(errCode, E_OK);
    errCode = g_loop->Add(timer);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step3. wait and check.
     * @tc.expected: step3. 'counter' increased by the timer and the timer object finalized.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_100));
    timer->KillAndDecObjRef(timer);
    timer = nullptr;
    g_loop->KillObj();
    loopThread.join();
    EXPECT_EQ(counter > 0, true);
    EXPECT_EQ(finalize, true);
}

/**
 * @tc.name: EventLoopTimerTest006
 * @tc.desc: Stop a timer
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest006, TestSize.Level1)
{
    // ready data
    ASSERT_EQ(g_loop != nullptr, true);

    /**
     * @tc.steps: step1. start the loop.
     * @tc.expected: step1. start successfully.
     */
    std::atomic<bool> running(false);
    std::thread loopThread([&running](){
            running = true;
            g_loop->Run();
        });

    int tryCounter = 0;
    while (!running && ++tryCounter <= MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_10));
    }
    EXPECT_EQ(running, true);

    /**
     * @tc.steps: step2. create and start a timer.
     * @tc.expected: step2. start successfully.
     */
    int errCode = E_OK;
    IEvent *timer = IEvent::CreateEvent(TIME_PIECE_10, errCode);
    ASSERT_EQ(timer != nullptr, true);
    std::atomic<int> counter(0);
    std::atomic<bool> finalize(false);
    errCode = timer->SetAction([&counter](EventsMask revents) -> int { ++counter; return -E_STALE; },
        [&finalize](){ finalize = true; });
    EXPECT_EQ(errCode, E_OK);
    errCode = g_loop->Add(timer);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step3. wait and check.
     * @tc.expected: step3. 'counter' increased by the timer and the timer object finalized.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_100));
    g_loop->KillObj();
    loopThread.join();
    timer->DecObjRef(timer);
    timer = nullptr;
    EXPECT_EQ(finalize, true);
    EXPECT_EQ(counter > 0, true);
}

/**
 * @tc.name: EventLoopTimerTest007
 * @tc.desc: Modify a timer
 * @tc.type: FUNC
 * @tc.require: AR000CKRTB AR000CQE0C
 * @tc.author: fangyi
 */
HWTEST_F(DistributedDBEventLoopTimerTest, EventLoopTimerTest007, TestSize.Level2)
{
    // ready data
    ASSERT_EQ(g_loop != nullptr, true);

    /**
     * @tc.steps: step1. start the loop.
     * @tc.expected: step1. start successfully.
     */
    std::atomic<bool> running(false);
    std::thread loopThread([&running](){
            running = true;
            g_loop->Run();
        });

    int tryCounter = 0;
    while (!running && ++tryCounter <= MAX_RETRY_TIMES) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EXPECT_EQ(running, true);

    /**
     * @tc.steps: step2. create and start a timer.
     * @tc.expected: step2. start successfully.
     */
    int errCode = E_OK;
    IEvent *timer = IEvent::CreateEvent(TIME_PIECE_1000, errCode);
    ASSERT_EQ(timer != nullptr, true);
    int counter = 1; // Interval: 1 * TIME_PIECE_100
    EventTime lastTime = TimerTester::GetCurrentTime();
    errCode = timer->SetAction(
        [timer, &counter, &lastTime](EventsMask revents) -> int {
            EventTime now = TimerTester::GetCurrentTime();
            EventTime delta = now - lastTime;
            delta -= counter * TIME_PIECE_1000;
            EXPECT_EQ(delta >= -TIME_INACCURACY && delta <= TIME_INACCURACY, true);
            if (++counter > RETRY_TIMES_5) {
                return -E_STALE;
            }
            lastTime = TimerTester::GetCurrentTime();
            int errCode = timer->SetTimeout(counter * TIME_PIECE_1000);
            EXPECT_EQ(errCode, E_OK);
            return E_OK;
        }, nullptr);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_loop->Add(timer);
    EXPECT_EQ(errCode, E_OK);

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_PIECE_10000));
    g_loop->KillObj();
    loopThread.join();
    timer->DecObjRef(timer);
}
