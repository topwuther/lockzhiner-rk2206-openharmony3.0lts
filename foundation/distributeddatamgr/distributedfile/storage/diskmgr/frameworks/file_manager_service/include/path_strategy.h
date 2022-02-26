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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_PATH_STRATEGY_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_PATH_STRATEGY_H

#include <cstdio>

#include <filesystem>
#include <uri.h>

namespace OHOS {
namespace FileManager {
class PathStrategy {
    /**
     * Return a Uri that represents the given File
     */
public:
    PathStrategy() = default;
    virtual ~PathStrategy() = default;
    /**
     * Return a  Uri that represents the given FilePath
     */
    virtual Uri FuzzyFileToUri(std::string &file, std::string &deviceId);
    /**
     * Return a  File that represents the given Uri
     */
    virtual std::string ParseFileInUri(const Uri &uri);
};
} // namespace FileManager
} // namespace OHOS
#endif // FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_PATH_STRATEGY_H