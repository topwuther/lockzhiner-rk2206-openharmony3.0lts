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

#include "xcollie_inner.h"

#include <future>
#include <pthread.h>
#include <unistd.h>

#include "hisysevent.h"

#include "xcollie_define.h"
#include "xcollie_utils.h"

namespace OHOS {
namespace HiviewDFX {
XCollieInner::XCollieInner()
    : timerRing_(nullptr), threadChecker_(nullptr), lockChecker_(nullptr),
      threadLoop_(nullptr), checkStatus_(CheckStatus::COMPLETED), startTime_(0),
      lockCheckResult_(false), thread_(nullptr), exitThread_(0),
      checkerInterval_(STACK_INTERVAL), recovery_(true), cntCallback_(0), timeCallback_(0)
{
}

XCollieInner::~XCollieInner()
{
    Stop();
    lockChecker_ = nullptr;
    threadChecker_ = nullptr;
}

void XCollieInner::StopChecker()
{
    if (thread_ != nullptr) {
        exitThread_++;
    }
    if (threadLoop_ != nullptr) {
        exitThread_++;
    }
    if (exitThread_ == 0) {
        return;
    }
    while (exitThread_ > 0) {
        {
            std::unique_lock<std::mutex> lock(lock_);
            xcollieCheckers_.clear();
            condition_.notify_all();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); /* 200: wait thread exit */
    }
    if (thread_ != nullptr && thread_->joinable()) {
        thread_->join();
        thread_ = nullptr;
    }
    if (threadLoop_ != nullptr && threadLoop_->joinable()) {
        threadLoop_->join();
        threadLoop_ = nullptr;
    }
}

void XCollieInner::Stop()
{
    /* when thread exit, timerRing_ exit safely. this scene used in test case. */
    if (timerRing_ != nullptr) {
        while (timerRing_->IsRunning()) {
            timerRing_->TryNotify();
            timerRing_->NotifyExitAsync();
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); /* 200: wait thread exit */
        }
        timerRing_ = nullptr;
    }
    StopChecker();
    XCOLLIE_LOGE("XCollieInner::~XCollieInner exit...");
}

bool XCollieInner::Start(bool isWatchdog)
{
    /* if watchdog, then start assist thread */
    if (isWatchdog) {
        if (exitThread_ > 0) {
            XCOLLIE_LOGE("watchdog is stopping");
            return false;
        }

        if (thread_ == nullptr) {
            thread_ = std::make_unique<std::thread>(&XCollieInner::RunChecker, this);
            (void)pthread_setname_np(thread_->native_handle(), XCOLLIE_CHECKER_NAME.c_str());
        }
        if (threadLoop_ == nullptr) {
            checkStatus_ = CheckStatus::COMPLETED;
            threadLoop_ = std::make_unique<std::thread>(&XCollieInner::CheckResult, this);
            (void)pthread_setname_np(threadLoop_->native_handle(), XCOLLIE_LOOP_NAME.c_str());
        }
        if ((thread_ == nullptr) || (threadLoop_ == nullptr)) {
            XCOLLIE_LOGE("XCollieInner: start thread failed");
            return false;
        }
    } else {
        /* init timer */
        if (timerRing_ == nullptr) {
            timerRing_ = std::make_unique<TimerRing>();
            if (timerRing_ == nullptr) {
                XCOLLIE_LOGE("XCollieInner:new TimerRing failed");
                return false;
            }
            if (timerRing_->Start(timerRing_->GetName()) != ThreadStatus::OK) {
                XCOLLIE_LOGE("XCollieInner:TimerRing thread failed");
                return false;
            }
        }
    }
    return true;
}

bool XCollieInner::IsCallbackLimit(unsigned int flag)
{
    bool ret = false;
    time_t startTime = time(nullptr);
    if (!(flag & XCOLLIE_FLAG_LOG)) {
        return ret;
    }
    if (timeCallback_ + XCOLLIE_CALLBACK_TIMEWIN_MAX < startTime) {
        timeCallback_ = startTime;
    } else {
        if (++cntCallback_ > XCOLLIE_CALLBACK_HISTORY_MAX) {
            ret = true;
        }
    }
    return ret;
}

void XCollieInner::SendEvent(int tid, const std::string &timerName, const std::string &keyMsg) const
{
    pid_t pid = getpid();
    gid_t gid = getgid();
    std::string cmd = "p=" + std::to_string(pid) + ",S";
    time_t curTime = time(nullptr);
    std::string sendMsg = std::string((ctime(&curTime) == nullptr) ? "" : ctime(&curTime)) + "\n" +
        keyMsg + "\ntimeout tid: " + std::to_string(tid) +
        "\ntimeout function: " + timerName;
#ifndef __XCOLLIE_OHOS__
    if (getprogname()) {
        sendMsg += "\n>>> " + std::string(getprogname()) + " <<<\n";
    } else {
        sendMsg += "\n>>> " + timerName + " <<<\n";
    }
#else
    sendMsg += "\n>>> " + timerName + " <<<\n";
#endif
    HiSysEvent::Write("FRAMEWORK", "WATCHDOG", HiSysEvent::EventType::FAULT,
        "PID", pid, "TGID", gid, "CMD", cmd, "MSG", sendMsg);
    XCOLLIE_LOGI("send event FRAMEWORK_WATCHDOG, msg=%s", keyMsg.c_str());
}

void XCollieInner::DoTimerCallback(struct InputTimerPara *task)
{
    if (task == nullptr) {
        XCOLLIE_LOGE("XCollieInner::DoTimerCallback data is null... ");
        return;
    }
    if (task->func) {
        XCOLLIE_LOGE("XCollieInner::DoTimerCallback %s callback", task->name.c_str());
        task->func(task->arg);
    }

    if (IsCallbackLimit(task->flag)) {
        XCOLLIE_LOGE("Too many callback triggered in a short time, %s skip", task->name.c_str());
        delete task;
        return;
    }
    if (task->flag & XCOLLIE_FLAG_LOG) {
        /* send to freezedetector */
        std::string msg = "timeout: " + task->name + " start at" +
            std::to_string(task->startTime) + " to check " + std::to_string(task->timeout) + "s ago";
        SendEvent(task->tid, task->name, msg);
    }
    if ((task->flag & XCOLLIE_FLAG_RECOVERY)) {
        XCOLLIE_LOGE("%s blocked, after timeout %u , will be exit", task->name.c_str(), task->timeout);
        if (recovery_) {
            std::thread exitFunc ([](unsigned int time) {
                std::this_thread::sleep_for(std::chrono::seconds(time));
                _exit(1);
            }, task->timeout);
            if (exitFunc.joinable()) {
                exitFunc.detach();
            }
        }
        XCOLLIE_LOGE("%s timeout, exit...", task->name.c_str());
    }
    delete task;
}

int XCollieInner::SetTimer(const std::string &name, unsigned int timeout,
    std::function<void(void *)> func, void *arg, unsigned int flag)
{
    std::lock_guard<std::mutex> lock(lock_);
    int id = INVALID_ID;
    /* 1. start timer ring */
    if (Start(false) == false) {
        return id;
    }

    /* 2. add timer ring task */
    auto *newTask = new InputTimerPara();
    if (newTask == nullptr) {
        return id;
    }
    newTask->name = name;
    newTask->flag = flag;
    newTask->timeout = timeout;
    newTask->tid = gettid();
    newTask->func = func;
    newTask->arg = arg;
    newTask->startTime = time(nullptr);

    struct TimerTask task = {
        .name = name,
        .timeout = timeout,
        .loop = false,
        .inputTimerPara = newTask,
    };
    task.func = [&](struct InputTimerPara *inputPara) {
        DoTimerCallback(inputPara);
    };
    id = timerRing_->AddTask(task);
    if (id == INVALID_ID) {
        delete newTask;
    }
    return id;
}

bool XCollieInner::UpdateTimer(int id, unsigned int timeout)
{
    std::lock_guard<std::mutex> lock(lock_);
    /* 1. start timer ring */
    if (Start(false) == false) {
        return false;
    }
    return timerRing_->UpdateTask(id, timeout);
}

void XCollieInner::DestroyTimer(const int id)
{
    auto *inputTimer = timerRing_->CancelTask(id);
    if (inputTimer == nullptr) {
        return;
    }
    delete inputTimer;
}

void XCollieInner::CancelTimer(int id)
{
    std::lock_guard<std::mutex> lock(lock_);
    /* 1. start timer ring */
    if (Start(false) == false) {
        return;
    }
    DestroyTimer(id);
}

void XCollieInner::RegisterXCollieChecker(const sptr<XCollieChecker> &checker, unsigned int type)
{
    /* param check */
    if (checker == nullptr) {
        XCOLLIE_LOGE("register failed, checker is null");
        return;
    }
    if (!(type & XCOLLIE_LOCK) && !(type & XCOLLIE_THREAD)) {
        XCOLLIE_LOGE("register failed, type %u invalid", type);
        return;
    }

    std::lock_guard<std::mutex> lock(lock_);
    /* 1. start timer ring */
    if (Start(true) == false) {
        return;
    }

    /* 2. add timeout, check */
    xcollieCheckers_[checker] = type;
}

void XCollieInner::UnRegisterXCollieChecker(const sptr<XCollieChecker> &checker)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto search = xcollieCheckers_.find(checker);
        if (search == xcollieCheckers_.end()) {
            return;
        }
        xcollieCheckers_.erase(search);
        if (threadChecker_ == checker) {
            threadChecker_ = nullptr;
        }
        if (lockChecker_ == checker) {
            lockChecker_ = nullptr;
        }
    }
    if (xcollieCheckers_.size() == 0) {
        StopChecker();
    }
}

int XCollieInner::StartCheckService()
{
    lockChecker_ = nullptr;
    threadChecker_ = nullptr;
    std::map<sptr<XCollieChecker>, unsigned int> checkers;
    {
        std::lock_guard<std::mutex> lock(lock_);
        checkers = xcollieCheckers_;
        lockCheckResult_ = false;
    }

    /* start check thread block */
    for (const auto &it : checkers) {
        unsigned int type = it.second;
        if (type & XCOLLIE_THREAD) {
            sptr<XCollieChecker> checker = it.first;
            checker->SetThreadBlockResult(false);
            checker->CheckThreadBlock();
        }
    }

    /* start check lock */
    for (const auto &it : checkers) {
        unsigned int type = it.second;
        if (type & XCOLLIE_LOCK) {
            lockChecker_ = it.first;
            lockChecker_->CheckLock();
        }
    }

    lockChecker_ = nullptr;
    lockCheckResult_ = true;
    return 0;
}

void XCollieInner::RunChecker()
{
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(lock_);
            condition_.wait(lock);
            if (exitThread_ > 0) {
                exitThread_--;
                break;
            }
        }
        StartCheckService();
    }
    XCOLLIE_LOGE("XCollieInner::RunChecker thread exit...");
}

void XCollieInner::CheckResult()
{
    for (;;) {
        /* per interval check services */
        std::unique_lock<std::mutex> lock(lock_);
        (void)condition_.wait_for(lock, std::chrono::seconds(checkerInterval_), [&]() {
            return exitThread_ > 0;
        });
        if (exitThread_ > 0) {
            exitThread_--;
            break;
        }
        if ((checkStatus_ == CheckStatus::COMPLETED) || (GetThreadBlockResult() && lockCheckResult_)) {
            startTime_ = time(nullptr);
            checkStatus_ = CheckStatus::WAITING;
            condition_.notify_all();
            continue;
        }
        /* check result */
        std::string name;
        std::string msg = GetBlockServiceMsg(name);
        if (checkStatus_ == CheckStatus::WAITING) {
            checkStatus_ = CheckStatus::WAITED_HALF;
            double duration = difftime(time(nullptr), startTime_);
            XCOLLIE_LOGW("%s, duration %{public}.0lf", msg.c_str(), duration);
            /* send to freezedetector start dump */
            SendEvent(getpid(), name, "watchdog: " + msg + " duration " + std::to_string(duration));
            continue;
        }
        if (checkStatus_ == CheckStatus::WAITED_HALF) {
            XCOLLIE_LOGE("%s, duration %{public}.01f, will restart", msg.c_str(), difftime(time(nullptr), startTime_));
            if (recovery_) {
                std::thread exitFunc([]() {
                    _exit(1);
                });
                if (exitFunc.joinable()) {
                    exitFunc.detach();
                }
            }
        }
    }
}

std::string XCollieInner::GetBlockServiceMsg(std::string &name) const
{
    std::string msg;
    if (threadChecker_ != nullptr) {
        name = threadChecker_->GetCheckerName();
        msg = "Blocked in main thread on service " + name;
    } else if (lockChecker_ != nullptr) {
        name = lockChecker_->GetCheckerName();
        msg = "Blocked in lock on service " + name;
    } else {
        name = "unknown";
        msg = "Blocked in lock on unknown service";
    }
    return msg;
}

bool XCollieInner::GetThreadBlockResult()
{
    for (const auto &it : xcollieCheckers_) {
        unsigned int type = it.second;
        sptr<XCollieChecker> checker = it.first;
        if ((type & XCOLLIE_THREAD) && (checker->GetThreadBlockResult() == false)) {
            threadChecker_ = checker;
            return false;
        }
    }
    return true;
}

std::string XCollieInner::GetBlockdServiceName()
{
    std::lock_guard<std::mutex> lock(lock_);

    if (threadChecker_ != nullptr) {
        return threadChecker_->GetCheckerName();
    } else if (lockChecker_ != nullptr) {
        return lockChecker_->GetCheckerName();
    } else {
        return std::string("");
    }
}
} // end of namespace HiviewDFX
} // end of namespace OHOS
