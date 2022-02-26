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

#include "samgr_death_recipient.h"

#include <chrono>

#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "local_ability_manager.h"
#include "safwk_log.h"

namespace OHOS {
namespace {
const std::string TAG = "SamgrDeathRecipient";

constexpr std::chrono::seconds SECONDS_TRY_WAITING_SAMGR_ONE_TIME(1);
constexpr int32_t RETRY_TIMES = 30;
}

void SamgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        return;
    }

    HILOGI(TAG, "remote samgr died!");
    SystemAbilityManagerClient::GetInstance().DestroySystemAbilityManagerObject();

    int32_t retryTimeout = RETRY_TIMES;
    constexpr int32_t duration = std::chrono::microseconds(SECONDS_TRY_WAITING_SAMGR_ONE_TIME).count();
    sptr<ISystemAbilityManager> samgrProxy;
    while (samgrProxy == nullptr) {
        HILOGD(TAG, "waiting for samgr...");
        if (retryTimeout > 0) {
            usleep(duration);
            samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        } else {
            HILOGE(TAG, "wait for samgr timeout (30s)");
            return;
        }
        retryTimeout--;
    }

    if (samgrProxy != nullptr && samgrProxy->AsObject() != nullptr) {
        auto recipient = sptr<IRemoteObject::DeathRecipient>(new SamgrDeathRecipient());
        bool ret = samgrProxy->AsObject()->AddDeathRecipient(recipient);
        if (!ret) {
            HILOGW(TAG, "failed to add death recipient for samgr");
        }
    }

    LocalAbilityManager::GetInstance().ReRegisterSA();
    HILOGI(TAG, "receive samgr death notification success");
}
} // namespace OHOS