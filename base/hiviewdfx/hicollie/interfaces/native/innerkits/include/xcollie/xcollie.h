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

#ifndef RELIABILITY_XCOLLIE_H
#define RELIABILITY_XCOLLIE_H
#include <string>
#include "singleton.h"
#include "xcollie_checker.h"
#include "xcollie_define.h"

namespace OHOS {
namespace HiviewDFX {
class XCollie : public Singleton<XCollie> {
    DECLARE_SINGLETON(XCollie);
public:
    // register XCollieChecker for service watchdog
    // checker : XCollieChecker object
    // type    : watchdog type. the value can be:XCOLLIE_LOCK, XCOLLIE_THREAD, XCOLLIE_LOCK|XCOLLIE_Thread
    void RegisterXCollieChecker(const sptr<XCollieChecker> &checker, unsigned int type);

    // set timer
    // name : timer name
    // timeout : timeout, unit s
    // func : callback
    // arg : the callback's param
    // flag : timer timeout operation. the value can be:
    //                               XCOLLIE_FLAG_DEFAULT :do all callback function
    //                               XCOLLIE_FLAG_NOOP  : do nothing but the caller defined function
    //                               XCOLLIE_FLAG_LOG :  generate log file
    //                               XCOLLIE_FLAG_RECOVERY  : die when timeout
    // return: the timer id
    int SetTimer(const std::string &name, unsigned int timeout,
        std::function<void (void *)> func, void *arg, unsigned int flag);

    // cancel timer
    // id: timer id
    void CancelTimer(int id);

    // update timer
    // id: timer id
    // timeout: timeout, unit s
    bool UpdateTimer(int id, unsigned int timeout);
};
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif

