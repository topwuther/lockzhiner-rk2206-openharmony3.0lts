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

#ifndef XCOLLIE_CHECKER_TEST_H
#define XCOLLIE_CHECKER_TEST_H

#include <memory>
#include <string>
#include <thread>

#include "xcollie_checker.h"

namespace OHOS {
namespace HiviewDFX {
class XCollieCheckerTest : public XCollieChecker {
public:
    explicit XCollieCheckerTest(const std::string name);
    ~XCollieCheckerTest();

    void CheckLock();
    void CheckThreadBlock();
    void SetBlockTime(int time)
    {
        timeout_ = time;
    }
    void SetThreadBlockComplete();
    int GetTestLockNumber() const
    {
        return testLockNumber_;
    }
    int GetTestThreadNumber() const
    {
        return testThreadNumber_;
    }
private:
    int timeout_;
    int testLockNumber_;
    int testThreadNumber_;
    bool threadEnd_;
    std::unique_ptr<std::thread> thread_;
};
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif

