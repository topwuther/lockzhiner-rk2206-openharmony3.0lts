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
#include <functional>
#include "db_errno.h"
#include "log_print.h"
#include "multi_ver_vacuum.h"
#include "multi_ver_vacuum_executor_stub.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    using Predication = std::function<bool(void)>;
    // repeatInterval in millisecond
    bool RepeatCheckAsyncResult(const Predication &inPred, uint8_t repeatLimit, uint32_t repeatInterval)
    {
        uint8_t limit = repeatLimit;
        while (limit != 0) {
            if (inPred()) {
                return true;
            }
            if (--limit == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(repeatInterval));
        }
        return false;
    }

    const string DB_IDENTITY_A = "DATABASE_A";
    const string DB_IDENTITY_B = "DATABASE_B";
    const string DB_IDENTITY_C = "DATABASE_C";

    bool CheckVacuumTaskStatus(const MultiVerVacuum &inVacuum, const string &inDbIdentifier,
        VacuumTaskStatus expectStatus, uint8_t repeatLimit = 5, uint32_t repeatInterval = 100) // 5 times, 100 ms
    {
        return RepeatCheckAsyncResult([&inVacuum, &inDbIdentifier, expectStatus]()->bool {
            VacuumTaskStatus outStatus = VacuumTaskStatus::RUN_WAIT;
            int errCode = inVacuum.QueryStatus(inDbIdentifier, outStatus);
            return errCode == E_OK && outStatus == expectStatus;
        }, repeatLimit, repeatInterval);
    }
}

class DistributedDBMultiVerVacuumTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBMultiVerVacuumTest::SetUpTestCase(void)
{
    MultiVerVacuum::Enable(true); // Make sure functionality is enabled.
}

void DistributedDBMultiVerVacuumTest::TearDownTestCase(void)
{
}

void DistributedDBMultiVerVacuumTest::SetUp()
{
}

void DistributedDBMultiVerVacuumTest::TearDown()
{
}

/**
 * @tc.name: SingleTaskNormalStatusSwitch001
 * @tc.desc: Test status switch for single task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskNormalStatusSwitch001, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseA(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskA for databaseA
     * @tc.expected: step1. dbTaskA RUN_NING
     */
    int errCode = vacuum.Launch(DB_IDENTITY_A, &databaseA);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskA
     * @tc.expected: step2. dbTaskA PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. pause dbTaskA again
     * @tc.expected: step3. dbTaskA PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. continue dbTaskA with autoRelaunch false
     * @tc.expected: step4. dbTaskA PAUSE_DONE
     */
    errCode = vacuum.Continue(DB_IDENTITY_A, false);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 ms
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::PAUSE_DONE);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. continue dbTaskA with autoRelaunch false again
     * @tc.expected: step5. dbTaskA RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_A, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFive, true);

    /**
     * @tc.steps: step6. wait for some time
     * @tc.expected: step6. dbTaskA FINISH
     */
    bool stepSix = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepSix, true);
}

/**
 * @tc.name: SingleTaskNormalStatusSwitch002
 * @tc.desc: Test status switch for single task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskNormalStatusSwitch002, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseB(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskB for databaseB, then wait for some time
     * @tc.expected: step1. dbTaskB FINISH
     */
    int errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskB
     * @tc.expected: step2. dbTaskB FINISH
     */
    errCode = vacuum.Pause(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. continue dbTaskB with autoRelaunch false
     * @tc.expected: step3. dbTaskB FINISH
     */
    errCode = vacuum.Continue(DB_IDENTITY_B, false);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 ms
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. pause dbTaskB again
     * @tc.expected: step4. dbTaskB FINISH
     */
    errCode = vacuum.Pause(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. continue dbTaskB again with autoRelaunch true
     * @tc.expected: step5. dbTaskB RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_B, true);
    EXPECT_EQ(errCode, E_OK);
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFive, true);

    /**
     * @tc.steps: step6. wait for some time
     * @tc.expected: step6. dbTaskB FINISH
     */
    bool stepSix = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepSix, true);
}

/**
 * @tc.name: SingleTaskNormalStatusSwitch003
 * @tc.desc: Test status switch for single task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskNormalStatusSwitch003, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseC(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskC for databaseC, then wait for some time
     * @tc.expected: step1. dbTaskC FINISH
     */
    int errCode = vacuum.Launch(DB_IDENTITY_C, &databaseC);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. AutoRelaunch dbTaskC
     * @tc.expected: step2. dbTaskC RUN_NING
     */
    errCode = vacuum.AutoRelaunchOnce(DB_IDENTITY_C);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. Abort dbTaskC
     * @tc.expected: step3. dbTaskC ABORT_DONE
     */
    errCode = vacuum.Abort(DB_IDENTITY_C);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. launch dbTaskC again
     * @tc.expected: step4. dbTaskC RUN_NING
     */
    errCode = vacuum.Launch(DB_IDENTITY_C, &databaseC);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. wait for some time
     * @tc.expected: step5. dbTaskC FINISH
     */
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepFive, true);

    /**
     * @tc.steps: step6. Abort dbTaskC again
     * @tc.expected: step6. dbTaskC ABORT_DONE
     */
    errCode = vacuum.Abort(DB_IDENTITY_C);
    EXPECT_EQ(errCode, E_OK);
    bool stepSix = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepSix, true);
}

/**
 * @tc.name: SingleTaskNormalStatusSwitch004
 * @tc.desc: Test status switch for single task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskNormalStatusSwitch004, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseA(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskA for databaseA, then wait for some time
     * @tc.expected: step1. dbTaskA FINISH
     */
    int errCode = vacuum.Launch(DB_IDENTITY_A, &databaseA);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskA
     * @tc.expected: step2. dbTaskA FINISH
     */
    errCode = vacuum.Pause(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. AutoRelaunch dbTaskA
     * @tc.expected: step3. dbTaskA FINISH
     */
    errCode = vacuum.AutoRelaunchOnce(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 ms
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. continue dbTaskA with autoRelaunch false
     * @tc.expected: step4. dbTaskA RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_A, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. wait for some time
     * @tc.expected: step5. dbTaskA FINISH
     */
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepFive, true);
}

/**
 * @tc.name: SingleTaskAbnormalStatusSwitch001
 * @tc.desc: Test status switch for single task under abnormal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskAbnormalStatusSwitch001, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseB(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskB for databaseB
     * @tc.expected: step1. dbTaskB RUN_NING
     */
    int errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskB
     * @tc.expected: step2. dbTaskB PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. abort dbTaskB
     * @tc.expected: step3. dbTaskB ABORT_DONE
     */
    errCode = vacuum.Abort(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. launch dbTaskB again
     * @tc.expected: step4. dbTaskB RUN_NING
     */
    errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. pause dbTaskB again
     * @tc.expected: step5. dbTaskB PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepFive, true);

    /**
     * @tc.steps: step6. continue dbTaskA with autoRelaunch false
     * @tc.expected: step6. dbTaskB RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_B, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepSix = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepSix, true);

    /**
     * @tc.steps: step7. wait for some time
     * @tc.expected: step7. dbTaskB FINISH
     */
    bool stepSeven = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepSeven, true);
}

namespace {
    bool ConcurrentPauseThenCheckResult(MultiVerVacuum &vacuum, const std::string &dbIdentifier)
    {
        int retForThreadA = E_OK;
        int retForThreadB = E_OK;
        int isQuitThreadA = false;
        int isQuitThreadB = false;
        std::thread threadA([&vacuum, &dbIdentifier, &retForThreadA, &isQuitThreadA]() {
            LOGI("[ConcurrentPauseThenCheckResult] ThreadA Enter Do Pause.");
            retForThreadA = vacuum.Pause(dbIdentifier);
            isQuitThreadA = true;
            LOGI("[ConcurrentPauseThenCheckResult] ThreadA Exit Do Pause.");
        });
        std::thread threadB([&vacuum, &dbIdentifier, &retForThreadB, &isQuitThreadB]() {
            LOGI("[ConcurrentPauseThenCheckResult] ThreadB Enter Do Pause.");
            retForThreadB = vacuum.Pause(dbIdentifier);
            isQuitThreadB = true;
            LOGI("[ConcurrentPauseThenCheckResult] ThreadB Exit Do Pause.");
        });
        threadA.detach();
        threadB.detach();
        bool result = RepeatCheckAsyncResult([&retForThreadA, &isQuitThreadA, &retForThreadB, &isQuitThreadB]()->bool {
            LOGI("[ConcurrentPauseThenCheckResult] Check.");
            return retForThreadA == E_OK && retForThreadB == E_OK && isQuitThreadA == true && isQuitThreadB == true;
        }, 6, 500); // 6 time, 500 ms
        if (!result) {
            LOGE("[ConcurrentPauseThenCheckResult] RepeatCheckAsyncResult Fail.");
            return false;
        }
        return CheckVacuumTaskStatus(vacuum, dbIdentifier, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    }

    bool ConcurrentPauseAndAbortThenCheckResult(MultiVerVacuum &vacuum, const std::string &dbIdentifier)
    {
        int retForThreadA = E_OK;
        int retForThreadB = E_OK;
        int isQuitThreadA = false;
        int isQuitThreadB = false;
        std::thread threadA([&vacuum, &dbIdentifier, &retForThreadA, &isQuitThreadA]() {
            LOGI("[ConcurrentPauseAndAbortThenCheckResult] ThreadA Enter Do Pause.");
            retForThreadA = vacuum.Pause(dbIdentifier);
            isQuitThreadA = true;
            LOGI("[ConcurrentPauseAndAbortThenCheckResult] ThreadA Exit Do Pause.");
        });
        std::thread threadB([&vacuum, &dbIdentifier, &retForThreadB, &isQuitThreadB]() {
            LOGI("[ConcurrentPauseAndAbortThenCheckResult] ThreadB Enter Do Abort.");
            retForThreadB = vacuum.Abort(dbIdentifier);
            isQuitThreadB = true;
            LOGI("[ConcurrentPauseAndAbortThenCheckResult] ThreadB Exit Do Abort.");
        });
        threadA.detach();
        threadB.detach();
        bool result = RepeatCheckAsyncResult([&retForThreadA, &isQuitThreadA, &retForThreadB, &isQuitThreadB]()->bool {
            LOGI("[ConcurrentPauseAndAbortThenCheckResult] Check."); // Pause May Fail if Abort First
            return retForThreadB == E_OK && isQuitThreadA == true && isQuitThreadB == true;
        }, 6, 500); // 6 time, 500 ms
        if (!result) {
            LOGE("[ConcurrentPauseAndAbortThenCheckResult] RepeatCheckAsyncResult Fail.");
            return false;
        }
        return CheckVacuumTaskStatus(vacuum, dbIdentifier, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    }
}

/**
 * @tc.name: SingleTaskConcurrentStatusSwitch001
 * @tc.desc: Test status switch for single task under Concurrent operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskConcurrentStatusSwitch001, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseC(DbScale{1, 1, 1, 1}, 1000); // 1 For Scale, 1000 For TimeCost, 11s in Total

    /**
     * @tc.steps: step1. launch dbTaskC for databaseC, databaseC is timecost
     * @tc.expected: step1. dbTaskC FINISH
     */
    int errCode = vacuum.Launch(DB_IDENTITY_C, &databaseC);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. Concurrently pause dbTaskC in two thread
     * @tc.expected: step2. thread can quit and dbTaskC PAUSE_DONE
     */
    bool stepTwo = ConcurrentPauseThenCheckResult(vacuum, DB_IDENTITY_C);
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. continue dbTaskC with autoRelaunch false
     * @tc.expected: step3. dbTaskC PAUSE_DONE
     */
    errCode = vacuum.Continue(DB_IDENTITY_C, false);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100 ms
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. continue dbTaskC with autoRelaunch false again
     * @tc.expected: step4. dbTaskC RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_C, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. Concurrently pause and abort dbTaskC in two thread
     * @tc.expected: step5. thread can quit and dbTaskC ABORT_DONE
     */
    bool stepFive = ConcurrentPauseAndAbortThenCheckResult(vacuum, DB_IDENTITY_C);
    EXPECT_EQ(stepFive, true);
}

/**
 * @tc.name: SingleTaskWriteHandleOccupy001
 * @tc.desc: Test write handle occupy for single task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, SingleTaskWriteHandleOccupy001, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseA(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskA for databaseA
     * @tc.expected: step1. dbTaskA RUN_NING
     */
    int errCode = vacuum.Launch(DB_IDENTITY_A, &databaseA);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);
    stepOne = RepeatCheckAsyncResult([&databaseA]()->bool{
        return databaseA.IsTransactionOccupied() == true;
    }, 5, 100); // 5 times, 100 ms
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskA
     * @tc.expected: step2. dbTaskA PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);
    EXPECT_EQ(databaseA.IsTransactionOccupied(), false);

    /**
     * @tc.steps: step3. Continue dbTaskA
     * @tc.expected: step3. dbTaskA RUN_NING
     */
    errCode = vacuum.Continue(DB_IDENTITY_A, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepThree, true);
    stepThree = RepeatCheckAsyncResult([&databaseA]()->bool{
        return databaseA.IsTransactionOccupied() == true;
    }, 5, 100); // 5 times, 100 ms
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. Abort dbTaskA
     * @tc.expected: step4. dbTaskA ABORT_DONE
     */
    errCode = vacuum.Abort(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFour, true);
    EXPECT_EQ(databaseA.IsTransactionOccupied(), false);
}

/**
 * @tc.name: MultipleTaskNormalStatusSwitch001
 * @tc.desc: Test status switch for multiple task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, MultipleTaskNormalStatusSwitch001, TestSize.Level1)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseA(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total
    MultiVerVacuumExecutorStub databaseB(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskA for databaseA and dbTaskB for databaseB
     * @tc.expected: step1. dbTaskA RUN_NING and dbTaskB RUN_WAIT
     */
    int errCode = vacuum.Launch(DB_IDENTITY_A, &databaseA);
    EXPECT_EQ(errCode, E_OK);
    errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);
    stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_WAIT);
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. pause dbTaskB
     * @tc.expected: step2. dbTaskA RUN_NING and dbTaskB PAUSE_DONE
     */
    errCode = vacuum.Pause(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);
    stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::PAUSE_DONE, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);
    /**
     * @tc.steps: step3. continue dbTaskB with autoRelaunch false
     * @tc.expected: step3. dbTaskA RUN_NING and dbTaskB RUN_WAIT
     */
    errCode = vacuum.Continue(DB_IDENTITY_B, false);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_NING, 1); // only 1 time
    EXPECT_EQ(stepThree, true);
    stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_WAIT, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. Abort dbTaskA
     * @tc.expected: step4. dbTaskA ABORT_DONE and dbTaskB RUN_NING
     */
    errCode = vacuum.Abort(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFour, true);
    stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. Abort dbTaskB
     * @tc.expected: step5. dbTaskA ABORT_DONE and dbTaskB ABORT_DONE
     */
    errCode = vacuum.Abort(DB_IDENTITY_B);
    EXPECT_EQ(errCode, E_OK);
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFive, true);
    stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFive, true);
}

/**
 * @tc.name: MultipleTaskNormalStatusSwitch002
 * @tc.desc: Test status switch for multiple task under normal operation
 * @tc.type: FUNC
 * @tc.require: AR000C6TRV AR000CQDTM
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerVacuumTest, MultipleTaskNormalStatusSwitch002, TestSize.Level2)
{
    // Preset
    MultiVerVacuum vacuum;
    MultiVerVacuumExecutorStub databaseA(DbScale{1, 1, 1, 1}, 30); // 1 For Scale, 30 For TimeCost, 330ms in Total
    MultiVerVacuumExecutorStub databaseB(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total
    MultiVerVacuumExecutorStub databaseC(DbScale{1, 1, 2, 2}, 100); // 1, 2 For Scale, 100 For TimeCost, 1.7s in Total

    /**
     * @tc.steps: step1. launch dbTaskA,B,C for databaseA,B,C and wait dbTaskA,B FINISH
     * @tc.expected: step1. dbTaskA FINISH and dbTaskB FINISH and dbTaskC RUN_NING
     */
    int errCode = vacuum.Launch(DB_IDENTITY_A, &databaseA);
    EXPECT_EQ(errCode, E_OK);
    errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    errCode = vacuum.Launch(DB_IDENTITY_C, &databaseC);
    EXPECT_EQ(errCode, E_OK);
    bool stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 10, 100); // 10 time, 100 ms
    EXPECT_EQ(stepOne, true);
    stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::FINISH, 10, 500); // 10 time, 500 ms
    EXPECT_EQ(stepOne, true);
    stepOne = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepOne, true);

    /**
     * @tc.steps: step2. abnormal operation, Launch dbTaskB again without abort dbTaskB
     * @tc.expected: step2. dbTaskA FINISH and dbTaskB RUN_WAIT and dbTaskC RUN_NING
     */
    errCode = vacuum.Launch(DB_IDENTITY_B, &databaseB);
    EXPECT_EQ(errCode, E_OK);
    bool stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);
    stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_WAIT, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);
    stepTwo = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING, 1); // only 1 time
    EXPECT_EQ(stepTwo, true);

    /**
     * @tc.steps: step3. AutoRelaunch dbTaskA
     * @tc.expected: step3. dbTaskA RUN_WAIT and dbTaskB RUN_WAIT and dbTaskC RUN_NING
     */
    errCode = vacuum.AutoRelaunchOnce(DB_IDENTITY_A);
    EXPECT_EQ(errCode, E_OK);
    bool stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_WAIT, 1); // only 1 time
    EXPECT_EQ(stepThree, true);
    stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_WAIT, 1); // only 1 time
    EXPECT_EQ(stepThree, true);
    stepThree = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::RUN_NING, 1); // only 1 time
    EXPECT_EQ(stepThree, true);

    /**
     * @tc.steps: step4. wait dbTaskC FINISH
     * @tc.expected: step4. dbTaskA RUN_WAIT and dbTaskB RUN_NING and dbTaskC FINISH
     */
    bool stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::FINISH, 3, 1000); // 3 time, 1000 ms
    EXPECT_EQ(stepFour, true);
    stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::RUN_WAIT, 1); // only 1 time
    EXPECT_EQ(stepFour, true);
    stepFour = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::RUN_NING);
    EXPECT_EQ(stepFour, true);

    /**
     * @tc.steps: step5. Abort dbTaskB and dbTaskB
     * @tc.expected: step5. dbTaskA ABORT_DONE and dbTaskB ABORT_DONE and dbTaskC FINISH
     */
    vacuum.Abort(DB_IDENTITY_A);
    vacuum.Abort(DB_IDENTITY_B);
    bool stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_A, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFive, true);
    stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_B, VacuumTaskStatus::ABORT_DONE, 1); // only 1 time
    EXPECT_EQ(stepFive, true);
    stepFive = CheckVacuumTaskStatus(vacuum, DB_IDENTITY_C, VacuumTaskStatus::FINISH, 1); // only 1 time
    EXPECT_EQ(stepFive, true);
}
