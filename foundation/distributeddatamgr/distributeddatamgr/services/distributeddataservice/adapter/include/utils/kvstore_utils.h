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

#ifndef KVSTORE_UTILS_H
#define KVSTORE_UTILS_H

#include <string>
#include "visibility.h"
#include "communication_provider.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreUtils {
public:
    // get app id; KvStore may use BundleName as appId, this function should run in IPC thread
    // bundleName: string to be hashed
    KVSTORE_API static std::string GetAppIdByBundleName(const std::string &bundleName);

    // convert the name to the anonymous
    // the anonymous string is name[0,3]"***"name[end-3, end]
    // when the anonymous string is shorter than 9, the anonymous is "******"
    KVSTORE_API static std::string ToBeAnonymous(const std::string &name);

    KVSTORE_API static AppDistributedKv::CommunicationProvider &GetProviderInstance();
private:
    static constexpr int MAIN_USER_ID = 0;
    static constexpr int SYSTEM_UID = 1000;
};
} // namespace DistributedKv
} // namespace OHOS
#endif // KVSTORE_UTILS_H
