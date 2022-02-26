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

#ifndef IPROCESS_SYSTEM_API_ADAPTER_H
#define IPROCESS_SYSTEM_API_ADAPTER_H

#include <string>
#include <functional>
#include "types.h"

namespace DistributedDB {
using OnAccessControlledEvent = std::function<void(bool isAccessControlled)>;

// For all functions with returnType DBStatus:
// return DBStatus::OK if successful, otherwise DBStatus::DB_ERROR if anything wrong.
// Additional information of reason why failed can be present in the log by the implementation.
// For "Get" or "Is" functions, implementation should notice that concurrent call is possible.
// The distributeddb in one process can only use one ProcessSystemApiAdapter at the same time
class IProcessSystemApiAdapter {
public:
    // Function used to register a AccessControlled listener, like screen locked.
    // There will only be one callback at the same time for each function
    // If register again, the latter callback replace the former callback.
    // Register nullptr as callback to do unregister semantic.
    // For concurrency security of implementation, there should be lock between register_operation and callback_event.
    virtual DBStatus RegOnAccessControlledEvent(const OnAccessControlledEvent &callback) = 0;

    // Check is the access of this device in locked state
    virtual bool IsAccessControlled() const = 0;

    // Set the SecurityOption to the targe filepath.
    // If the filePath is a directory, All the files and directories in the filePath should be effective.
    virtual DBStatus SetSecurityOption(const std::string &filePath, const SecurityOption &option) = 0;

    // Get the SecurityOption of the targe filepath.
    virtual DBStatus GetSecurityOption(const std::string &filePath, SecurityOption &option) const = 0;

    // Check if the target device can save the data at the give sensitive class.
    virtual bool CheckDeviceSecurityAbility(const std::string &devId, const SecurityOption &option) const = 0;

    virtual ~IProcessSystemApiAdapter() {};
};
} // namespace DistributedDB
#endif // IPROCESS_SYSTEM_API_ADAPTER_H
