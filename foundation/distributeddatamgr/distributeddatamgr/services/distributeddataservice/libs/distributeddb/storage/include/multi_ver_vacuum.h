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

#ifndef MULTI_VER_VACUUM_H
#define MULTI_VER_VACUUM_H

#ifndef OMIT_MULTI_VER
#include <map>
#include <list>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <cstdint>
#include <condition_variable>
#include "macro_utils.h"
#include "multi_ver_vacuum_executor.h"

namespace DistributedDB {
enum class VacuumTaskStatus {
    RUN_WAIT,
    RUN_NING,
    PAUSE_WAIT,
    PAUSE_DONE,
    ABORT_WAIT,
    ABORT_DONE,
    FINISH,
};

struct VacuumTaskContext {
    VacuumTaskStatus status = VacuumTaskStatus::RUN_WAIT;
    bool launchErrorHappen = false;
    bool autoRelaunchOnce = false;
    bool immediatelyRelaunchable = true;
    uint64_t runWaitOrder = 0;
    uint64_t pauseNeedCount = 0;
    MultiVerVacuumExecutor *databaseHandle = nullptr;
    // Information to conduct the vacuum task and record the progress.
    // When in RUN_NING, PAUSE_WAIT, ABORT_WAIT status, no other thread except the only one background task thread
    // will access and change these field, so there is no concurrency risk accessing and changing it without a lock.
    std::list<MultiVerCommitInfo> leftBranchCommits;
    std::list<MultiVerCommitInfo> rightBranchCommits;
    std::list<MultiVerRecordInfo> vacuumNeedRecords;
    std::list<MultiVerRecordInfo> shadowRecords;
    bool isTransactionStarted = false;
};

// Pause and Continue should be call in pair for the same database. If Pause called more than Continue, then the task
// will stay in "inactive status". If Continue called more than Pause, then the excessive Continue will be neglected.
// It expected that every Pause following by a Continue sooner or later before Abort is called.
class MultiVerVacuum {
public:
    // Default is enable, should be called at very first to change it, take effect only on instances created after it.
    static void Enable(bool isEnable);

    DISABLE_COPY_ASSIGN_MOVE(MultiVerVacuum);

    // Call it when database firstly open
    int Launch(const std::string &dbIdentifier, MultiVerVacuumExecutor *dbHandle);

    // Call it before database do write operation, it may block for a while if task is running
    // It is guaranteed that no write transaction of this database will be used by vacuum after pause return
    int Pause(const std::string &dbIdentifier);

    // Call it after database do write operation, and write transaction of this database may be used by vacuum.
    // If autoRelaunchOnce true, the task will relaunch itself when finish, set false will not override previous true
    int Continue(const std::string &dbIdentifier, bool autoRelaunchOnce);

    // Call it when database is about to close, it may block for a while if task is running
    int Abort(const std::string &dbIdentifier);

    // Call it when observer_callback done or release an snapshot, to relaunch with some newer vacuumable commits
    int AutoRelaunchOnce(const std::string &dbIdentifier);

    int QueryStatus(const std::string &dbIdentifier, VacuumTaskStatus &outStatus) const;

    MultiVerVacuum() = default;
    ~MultiVerVacuum();
private:
    void VacuumTaskExecutor();
    void ExecuteSpecificVacuumTask(VacuumTaskContext &inTask);

    int DealWithLeftBranchCommit(VacuumTaskContext &inTask);
    int DealWithLeftBranchVacuumNeedRecord(VacuumTaskContext &inTask);
    int DealWithLeftBranchShadowRecord(VacuumTaskContext &inTask);
    int DealWithRightBranchCommit(VacuumTaskContext &inTask);
    int DealWithRightBranchVacuumNeedRecord(VacuumTaskContext &inTask);

    // Reducing duplicated code by merging similar code procedure of "DealLeftCommit" and "DealRightCommit"
    int DoDealCommitOfLeftOrRight(VacuumTaskContext &inTask, std::list<MultiVerCommitInfo> &commitList, bool isLeft);
    // Reducing duplicated code by merging similar code procedure of "DealLeftShadow" and "DealRightVacuumNeed"
    int DoDeleteRecordOfLeftShadowOrRightVacuumNeed(VacuumTaskContext &inTask,
        std::list<MultiVerRecordInfo> &recordList);
    // Only for reducing duplicated code
    void DoRollBackAndFinish(VacuumTaskContext &inTask);
    int DoCommitAndQuitIfWaitStatusObserved(VacuumTaskContext &inTask); // Return E_OK continue otherwise quit

    // Call this immediately before changing the database
    int StartTransactionIfNotYet(VacuumTaskContext &inTask);
    // Call this immediately before normally quit
    int CommitTransactionIfNeed(VacuumTaskContext &inTask);
    // Call this immediately before abnormally quit, return void since already in abnormal.
    void RollBackTransactionIfNeed(VacuumTaskContext &inTask);

    // All these following functions should be protected by the vacuumTaskMutex_ when called
    void FinishVaccumTask(VacuumTaskContext &inTask);
    void RelaunchVacuumTask(VacuumTaskContext &inTask);
    void AbortVacuumTask(VacuumTaskContext &inTask);
    void ResetNodeAndRecordContextInfo(VacuumTaskContext &inTask);
    int SearchVacuumTaskToExecute(std::string &outDbIdentifier);
    void ActivateBackgroundVacuumTaskExecution();
    void IncPauseNeedCount(VacuumTaskContext &inTask);
    void DecPauseNeedCount(VacuumTaskContext &inTask);
    bool IsPauseNotNeed(VacuumTaskContext &inTask);

    static std::atomic<bool> enabled_;

    mutable std::mutex vacuumTaskMutex_;
    std::condition_variable vacuumTaskCv_;
    uint64_t incRunWaitOrder_ = 0;
    std::map<std::string, VacuumTaskContext> dbMapVacuumTask_;
    // the search of available vacuumtask, the change of isBackgroundVacuumTaskInExecution_, and the activation of
    // background execution, should all be protected by vacuumTaskMutex_, In order to avoid malfunction caused by
    // concurrency situation which is described below:
    // 1:Background search vacuumtask return none so decided to exit.
    // 2:Foreground make vacuumtask available.
    // 3:Foreground check isBackgroundVacuumTaskInExecution_ true so decided to nothing.
    // 4:Background set isBackgroundVacuumTaskInExecution_ to false and exit.
    // In this situation, no background execution running with available vacuumtask needs to be done.
    bool isBackgroundVacuumTaskInExecution_ = false;
};
} // namespace DistributedDB

#endif // MULTI_VER_VACUUM_H
#endif
