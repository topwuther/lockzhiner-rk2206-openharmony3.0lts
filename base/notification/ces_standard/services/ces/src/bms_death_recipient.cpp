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

#include "bms_death_recipient.h"
#include "bundle_manager_helper.h"
#include "event_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
void BMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &wptrDeath)
{
    EVENT_LOGI("bundle manager service died, remove the proxy object");

    if (wptrDeath == nullptr) {
        EVENT_LOGE("wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        EVENT_LOGE("object is null");
        return;
    }

    DelayedSingleton<BundleManagerHelper>::GetInstance()->ClearBundleManagerHelper();

    return;
}
}  // namespace EventFwk
}  // namespace OHOS