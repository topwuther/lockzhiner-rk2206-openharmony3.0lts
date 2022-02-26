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

#ifndef DISTRIBUTEDDATAMGR_ACCOUNT_DELEGATE_H
#define DISTRIBUTEDDATAMGR_ACCOUNT_DELEGATE_H

#include <mutex>
#include <memory>
#include "types.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
enum class AccountStatus {
    HARMONY_ACCOUNT_LOGIN = 0, // the openHarmony account is logged in
    HARMONY_ACCOUNT_LOGOUT, // the openHarmony account is logged out
    HARMONY_ACCOUNT_DELETE, // the openHarmony account is deleted
    DEVICE_ACCOUNT_DELETE, // the device account is deleted
};

struct AccountEventInfo {
    std::string harmonyAccountId;
    std::string deviceAccountId;
    AccountStatus status;
};

class AccountDelegate {
public:
    class Observer {
    public:
        KVSTORE_API virtual ~Observer() = default;
        KVSTORE_API virtual void OnAccountChanged(const AccountEventInfo &eventInfo) = 0;

        // must specify unique name for observer
        KVSTORE_API virtual std::string Name() = 0;
    };
    KVSTORE_API virtual ~AccountDelegate() = default;
    KVSTORE_API virtual Status Subscribe(std::shared_ptr<Observer> observer) = 0;
    KVSTORE_API virtual Status Unsubscribe(std::shared_ptr<Observer> observer) = 0;
    KVSTORE_API virtual std::string GetCurrentHarmonyAccountId(const std::string &bundleName = "") const = 0;
    KVSTORE_API virtual std::string GetDeviceAccountIdByUID(int32_t uid) const = 0;
    KVSTORE_API virtual void SubscribeAccountEvent() = 0;
    KVSTORE_API static AccountDelegate *GetInstance();
    const static std::string MAIN_DEVICE_ACCOUNT_ID;
private:
    using BaseInstance = AccountDelegate *(*)();
    static BaseInstance getInstance_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_ACCOUNT_DELEGATE_H
