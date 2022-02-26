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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_TEST_MOCK_MOCK_COMMON_EVENT_STUB_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_TEST_MOCK_MOCK_COMMON_EVENT_STUB_H

#include "gmock/gmock.h"

#include "common_event_stub.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
class MockCommonEventStub : public CommonEventStub {
public:
    MOCK_METHOD4(FinishReceiver, bool(const sptr<IRemoteObject> &proxy, const int &code,
                                     const std::string &receiverData, const bool &abortEvent));

    bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener) override;

    bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener) override;

    bool DumpState(const std::string &event, std::vector<std::string> &state) override;

private:
    std::shared_ptr<CommonEventSubscribeInfo> subscribeInfoPtr;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_TEST_MOCK_MOCK_COMMON_EVENT_STUB_H
