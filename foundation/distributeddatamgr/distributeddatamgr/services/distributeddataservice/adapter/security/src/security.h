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

#ifndef OHOS_SECURITY_H
#define OHOS_SECURITY_H

#include <functional>
#include <atomic>
#include <map>
#include <memory>
#include "iprocess_system_api_adapter.h"
#include "kv_store_delegate_manager.h"
#include "visibility.h"
#include "sensitive.h"

namespace OHOS::DistributedKv {
class Security
    : public DistributedDB::IProcessSystemApiAdapter, public std::enable_shared_from_this<Security> {
public:
    using DBStatus = DistributedDB::DBStatus;
    using OnAccessControlledEvent = DistributedDB::OnAccessControlledEvent;
    using SecurityOption = DistributedDB::SecurityOption;
    Security(const std::string &appId, const std::string &userId, const std::string &dir);
    ~Security() override;

    static bool IsFirstInit();
    static bool IsSupportSecurity();

    DBStatus RegOnAccessControlledEvent(const OnAccessControlledEvent &callback) override;

    // Check is the access of this device in locked state
    bool IsAccessControlled() const override;

    // Set the SecurityOption to the targe filepath.
    // If the filePath is a directory, All the files and directories in the filePath should be effective.
    DBStatus SetSecurityOption(const std::string &filePath, const SecurityOption &option) override;

    // Get the SecurityOption of the targe filepath.
    DBStatus GetSecurityOption(const std::string &filePath, SecurityOption &option) const override;

    // Check if the target device can save the data at the give sensitive class.
    bool CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const override;

    static const char *Convert2Name(const SecurityOption &option, bool isCE);
    static int Convert2Security(const std::string &name);
private:
    enum {
        NO_PWD = -1,
        UNLOCK,
        LOCKED,
        UNINITIALIZED,
    };

    // the key is security_chain/{deviceId}
    static const char * const LABEL_VALUES[DistributedDB::S4 + 1];
    static const char * const DATA_DE[]; // = "/data/misc_de/", "/data/user_de/";
    static const char * const DATA_CE[];
    static constexpr int LABEL_VALUE_LEN = 10;
    static constexpr int RETRY_MAX_TIMES = 10;
    int32_t GetCurrentUserId() const;
    int32_t GetCurrentUserStatus() const;
    bool SubscribeUserStatus(std::function<int32_t(int32_t, int32_t)> &observer) const;
    bool IsExits(const std::string &file) const;
    bool InPathsBox(const std::string &file, const char * const pathsBox[]) const;
    static Sensitive GetDeviceNodeByUuid(const std::string &uuid,
                                         const std::function<std::vector<uint8_t>(void)> &getValue);
    DBStatus GetDirSecurityOption(const std::string &filePath, SecurityOption &option) const;
    DBStatus GetFileSecurityOption(const std::string &filePath, SecurityOption &option) const;
    DBStatus SetDirSecurityOption(const std::string &filePath, const SecurityOption &option);
    DBStatus SetFileSecurityOption(const std::string &filePath, const SecurityOption &option);

    std::map<int32_t, OnAccessControlledEvent> observers_ { };
    static std::atomic_bool isInitialized_;
};
}

#endif // OHOS_SECURITY_H
