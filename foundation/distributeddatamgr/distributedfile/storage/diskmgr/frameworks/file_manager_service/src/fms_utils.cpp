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
#include "../include/fms_utils.h"

#include <sys/stat.h>
#include <uri.h>
#include "../include/file_info.h"
#include "../include/log_util.h"
#include "../include/root_info.h"
#include "parcel.h"
#include "message_parcel.h"

namespace OHOS {
namespace FileManager {
using namespace OHOS::AppExecFwk;
using namespace OHOS::NativeRdb;
using namespace std;
using Uri = OHOS::Uri;
unique_ptr<FmsUtils> FmsUtils::mInstance = nullptr;
FmsUtils *FmsUtils::Instance()
{
    if (mInstance.get() == nullptr) {
        mInstance.reset(new FmsUtils());
    }
    return mInstance.get();
}

FmsUtils::FmsUtils() {}
bool FmsUtils::IsPublicStorage(const Uri &uri) const
{
    std::vector<string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    string mAbilityPath = pathVector[0];
    if (mAbilityPath.empty()) {
        return false;
    }
    if (!strcmp(mAbilityPath.c_str(), PUBLIC_STORAGE_ABILITY.c_str())) {
        return true;
    }
    return false;
}
bool FmsUtils::IsPrimaryUser(const Uri &uri) const
{
    std::vector<string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    string mUserPath = pathVector[1];
    if (mUserPath.empty()) {
        return false;
    }
    if (!strcmp(mUserPath.c_str(), PRIMARY_USER.c_str())) {
        return true;
    }
    return false;
}

bool FmsUtils::IsUpdateRootInfo(const Uri &uri) const
{
    if (!strcmp(UPDATE_ROOT_INFO.c_str(), const_cast<Uri &>(uri).GetQuery().c_str())) {
        return true;
    }
    return false;
}

bool FmsUtils::IsCreateDir(const Uri &uri) const
{
    if (!strcmp(CREATE_DIR.c_str(), const_cast<Uri &>(uri).GetQuery().c_str())) {
        return true;
    }
    return false;
}

bool FmsUtils::IsSaveFiles(const Uri &uri) const
{
    if (!strcmp(SAVE_FILES.c_str(), const_cast<Uri &>(uri).GetQuery().c_str())) {
        return true;
    }
    return false;
}

bool FmsUtils::IsRootDirectory(const Uri &uri) const
{
    constexpr int MIN_PATHVECTOR_LENGTH = 3;
    std::vector<string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    int size = pathVector.size();
    if (size < MIN_PATHVECTOR_LENGTH) {
        return false;
    }
    string mRootPath = pathVector[static_cast<int>(COMMON_NUM::TWO)];
    if (mRootPath.empty()) {
        return false;
    }
    if (!strcmp(mRootPath.c_str(), MATCH_ROOTS.c_str())) {
        return true;
    }
    return false;
}
bool FmsUtils::IsChildDirectory(const Uri &uri) const
{
    std::vector<string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    string mLastPath;
    if (pathVector.size() - 1 >= 0) {
        mLastPath = pathVector[pathVector.size() - 1];
    }
    string mRootPath = pathVector[static_cast<int>(COMMON_NUM::TWO)];
    if (mRootPath.empty() || mLastPath.empty()) {
        return false;
    }
    if ((!strcmp(mRootPath.c_str(), MATCH_FILE.c_str())) &&
        (!strcmp(mLastPath.c_str(), MATCH_FILE_CHILDREN.c_str()))) {
        return true;
    }
    return false;
}
string GetInternalPath(void)
{
    string internalPath = "/data/ss/d4ab5596-a98b-4753-8e8f-7cbf342ab15d";
    return internalPath;
}
string FmsUtils::GetCurrentPath(const Uri &uri) const
{
    std::vector<string> pathVector;
    std::string mPath = "";
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    if (IsPublicStorage(uri)) {
        mPath = GetInternalPath();
        mPath.append("/");
        if (IsChildDirectory(uri)) {
            for (unsigned int count = 3; count < pathVector.size() - 1; count++) {
                mPath.append(pathVector[count]);
                mPath.append("/");
            }
        } else if (IsRootDirectory(uri)) {
            for (unsigned int count = 3; count < pathVector.size(); count++) {
                mPath.append(pathVector[count]);
                mPath.append("/");
            }
        } else {
            mPath = "";
        }
    }
    return mPath;
}
string FmsUtils::GetCurrentUser(const Uri &uri) const
{
    std::vector<string> pathVector;
    const_cast<Uri &>(uri).GetPathSegments(pathVector);
    string mUserPath;
    if (pathVector.size() > 1) {
        mUserPath = pathVector[1];
    }
    return mUserPath;
}
std::shared_ptr<NativeRdb::AbsSharedResultSet> FmsUtils::VectorToResultset1(const std::vector<std::string> &columns) const
{
    int listSize = columns.size();
    string column = "";
    for (int i = 0; i < listSize; i++) {
        column = columns.at(i);
    }
    MessageParcel parcel(nullptr);
    bool result = false;

    int size = columns.size();
    if (size <= 0) {
        return nullptr;
    }
    result = parcel.WriteStringVector(columns);
    if (result) {
        return make_shared<NativeRdb::AbsSharedResultSet>();
    } else {
        return nullptr;
    }
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> FmsUtils::Int32ToResultset(int32_t parm) const
{
    MessageParcel parcel(nullptr);
    bool result = false;
    result = parcel.WriteInt32(parm);
    if (result) {
        return make_shared<NativeRdb::AbsSharedResultSet>();
    } else {
        return nullptr;
    }
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> FmsUtils::VectorToResultset(const std::vector<FileInfo> &columns) const
{
    int listSize = columns.size();
    FileInfo fileInfo;
    for (int i = 0; i < listSize; i++) {
        fileInfo = columns.at(i);
        MessageParcel parcel(nullptr);
        return make_shared<NativeRdb::AbsSharedResultSet>();
    }
    return nullptr;
}
std::shared_ptr<NativeRdb::AbsSharedResultSet> FmsUtils::VectorToResultset2(const std::vector<RootInfo> &columns) const
{
    int listSize = columns.size();
    RootInfo rootInfo;
    for (int i = 0; i < listSize; i++) {
        rootInfo = columns.at(i);
    }
    MessageParcel parcel(nullptr);
    return nullptr;
}
int32_t FmsUtils::Mkdirs(string path) const
{
    constexpr int DIR_FAULT_PERM = 0775;
    for (size_t i = 1; i < path.length(); ++i) {
        if (path[i] == '/') {
            path[i] = '\0';
            if (access(path.c_str(), 0) != 0) {
                int t = mkdir(path.c_str(), DIR_FAULT_PERM);
                if (t == -1) {
                    return static_cast<int>(STATUS_NUM::FAIL);
                }
            }
            path[i] = '/';
        }
    }
    if (path.length() > 0 && access(path.c_str(), 0) != 0) {
        int t = mkdir(path.c_str(), DIR_FAULT_PERM);
        if (t == -1) {
            return static_cast<int>(STATUS_NUM::FAIL);
        }
    }
    return static_cast<int>(STATUS_NUM::OK);
}

int FmsUtils::GetCurrentDirFileInfoList(std::string path, std::vector<FileInfo> &fileList) const
{
    DIR *pDir = nullptr;
    struct dirent *ptr;
    FileInfo fileInfo;
    string fullPath;
    if (!(pDir = opendir(path.c_str()))) {
        return static_cast<int>(STATUS_NUM::FAIL);
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            fullPath = path + "/" + ptr->d_name;
            if (ptr->d_type == DT_DIR) {
                fileInfo = GetFileInfo(fullPath, ptr->d_name);
            } else {
                fileInfo = GetFileInfo(fullPath, ptr->d_name);
            }
            fileList.push_back(fileInfo);
        }
    }
    closedir(pDir);
    return static_cast<int>(STATUS_NUM::OK);
}

size_t FmsUtils::GetFileSize(const std::string &fileName) const
{
    FILE *fp = fopen(fileName.c_str(), "r");
    size_t size = static_cast<int>(STATUS_NUM::FAIL);
    if (fseek(fp, 0, SEEK_END) == 0) {
        size = ftell(fp);
    }
    fclose(fp);
    return size;
}
int FmsUtils::GetDirNum(string path) const
{
    int size = 0;
    DIR *pDir = nullptr;
    struct dirent *ptr;
    if ((pDir = opendir(path.c_str())) == nullptr) {
        return -1;
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        size = size + 1;
    }
    return size;
}

FileInfo FmsUtils::GetFileInfo(string sourcePath, string sourceName) const
{
    constexpr int DIVIDE_BY_MILLION = 1000000;
    constexpr int MULTIPLY_BY_THOUSAND = 1000;
    FileInfo fi;
    struct stat tmp;
    int r = stat(sourcePath.c_str(), &tmp);
    if (r == 0 && S_ISDIR(tmp.st_mode)) {
        fi.typeDir = 1;
        fi.dirNum = GetDirNum(sourcePath);
    } else if (r == 0) {
        fi.typeDir = 0;
        fi.mimeType = GetFileType(sourcePath);
        fi.fileSize = tmp.st_size;
    }
    fi.lastUseTime =
        tmp.st_mtime * MULTIPLY_BY_THOUSAND + static_cast<int64_t>((tmp.st_mtim).tv_nsec / DIVIDE_BY_MILLION);
    fi.fileUri = RealPathToUri(sourcePath);
    fi.fileName = sourceName;
    return fi;
}

int FmsUtils::GetSearchFileInfoList(string dirPath,
                                    string fileName,
                                    std::vector<FileInfo> &fileInfoList) const
{
    DIR *pDir = nullptr;
    struct dirent *ptr;
    FileInfo fileInfo;
    string mCurrentFileName;
    if ((pDir = opendir(dirPath.c_str())) == nullptr) {
        return static_cast<int>(STATUS_NUM::FAIL);
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            mCurrentFileName = ptr->d_name;
            if (mCurrentFileName.find(fileName.c_str(), 0) != mCurrentFileName.npos) {
                fileInfo = GetFileInfo(dirPath + "/" + ptr->d_name, ptr->d_name);
                fileInfoList.push_back(fileInfo);
            }
            if (ptr->d_type == DT_DIR) {
                GetSearchFileInfoList(dirPath + "/" + ptr->d_name, fileName, fileInfoList);
            }
        }
    }
    closedir(pDir);
    return static_cast<int>(STATUS_NUM::OK);
}
string FmsUtils::RealPathToUri(const string &realPath) const
{
    string uriStr = "";
    uriStr = SCHEMEOHOS + "://" + PATTERN + PUBLIC_STORAGE_ABILITY;
    string internalPath = GetInternalPath();
    string deviceId = internalPath.substr(internalPath.find_last_of("/"));
    uriStr = uriStr + deviceId + PATTERN;
    if (internalPath == realPath) {
        uriStr = uriStr + MATCH_ROOTS;
    } else {
        string directory = "";
        if (realPath.find(deviceId) != realPath.npos) {
            int deviceIdLen = deviceId.length();
            directory = realPath.substr(realPath.find(deviceId) + deviceIdLen);
        }
        uriStr = uriStr + MATCH_FILE + directory + PATTERN + MATCH_FILE_CHILDREN;
    }
    return uriStr;
}
std::string FmsUtils::GetFileType(const std::string &fileName) const
{
    string suffixStr = fileName.substr(fileName.find_last_of('.') + 1);
    return suffixStr;
}
} // namespace FileManager
} // namespace OHOS
