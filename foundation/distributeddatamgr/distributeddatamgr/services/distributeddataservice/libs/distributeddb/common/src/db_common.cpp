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

#include "db_common.h"

#include <climits>
#include <cstdio>

#include "db_errno.h"
#include "platform_specific.h"
#include "hash.h"
#include "value_hash_calc.h"

namespace DistributedDB {
namespace {
    void RemoveFiles(const std::list<OS::FileAttr> &fileList, OS::FileType type)
    {
        for (const auto &item : fileList) {
            if (item.fileType != type) {
                continue;
            }
            int errCode = remove(item.fileName.c_str());
            if (errCode != 0) {
                LOGE("Remove file failed:%d", errno);
            }
        }
    }

    void RemoveDirectories(const std::list<OS::FileAttr> &fileList, OS::FileType type)
    {
        for (const auto &item : fileList) {
            if (item.fileType != type) {
                continue;
            }
            int errCode = OS::RemoveDBDirectory(item.fileName);
            if (errCode != 0) {
                LOGE("Remove directory failed:%d", errno);
            }
        }
    }
}

int DBCommon::CreateDirectory(const std::string &directory)
{
    bool isExisted = OS::CheckPathExistence(directory);
    if (!isExisted) {
        int errCode = OS::MakeDBDirectory(directory);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

void DBCommon::StringToVector(const std::string &src, std::vector<uint8_t> &dst)
{
    dst.resize(src.size());
    dst.assign(src.begin(), src.end());
}

void DBCommon::VectorToString(const std::vector<uint8_t> &src, std::string &dst)
{
    dst.clear();
    dst.assign(src.begin(), src.end());
}

std::string DBCommon::VectorToHexString(const std::vector<uint8_t> &inVec, const std::string &separator)
{
    std::string hexChar = "0123456789ABCDEF";
    std::string outString;
    for (auto &entry : inVec) {
        outString.push_back(hexChar[entry >> 4]); // high 4 bit to one hex.
        outString.push_back(hexChar[entry & 0x0F]); // low 4 bit to one hex.
        outString += separator;
    }
    outString.erase(outString.size() - separator.size(), separator.size()); // remove needless separator at last
    return outString;
}

void DBCommon::PrintHexVector(const std::vector<uint8_t> &data, int line, const std::string &tag)
{
    const char *hex = "0123456789ABCDEF";
    const size_t maxDataLength = 1024;
    const int byteHexNum = 2;
    size_t dataLength = data.size();

    if (data.size() > maxDataLength) {
        dataLength = maxDataLength;
    }

    char *buff = new (std::nothrow) char[dataLength * byteHexNum + 1]; // dual and add one for the end;
    if (buff == nullptr) {
        return;
    }

    for (std::vector<uint8_t>::size_type i = 0; i < dataLength; ++i) {
        buff[byteHexNum * i] = hex[data[i] >> 4]; // high 4 bit to one hex.
        buff[byteHexNum * i + 1] = hex[data[i] & 0x0F]; // low 4 bit to one hex.
    }
    buff[dataLength * byteHexNum] = '\0';

    if (line == 0) {
        LOGD("[%s] size:%zu -- %s", tag.c_str(), data.size(), buff);
    } else {
        LOGD("[%s][%d] size:%zu -- %s", tag.c_str(), line, data.size(), buff);
    }

    delete []buff;
    return;
}

std::string DBCommon::TransferHashString(const std::string &devName)
{
    if (devName.empty()) {
        return "";
    }
    std::vector<uint8_t> devVect(devName.begin(), devName.end());
    std::vector<uint8_t> hashVect;
    int errCode = CalcValueHash(devVect, hashVect);
    if (errCode != E_OK) {
        return "";
    }

    return std::string(hashVect.begin(), hashVect.end());
}

std::string DBCommon::TransferStringToHex(const std::string &origStr)
{
    if (origStr.empty()) {
        return "";
    }
    const char *hex = "0123456789abcdef";
    std::string tmp;
    for (auto item : origStr) {
        unsigned char currentByte = static_cast<unsigned char>(item);
        tmp.push_back(hex[currentByte >> 4]); // high 4 bit to one hex.
        tmp.push_back(hex[currentByte & 0x0F]); // low 4 bit to one hex.
    }
    return tmp;
}

int DBCommon::CalcValueHash(const std::vector<uint8_t> &value, std::vector<uint8_t> &hashValue)
{
    ValueHashCalc hashCalc;
    int errCode = hashCalc.Initialize();
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }

    errCode = hashCalc.Update(value);
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }

    hashCalc.GetResult(hashValue);
    if (errCode != E_OK) {
        return -E_INTERNAL_ERROR;
    }

    return E_OK;
}

int DBCommon::CreateStoreDirectory(const std::string &directory, const std::string &identifierName,
    const std::string &subDir, bool isCreate)
{
    if (!isCreate) {
        return E_OK;
    }
    if (directory.empty()) {
        return -E_INVALID_ARGS;
    }

    std::string newDir = directory;
    if (newDir.back() != '/') {
        newDir += "/";
    }

    newDir += identifierName;
    int errCode = DBCommon::CreateDirectory(newDir);
    if (errCode != E_OK) {
        return errCode;
    }

    newDir += ("/" + subDir);
    return DBCommon::CreateDirectory(newDir);
}

int DBCommon::CopyFile(const std::string &srcFile, const std::string &dstFile)
{
    const int copyBlockSize = 4096;
    std::vector<uint8_t> tmpBlock(copyBlockSize, 0);
    int errCode;
    FILE *fileIn = fopen(srcFile.c_str(), "rb");
    if (fileIn == nullptr) {
        LOGE("[Common:CpFile] open the source file error:%d", errno);
        return -E_INVALID_FILE;
    }
    FILE *fileOut = fopen(dstFile.c_str(), "wb");
    if (fileOut == nullptr) {
        LOGE("[Common:CpFile] open the target file error:%d", errno);
        errCode = -E_INVALID_FILE;
        goto END;
    }
    for (;;) {
        size_t readSize = fread(static_cast<void *>(tmpBlock.data()), 1, copyBlockSize, fileIn);
        if (readSize < copyBlockSize) {
            // not end and have error.
            if (feof(fileIn) != 0 && ferror(fileIn) != 0) {
                LOGE("Copy the file error:%d", errno);
                errCode = -E_SYSTEM_API_FAIL;
                break;
            }
        }

        if (readSize != 0) {
            size_t writeSize = fwrite(static_cast<void *>(tmpBlock.data()), 1, readSize, fileOut);
            if (ferror(fileOut) != 0 || writeSize != readSize) {
                LOGE("Write the data while copy:%d", errno);
                errCode = -E_SYSTEM_API_FAIL;
                break;
            }
        }

        if (feof(fileIn) != 0) {
            errCode = E_OK;
            break;
        }
    }

END:
    if (fileIn != nullptr) {
        (void)fclose(fileIn);
        fileIn = nullptr;
    }
    if (fileOut != nullptr) {
        (void)fclose(fileOut);
        fileOut = nullptr;
    }
    return errCode;
}

int DBCommon::RemoveAllFilesOfDirectory(const std::string &dir, bool isNeedRemoveDir)
{
    std::list<OS::FileAttr> fileList;
    bool isExisted = OS::CheckPathExistence(dir);
    if (!isExisted) {
        return E_OK;
    }
    int errCode = OS::GetFileAttrFromPath(dir, fileList, true);
    if (errCode != E_OK) {
        return errCode;
    }

    RemoveFiles(fileList, OS::FileType::FILE);
    RemoveDirectories(fileList, OS::FileType::PATH);
    if (isNeedRemoveDir) {
        // Pay attention to the order of deleting the directory
        if (OS::CheckPathExistence(dir) && OS::RemoveDBDirectory(dir.c_str()) != 0) {
            LOGI("Remove the directory error:%d", errno);
            errCode = -E_SYSTEM_API_FAIL;
        }
    }

    return errCode;
}

std::string DBCommon::GenerateIdentifierId(const std::string &storeId,
    const std::string &appId, const std::string &userId)
{
    return userId + "-" + appId + "-" + storeId;
}

void DBCommon::SetDatabaseIds(KvDBProperties &properties, const std::string &appId, const std::string &userId,
    const std::string &storeId)
{
    properties.SetStringProp(KvDBProperties::APP_ID, appId);
    properties.SetStringProp(KvDBProperties::USER_ID, userId);
    properties.SetStringProp(KvDBProperties::STORE_ID, storeId);
    std::string oriStoreDir;
    std::string identifier = GenerateIdentifierId(storeId, appId, userId);
    if (properties.GetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, false)) {
        oriStoreDir = storeId;
    } else {
        oriStoreDir = identifier;
    }
    std::string hashIdentifier = TransferHashString(identifier);
    properties.SetStringProp(KvDBProperties::IDENTIFIER_DATA, hashIdentifier);
    std::string hashDir = TransferHashString(oriStoreDir);
    std::string hexHashDir = TransferStringToHex(hashDir);
    properties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, hexHashDir);
}
} // namespace DistributedDB
