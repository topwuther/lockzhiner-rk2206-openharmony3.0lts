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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FMS_UTILS_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FMS_UTILS_H

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <uri.h>
#include <vector>

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "dummy_result_set.h"
#include "file_info.h"
#include "parcel.h"
#include "root_info.h"

#include "values_bucket.h"

namespace OHOS {
namespace FileManager {
enum class STATUS_NUM {
    PARAMETER_EXCEPTION = 202,
    IO_EXCEPTION = 300,
    URI_EXCEPTION = 302,
    SUCCESS = 200,
    FAIL = -1,
    OK = 0,
};
enum class COMMON_NUM {
    ZERO = 0,
    ONE = 1,
    TWO = 2,
};
class FmsUtils {
public:
    FmsUtils();
    ~FmsUtils() = default;
    static FmsUtils *Instance();

    bool IsChildDirectory(const Uri &uri) const;
    bool IsRootDirectory(const Uri &uri) const;
    bool IsPrimaryUser(const Uri &uri) const;
    bool IsPublicStorage(const Uri &uri) const;
    bool IsUpdateRootInfo(const Uri &uri) const;
    bool IsCreateDir(const Uri &uri) const;
    bool IsSaveFiles(const Uri &uri) const;
    std::string GetCurrentPath(const Uri &uri) const;
    std::string GetCurrentUser(const Uri &uri) const;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> VectorToResultset1(const std::vector<std::string> &columns) const;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> VectorToResultset2(const std::vector<RootInfo> &columns) const;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Int32ToResultset(int32_t parm) const;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> VectorToResultset(const std::vector<FileInfo> &columns) const;
    int GetCurrentDirFileInfoList(std::string path, std::vector<FileInfo> &fileInfoList) const;
    int32_t Mkdirs(std::string path) const;
    size_t GetFileSize(const std::string &fileName) const;
    std::string GetFileType(const std::string &fileName) const;
    FileInfo GetFileInfo(std::string sourcePath, std::string sourceName) const;
    int GetDirNum(std::string path) const;
    int GetSearchFileInfoList(std::string dirPath, std::string fileName, std::vector<FileInfo> &fileList) const;
    std::string RealPathToUri(const std::string &realPath) const;

private:
    static std::unique_ptr<FmsUtils> mInstance;
    const std::string SCHEMEOHOS = "dataability";
    const std::string PUBLIC_STORAGE_ABILITY = "public.storage.ability";
    const std::string PRIMARY_USER = "primary";
    const std::string MATCH_ROOTS = "root";
    const std::string MATCH_FILE = "document";
    const std::string MATCH_FILE_CHILDREN = "leaf";
    const std::string UPDATE_ROOT_INFO = "UpdateRootInfo";
    const std::string CREATE_DIR = "CreateDir";
    const std::string SAVE_FILES = "SaveFiles";
    const std::string PATTERN = "/";
};
} // namespace FileManager
} // namespace OHOS
#endif // FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FMS_UTILS_H
