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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H

#include "async_common_event_result.h"
#include "common_event_data.h"
#include "common_event_subscribe_info.h"

namespace OHOS {
namespace EventFwk {
class CommonEventSubscriber {
public:
    /**
     * A constructor used to create a CommonEventSubscriber instance
     *
     */
    CommonEventSubscriber();

    /**
     * A constructor used to create a CommonEventSubscriber instance with the
     * subscribeInfo parameter passed.
     *
     * @param subscriberInfo the subscriberInfo
     */
    explicit CommonEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo);

    /**
     * A deconstructor used to deconstruct
     *
     */
    virtual ~CommonEventSubscriber();

    /**
     * Call back when the application receives a new common event.
     *
     * @param data the common event data
     */
    virtual void OnReceiveEvent(const CommonEventData &data) = 0;

    /**
     * Get common event subscriber info
     *
     * @return common event subscriber info
     */
    const CommonEventSubscribeInfo &GetSubscribeInfo() const;

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
     * Enable asynchronous processing for the current ordered common event.
     */
    std::shared_ptr<AsyncCommonEventResult> GoAsyncCommonEvent();

    /**
     * Check whether the current common event is an ordered common event.
     */
    bool IsOrderedCommonEvent() const;

    /**
     * Check whether the current common event is a sticky common event.
     */
    bool IsStickyCommonEvent() const;

private:
    /**
     * Set AsyncCommonEventResult for init before perform onReceiveEvent.
     */
    bool SetAsyncCommonEventResult(const std::shared_ptr<AsyncCommonEventResult> &result);

    /**
     * Get AsyncCommonEventResult for check after perform onReceiveEvent.
     */
    std::shared_ptr<AsyncCommonEventResult> GetAsyncCommonEventResult();

    /**
     * Check whether the current common event is ordered.
     */
    bool CheckSynchronous() const;

private:
    friend class CommonEventListener;

    CommonEventSubscribeInfo subscribeInfo_;
    std::shared_ptr<AsyncCommonEventResult> result_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H
