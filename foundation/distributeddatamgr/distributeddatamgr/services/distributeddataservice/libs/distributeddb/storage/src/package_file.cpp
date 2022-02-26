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

#include "package_file.h"

#include <fstream>

#include "db_errno.h"
#include "value_hash_calc.h"
#include "parcel.h"
#include "platform_specific.h"

namespace DistributedDB {
using std::string;
using std::vector;
using std::list;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::ios_base;

namespace {
    constexpr uint32_t MAX_FILE_NAME_LEN = 256;
    constexpr uint32_t CHECKSUM_LEN = SHA256_DIGEST_LENGTH;
    constexpr uint32_t CHECKSUM_BLOCK_SIZE = 64;
    constexpr uint32_t DEVICE_ID_LEN = SHA256_DIGEST_LENGTH;
    constexpr uint32_t MAGIC_LEN = 16;
    constexpr uint32_t CURRENT_VERSION = 0;
    constexpr uint64_t BUFFER_LEN = 4096;
    const string MAGIC = "HW package file";
    const string FILE_SEPARATOR = "/";
    const string INVALID_FILE_WORDS = "..";

    const uint32_t FILE_HEADER_LEN = MAGIC_LEN + CHECKSUM_LEN + DEVICE_ID_LEN + Parcel::GetUInt32Len() * 3;
    const uint32_t FILE_CONTEXT_LEN = MAX_FILE_NAME_LEN + Parcel::GetUInt32Len() * 2 + Parcel::GetUInt64Len() * 2;
}

struct FileContext {
    char fileName[MAX_FILE_NAME_LEN] = {0};
    uint32_t fileType = 0;
    uint32_t parentID = 0;
    uint64_t fileLen = 0;
    uint64_t offset = 0;
};

static void Clear(ofstream &target, string targetFile)
{
    if (target.is_open()) {
        target.close();
    }
    if (remove(targetFile.c_str()) != EOK) {
        LOGE("Remove file failed.");
    }
    return;
}

static int GetChecksum(const string &file, vector<char> &result)
{
    ifstream fileHandle(file, ios::in | ios::binary);
    if (!fileHandle.good()) {
        LOGE("[GetChecksum]Error fileHandle!");
        return -E_INVALID_PATH;
    }
    ValueHashCalc calc;
    int errCode = calc.Initialize();
    if (errCode != E_OK) {
        LOGE("[GetChecksum]Calc Initialize fail!");
        return errCode;
    }
    fileHandle.seekg(static_cast<int64_t>(MAGIC_LEN + Parcel::GetUInt32Len() + CHECKSUM_LEN), ios_base::beg);
    vector<uint8_t> buffer(CHECKSUM_BLOCK_SIZE, 0);
    bool readEnd = false;
    while (!readEnd) {
        fileHandle.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (fileHandle.eof()) {
            readEnd = true;
        } else if (!fileHandle.good()) {
            LOGE("[GetChecksum]fileHandle error!");
            return -E_INVALID_PATH;
        }
        errCode = calc.Update(buffer);
        if (errCode != E_OK) {
            LOGE("[GetChecksum]Calc Update fail!");
            return errCode;
        }
        buffer.assign(CHECKSUM_BLOCK_SIZE, 0);
    }
    vector<uint8_t> resultBuf;
    errCode = calc.GetResult(resultBuf);
    if (errCode != E_OK) {
        LOGE("[GetChecksum]Calc GetResult fail!");
        return errCode;
    }
    result.assign(resultBuf.begin(), resultBuf.end());
    return E_OK;
}

static int GetFileContexts(const string &sourcePath, list<FileContext> &fileContexts)
{
    list<OS::FileAttr> files;
    int errCode = OS::GetFileAttrFromPath(sourcePath, files, false);
    if (errCode != E_OK) {
        LOGE("[GetFileContexts] get file attr from path fail, errCode = [%d]", errCode);
        return errCode;
    }
    FileContext fileContext;
    int countLimit = 0;
    for (auto file = files.begin(); file != files.end(); file++, countLimit++) {
        if (countLimit > 20) { // Limit number of files 20 for security
            LOGE("Too deep access for get file context!");
            return -E_INVALID_PATH;
        }

        if (file->fileType != OS::FILE && file->fileType != OS::PATH) {
            continue;
        }

        errCode = memset_s(fileContext.fileName, MAX_FILE_NAME_LEN, 0, MAX_FILE_NAME_LEN);
        if (errCode != EOK) {
            return -E_SECUREC_ERROR;
        }

        if (file->fileName.size() >= MAX_FILE_NAME_LEN) {
            LOGE("file name is too long!");
            return -E_INVALID_FILE;
        }

        errCode = memcpy_s(fileContext.fileName, MAX_FILE_NAME_LEN, file->fileName.c_str(), file->fileName.size());
        if (errCode != EOK) {
            return -E_SECUREC_ERROR;
        }

        fileContext.fileLen = file->fileLen;
        fileContext.fileType = file->fileType;
        fileContexts.push_back(fileContext);
    }
    LOGD("Get file contexts, fileContexts size is [%zu]", fileContexts.size());
    return E_OK;
}

static int FileContentCopy(ifstream &sourceFile, ofstream &targetFile, uint64_t fileLen)
{
    uint64_t leftLen = fileLen;
    vector<char> buffer(BUFFER_LEN, 0);
    while (leftLen > 0) {
        uint64_t readLen = (leftLen > BUFFER_LEN) ? BUFFER_LEN : leftLen;
        sourceFile.read(buffer.data(), readLen);
        if (!sourceFile.good()) {
            LOGE("[FileContentCopy] SourceFile error! sys[%d]", errno);
            return -E_INVALID_PATH;
        }
        targetFile.write(buffer.data(), readLen);
        if (!targetFile.good()) {
            LOGE("[FileContentCopy] TargetFile error! sys[%d]", errno);
            return -E_INVALID_PATH;
        }
        leftLen -= readLen;
    }
    return E_OK;
}

static int PackFileHeader(ofstream &targetFile, const FileInfo &fileInfo, uint32_t fileNum)
{
    if (fileInfo.deviceID.size() != DEVICE_ID_LEN) {
        return -E_INVALID_ARGS;
    }
    vector<uint8_t> buffer(FILE_HEADER_LEN, 0);
    vector<char> checksum(CHECKSUM_LEN, 0);
    Parcel parcel(buffer.data(), FILE_HEADER_LEN);

    int errCode = parcel.WriteBlob(MAGIC.c_str(), MAGIC_LEN);
    if (errCode != E_OK) {
        return errCode;
    }
    // before current version package version is always 0
    errCode = parcel.WriteUInt32(CURRENT_VERSION);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteBlob(checksum.data(), CHECKSUM_LEN);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteBlob(fileInfo.deviceID.c_str(), DEVICE_ID_LEN);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(fileInfo.dbType);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(fileNum);
    if (errCode != E_OK) {
        return errCode;
    }
    targetFile.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
    if (!targetFile.good()) {
        return -E_INVALID_PATH;
    }
    return E_OK;
}

static int CheckMagicHeader(Parcel &fileHeaderParcel)
{
    vector<char> buffer(MAGIC_LEN, 0);
    (void)fileHeaderParcel.ReadBlob(buffer.data(), MAGIC_LEN);
    if (fileHeaderParcel.IsError()) {
        LOGE("[CheckMagicHeader]fileHeaderParcel error!");
        return -E_PARSE_FAIL;
    }
    if (memcmp(MAGIC.c_str(), buffer.data(), MAGIC_LEN) != 0) {
        return -E_INVALID_FILE;
    }
    return E_OK;
}

static int UnpackFileHeader(ifstream &sourceFile, const string &sourceFileName, FileInfo &fileInfo, uint32_t &fileNum)
{
    vector<uint8_t> fileHeader(FILE_HEADER_LEN, 0);
    sourceFile.read(reinterpret_cast<char *>(fileHeader.data()), FILE_HEADER_LEN);
    if (!sourceFile.good()) {
        LOGE("UnpackFileHeader sourceFile error!");
        return -E_INVALID_FILE;
    }
    Parcel parcel(fileHeader.data(), FILE_HEADER_LEN);
    int errCode = CheckMagicHeader(parcel);
    if (errCode != E_OK) {
        return errCode;
    }
    uint32_t version;
    vector<char> buffer(CHECKSUM_LEN, 0);
    (void)parcel.ReadUInt32(version);
    (void)parcel.ReadBlob(buffer.data(), CHECKSUM_LEN);
    if (parcel.IsError()) {
        LOGE("UnpackFileHeader parcel version error!");
        return -E_PARSE_FAIL;
    }
    vector<char> checksum(CHECKSUM_LEN, 0);
    errCode = GetChecksum(sourceFileName, checksum);
    if (errCode != E_OK) {
        LOGE("Get checksum failed.");
        return errCode;
    }
    if (buffer != checksum) {
        LOGE("Checksum check failed.");
        return -E_INVALID_FILE;
    }
    buffer.resize(DEVICE_ID_LEN);
    (void)parcel.ReadBlob(buffer.data(), DEVICE_ID_LEN);
    if (parcel.IsError()) {
        return -E_PARSE_FAIL;
    }
    fileInfo.deviceID.resize(DEVICE_ID_LEN);
    fileInfo.deviceID.assign(buffer.begin(), buffer.end());
    (void)parcel.ReadUInt32(fileInfo.dbType);
    (void)parcel.ReadUInt32(fileNum);
    if (parcel.IsError()) {
        LOGE("UnpackFileHeader parcel dbType error!");
        return -E_PARSE_FAIL;
    }
    return E_OK;
}

static int PackFileContext(ofstream &targetFile, const FileContext &fileContext)
{
    vector<uint8_t> buffer(FILE_CONTEXT_LEN, 0);
    Parcel parcel(buffer.data(), FILE_CONTEXT_LEN);
    int errCode = parcel.WriteBlob(fileContext.fileName, MAX_FILE_NAME_LEN);
    if (errCode != E_OK) {
        LOGE("PackFileContext fileContext fileName error!");
        return errCode;
    }
    errCode = parcel.WriteUInt32(fileContext.fileType);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt32(0);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt64(fileContext.fileLen);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt64(fileContext.offset);
    if (errCode != E_OK) {
        return errCode;
    }
    targetFile.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
    if (!targetFile.good()) {
        return -E_INVALID_PATH;
    }
    return E_OK;
}

static int UnpackFileContext(ifstream &sourceFile, FileContext &fileContext)
{
    vector<uint8_t> buffer(FILE_CONTEXT_LEN, 0);
    sourceFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    if (!sourceFile.good()) {
        return -E_INVALID_PATH;
    }
    Parcel parcel(buffer.data(), FILE_CONTEXT_LEN);
    (void)parcel.ReadBlob(fileContext.fileName, MAX_FILE_NAME_LEN);
    (void)parcel.ReadUInt32(fileContext.fileType);
    (void)parcel.ReadUInt32(fileContext.parentID);
    (void)parcel.ReadUInt64(fileContext.fileLen);
    (void)parcel.ReadUInt64(fileContext.offset);
    if (parcel.IsError()) {
        return -E_PARSE_FAIL;
    }
    return E_OK;
}

static int PackFileContent(ofstream &targetFile, const string &sourcePath, const FileContext &fileContext)
{
    if (fileContext.fileType != OS::FILE) {
        return E_OK;
    }
    string fileName = sourcePath + fileContext.fileName;
    ifstream file(fileName, ios::in | ios::binary);
    if (!file.good()) {
        LOGE("[PackFileContent] File error! sys[%d]", errno);
        return -E_INVALID_PATH;
    }
    file.seekg(0, ios_base::end);
    if (!file.good()) {
        LOGE("[PackFileContent]file error after seekg! sys[%d]", errno);
        return -E_INVALID_PATH;
    }
    if (file.tellg() < 0) {
        LOGE("[PackFileContent]file error after tellg! sys[%d]", errno);
        return -E_INVALID_PATH;
    }
    uint64_t fileLen = static_cast<uint64_t>(file.tellg());
    file.seekg(0, ios_base::beg);
    if (!file.good()) {
        LOGE("[PackFileContent]file error after seekg fileLen! sys[%d]", errno);
        return -E_INVALID_PATH;
    }

    return FileContentCopy(file, targetFile, fileLen);
}

static int UnpackFileContent(ifstream &sourceFile, const string &targetPath, const FileContext &fileContext)
{
    if (fileContext.fileType != OS::FILE) {
        return E_OK;
    }

    string fileName = fileContext.fileName;
    fileName = targetPath + FILE_SEPARATOR + fileName;

    // check if fileName contains the words ".."
    std::string::size_type pos = fileName.find(INVALID_FILE_WORDS);
    if (pos != std::string::npos) {
        LOGE("[UnpackFileContent]fileName contains the words double dot!!!");
        return -E_INVALID_PATH;
    }

    ofstream file(fileName, ios::out | ios::binary);
    if (!file.good()) {
        file.close();
        LOGE("[UnpackFileContent]Get checksum failed.");
        return -E_INVALID_PATH;
    }
    int errCode = FileContentCopy(sourceFile, file, fileContext.fileLen);
    file.close();
    return errCode;
}

static int WriteChecksum(const string &targetFile)
{
    vector<char> checksum(CHECKSUM_LEN, 0);
    int errCode = GetChecksum(targetFile, checksum);
    if (errCode != E_OK) {
        LOGE("Get checksum failed.");
        return errCode;
    }
    ofstream targetHandle(targetFile, ios::in | ios::out | ios::binary);
    if (!targetHandle.good()) {
        Clear(targetHandle, targetFile);
        LOGE("[WriteChecksum]targetHandle error, sys err [%d]", errno);
        return -E_INVALID_PATH;
    }
    targetHandle.seekp(static_cast<int64_t>(MAGIC_LEN + Parcel::GetUInt32Len()), ios_base::beg);
    if (!targetHandle.good()) {
        Clear(targetHandle, targetFile);
        LOGE("[WriteChecksum]targetHandle error after seekp, sys err [%d]", errno);
        return -E_INVALID_PATH;
    }
    targetHandle.write(checksum.data(), checksum.size());
    if (!targetHandle.good()) {
        Clear(targetHandle, targetFile);
        LOGE("[WriteChecksum]targetHandle error after write, sys err [%d]", errno);
        return -E_INVALID_PATH;
    }
    targetHandle.close();
    return E_OK;
}

static int CopyFilePermissions(const string &sourceFile, const string &targetFile)
{
    uint32_t permissions;
    int errCode = OS::GetFilePermissions(sourceFile, permissions);
    if (errCode != E_OK) {
        LOGE("Get file permissions failed.");
        return errCode;
    }
    errCode = OS::SetFilePermissions(targetFile, permissions);
    if (errCode != E_OK) {
        LOGE("Set file permissions failed.");
    }
    return errCode;
}

int PackageFile::PackageFiles(const string &sourcePath, const string &targetFile,
    const FileInfo &fileInfo)
{
    int errCode = ExePackage(sourcePath, targetFile, fileInfo);
    if (errno == EKEYREVOKED) {
        errCode = -E_EKEYREVOKED;
        LOGE("[PackageFile][PackageFiles] Forbid access files errCode [%d].", errCode);
    }
    return errCode;
}

int PackageFile::GetPackageVersion(const std::string &sourceFile, uint32_t &version)
{
    int errCode = E_OK;
    vector<uint8_t> fileHeader(FILE_HEADER_LEN, 0);
    Parcel parcel(fileHeader.data(), FILE_HEADER_LEN);

    ifstream sourceHandle(sourceFile, ios::in | ios::binary);
    if (!sourceHandle.good()) {
        LOGE("sourceHandle error, sys err [%d]", errno);
        errCode = -E_INVALID_PATH;
        goto END;
    }

    sourceHandle.read(reinterpret_cast<char *>(fileHeader.data()), FILE_HEADER_LEN);
    if (!sourceHandle.good()) {
        LOGE("GetPackageVersion read sourceFile handle error!");
        errCode = -E_INVALID_PATH;
        goto END;
    }

    errCode = CheckMagicHeader(parcel);
    if (errCode != E_OK) {
        errCode = -E_INVALID_PATH;
        goto END;
    }

    (void)parcel.ReadUInt32(version);
END:
    if (errno == EKEYREVOKED) {
        errCode = -E_EKEYREVOKED;
        LOGE("[PackageFile][PackageFiles] Forbid access files by secLabel, errCode [%d].", errCode);
    }
    return errCode;
}

int PackageFile::ExePackage(const string &sourcePath, const string &targetFile,
    const FileInfo &fileInfo)
{
    list<FileContext> fileContexts;
    int errCode = GetFileContexts(sourcePath, fileContexts);
    if (errCode != E_OK) {
        return errCode;
    }
    if (fileContexts.empty()) {
        return -E_EMPTY_PATH;
    }
    ofstream targetHandle(targetFile, ios::out | ios::binary);
    if (!targetHandle.good()) {
        Clear(targetHandle, targetFile);
        LOGE("[PackageFiles]targetHandle error, sys err [%d], [%zu]", errno, fileContexts.size());
        return -E_INVALID_PATH;
    }

    errCode = CopyFilePermissions(sourcePath + FILE_SEPARATOR + string(fileContexts.front().fileName), targetFile);
    if (errCode != E_OK) {
        LOGE("Copy file fail when execute pack files! errCode = [%d]", errCode);
        Clear(targetHandle, targetFile);
        return errCode;
    }

    errCode = PackFileHeader(targetHandle, fileInfo, static_cast<uint32_t>(fileContexts.size()));
    if (errCode != E_OK) {
        Clear(targetHandle, targetFile);
        LOGE("[PackageFiles]Pack file header err[%d]!!!", errCode);
        return errCode;
    }
    // FILE_HEADER_LEN is 92, FILE_CONTEXT_LEN is 280, fileContexts.size() < UINT_MAX, the offset will never overflow.
    uint64_t offset = FILE_HEADER_LEN + FILE_CONTEXT_LEN * static_cast<uint64_t>(fileContexts.size());
    for (auto &file : fileContexts) {
        file.offset = offset;
        errCode = PackFileContext(targetHandle, file);
        if (errCode != E_OK) {
            Clear(targetHandle, targetFile);
            LOGE("[PackageFiles]Pack file context err[%d]!!!", errCode);
            return errCode;
        }
        offset += file.fileLen;
    }
    for (const auto &file : fileContexts) {
        // If file type is path no need pack content in PackFileContent
        errCode = PackFileContent(targetHandle, sourcePath, file);
        if (errCode != E_OK) {
            Clear(targetHandle, targetFile);
            return errCode;
        }
    }
    targetHandle.close();
    return WriteChecksum(targetFile);
}

int PackageFile::UnpackFile(const string &sourceFile, const string &targetPath, FileInfo &fileInfo)
{
    ifstream sourceHandle(sourceFile, ios::in | ios::binary);
    if (!sourceHandle.good()) {
        LOGE("sourceHandle error, sys err [%d]", errno);
        return -E_INVALID_PATH;
    }
    uint32_t fileNum;
    int errCode = UnpackFileHeader(sourceHandle, sourceFile, fileInfo, fileNum);
    if (errCode != E_OK) {
        return errCode;
    }
    FileContext fileContext;
    list<FileContext> fileContexts;
    sourceHandle.seekg(static_cast<int64_t>(FILE_HEADER_LEN), ios_base::beg);
    if (!sourceHandle.good()) {
        return -E_INVALID_PATH;
    }
    for (uint32_t fileCount = 0; fileCount < fileNum; fileCount++) {
        errCode = UnpackFileContext(sourceHandle, fileContext);
        if (errCode != E_OK) {
            return errCode;
        }
        fileContexts.push_back(fileContext);
    }

    for (const auto &file : fileContexts) {
        if (file.fileType == OS::PATH) {
            std::string dirPath = targetPath + "/" + std::string(file.fileName);
            if (!OS::CheckPathExistence(dirPath) && OS::MakeDBDirectory(dirPath) != E_OK) {
                return -E_SYSTEM_API_FAIL;
            }
            continue;
        }
        errCode = UnpackFileContent(sourceHandle, targetPath, file);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}
}
