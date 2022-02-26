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

#define LOG_TAG "DirectoryUtils"

#include <dirent.h>
#include <cstring>
#include "directory_utils.h"
#include "log_print.h"
#include "unistd.h"

namespace OHOS {
namespace DistributedKv {
// change the mode of all files in the specified directory recursively.
bool DirectoryUtils::ChangeModeFileOnly(const std::string &path, const mode_t &mode)
{
    ZLOGI("begin.");
    std::string subPath;
    bool ret = true;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // skip current directory and parent directory.
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        subPath = IncludeDelimiterAtPathTail(path) + std::string(ptr->d_name);
        if (ptr->d_type == DT_DIR) {
            ret = ChangeModeFileOnly(subPath, mode);
            continue;
        }

        // change the mode of file only.
        if ((access(subPath.c_str(), F_OK) == 0) && (ptr->d_type == DT_REG)) {
            ZLOGD("[Von-Debug]change the file[%s] to mode[%d].", subPath.c_str(), mode);
            if (!ChangeMode(subPath, mode)) {
                closedir(dir);
                ZLOGD("[Von-Debug]change the file[%s] to mode[%d] failed.", subPath.c_str(), mode);
                return false;
            }
        }
    }
    closedir(dir);
    return ret;
}

// change the mode of all subdirectories in the specified directory recursively.
bool DirectoryUtils::ChangeModeDirOnly(const std::string &path, const mode_t &mode)
{
    ZLOGI("begin.");
    std::string subPath;
    bool ret = true;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // skip current directory and parent directory.
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }

        subPath = IncludeDelimiterAtPathTail(path) + std::string(ptr->d_name);
        if (ptr->d_type == DT_DIR) {
            ret = ChangeModeDirOnly(subPath, mode);
            continue;
        }

        // change the mode of directory only.
        if ((access(subPath.c_str(), F_OK) == 0) && (ptr->d_type == DT_DIR)) {
            ZLOGD("[Von-Debug]change the dir[%s] to mode[%d].", subPath.c_str(), mode);
            if (!ChangeMode(subPath, mode)) {
                closedir(dir);
                ZLOGD("[Von-Debug]change the dir[%s] to mode[%d] failed.", subPath.c_str(), mode);
                return false;
            }
        }
    }
    closedir(dir);

    std::string currentPath = ExcludeDelimiterAtPathTail(path);
    if (access(currentPath.c_str(), F_OK) == 0) {
        if (!ChangeMode(currentPath, mode)) {
            return false;
        }
    }
    return ret;
}

// create all subdirectories in the specified directory recursively.
bool DirectoryUtils::CreateDirectory(const std::string &path)
{
    std::string::size_type index = 0;
    do {
        std::string subPath;
        index = path.find('/', index + 1);
        if (index == std::string::npos) {
            subPath = path;
        } else {
            subPath = path.substr(0, index);
        }

        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) != 0) {
                return false;
            }
        }
    } while (index != std::string::npos);

    return access(path.c_str(), F_OK) == 0;
}

// exclude separators '/' at the end of the path.
std::string DirectoryUtils::ExcludeDelimiterAtPathTail(const std::string &path)
{
    if (path.rfind('/') != path.size() - 1) {
        return path;
    }

    if (!path.empty()) {
        return path.substr(0, static_cast<int>(path.size()) - 1);
    }

    return path;
}

// include separators '/' at the end of the path.
std::string DirectoryUtils::IncludeDelimiterAtPathTail(const std::string &path)
{
    if (path.rfind('/') != path.size() - 1) {
        return path + "/";
    }

    return path;
}
} // namespace DistributedKv
} // namespace OHOS