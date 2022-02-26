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

#ifndef RELIABILITY_XCOLLIE_INNER_H
#define RELIABILITY_XCOLLIE_INNER_H

#include <time.h>

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "refbase.h"
#include "singleton.h"

#include "timer_ring.h"
#include "xcollie_checker.h"

namespace OHOS {
namespace HiviewDFX {
enum class CheckStatus {
    COMPLETED = 0,
    WAITING = 1,
    WAITED_HALF = 2,
};

static const std::string XCOLLIE_CHECKER_NAME = "XCollie";
static const std::string XCOLLIE_LOOP_NAME = "XCollieLoop";

class XCollieInner : public Singleton<XCollieInner> {
    DECLARE_SINGLETON(XCollieInner);
public:
    static const int XCOLLIE_CALLBACK_HISTORY_MAX = 5;
    static const int XCOLLIE_CALLBACK_TIMEWIN_MAX = 60;
    static constexpr int STACK_INTERVAL = 30;
    void RegisterXCollieChecker(const sptr<XCollieChecker> &checker, unsigned int type);
    int SetTimer(const std::string &name, unsigned int timeout,
        std::function<void (void *)> func, void *arg, unsigned int flag);
    void CancelTimer(int id);
    bool UpdateTimer(int id, unsigned int timeout);

    void SetCheckerInterval(int interval)
    {
        checkerInterval_ = interval;
    }
    void SetRecoveryFlag(bool recovery)
    {
        recovery_ = recovery;
    }
    void SetCheckStatus(CheckStatus status)
    {
        checkStatus_ = status;
    }
    std::string GetBlockdServiceName();
    void UnRegisterXCollieChecker(const sptr<XCollieChecker> &checker);

private:
    void RunChecker();
    bool Start(bool isWatchdog);
    int StartCheckService();
    std::string GetBlockServiceMsg(std::string &name) const;
    bool GetThreadBlockResult();
    void CheckResult();
    void Stop();
    void StopChecker();
    void SendEvent(int tid, const std::string &timerName, const std::string &keyMsg) const;

    void DoTimerCallback(struct InputTimerPara *task);
    bool IsCallbackLimit(unsigned int flag);
    void DestroyTimer(const int id);

    std::unique_ptr<TimerRing> timerRing_;
    mutable std::mutex lock_;
    std::condition_variable condition_;

    /* monitor checker */
    std::map<sptr<XCollieChecker>, unsigned int> xcollieCheckers_ = std::map<sptr<XCollieChecker>, unsigned int>();
    sptr<XCollieChecker> threadChecker_; // current thread blocked service
    sptr<XCollieChecker> lockChecker_; // current lock blocked service
    std::unique_ptr<std::thread> threadLoop_; // watchdog thread
    CheckStatus checkStatus_;
    time_t startTime_;
    bool lockCheckResult_;

    std::unique_ptr<std::thread> thread_;
    volatile int exitThread_;

    unsigned int checkerInterval_;
    bool recovery_;

    /* timer */
    int cntCallback_;
    time_t timeCallback_;
};
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif
