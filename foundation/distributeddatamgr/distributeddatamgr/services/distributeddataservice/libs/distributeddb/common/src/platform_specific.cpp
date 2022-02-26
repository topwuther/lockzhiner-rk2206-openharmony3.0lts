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

#include "platform_specific.h"

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#include "securec.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
namespace OS {
/*
 * Common part that is the same between each os
 */
namespace {
    const int ACCESS_MODE_EXISTENCE = 0;
    const uint64_t MULTIPLES_BETWEEN_SECONDS_AND_MICROSECONDS = 1000000;
}
bool CheckPathExistence(const std::string &filePath)
{
    if (access(filePath.c_str(), ACCESS_MODE_EXISTENCE) < 0) {
        return false;
    }
    return true;
}

int RenameFilePath(const std::string &oldFilePath, const std::string &newFilePath)
{
    int errCode = rename(oldFilePath.c_str(), newFilePath.c_str());
    if (errCode < 0) {
        LOGE("[Rename] Rename file fail. err = %d", errno);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}

int RemoveFile(const std::string &filePath)
{
    int errCode = remove(filePath.c_str());
    if (errCode < 0) {
        LOGE("[RemoveFile] Remove file fail. err = %d", errno);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}

int CalFileSize(const std::string &fileUrl, uint64_t &size)
{
    struct stat fileStat;
    if (fileUrl.empty() || stat(fileUrl.c_str(), &fileStat) < 0 || fileStat.st_size < 0) {
        LOGE("Get file[%zu] size failed, errno [%d].", fileUrl.size(), errno);
        return -E_INVALID_ARGS;
    }

    size = fileStat.st_size;
    return E_OK;
}

void SplitFilePath(const std::string &filePath, std::string &fileDir, std::string &fileName)
{
    if (filePath.empty()) {
        return;
    }

    auto slashPos = filePath.find_last_of('/');
    if (slashPos == std::string::npos) {
        fileName = filePath;
        fileDir = "";
        return;
    }

    fileDir = filePath.substr(0, slashPos);
    fileName = filePath.substr(slashPos + 1);
    return;
}

int MakeDBDirectory(const std::string &directory)
{
    int errCode = mkdir(directory.c_str(), (S_IRWXU | S_IRGRP | S_IXGRP)); // The permission is 750 for linux based os
    if (errCode < 0) {
        LOGE("[MakeDir] Make directory fail:%d.", errno);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}

int RemoveDBDirectory(const std::string &directory)
{
    return remove(directory.c_str());
}

int CreateFileByFileName(const std::string &fileName)
{
    int fp = open(fileName.c_str(), (O_WRONLY | O_CREAT), (S_IRUSR | S_IWUSR | S_IRGRP));
    if (fp < 0) {
        LOGE("[CreateFile] Create file fail:%d.", errno);
        return -E_SYSTEM_API_FAIL;
    }
    close(fp);
    return E_OK;
}

int GetRealPath(const std::string &inOriPath, std::string &outRealPath)
{
    const unsigned int MAX_PATH_LENGTH = PATH_MAX;
    if (inOriPath.length() > MAX_PATH_LENGTH || MAX_PATH_LENGTH > 0x10000) { // max limit is 64K(0x10000).
        LOGE("[RealPath] OriPath too long.");
        return -E_INVALID_ARGS;
    }

    char *realPath = new (std::nothrow) char[MAX_PATH_LENGTH + 1];
    if (realPath == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    (void)memset_s(realPath, MAX_PATH_LENGTH + 1, 0, MAX_PATH_LENGTH + 1);
    if (realpath(inOriPath.c_str(), realPath) == nullptr) {
        LOGE("[RealPath] Get realpath for inOriPath fail:%d.", errno);
        delete []realPath;
        return -E_SYSTEM_API_FAIL;
    }
    outRealPath = std::string(realPath);
    delete []realPath;
    return E_OK;
}

int GetCurrentSysTimeInMicrosecond(uint64_t &outTime)
{
    struct timeval rawTime;
    int errCode = gettimeofday(&rawTime, nullptr);
    if (errCode < 0) {
        LOGE("[GetSysTime] Fail:%d.", errCode);
        return -E_SYSTEM_API_FAIL;
    }
    outTime = static_cast<uint64_t>(rawTime.tv_sec) * MULTIPLES_BETWEEN_SECONDS_AND_MICROSECONDS +
        static_cast<uint64_t>(rawTime.tv_usec);
    return E_OK;
}

namespace {
    const uint64_t MULTIPLES_BETWEEN_MICROSECONDS_AND_NANOSECONDS = 1000;
}

int GetMonotonicRelativeTimeInMicrosecond(uint64_t &outTime)
{
    struct timespec rawTime;
    int errCode = clock_gettime(CLOCK_BOOTTIME, &rawTime);
    if (errCode < 0) {
        LOGE("[GetMonoTime] Fail.");
        return -E_SYSTEM_API_FAIL;
    }
    outTime = static_cast<uint64_t>(rawTime.tv_sec) * MULTIPLES_BETWEEN_SECONDS_AND_MICROSECONDS +
        static_cast<uint64_t>(rawTime.tv_nsec) / MULTIPLES_BETWEEN_MICROSECONDS_AND_NANOSECONDS;
    return E_OK;
}

static int GetFilePathAttr(const std::string &topPath, const std::string &relativePath,
    std::list<FileAttr> &files, bool isNeedAllPath)
{
    DIR *dir = opendir(topPath.c_str());
    if (dir == nullptr) {
        LOGE("Open dir error:%d.", errno);
        return -E_INVALID_PATH;
    }
    struct stat fileStat;
    std::string fileAbsName;
    int errCode = E_OK;
    FileAttr file;
    for (struct dirent *fileDirInfo = readdir(dir); fileDirInfo != nullptr; fileDirInfo = readdir(dir)) {
        switch (fileDirInfo->d_type) {
            case DT_REG:
                file.fileType = FILE;
                break;
            case DT_DIR:
                file.fileType = PATH;
                break;
            default:
                file.fileType = OTHER;
        }
        if (strlen(fileDirInfo->d_name) == 0 || strcmp(fileDirInfo->d_name, ".") == 0 ||
            strcmp(fileDirInfo->d_name, "..") == 0) {
            continue;
        }
        file.fileName = relativePath + fileDirInfo->d_name;
        fileAbsName = topPath + "/" + fileDirInfo->d_name;
        errCode = stat(fileAbsName.c_str(), &fileStat);
        if (errCode != 0) {
            LOGE("[GetFileAttr]Get file stat failed, error = %d.", errno);
            errCode = -E_INVALID_PATH;
            break;
        }
        if (isNeedAllPath) {
            file.fileName = fileAbsName;
        }
        file.fileLen = fileStat.st_size;
        files.push_back(file);
        if (file.fileType == PATH) {
            errCode = GetFilePathAttr(fileAbsName, relativePath + fileDirInfo->d_name + "/", files, isNeedAllPath);
            if (errCode != E_OK) {
                break;
            }
        }
    }

    closedir(dir);
    return errCode;
}

int GetFileAttrFromPath(const std::string &filePath, std::list<FileAttr> &files, bool isNeedAllPath)
{
    return GetFilePathAttr(filePath, std::string(), files, isNeedAllPath);
}

int GetFilePermissions(const std::string &fileName, uint32_t &permissions)
{
    struct stat fileStat;
    int errCode = stat(fileName.c_str(), &fileStat);
    if (errCode != E_OK) {
        permissions = S_IRUSR | S_IWUSR;
        LOGE("Get file stat failed, error = %d.", errno);
        return -E_SYSTEM_API_FAIL;
    }
    permissions = fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    return E_OK;
}

int SetFilePermissions(const std::string &fileName, uint32_t permissions)
{
    if (permissions > (S_IRWXU | S_IRWXG | S_IRWXO)) {
        return -E_INVALID_ARGS;
    }
    int errCode = chmod(fileName.c_str(), permissions);
    if (errCode != E_OK) {
        LOGE("Set file permissions failed, error = %d.", errno);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}
} // namespace OS
} // namespace DistributedDB
