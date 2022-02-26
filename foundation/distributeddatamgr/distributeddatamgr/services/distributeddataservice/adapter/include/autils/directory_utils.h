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

#ifndef DIRECTORY_UTILS_H
#define DIRECTORY_UTILS_H

#include <string>
#include <sys/stat.h>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class DirectoryUtils {
public:
    KVSTORE_API static bool ChangeModeFileOnly(const std::string &path, const mode_t &mode);

    KVSTORE_API static bool ChangeModeDirOnly(const std::string &path, const mode_t &mode);

    KVSTORE_API static bool CreateDirectory(const std::string &path);

private:
    DirectoryUtils() = default;

    ~DirectoryUtils() = default;

    static std::string ExcludeDelimiterAtPathTail(const std::string &path);

    static std::string IncludeDelimiterAtPathTail(const std::string &path);

    // change the mode of the specified file or directory.
    static inline bool ChangeMode(const std::string &name, const mode_t &mode)
    {
        return (chmod(name.c_str(), mode) == 0);
    }
};
} // namespace DistributedKv
} // namespace OHOS
#endif // DIRECTORY_UTILS_H