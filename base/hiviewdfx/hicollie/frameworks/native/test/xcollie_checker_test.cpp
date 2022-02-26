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

#include "xcollie_checker_test.h"

#include <future>
#include <thread>

namespace OHOS {
namespace HiviewDFX {
XCollieCheckerTest::XCollieCheckerTest(const std::string name)
    : XCollieChecker(name),
      timeout_(0),
      testLockNumber_(0),
      testThreadNumber_(0),
      threadEnd_(false),
      thread_(nullptr)
{
}

XCollieCheckerTest::~XCollieCheckerTest()
{
    if (thread_ != nullptr && thread_->joinable()) {
        thread_->join();
    }
    thread_ = nullptr;
}

void XCollieCheckerTest::CheckLock()
{
    testLockNumber_++;
    if (timeout_ == 0) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::seconds(timeout_));
    return;
}

void XCollieCheckerTest::CheckThreadBlock()
{
    testThreadNumber_++;
    if (timeout_ == 0) {
        SetThreadBlockResult(true);
        return;
    }
    if (threadEnd_) {
        return;
    }
    thread_ = std::make_unique<std::thread>(&XCollieCheckerTest::SetThreadBlockComplete, this);
    threadEnd_ = true;
}

void XCollieCheckerTest::SetThreadBlockComplete()
{
    std::this_thread::sleep_for(std::chrono::seconds(timeout_));
    SetThreadBlockResult(true);
    threadEnd_ = false;
    return;
}
} // end of namespace HiviewDFX
} // end of namespace OHOS
