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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_ASYNC_COMMON_EVENT_RESULT_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_ASYNC_COMMON_EVENT_RESULT_H

#include <string>

#include "iremote_object.h"

namespace OHOS {
namespace EventFwk {
class AsyncCommonEventResult {
public:
    /**
     * Create a AsyncCommonEventResult instance based on the parameters.
     *
     * @param resultCode the result code of the current ordered common event
     * @param resultData the result data of the current ordered common event
     * @param ordered the type of the current ordered common event is ordered or not
     * @param sticky the type of the current sticky common event is sticky or not
     * @param token the remote object of the current ordered common event
     */
    AsyncCommonEventResult(const int &resultCode, const std::string &resultData, const bool &ordered,
        const bool &sticky, const sptr<IRemoteObject> &token);

    /**
     * Default deconstructor used to deconstruct.
     *
     */
    ~AsyncCommonEventResult();

    /**
     * Set the result code of the current ordered common event.
     *
     * @param code the result code of the current ordered common event
     */
    bool SetCode(const int &code);

    /**
     * Obtain the result code of the current ordered common event.
     *
     * @return the result code of the current ordered common event
     */
    int GetCode() const;

    /**
     * Set the result data of the current ordered common event.
     *
     * @param data the result data of the current ordered common event.
     */
    bool SetData(const std::string &data);

    /**
     * Obtain the result data of the current ordered common event.
     *
     * @return the result data of the current ordered common event
     */
    std::string GetData() const;

    /**
     * Set the result of the current ordered common event.
     *
     * @param code the result code of the current ordered common event.
     * @param data the result data of the current ordered common event.
     */
    bool SetCodeAndData(const int &code, const std::string &data);

    /**
     * Cancel the current ordered common event.
     */
    bool AbortCommonEvent();

    /**
     * Clear the abort state of the current ordered common event.
     */
    bool ClearAbortCommonEvent();

    /**
     * Check whether the current ordered common event should be aborted.
     */
    bool GetAbortCommonEvent() const;

    /**
     * Finishe processing the current ordered common event.
     */
    bool FinishCommonEvent();

    /**
     * Check whether the current common event is an ordered common event.
     */
    bool IsOrderedCommonEvent() const;

    /**
     * Check whether the current common event is a sticky common event.
     */
    bool IsStickyCommonEvent() const;

    /**
     * Check whether the current common event is ordered.
     */
    bool CheckSynchronous() const;

private:
    int resultCode_;
    std::string resultData_;
    bool ordered_;
    bool sticky_;
    sptr<IRemoteObject> token_;
    bool abortEvent_;
    bool finished_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_ASYNC_COMMON_EVENT_RESULT_H