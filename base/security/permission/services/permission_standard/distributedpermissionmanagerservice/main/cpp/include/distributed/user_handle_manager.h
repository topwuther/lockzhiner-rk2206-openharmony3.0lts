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

#ifndef USER_HANDLE_MANAGER_H
#define USER_HANDLE_MANAGER_H

#include <string>
#include "constant.h"
namespace OHOS {
namespace Security {
namespace Permission {
class UserHandleManager {
public:
    UserHandleManager() = default;
    ~UserHandleManager() = default;
    static bool IsRootOrSystemUid(const int32_t &uid);

public:
    // Range of uids allocated for a user ,normally 100000.
    static constexpr int32_t PER_USER_RANGE = 100000;
    // Indicates the current user in linux,normally 0.
    static constexpr int32_t USER_SYSTEM = 0;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif
