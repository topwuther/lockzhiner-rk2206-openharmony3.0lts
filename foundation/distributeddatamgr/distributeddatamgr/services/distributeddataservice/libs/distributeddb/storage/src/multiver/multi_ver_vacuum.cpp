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

#ifndef OMIT_MULTI_VER
#include "multi_ver_vacuum.h"

#include <string>
#include <vector>
#include <cstdint>

#include "db_errno.h"
#include "db_common.h"
#include "log_print.h"
#include "macro_utils.h"
#include "runtime_context.h"

namespace DistributedDB {
std::atomic<bool> MultiVerVacuum::enabled_{true};

void MultiVerVacuum::Enable(bool isEnable)
{
    enabled_ = isEnable;
}

int MultiVerVacuum::Launch(const std::string &dbIdentifier, MultiVerVacuumExecutor *dbHandle)
{
    if (!enabled_) {
        LOGW("[Vacuum][Launch] Functionality Not Enabled!");
        return E_OK;
    }
    if (dbIdentifier.empty() || dbHandle == nullptr) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        dbMapVacuumTask_[dbIdentifier].runWaitOrder = incRunWaitOrder_++;
        dbMapVacuumTask_[dbIdentifier].databaseHandle = dbHandle;
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::ABORT_DONE ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH) {
        // Reset vacuum task
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::RUN_WAIT;
        dbMapVacuumTask_[dbIdentifier].launchErrorHappen = false;
        dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = false;
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = true;
        dbMapVacuumTask_[dbIdentifier].runWaitOrder = incRunWaitOrder_++;
        dbMapVacuumTask_[dbIdentifier].pauseNeedCount = 0;
        dbMapVacuumTask_[dbIdentifier].databaseHandle = dbHandle;
    } else {
        dbMapVacuumTask_[dbIdentifier].launchErrorHappen = true;
        LOGE("[Vacuum][Launch] Unexpected pre-status=%d!", static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    }
    ActivateBackgroundVacuumTaskExecution();
    return E_OK;
}

int MultiVerVacuum::Pause(const std::string &dbIdentifier)
{
    if (!enabled_) {
        return E_OK;
    }
    if (dbIdentifier.empty()) {
        return -E_INVALID_ARGS;
    }

    std::unique_lock<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        return -E_NOT_FOUND;
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::RUN_WAIT ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_DONE) {
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::PAUSE_DONE;
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = false;
        IncPauseNeedCount(dbMapVacuumTask_[dbIdentifier]);
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::RUN_NING ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_WAIT) {
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::PAUSE_WAIT;
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = false;
        IncPauseNeedCount(dbMapVacuumTask_[dbIdentifier]);
        vacuumTaskCv_.wait(vacuumTaskLockGuard, [this, &dbIdentifier] {
            // In concurrency scenario that executor is about to finish this task, the final status may be FINISH.
            // Even more, in case Abort be called immediately after task finished, the final status may be ABORT_DONE.
            return dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_DONE ||
                dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::ABORT_DONE ||
                dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH;
        });
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH) {
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = false;
        IncPauseNeedCount(dbMapVacuumTask_[dbIdentifier]);
    } else {
        LOGE("[Vacuum][Pause] Unexpected pre-status=%d!", static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    }
    return E_OK;
}

int MultiVerVacuum::Continue(const std::string &dbIdentifier, bool autoRelaunchOnce)
{
    if (!enabled_) {
        return E_OK;
    }
    if (dbIdentifier.empty()) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        return -E_NOT_FOUND;
    } else if (dbMapVacuumTask_[dbIdentifier].launchErrorHappen) {
        LOGE("[Vacuum][Continue] LaunchErrorHappen detected, pre-status=%d!",
            static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_DONE) {
        DecPauseNeedCount(dbMapVacuumTask_[dbIdentifier]);
        bool relaunchFlag = (dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce || autoRelaunchOnce);
        dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = relaunchFlag;
        // Truly continue this task only when all pause had been counteracted
        if (IsPauseNotNeed(dbMapVacuumTask_[dbIdentifier])) {
            dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::RUN_WAIT;
            dbMapVacuumTask_[dbIdentifier].runWaitOrder = incRunWaitOrder_++;
            dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = true;
            ActivateBackgroundVacuumTaskExecution();
        }
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH) {
        // Update relaunch flag first
        DecPauseNeedCount(dbMapVacuumTask_[dbIdentifier]);
        bool relaunchFlag = (dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce || autoRelaunchOnce);
        dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = relaunchFlag;
        // All pause had been counteracted, so this task is immediatelyRelaunchable, but not necessarily relaunch now.
        if (IsPauseNotNeed(dbMapVacuumTask_[dbIdentifier])) {
            dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = true;
            // Do autoRelaunch if need
            if (dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce) {
                dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::RUN_WAIT;
                dbMapVacuumTask_[dbIdentifier].runWaitOrder = incRunWaitOrder_++;
                dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = false;
                ActivateBackgroundVacuumTaskExecution();
            }
        }
    } else {
        LOGE("[Vacuum][Continue] Unexpected pre-status=%d!", static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    }
    return E_OK;
}

int MultiVerVacuum::Abort(const std::string &dbIdentifier)
{
    if (!enabled_) {
        return E_OK;
    }
    if (dbIdentifier.empty()) {
        return -E_INVALID_ARGS;
    }

    // The pauseNeedCount must be zero in RUN_WAIT and RUN_NING case, but not always zero in FINISH case.
    // If pause is called more than continue, status may be PAUSE_WAIT, PAUSE_DONE, which is not expected.
    // The pauseNeedCount, runWaitOrder and autoRelaunchOnce will be reset when launch(Not Auto) if abort normally
    std::unique_lock<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        return -E_NOT_FOUND;
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::RUN_WAIT ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_DONE ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH) {
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::ABORT_DONE;
        dbMapVacuumTask_[dbIdentifier].launchErrorHappen = false;
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = false;
        // In this place, the background will not access information of this vacuum task
        dbMapVacuumTask_[dbIdentifier].databaseHandle = nullptr;
        ResetNodeAndRecordContextInfo(dbMapVacuumTask_[dbIdentifier]);
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::RUN_NING ||
        dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::PAUSE_WAIT) {
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::ABORT_WAIT;
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable = false;
        vacuumTaskCv_.wait(vacuumTaskLockGuard, [this, &dbIdentifier] {
            // In concurrency scenario that executor is about to finish this task, the final status may be FINISH
            return dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::ABORT_DONE ||
                dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH;
        });
        // Resource is cleaned by background task, still set ABORT_DONE and reset launchErrorHappen and databaseHandle.
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::ABORT_DONE;
        dbMapVacuumTask_[dbIdentifier].launchErrorHappen = false;
        dbMapVacuumTask_[dbIdentifier].databaseHandle = nullptr;
    } else {
        LOGE("[Vacuum][Abort] Unexpected pre-status=%d!", static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    }
    return E_OK;
}

int MultiVerVacuum::AutoRelaunchOnce(const std::string &dbIdentifier)
{
    if (!enabled_) {
        return E_OK;
    }
    if (dbIdentifier.empty()) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        return -E_NOT_FOUND;
    } else if (dbMapVacuumTask_[dbIdentifier].launchErrorHappen) {
        LOGE("[Vacuum][AutoRelaunch] LaunchErrorHappen detected, pre-status=%d!",
            static_cast<int>(dbMapVacuumTask_[dbIdentifier].status));
        return -E_NOT_PERMIT;
    } else if (dbMapVacuumTask_[dbIdentifier].status == VacuumTaskStatus::FINISH &&
        dbMapVacuumTask_[dbIdentifier].immediatelyRelaunchable) {
        // Relaunch this task immediately
        dbMapVacuumTask_[dbIdentifier].status = VacuumTaskStatus::RUN_WAIT;
        dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = false;
        dbMapVacuumTask_[dbIdentifier].runWaitOrder = incRunWaitOrder_++;
    } else {
        // Set flag true in order to Relaunch this task once when it finish
        dbMapVacuumTask_[dbIdentifier].autoRelaunchOnce = true;
    }
    ActivateBackgroundVacuumTaskExecution();
    return E_OK;
}

int MultiVerVacuum::QueryStatus(const std::string &dbIdentifier, VacuumTaskStatus &outStatus) const
{
    if (dbIdentifier.empty()) {
        return -E_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (dbMapVacuumTask_.count(dbIdentifier) == 0) {
        return -E_NOT_FOUND;
    }

    outStatus = dbMapVacuumTask_.at(dbIdentifier).status;
    return E_OK;
}

MultiVerVacuum::~MultiVerVacuum()
{
    // Mainly for stop the background task, resources automatically clean by this deconstruction
    std::unique_lock<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    for (auto &each : dbMapVacuumTask_) {
        if (each.second.status == VacuumTaskStatus::RUN_WAIT || each.second.status == VacuumTaskStatus::PAUSE_DONE) {
            // For RUN_WAIT and PAUSE_DONE, change to ABORT_DONE
            each.second.status = VacuumTaskStatus::ABORT_DONE;
        } else if (each.second.status == VacuumTaskStatus::RUN_NING ||
            each.second.status == VacuumTaskStatus::PAUSE_WAIT) {
            // For RUN_NING and PAUSE_WAIT, change to ABORT_WAIT
            each.second.status = VacuumTaskStatus::ABORT_WAIT;
        }
        // For ABORT_WAIT, ABORT_DONE and FINISH, remain as it is.
    }
    // Wait for background task to quit
    vacuumTaskCv_.wait(vacuumTaskLockGuard, [this] {
        return !isBackgroundVacuumTaskInExecution_;
    });
}

void MultiVerVacuum::VacuumTaskExecutor()
{
    // Endless loop until nothing to do
    while (true) {
        std::string nextDatabase;
        {
            std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
            int errCode = SearchVacuumTaskToExecute(nextDatabase);
            if (errCode != E_OK) {
                LOGI("[Vacuum][Executor] No available task to execute, about to quit.");
                isBackgroundVacuumTaskInExecution_ = false;
                // Awake the deconstruction that background thread is about to quit
                vacuumTaskCv_.notify_all();
                return;
            }
        }
        // No thread will remove entry from dbMapVacuumTask_, so here is concurrency safe.
        LOGI("[Vacuum][Executor] Execute vacuum task for database=%s.", nextDatabase.c_str());
        ExecuteSpecificVacuumTask(dbMapVacuumTask_[nextDatabase]);
        // Awake foreground thread at this task switch point
        vacuumTaskCv_.notify_all();
    }
}

void MultiVerVacuum::ExecuteSpecificVacuumTask(VacuumTaskContext &inTask)
{
    // No other thread will access handle, node and record field of a RUN_NING, PAUSE_WAIT, ABORT_WAIT status task
    // So it is concurrency safe to access or change these field without protection of lockguard
    if (inTask.leftBranchCommits.empty() && inTask.rightBranchCommits.empty()) {
        // Newly launched task
        int errCode = inTask.databaseHandle->GetVacuumAbleCommits(inTask.leftBranchCommits, inTask.rightBranchCommits);
        if (errCode != E_OK) {
            LOGE("[Vacuum][Execute] GetVacuumAbleCommits fail, errCode=%d.", errCode);
            std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
            FinishVaccumTask(inTask);
            return;
        }
    }

    // Vacuum left branch first, since record of left branch will be synced out, more urgently
    while (!inTask.leftBranchCommits.empty()) {
        int errCode = DealWithLeftBranchCommit(inTask);
        if (errCode != E_OK) {
            return;
        }
    }
    LOGD("[Vacuum][Execute] All vacuum able commits of left branch have been dealt with for this database!");

    // Vacuum right branch later, since record of right branch will not be synced out, not so urgent
    while (!inTask.rightBranchCommits.empty()) {
        int errCode = DealWithRightBranchCommit(inTask);
        if (errCode != E_OK) {
            return;
        }
    }
    LOGD("[Vacuum][Execute] All vacuum able commits of right branch have been dealt with for this database!");

    // Commit changes before finish this task, if fail, just finish it(commit fail auto rollback)
    int errCode = CommitTransactionIfNeed(inTask);
    if (errCode != E_OK) {
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        FinishVaccumTask(inTask);
        return;
    }

    // Every commit of this task has been treated, consider finish or relaunch the task
    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    if (inTask.status == VacuumTaskStatus::RUN_NING && inTask.autoRelaunchOnce) {
        RelaunchVacuumTask(inTask);
    } else {
        // If in PAUSE_WAIT or ABORT_WAIT status, shall not relaunch it, just finish it to make sure it be unactive
        // The autoRelaunchOnce will be set false, if need relaunch, the continue operation will set it true again
        FinishVaccumTask(inTask);
    }
}

int MultiVerVacuum::DealWithLeftBranchCommit(VacuumTaskContext &inTask)
{
    return DoDealCommitOfLeftOrRight(inTask, inTask.leftBranchCommits, true);
}

int MultiVerVacuum::DealWithLeftBranchVacuumNeedRecord(VacuumTaskContext &inTask)
{
    int errCode = DoCommitAndQuitIfWaitStatusObserved(inTask);
    if (errCode != E_OK) {
        return errCode;
    }
    // No other thread will access handle, node and record field of a RUN_NING, PAUSE_WAIT, ABORT_WAIT status task
    // So it is concurrency safe to access or change these field without protection of lockguard
    const MultiVerRecordInfo &record = inTask.vacuumNeedRecords.front();
    LOGD("[Vacuum][DealLeftRecord] Type=%d, Version=%llu, HashKey=%s.", record.type, ULL(record.version),
        VEC_TO_STR(record.hashKey));
    if (inTask.shadowRecords.empty()) {
        if (record.type == RecordType::CLEAR) {
            errCode = inTask.databaseHandle->GetShadowRecordsOfClearTypeRecord(record.version, record.hashKey,
                inTask.shadowRecords);
        } else {
            errCode = inTask.databaseHandle->GetShadowRecordsOfNonClearTypeRecord(record.version, record.hashKey,
                inTask.shadowRecords);
        }
        if (errCode != E_OK) {
            LOGE("[Vacuum][DealLeftRecord] GetShadowRecords fail, Type=%d, Version=%llu, HashKey=%s, errCode=%d.",
                record.type, ULL(record.version), VEC_TO_STR(record.hashKey), errCode);
            DoRollBackAndFinish(inTask);
            return errCode;
        }
    }

    while (!inTask.shadowRecords.empty()) {
        errCode = DealWithLeftBranchShadowRecord(inTask);
        if (errCode != E_OK) {
            return errCode;
        }
    }

    // Every shadowRecords of this vacuumNeedRecord has been treated, mark this vacuumNeedRecord as vacuum done
    errCode = StartTransactionIfNotYet(inTask);
    if (errCode != E_OK) {
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        FinishVaccumTask(inTask);
        return errCode;
    }
    errCode = inTask.databaseHandle->MarkRecordAsVacuumDone(record.version, record.hashKey);
    if (errCode != E_OK) {
        LOGE("[Vacuum][DealLeftRecord] MarkRecordAsVacuumDone fail, Type=%d, Version=%llu, HashKey=%s, errCode=%d.",
            record.type, ULL(record.version), VEC_TO_STR(record.hashKey), errCode);
        DoRollBackAndFinish(inTask);
        return errCode;
    }
    // Pop out this vacuumNeedRecord
    inTask.vacuumNeedRecords.pop_front();
    return E_OK;
}

int MultiVerVacuum::DealWithLeftBranchShadowRecord(VacuumTaskContext &inTask)
{
    return DoDeleteRecordOfLeftShadowOrRightVacuumNeed(inTask, inTask.shadowRecords);
}

int MultiVerVacuum::DealWithRightBranchCommit(VacuumTaskContext &inTask)
{
    return DoDealCommitOfLeftOrRight(inTask, inTask.rightBranchCommits, false);
}

int MultiVerVacuum::DealWithRightBranchVacuumNeedRecord(VacuumTaskContext &inTask)
{
    return DoDeleteRecordOfLeftShadowOrRightVacuumNeed(inTask, inTask.vacuumNeedRecords);
}

int MultiVerVacuum::DoDealCommitOfLeftOrRight(VacuumTaskContext &inTask, std::list<MultiVerCommitInfo> &commitList,
    bool isLeft)
{
    int errCode = DoCommitAndQuitIfWaitStatusObserved(inTask);
    if (errCode != E_OK) {
        return errCode;
    }
    // No other thread will access handle, node and record field of a RUN_NING, PAUSE_WAIT, ABORT_WAIT status task
    // So it is concurrency safe to access or change these field without protection of lockguard
    const MultiVerCommitInfo &commit = commitList.front();
    LOGD("[Vacuum][DoDealCommit] Version=%llu, CommitId=%s, isLeft=%d.", ULL(commit.version),
        VEC_TO_STR(commit.commitId), isLeft);
    if (inTask.vacuumNeedRecords.empty()) {
        errCode = inTask.databaseHandle->GetVacuumNeedRecordsByVersion(commit.version, inTask.vacuumNeedRecords);
        if (errCode != E_OK) {
            LOGE("[Vacuum][DoDealCommit] GetVacuumNeedRecordsByVersion fail, Version=%llu, CommitId=%s, isLeft=%d, "
                "errCode=%d.", ULL(commit.version), VEC_TO_STR(commit.commitId), isLeft, errCode);
            DoRollBackAndFinish(inTask);
            return errCode;
        }
    }

    while (!inTask.vacuumNeedRecords.empty()) {
        if (isLeft) {
            errCode = DealWithLeftBranchVacuumNeedRecord(inTask);
        } else {
            errCode = DealWithRightBranchVacuumNeedRecord(inTask);
        }
        if (errCode != E_OK) {
            return errCode;
        }
    }

    // Every vacuumNeedRecords of this commit has been treated, mark this commit as vacuum done
    errCode = StartTransactionIfNotYet(inTask);
    if (errCode != E_OK) {
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        FinishVaccumTask(inTask);
        return errCode;
    }
    errCode = inTask.databaseHandle->MarkCommitAsVacuumDone(commit.commitId);
    if (errCode != E_OK) {
        LOGE("[Vacuum][DoDealCommit] MarkCommitAsVacuumDone fail, Version=%llu, CommitId=%s, isLeft=%d, errCode=%d.",
            ULL(commit.version), VEC_TO_STR(commit.commitId), isLeft, errCode);
        DoRollBackAndFinish(inTask);
        return errCode;
    }
    // Pop out this commit
    commitList.pop_front();
    return E_OK;
}

int MultiVerVacuum::DoDeleteRecordOfLeftShadowOrRightVacuumNeed(VacuumTaskContext &inTask,
    std::list<MultiVerRecordInfo> &recordList)
{
    int errCode = DoCommitAndQuitIfWaitStatusObserved(inTask);
    if (errCode != E_OK) {
        return errCode;
    }
    // No other thread will access handle, node and record field of a RUN_NING, PAUSE_WAIT, ABORT_WAIT status task
    // So it is concurrency safe to access or change these field without protection of lockguard
    const MultiVerRecordInfo &record = recordList.front();
    LOGD("[Vacuum][DoDeleteRecord] Type=%d, Version=%llu, HashKey=%s.", record.type, ULL(record.version),
        VEC_TO_STR(record.hashKey));
    errCode = StartTransactionIfNotYet(inTask);
    if (errCode != E_OK) {
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        FinishVaccumTask(inTask);
        return errCode;
    }
    errCode = inTask.databaseHandle->DeleteRecordTotally(record.version, record.hashKey);
    if (errCode != E_OK) {
        LOGE("[Vacuum][DoDeleteRecord] DeleteRecordTotally fail, Type=%d, Version=%llu, HashKey=%s, errCode=%d.",
            record.type, ULL(record.version), VEC_TO_STR(record.hashKey), errCode);
        DoRollBackAndFinish(inTask);
        return errCode;
    }
    // Pop out this shadowRecord or vacuumNeedRecord
    recordList.pop_front();
    return E_OK;
}

void MultiVerVacuum::DoRollBackAndFinish(VacuumTaskContext &inTask)
{
    RollBackTransactionIfNeed(inTask);
    std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
    FinishVaccumTask(inTask);
}

int MultiVerVacuum::DoCommitAndQuitIfWaitStatusObserved(VacuumTaskContext &inTask)
{
    bool waitStatusObserved = false;
    {
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        if (inTask.status == VacuumTaskStatus::PAUSE_WAIT || inTask.status == VacuumTaskStatus::ABORT_WAIT) {
            waitStatusObserved = true;
        }
    }
    // Only this TaskThread will change a PAUSE_WAIT or ABORT_WAIT status to other status
    // So here during the gap of miss-lockguard-protection, the status of this inTask will not change
    if (waitStatusObserved) {
        // CommitTransactionIfNeed may be an time cost operation, should not be called within the range of lockguard
        int errCode = CommitTransactionIfNeed(inTask);
        // Change status operation should be protected within the lockguard
        std::lock_guard<std::mutex> vacuumTaskLockGuard(vacuumTaskMutex_);
        if (errCode != E_OK) {
            // If commit fail, just finish this task(commit fail auto rollback)
            FinishVaccumTask(inTask);
            return errCode;
        }
        if (inTask.status == VacuumTaskStatus::ABORT_WAIT) {
            AbortVacuumTask(inTask);
            return -E_TASK_BREAK_OFF;
        }
        // Nor commit fail, nor Abort_wait case, here is Pause_wait Case, just set status to Pause_done
        inTask.status = VacuumTaskStatus::PAUSE_DONE;
        return -E_TASK_BREAK_OFF;
    }
    return E_OK;
}

int MultiVerVacuum::StartTransactionIfNotYet(VacuumTaskContext &inTask)
{
    if (!inTask.isTransactionStarted) {
        int errCode = inTask.databaseHandle->StartTransactionForVacuum();
        if (errCode != E_OK) {
            LOGE("[Vacuum][StartTransact] StartTransactionForVacuum fail, errCode=%d.", errCode);
            return errCode;
        }
        inTask.isTransactionStarted = true;
    }
    return E_OK;
}

int MultiVerVacuum::CommitTransactionIfNeed(VacuumTaskContext &inTask)
{
    if (inTask.isTransactionStarted) {
        // Whether CommitTransactionForVacuum fail or not, the transaction is ended.
        inTask.isTransactionStarted = false;
        int errCode = inTask.databaseHandle->CommitTransactionForVacuum();
        if (errCode != E_OK) {
            LOGE("[Vacuum][CommitTransact] CommitTransactionForVacuum fail, errCode=%d.", errCode);
            return errCode;
        }
    }
    return E_OK;
}

void MultiVerVacuum::RollBackTransactionIfNeed(VacuumTaskContext &inTask)
{
    if (inTask.isTransactionStarted) {
        // Whether RollBackTransactionForVacuum fail or not, the transaction is ended.
        inTask.isTransactionStarted = false;
        int errCode = inTask.databaseHandle->RollBackTransactionForVacuum();
        if (errCode != E_OK) {
            LOGE("[Vacuum][RollBackTransact] RollBackTransactionForVacuum fail, errCode=%d.", errCode);
        }
    }
}

void MultiVerVacuum::FinishVaccumTask(VacuumTaskContext &inTask)
{
    inTask.status = VacuumTaskStatus::FINISH;
    // It is OK to reset the autoRelaunchOnce. Since this is called when this task is RUN_NING status, all pause to
    // this task will block and wait, and all continue to this task happens after we reset the autoRelaunchOnce
    inTask.autoRelaunchOnce = false;
    // Do not reset the databaseHandle while finish a task, because it will be reused after autoRelaunch
    ResetNodeAndRecordContextInfo(inTask);
}

void MultiVerVacuum::RelaunchVacuumTask(VacuumTaskContext &inTask)
{
    inTask.status = VacuumTaskStatus::RUN_WAIT;
    inTask.runWaitOrder = incRunWaitOrder_++; // Queue at the back
    inTask.autoRelaunchOnce = false;
    // Obviously can not reset the databaseHandle while relaunch a task
    ResetNodeAndRecordContextInfo(inTask);
}

void MultiVerVacuum::AbortVacuumTask(VacuumTaskContext &inTask)
{
    inTask.status = VacuumTaskStatus::ABORT_DONE;
    inTask.autoRelaunchOnce = false;
    inTask.databaseHandle = nullptr; // reset handle in abort case
    ResetNodeAndRecordContextInfo(inTask);
}

void MultiVerVacuum::ResetNodeAndRecordContextInfo(VacuumTaskContext &inTask)
{
    inTask.leftBranchCommits.clear();
    inTask.rightBranchCommits.clear();
    inTask.vacuumNeedRecords.clear();
    inTask.shadowRecords.clear();
    inTask.isTransactionStarted = false;
}

int MultiVerVacuum::SearchVacuumTaskToExecute(std::string &outDbIdentifier)
{
    // Find a vacuum task with the smallest runWaitOrder among tasks that is in RUN_WAIT Status(Except In Error).
    uint64_t minRunWaitOrder = UINT64_MAX;
    for (auto &eachTask : dbMapVacuumTask_) {
        LOGD("[Vacuum][Search] db=%s, status=%d, error=%d, relaunch=%d, immediate=%d, runWait=%llu, pauseCount=%llu.",
            eachTask.first.c_str(), static_cast<int>(eachTask.second.status), eachTask.second.launchErrorHappen,
            eachTask.second.autoRelaunchOnce, eachTask.second.immediatelyRelaunchable,
            ULL(eachTask.second.runWaitOrder), ULL(eachTask.second.pauseNeedCount));
        if (eachTask.second.status == VacuumTaskStatus::RUN_WAIT && !eachTask.second.launchErrorHappen) {
            if (eachTask.second.runWaitOrder < minRunWaitOrder) {
                minRunWaitOrder = eachTask.second.runWaitOrder;
                outDbIdentifier = eachTask.first;
            }
        }
    }
    if (!outDbIdentifier.empty()) {
        dbMapVacuumTask_[outDbIdentifier].status = VacuumTaskStatus::RUN_NING;
        return E_OK;
    } else {
        return -E_NOT_FOUND;
    }
}

void MultiVerVacuum::ActivateBackgroundVacuumTaskExecution()
{
    if (!isBackgroundVacuumTaskInExecution_) {
        TaskAction backgroundTask = [this]() {
            LOGI("[Vacuum][Activate] Begin Background Execution.");
            VacuumTaskExecutor();
            LOGI("[Vacuum][Activate] End Background Execution.");
        };
        int errCode = RuntimeContext::GetInstance()->ScheduleTask(backgroundTask);
        if (errCode != E_OK) {
            LOGE("[Vacuum][Activate] ScheduleTask failed, errCode = %d.", errCode);
            return;
        }
        isBackgroundVacuumTaskInExecution_ = true;
    }
}

void MultiVerVacuum::IncPauseNeedCount(VacuumTaskContext &inTask)
{
    inTask.pauseNeedCount++;
}

void MultiVerVacuum::DecPauseNeedCount(VacuumTaskContext &inTask)
{
    if (inTask.pauseNeedCount == 0) {
        LOGE("[Vacuum][DecPause] PauseNeedCount Zero Before Decrease.");
        return;
    }
    inTask.pauseNeedCount--;
}

bool MultiVerVacuum::IsPauseNotNeed(VacuumTaskContext &inTask)
{
    return inTask.pauseNeedCount == 0;
}
} // namespace DistributedDB
#endif
