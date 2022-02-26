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

#ifndef RELIABILITY_XCOLLIE_CHECKER_H
#define RELIABILITY_XCOLLIE_CHECKER_H

#include <thread>
#include <mutex>
#include <string>
#include <map>
#include <singleton.h>
#include <refbase.h>

namespace OHOS {
namespace HiviewDFX {
class XCollieChecker : public virtual RefBase {
public:
    XCollieChecker(const std::string &serviceName):checkerName_(serviceName), complete_(false)
    {
    };
    virtual ~XCollieChecker()
    {
    };
    virtual void CheckLock()
    {
        return;
    };
    virtual void CheckThreadBlock()
    {
        return;
    };
    void SetThreadBlockResult(bool complete)
    {
        complete_ = complete;
    }
    virtual bool GetThreadBlockResult()
    {
        return complete_;
    }
    const std::string GetCheckerName()
    {
        return checkerName_;
    }
private:
    std::string checkerName_;
    bool complete_;
};
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif

