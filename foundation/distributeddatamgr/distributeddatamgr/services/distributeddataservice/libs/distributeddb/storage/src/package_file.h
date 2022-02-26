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

#ifndef PACKAGE_FILE_H
#define PACKAGE_FILE_H

#include <string>
namespace DistributedDB {
struct FileInfo {
    uint32_t dbType;
    std::string deviceID;
};

class PackageFile {
public:
    PackageFile() {}
    ~PackageFile() {}
    static int PackageFiles(const std::string &sourcePath, const std::string &targetFile, const FileInfo &fileInfo);
    static int UnpackFile(const std::string &sourceFile, const std::string &targetPath, FileInfo &fileInfo);
    static int GetPackageVersion(const std::string &sourceFile, uint32_t &version);
private:
    static int ExePackage(const std::string &sourcePath, const std::string &targetFile, const FileInfo &fileInfo);
};
}

#endif // PACKAGE_FILE_H