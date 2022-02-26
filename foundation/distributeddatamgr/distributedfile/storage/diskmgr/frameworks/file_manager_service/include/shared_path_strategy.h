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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_SIMPLE_PATH_STRATEGY_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_SIMPLE_PATH_STRATEGY_H

#include <cstdio>
#include <map>
#include <string>

#include "path_strategy.h"

namespace OHOS {
namespace FileManager {
class SharedPathStrategy : public PathStrategy {
    /**
     * Return a Uri that represents the given File
     */

public:
    SharedPathStrategy();
    virtual ~SharedPathStrategy() = default;
    explicit SharedPathStrategy(const std::string &authority);
    /**
     * Return a  Uri that represents the given FilePath
     */
    virtual Uri FuzzyFileToUri(std::string &file, std::string &deviceId) override;
    /**
     * Return a  FilePath that represents the given Uri
     */
    virtual std::string ParseFileInUri(const Uri &uri) override;
    /**
     * Add a mapping from a name to a filesystem root. The provider only offers
     * access to files that live under configured roots.
     */
    void AddRoot(const std::string &name, const std::string &root);
    std::string GetUriStr(std::string &file, const std::string &deviceId);

private:
    std::string mAuthority;
    std::map<std::string, std::string> mRoots;
    const std::string PILING_AUTHORITY = "ohos.acts.test.ability";
};
} // namespace FileManager
} // namespace OHOS
#endif // FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_SIMPLE_PATH_STRATEGY_H