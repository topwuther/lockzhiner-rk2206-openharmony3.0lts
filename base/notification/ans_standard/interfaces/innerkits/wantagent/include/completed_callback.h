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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_COMPLETED_CALLBACK_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_COMPLETED_CALLBACK_H

#include <string>
#include <memory>
#include "want.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
class CompletedCallback {

    /**
     * Called when a Send operation as completed.
     *
     * @param want The original Want that was sent.
     * @param resultCode The final result code determined by the Send.
     * @param resultData The final data collected by a broadcast.
     * @param resultExtras The final extras collected by a broadcast.
     */
public:
    virtual ~CompletedCallback() = default;
    virtual void OnSendFinished(const AAFwk::Want &want, int resultCode, const std::string &resultData,
        const AAFwk::WantParams &resultExtras) = 0;
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_COMPLETED_CALLBACK_H