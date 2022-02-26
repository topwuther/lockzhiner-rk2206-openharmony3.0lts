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

#include "fms_file_share_ability_test.h"

#include "../include/fd_holder.h"
#include "../include/file_share_ability.h"
#include "../include/fms_utils.h"
#include "../include/log_util.h"
#include "parcel.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <uri.h>

namespace OHOS {
using namespace std;
using Uri = OHOS::Uri;
using namespace OHOS::AppExecFwk;
using namespace OHOS::FileManager;
using namespace testing::ext;
FileShareAbility *mFileShareAbility;
class FmsFileShareAbilityTest : public testing::Test {
public:
    FmsFileShareAbilityTest();
    ~FmsFileShareAbilityTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
};

FmsFileShareAbilityTest::FmsFileShareAbilityTest() {}

FmsFileShareAbilityTest::~FmsFileShareAbilityTest() {}

void FmsFileShareAbilityTest::SetUpTestCase()
{
    /*
     * Get instance of FileShareAbility for tests
     */
    mFileShareAbility = FileShareAbility::Instance();

    /*
     * Prepare directorys for tests
     */
    string dpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test";
    int a = 0777;
    int mkdirRes = mkdir(dpath.c_str(), a);
    if (mkdirRes == -1) {
        cout << "Failed to create dir: " << dpath << endl;
    } else {
        cout << "success to create dir: " << dpath << endl;
    }

    /*
     * Prepare files for tests
     */
    string fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/a.txt";
    int fd = open(fpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, a);
    if (fd == -1) {
        cout << "Failed to create file: " << fpath << endl;
    } else {
        cout << "success to create file: " << fpath << endl;
        close(fd);
    }
}
void FmsFileShareAbilityTest::TearDownTestCase()
{
    /*
     * remove files for tests
     */
    string fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/a.txt";
    int rmFileRes = remove(fpath.c_str());
    if (rmFileRes == -1) {
        cout << "Failed to remove file: " << fpath << endl;
    } else {
        cout << "success to remove file: " << fpath << endl;
    }

    /*
     * remove directorys for tests
     */
    string dpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test";
    int rmDirRes = remove(dpath.c_str());
    if (rmDirRes == -1) {
        cout << "Failed to remove dir: " << dpath << endl;
    } else {
        cout << "success to remove dir: " << fpath << endl;
    }
}

void FmsFileShareAbilityTest::SetUp() {}

void FmsFileShareAbilityTest::TearDown() {}

/**
 * @tc.number SUB_STORAGE_FMS_Function_4700
 * @tc.name FMS_Function_4700
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_4700, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::vector<std::string> columns;
        columns.push_back("fileSize");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_4800
 * @tc.name FMS_Function_4800
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_4800, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_"
                "leaf/a.txt/leaf?displayName=dsf#");
        std::vector<std::string> columns;
        columns.push_back("fileSize");
        columns.push_back("fileName");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_4900
 * @tc.name FMS_Function_4900
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_4900, testing::ext::TestSize.Level1)
{
    int resultSet;
    const char *fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
    int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
        cout << "create failed." << endl;
    } else {
        close(fd);
        cout << "create success." << endl;
    }
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf2/abc.txt");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Delete(uri, predicates);
        cout << resultSet << endl;
        EXPECT_EQ(resultSet, 1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5000
 * @tc.name FMS_Function_5000
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5000, testing::ext::TestSize.Level1)
{
    int resultSet;
    const char *fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
    int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
        cout << "create failed." << endl;
    } else {
        close(fd);
        cout << "create success." << endl;
    }
    try {
        Uri uri("dataability://devices_id/ohos.acts.test/primary/document/files/primary_leaf2/abc.txt");
        std::vector<std::string> columns;
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Delete(uri, predicates);
        EXPECT_EQ(resultSet, 1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5100
 * @tc.name FMS_Function_5100
 * @tc.desc Test function of GetType interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5100, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::string result = mFileShareAbility->GetType(uri);
        EXPECT_NE(result, "");
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5200
 * @tc.name FMS_Function_5200
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5200, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::string mode = "r";
        int result = mFileShareAbility->OpenFile(uri, mode);
        EXPECT_NE(result, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5300
 * @tc.name FMS_Function_5300
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5300, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::string mode = "w";
        int result = mFileShareAbility->OpenFile(uri, mode);
        EXPECT_NE(result, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5400
 * @tc.name FMS_Function_5400
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5400, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::string mode = "wa";
        int result = mFileShareAbility->OpenFile(uri, mode);
        EXPECT_NE(result, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5500
 * @tc.name FMS_Function_5500
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5500, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::string mode = "rwt";
        int result = mFileShareAbility->OpenFile(uri, mode);
        EXPECT_NE(result, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Function_5600
 * @tc.name FMS_Function_5600
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Function_5600, testing::ext::TestSize.Level1)
{
    int resultSet;
    try {
        Uri uri(
            "dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt/leaf?displayName=dsf#");
        std::string mode = "rwt";
        resultSet = mFileShareAbility->OpenFile(uri, mode);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3000
 * @tc.name FMS_Error_3000
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3000, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test/primary/document/primary_leaf/test/abcd.txt");
        std::vector<std::string> columns;
        columns.push_back("fileSize");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3100
 * @tc.name FMS_Error_3100
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3100, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test/primary/document/primary_leaf");
        std::vector<std::string> columns;
        columns.push_back("fileSize");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3200
 * @tc.name FMS_Error_3200
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3200, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::vector<std::string> columns;
        columns.push_back("notExist");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_EQ(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3250
 * @tc.name FMS_Error_3250
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3250, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("undatunaability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
        std::vector<std::string> columns;
        columns.push_back("fileSize");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3300
 * @tc.name FMS_Error_3300
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3300, testing::ext::TestSize.Level1)
{
    int resultSet;
    try {
        Uri uri("");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Delete(uri, predicates);
        cout << "after mFileShareAbility->Delete" << endl;
        EXPECT_EQ(resultSet, 0);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3400
 * @tc.name FMS_Error_3400
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3400, testing::ext::TestSize.Level1)
{
    try {
        string fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
        int fd = open(fpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd == -1) {
            cout << "create failed." << endl;
        } else {
            cout << "create success." << endl;
        }
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/abc.txt");
        DataAbilityPredicates predicates("predicatestest");
        int resultSet = mFileShareAbility->Delete(uri, predicates);
        EXPECT_EQ(resultSet, 0);
        close(fd);
        string fpath1 = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
        int ret = remove(fpath1.c_str());
        if (ret == -1) {
            cout << "Failed to remove file: " << fpath1 << endl;
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3500
 * @tc.name FMS_Error_3500
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3500, testing::ext::TestSize.Level1)
{
    try {
        string fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc";
        int fd = open(fpath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd == -1) {
            cout << "create failed." << endl;
        } else {
            cout << "create success." << endl;
        }
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf9/abc");
        DataAbilityPredicates predicates("predicatestest");
        int resultSet = mFileShareAbility->Delete(uri, predicates);
        EXPECT_EQ(resultSet, 0);
        string fpath1 = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc";
        int ret = remove(fpath1.c_str());
        if (ret == -1) {
            cout << "Failed to remove file: " << fpath1 << endl;
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3600
 * @tc.name FMS_Error_3600
 * @tc.desc Test function of GetType interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3600, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/unexist/a.txt");
        std::string result = mFileShareAbility->GetType(uri);
        EXPECT_EQ(result, "");
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3700
 * @tc.name FMS_Error_3700
 * @tc.desc Test function of GetType interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3700, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf11/a");
        std::string resultSet = mFileShareAbility->GetType(uri);
        EXPECT_EQ(resultSet, "application/octet-stream");
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3800
 * @tc.name FMS_Error_3800
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3800, testing::ext::TestSize.Level1)
{
    try {
        Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/deh.txt");
        std::string mode = "rwt";
        int result = mFileShareAbility->OpenFile(uri, mode);
        EXPECT_EQ(result, -1);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Error_3900
 * @tc.name FMS_Error_3900
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Error_3900, testing::ext::TestSize.Level1)
{
    try {
        std::shared_ptr<ResultSet> resultSet;
        Uri uri("dataability:///ohos.acts.test.ability/primary/document/files/primary_leaf/a.txt");
        std::vector<std::string> columns;
        columns.push_back("fileName");
        DataAbilityPredicates predicates("predicatestest");
        resultSet = mFileShareAbility->Query(uri, columns, predicates);
        cout << resultSet << endl;
        EXPECT_NE(resultSet, nullptr);
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2000
 * @tc.name FMS_Performance_2000
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2000, testing::ext::TestSize.Level1)
{
    std::shared_ptr<ResultSet> resultSet;
    for (int i = 0; i < 1000; i++) {
        try {
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::vector<std::string> columns;
            columns.push_back("fileSize");
            DataAbilityPredicates predicates("predicatestest");
            resultSet = mFileShareAbility->Query(uri, columns, predicates);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2000 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << i
                      << endl;
            EXPECT_NE(resultSet, nullptr);
        } catch (...) {
            EXPECT_NE(nullptr, nullptr);
            cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
        }
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2100
 * @tc.name FMS_Performance_2100
 * @tc.desc Test function of Query interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2100, testing::ext::TestSize.Level1)
{
    for (int i = 1; i < 1000; i++) {
        try {
            std::shared_ptr<ResultSet> resultSet;
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::vector<std::string> columns;
            columns.push_back("fileSize");
            DataAbilityPredicates predicates("predicatestest");
            resultSet = mFileShareAbility->Query(uri, columns, predicates);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2100 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << i
                      << endl;
            EXPECT_EQ(resultSet, nullptr);
        } catch (...) {
            EXPECT_NE(nullptr, nullptr);
            cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
        }
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2200
 * @tc.name FMS_Performance_2200
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2200, testing::ext::TestSize.Level1)
{
    for (int i = 1; i < 1000; i++) {
        const char *fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
        int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd == -1) {
            cout << "create failed." << endl;
        } else {
            close(fd);
            cout << "create success." << endl;
        }
        try {
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf2/abc.txt");
            DataAbilityPredicates predicates("predicatestest");
            int resultSet = mFileShareAbility->Delete(uri, predicates);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2200 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << i
                      << endl;
            EXPECT_EQ(resultSet, 1);
        } catch (...) {
            EXPECT_NE(nullptr, nullptr);
            cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
        }
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2300
 * @tc.name FMS_Performance_2300
 * @tc.desc Test function of Delete interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2300, testing::ext::TestSize.Level1)
{
    for (int i = 1; i < 1000; i++) {
        const char *fpath = "/data/accounts/account_0/appdata/ohos.acts.test/files/test/abc.txt";
        int fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd == -1) {
            cout << "create failed." << endl;
        } else {
            close(fd);
            cout << "create success." << endl;
        }
        try {
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            Uri uri("dataability://devices_id/ohos.acts.test/primary/document/files/primary_leaf2/abc.txt");
            DataAbilityPredicates predicates("predicatestest");
            int resultSet = mFileShareAbility->Delete(uri, predicates);
            std::cout << tv1.tv_usec << "us" << endl;
            gettimeofday(&tv1, nullptr);
            std::cout << "FMS_performance_2300 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << i
                      << endl;
            EXPECT_EQ(resultSet, 1);
        } catch (...) {
            EXPECT_NE(nullptr, nullptr);
            cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
        }
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2400
 * @tc.name FMS_Performance_2400
 * @tc.desc Test function of GetType interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2400, testing::ext::TestSize.Level1)
{
    try {
        for (int a = 0; a < 1000; a++) {
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            std::string result = mFileShareAbility->GetType(uri);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2400 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << a
                      << endl;
            EXPECT_NE(result, "");
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2500
 * @tc.name FMS_Performance_2500
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2500, testing::ext::TestSize.Level1)
{
    try {
        for (int a = 0; a < 1000; a++) {
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::string mode = "r";
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            int result = mFileShareAbility->OpenFile(uri, mode);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2500 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << a
                      << endl;
            EXPECT_NE(result, -1);
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2600
 * @tc.name FMS_Performance_2600
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2600, testing::ext::TestSize.Level1)
{
    try {
        for (int a = 0; a < 1000; a++) {
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::string mode = "w";
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            int result = mFileShareAbility->OpenFile(uri, mode);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2600 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << a
                      << endl;
            EXPECT_NE(result, -1);
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2700
 * @tc.name FMS_Performance_2700
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2700, testing::ext::TestSize.Level1)
{
    try {
        for (int a = 0; a < 1000; a++) {
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::string mode = "wa";
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            int result = mFileShareAbility->OpenFile(uri, mode);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2700 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << a
                      << endl;
            EXPECT_NE(result, -1);
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}

/**
 * @tc.number SUB_STORAGE_FMS_Performance_2800
 * @tc.name FMS_Performance_2800
 * @tc.desc Test function of OpenFile interface.
 */
HWTEST_F(FmsFileShareAbilityTest, FMS_Performance_2800, testing::ext::TestSize.Level1)
{
    try {
        for (int a = 0; a < 1000; a++) {
            Uri uri("dataability:///ohos.acts.test/primary/document/files/primary_leaf/a.txt");
            std::string mode = "rwt";
            struct timeval tv;
            struct timeval tv1;
            gettimeofday(&tv, nullptr);
            std::cout << tv.tv_usec << "us" << endl;
            int result = mFileShareAbility->OpenFile(uri, mode);
            gettimeofday(&tv1, nullptr);
            std::cout << tv1.tv_usec << "us" << endl;
            std::cout << "FMS_performance_2800 Time use:"
                      << (tv1.tv_sec - tv.tv_sec) * 1000000 + (tv1.tv_usec - tv.tv_usec) << "us," << a
                      << endl;
            EXPECT_NE(result, -1);
        }
    } catch (...) {
        EXPECT_NE(nullptr, nullptr);
        cout << "FmsFileShareAbilityTest-an exception occurred." << endl;
    }
}
} // namespace OHOS
