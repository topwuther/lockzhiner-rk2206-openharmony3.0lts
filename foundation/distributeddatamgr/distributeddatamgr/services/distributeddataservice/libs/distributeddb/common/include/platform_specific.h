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

#ifndef PLATFORM_SPECIFIC_H
#define PLATFORM_SPECIFIC_H

#include <string>
#include <cstdint>
#include <list>

namespace DistributedDB {
namespace OS {
enum FileType {
    FILE = 0,
    PATH = 1,
    OTHER = 2,
};

struct FileAttr {
    std::string fileName;
    FileType fileType;
    uint64_t fileLen;
};

int CalFileSize(const std::string &fileUrl, uint64_t &size);

bool CheckPathExistence(const std::string &filePath);

int MakeDBDirectory(const std::string &directory);

int RemoveFile(const std::string &filePath);
// Can only remove empty directory
int RemoveDBDirectory(const std::string &directory);

int GetRealPath(const std::string &inOriPath, std::string &outRealPath);

int GetCurrentSysTimeInMicrosecond(uint64_t &outTime);

int GetMonotonicRelativeTimeInMicrosecond(uint64_t &outTime);

int CreateFileByFileName(const std::string &fileName);

void SplitFilePath(const std::string &filePath, std::string &fileDir, std::string &fileName);

int GetFileAttrFromPath(const std::string &filePath, std::list<FileAttr> &files, bool isNeedAllPath = false);

int GetFilePermissions(const std::string &fileName, uint32_t &permissions);

int SetFilePermissions(const std::string &fileName, uint32_t permissions);

int RenameFilePath(const std::string &oldFilePath, const std::string &newFilePath);
} // namespace OS
} // namespace DistributedDB

#endif // PLATFORM_SPECIFIC_H