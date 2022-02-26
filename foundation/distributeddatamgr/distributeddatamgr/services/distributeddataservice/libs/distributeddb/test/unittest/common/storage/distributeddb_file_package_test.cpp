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

#include <gtest/gtest.h>
#include <fstream>

#include "distributeddb_tools_unit_test.h"
#include "securec.h"
#include "package_file.h"
#include "db_errno.h"
#include "platform_specific.h"
#include "value_hash_calc.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testPath;
    string g_sourcePath = "/source/";
    string g_packageResultPath = "/package_result/";
    string g_unpackResultPath = "/unpack_result/";
    FileInfo g_fileInfo;
    const string PACKAGE_RESULT_FILE_NAME = "package_result.dat";
    const string NON_EXIST_PATH = "/nonexist/";
    const string FILE_NAME_1 = "file1.txt";
    const string FILE_NAME_2 = "file2.dat";
    const string FILE_CONTENT_1 = "Hello world.";
    const int FILE_CONTENT_2_LEN = 4;
    const char FILE_CONTENT_2[FILE_CONTENT_2_LEN] = {0x5B, 0x3A, 0x29, 0x3E};
    const int BUFFER_SIZE = 4096;
    const int DEVICE_ID_LEN = 32;
    const vector<uint8_t> DIVICE_ID = {'a', 'e', 'i', 'o', 'u'};

    void RemovePath(const string &path)
    {
        list<OS::FileAttr> files;
        int errCode = OS::GetFileAttrFromPath(path, files);
        ASSERT_EQ(errCode, E_OK);
        string fileName;
        for (auto file : files) {
            fileName = path + "/" + file.fileName;
            switch (file.fileType) {
                case OS::FILE:
                    (void)remove(fileName.c_str());
                    break;
                case OS::PATH:
                    if (file.fileName != "." && file.fileName != "..") {
                        RemovePath(fileName);
                    }
                    break;
                default:
                    break;
            }
        }
        (void)OS::RemoveDBDirectory(path);
    }

    bool CompareFileName(const OS::FileAttr &file1, const OS::FileAttr &file2)
    {
        return file1.fileName <= file2.fileName;
    }

    void ComparePath(const string &path1, const string &path2)
    {
        list<OS::FileAttr> files1;
        int errCode = OS::GetFileAttrFromPath(path1, files1);
        ASSERT_EQ(errCode, E_OK);
        files1.sort(CompareFileName);
        list<OS::FileAttr> files2;
        errCode = OS::GetFileAttrFromPath(path2, files2);
        ASSERT_EQ(errCode, E_OK);
        files2.sort(CompareFileName);
        ASSERT_EQ(files1.size(), files2.size());
        auto fileIter1 = files1.begin();
        auto fileIter2 = files2.begin();
        vector<char> buffer1(BUFFER_SIZE, 0);
        vector<char> buffer2(BUFFER_SIZE, 0);
        string bufferStr1;
        string bufferStr2;
        for (; fileIter1 != files1.end() && fileIter2 != files2.end(); fileIter1++, fileIter2++) {
            ASSERT_STREQ(fileIter1->fileName.c_str(), fileIter2->fileName.c_str());
            ASSERT_EQ(fileIter1->fileType, fileIter2->fileType);
            ASSERT_EQ(fileIter1->fileLen, fileIter2->fileLen);
            if (fileIter1->fileType != OS::FILE) {
                continue;
            }
            ifstream file1(path1 + fileIter1->fileName, ios::out | ios::binary);
            ASSERT_TRUE(file1.is_open());
            ifstream file2(path2 + fileIter2->fileName, ios::out | ios::binary);
            ASSERT_TRUE(file2.is_open());
            buffer1.assign(BUFFER_SIZE, 0);
            buffer2.assign(BUFFER_SIZE, 0);
            for (file1.read(buffer1.data(), BUFFER_SIZE), file2.read(buffer2.data(), BUFFER_SIZE);
                !(file1.eof() || file2.eof());
                file1.read(buffer1.data(), BUFFER_SIZE), file2.read(buffer2.data(), BUFFER_SIZE)) {
                bufferStr1.assign(buffer1.begin(), buffer1.end());
                bufferStr2.assign(buffer2.begin(), buffer2.end());
                ASSERT_STREQ(bufferStr1.c_str(), bufferStr2.c_str());
            }
            file1.close();
            file2.close();
            bufferStr1.assign(buffer1.begin(), buffer1.end());
            bufferStr2.assign(buffer2.begin(), buffer2.end());
            ASSERT_STREQ(bufferStr1.c_str(), bufferStr2.c_str());
        }
    }
}

class DistributedDBFilePackageTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBFilePackageTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testPath);
    g_sourcePath = g_testPath + g_sourcePath;
    g_packageResultPath = g_testPath + g_packageResultPath;
    g_unpackResultPath = g_testPath + g_unpackResultPath;
    (void)OS::MakeDBDirectory(g_sourcePath);
    ofstream file1(g_sourcePath + FILE_NAME_1, ios::out | ios::binary | ios::trunc);
    ASSERT_TRUE(file1.is_open());
    file1.write(FILE_CONTENT_1.c_str(), FILE_CONTENT_1.size());
    file1.close();
    ofstream file2(g_sourcePath + FILE_NAME_2, ios::out | ios::binary | ios::trunc);
    ASSERT_TRUE(file2.is_open());
    file2.write(FILE_CONTENT_2, FILE_CONTENT_2_LEN);
    file2.close();
    g_fileInfo.dbType = 1;
    ValueHashCalc calc;
    int errCode = calc.Initialize();
    ASSERT_EQ(errCode, E_OK);
    errCode = calc.Update(DIVICE_ID);
    ASSERT_EQ(errCode, E_OK);
    vector<uint8_t> deviceIDVec;
    errCode = calc.GetResult(deviceIDVec);
    ASSERT_EQ(errCode, E_OK);
    g_fileInfo.deviceID.resize(DEVICE_ID_LEN);
    g_fileInfo.deviceID.assign(deviceIDVec.begin(), deviceIDVec.end());
}

void DistributedDBFilePackageTest::TearDownTestCase(void)
{
    RemovePath(g_testPath);
}

void DistributedDBFilePackageTest::SetUp(void)
{
    (void)OS::MakeDBDirectory(g_packageResultPath);
    (void)OS::MakeDBDirectory(g_unpackResultPath);
}

void DistributedDBFilePackageTest::TearDown(void)
{
    RemovePath(g_packageResultPath);
    RemovePath(g_unpackResultPath);
}

/**
  * @tc.name: PackageFileTest001
  * @tc.desc: Test file package and unpack functions.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest001, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath, g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, E_OK);
    FileInfo fileInfo;
    errCode = PackageFile::UnpackFile(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_unpackResultPath, fileInfo);
    ASSERT_EQ(errCode, E_OK);
    ComparePath(g_sourcePath, g_unpackResultPath);
    ASSERT_EQ(fileInfo.dbType, g_fileInfo.dbType);
    ASSERT_EQ(fileInfo.deviceID == g_fileInfo.deviceID, true);
    return;
}

/**
  * @tc.name: PackageFileTest002
  * @tc.desc: Test file package if source path is not exist.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest002, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath + NON_EXIST_PATH,
        g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_PATH);
    return;
}

/**
  * @tc.name: PackageFileTest003
  * @tc.desc: Test file package if result path is not exist.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest003, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath,
        g_packageResultPath + NON_EXIST_PATH + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_PATH);
    return;
}

/**
  * @tc.name: PackageFileTest004
  * @tc.desc: Test file package if source path is empty.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest004, TestSize.Level0)
{
    // Clear source files.
    RemovePath(g_sourcePath);
    (void)OS::MakeDBDirectory(g_sourcePath);
    // Test function.
    int errCode = PackageFile::PackageFiles(g_sourcePath, g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, -E_EMPTY_PATH);
    // Create source files again.
    ofstream file1(g_sourcePath + FILE_NAME_1, ios::out | ios::binary | ios::trunc);
    ASSERT_TRUE(file1.is_open());
    file1.write(FILE_CONTENT_1.c_str(), FILE_CONTENT_1.size());
    file1.close();
    ofstream file2(g_sourcePath + FILE_NAME_2, ios::out | ios::binary | ios::trunc);
    ASSERT_TRUE(file2.is_open());
    file2.write(FILE_CONTENT_2, 4);
    file2.close();
    return;
}

/**
  * @tc.name: PackageFileTest005
  * @tc.desc: Test file unpack if source file is not exist.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest005, TestSize.Level0)
{
    FileInfo fileInfo;
    int errCode = PackageFile::UnpackFile(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_unpackResultPath, fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_PATH);
    return;
}

/**
  * @tc.name: PackageFileTest006
  * @tc.desc: Test file unpack if result path is not exist.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest006, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath, g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, E_OK);
    FileInfo fileInfo;
    errCode = PackageFile::UnpackFile(g_packageResultPath + PACKAGE_RESULT_FILE_NAME,
        g_unpackResultPath + NON_EXIST_PATH, fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_PATH);
    return;
}

/**
  * @tc.name: PackageFileTest007
  * @tc.desc: Test file unpack if magic check failed.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest007, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath, g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, E_OK);
    // Change package file header.
    const string REPLACE_FILE_HEADER = "test";
    fstream file(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, ios::in | ios::out | ios::binary);
    ASSERT_TRUE(file.is_open());
    file.seekp(0, ios_base::beg);
    file.write(REPLACE_FILE_HEADER.c_str(), REPLACE_FILE_HEADER.size());
    file.close();
    // Unpack file.
    FileInfo fileInfo;
    errCode = PackageFile::UnpackFile(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_unpackResultPath, fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_FILE);
    return;
}

/**
  * @tc.name: PackageFileTest008
  * @tc.desc: Test file unpack if checksum check failed.
  * @tc.type: FUNC
  * @tc.require: AR000D4879
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBFilePackageTest, PackageFileTest008, TestSize.Level0)
{
    int errCode = PackageFile::PackageFiles(g_sourcePath, g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_fileInfo);
    ASSERT_EQ(errCode, E_OK);
    // Rewrite package file without file tail.
    ifstream fileIn(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, ios::in | ios::binary);
    ASSERT_TRUE(fileIn.is_open());
    fileIn.seekg(0, ios_base::end);
    int fileLen = fileIn.tellg();
    fileIn.seekg(0, ios_base::beg);
    const int CUT_TAIL = 3;
    int bufferLen = fileLen > BUFFER_SIZE ? BUFFER_SIZE : fileLen - CUT_TAIL;
    vector<char> buffer(bufferLen, 0);
    fileIn.read(buffer.data(), buffer.size());
    fileIn.close();
    ofstream fileOut(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, ios::out | ios::binary | ios::trunc);
    ASSERT_TRUE(fileOut.is_open());
    fileOut.write(buffer.data(), buffer.size());
    fileOut.close();
    // Unpack file.
    FileInfo fileInfo;
    errCode = PackageFile::UnpackFile(g_packageResultPath + PACKAGE_RESULT_FILE_NAME, g_unpackResultPath, fileInfo);
    ASSERT_EQ(errCode, -E_INVALID_FILE);
    return;
}

