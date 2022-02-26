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

#include <string>

#ifndef DATA_VALIDATOR_H
#define DATA_VALIDATOR_H
namespace OHOS {
namespace Security {
namespace Permission {
class DataValidator final {
public:
    static bool IsBundleNameValid(const std::string& bundleName);

    static bool IsPermissionNameValid(const std::string& permissionName);

    static bool IsUserIdValid(const int userId);

private:
    const static int MAX_LENGTH = 256;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // DATA_VALIDATOR_H
