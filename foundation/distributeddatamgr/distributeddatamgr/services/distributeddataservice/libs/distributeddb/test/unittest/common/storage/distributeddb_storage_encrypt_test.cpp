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

#include <functional>
#include <thread>
#include <chrono>
#include <thread>

#include <openssl/rand.h>

#include "db_types.h"
#include "log_print.h"
#include "sqlite3.h"
#include "securec.h"

#ifndef OMIT_ENCRYPT
using namespace testing::ext;
using namespace DistributedDB;
using namespace std;
using namespace std::placeholders;

namespace {
    sqlite3 *g_db = nullptr;
    const string STORE_ID = "test";
    const string STORE_ID2 = "test2";
    const string STORE_ID3 = "test3";
    const int PASSWD_TEST_SIZE = 16;
    char g_oldPasswd[PASSWD_TEST_SIZE + 1] = {0};
    char g_newPasswd[PASSWD_TEST_SIZE + 1] = {0};
    char g_diffPasswd[PASSWD_TEST_SIZE + 1] = {0};
    const string ALG1 = "'aes-256-gcm'";
    const string ALG2 = "'aes-256-cbc'";
    const string ALG3 = "'ABCDEG'";
    const int ITERATION = 64000;
    const int ITERATION2 = 1000;
    const std::string CREATE_SQL = "CREATE TABLE IF NOT EXISTS data(key TEXT PRIMARY KEY, value TEXT);";
    const int SLEEP_TIME = 1;

    const std::vector<uint8_t> KEY_1 = {'A'};
    const std::vector<uint8_t> VALUE_1 = {'1'};
    const std::vector<uint8_t> VALUE_2 = {'2'};

    const std::string PRAGMA_CIPHER = "PRAGMA codec_cipher=";
    const std::string PRAGMA_KDF_ITER = "PRAGMA codec_kdf_iter=";
    const std::string EXPORT_STRING = "export_database";

    int Callback(void *data, int argc, char **argv, char **azColName)
    {
        vector<uint8_t> *value = static_cast<vector<uint8_t> *>(data);
        value->push_back(*argv[0]);
        return 0;
    }

    int Open(sqlite3 *&db, const string &storeID)
    {
        std::string uri = "file:" + storeID + ".db";
        sqlite3 *dbTemp = nullptr;
        int errCode = sqlite3_open_v2(uri.c_str(), &dbTemp,
            SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        if (errCode != SQLITE_OK) {
            if (dbTemp != nullptr) {
                (void)sqlite3_close_v2(dbTemp);
                dbTemp = nullptr;
            }
            return errCode;
        }
        db = dbTemp;

        return errCode;
    }

    int CreateTable()
    {
        char *errMsg = nullptr;
        int errCode = sqlite3_exec(g_db, CREATE_SQL.c_str(), nullptr, nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }

        return errCode;
    }

    int SetEncryptParam(const char *passwd, int iterNumber, const string &algName)
    {
        char *errMsg = nullptr;
        int errCode = sqlite3_key(g_db, static_cast<const void *>(passwd), strlen(passwd));
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }

        errCode = sqlite3_exec(g_db, (PRAGMA_KDF_ITER + to_string(iterNumber)).c_str(), nullptr, nullptr,
                               &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }

        errCode = sqlite3_exec(g_db, (PRAGMA_CIPHER + algName + ";").c_str(), nullptr, nullptr,
                               &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }

        return errCode;
    }

    int OpenWithKey(const char *passwd, int iterNumber, const string &algName, bool isEncrypted)
    {
        int errCode = Open(g_db, STORE_ID);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        if (isEncrypted) {
            errCode = SetEncryptParam(passwd, iterNumber, algName);
            if (errCode != SQLITE_OK) {
                return errCode;
            }
        }

        errCode = CreateTable();
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        errCode = sqlite3_close(g_db);
        if (errCode != SQLITE_OK) {
            return errCode;
        }

        errCode = Open(g_db, STORE_ID);
        if (errCode != SQLITE_OK) {
            return errCode;
        }

        return errCode;
    }

    int InputPasswd(const char *passwd, int iterNumber, const string &algName)
    {
        int errCode = SetEncryptParam(passwd, iterNumber, algName);
        if (errCode != SQLITE_OK) {
            return errCode;
        }

        return errCode;
    }

    int Reconnect(const char *passwd, int iterNumber, const string &algName)
    {
        int errCode = Open(g_db, STORE_ID);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        errCode = InputPasswd(passwd, iterNumber, algName);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        errCode = InputPasswd(passwd, iterNumber, algName);
        if (errCode != SQLITE_OK) {
            return errCode;
        }

        return errCode;
    }

    int PutValue(const Key &key, const Value &value)
    {
        char *errMsg = nullptr;
        string keyStr(key.begin(), key.end());
        string valueStr(value.begin(), value.end());
        int errCode = sqlite3_exec(g_db, ("INSERT OR REPLACE INTO data VALUES('" + keyStr + "','" + valueStr +
            "');").c_str(), nullptr, nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int DeleteValue(const Key &key)
    {
        char *errMsg = nullptr;
        string keyStr(key.begin(), key.end());
        int errCode = sqlite3_exec(g_db, ("DELETE FROM data WHERE key='" + keyStr + "';").c_str(), nullptr,
            nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int UpdateValue(const Key &key, const Value &value)
    {
        char *errMsg = nullptr;
        string keyStr(key.begin(), key.end());
        string valueStr(value.begin(), value.end());
        int errCode = sqlite3_exec(g_db, ("INSERT OR REPLACE INTO data VALUES('" + keyStr + "','" + valueStr +
            "');").c_str(), nullptr, nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int GetValue(const Key &key, Value &value)
    {
        char *errMsg = nullptr;
        string keyStr(key.begin(), key.end());
        int errCode = sqlite3_exec(g_db, ("SELECT value from data WHERE key='" + keyStr + "';").c_str(),
            Callback, static_cast<void *>(&value), &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int Export(const string &dbName)
    {
        char *errMsg = nullptr;
        int errCode = sqlite3_exec(g_db, ("SELECT " + EXPORT_STRING + "('" + dbName + "');").c_str(), nullptr, nullptr,
            &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int Attach(const string &dbName)
    {
        char *errMsg = nullptr;
        int errCode = sqlite3_exec(g_db, ("attach '" + dbName + ".db' as " + dbName + " key '';").c_str(),
            nullptr, nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int AttachWithKey(const string &dbName, const char *passwd)
    {
        char *errMsg = nullptr;
        int errCode = sqlite3_exec(g_db, ("attach '" + dbName + ".db' as " + dbName + " key '" + passwd + "';").c_str(),
            nullptr, nullptr, &errMsg);
        if (errCode != SQLITE_OK && errMsg != nullptr) {
            LOGE(" [SQLITE]: %s", errMsg);
            sqlite3_free(errMsg);
            return errCode;
        }
        return errCode;
    }

    int MultipleOperation(Value &valueGet, const Value &valueUpdate)
    {
        int errCode = PutValue(KEY_1, VALUE_1);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        errCode = UpdateValue(KEY_1, valueUpdate);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        GetValue(KEY_1, valueGet);
        errCode = DeleteValue(KEY_1);
        if (errCode != SQLITE_OK) {
            return errCode;
        }
        return errCode;
    }
}

class DistributedDBStorageEncryptTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageEncryptTest::SetUpTestCase(void)
{
    unsigned char initialByte = 0;
    RAND_bytes(&initialByte, 1);
    for (int i = 0; i < PASSWD_TEST_SIZE; i++) {
        initialByte %= 20; // keep the number < 20, so 'A' + 20 is the maximum alphabet.
        g_oldPasswd[i] = ('A' + initialByte++);
        g_newPasswd[i] = ('A' + initialByte++);
        g_diffPasswd[i] = ('A' + initialByte++);
    }
}

void DistributedDBStorageEncryptTest::TearDownTestCase(void)
{
}

void DistributedDBStorageEncryptTest::SetUp(void)
{
    /**
      * @tc.Clean DB files created from every test case.
      */
    remove((STORE_ID + ".db").c_str());
    remove((STORE_ID2 + ".db").c_str());
}

void DistributedDBStorageEncryptTest::TearDown(void)
{
    /**
      * @tc.make sure g_db is nullptr and is closed.
      */
    if (g_db != nullptr) {
        g_db = nullptr;
    }
    /**
      * @tc.Clean DB files created from every test case.
      */
    remove((STORE_ID + ".db").c_str());
    remove((STORE_ID2 + ".db").c_str());
    /**
      * @tc.Wait a number of SLEEP_TIME until remove done.
      */
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
}

/**
  * @tc.name: EncryptTest001
  * @tc.desc: Check if opening database possible without encryption
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest001, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without being encrypted.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2/5. Add, Update, Get and Delete the data.
      * @tc.expected: step2/5. Return SQLITE_OK.
      */
    Value valueGet;
    Value valueUpdate = VALUE_2;
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step6. Close DB.
      * @tc.expected: step6. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest002
  * @tc.desc: Check if it is possible to open nonencrypted database with password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest002, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a nonencrypted DB.
      * @tc.expected: step1. Return 0.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2. Set the key to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return NOT_EQUAL_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_NE(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_NE(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest003
  * @tc.desc: Check if deciphering an encrypted database possible with wrong password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest003, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
      * @tc.expected: step1. Return 0.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_diffPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_diffPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
     * @tc.steps:step3/6. Add, Update, Get and Delete the data.
     * @tc.expected: step3/6. Return NOT_EQUAL_OK values.
     */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_NE(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_NE(valueGet, valueUpdate);

    /**
     * @tc.steps:step7. Close DB.
     * @tc.expected: step7. Return SQLITE_OK.
     */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest004
  * @tc.desc: Check if deciphering an encrypted database possible with correct password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
     * @tc.expected: step1. Return 0.
     */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
     * @tc.steps:step2. Set password to g_oldPasswd.
     * @tc.expected: step2. Return SQLITE_OK.
     */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
     * @tc.steps:step3/6. Add, Update, Get and Delete the data.
     * @tc.expected: step3/6. Return SQLITE_OK values.
     */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
     * @tc.steps:step7. Close DB.
     * @tc.expected: step7. Return SQLITE_OK.
     */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest005
  * @tc.desc: Check if rekeying possible with wrong password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
     * @tc.expected: step1. Return 0.
     */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
     * @tc.steps:step2. Set password to g_diffPasswd.
     * @tc.expected: step2. Return SQLITE_OK.
     */
    EXPECT_EQ(InputPasswd(g_diffPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
     * @tc.steps:step3. Reset the key by invoking the sqlite3_rekey() with the password as g_newPasswd.
     * @tc.expected: step3. Return SQLITE_ERROR values.
     */
    EXPECT_EQ(sqlite3_rekey(g_db, static_cast<const void *>(g_newPasswd), strlen(g_newPasswd)), SQLITE_ERROR);

    /**
     * @tc.steps:step4. Close DB.
     * @tc.expected: step4. Return SQLITE_OK.
     */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest006
  * @tc.desc: Check if rekeying possible with correct password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
 */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
     * @tc.expected: step1. Return 0.
     */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Reset the key by invoking the sqlite3_rekey() with the password as g_newPasswd.
      * @tc.expected: step3. Return SQLITE_OK values.
      */
    EXPECT_EQ(sqlite3_rekey(g_db, static_cast<const void *>(g_newPasswd), strlen(g_newPasswd)), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest007
  * @tc.desc: Check if manipulating data possible after rekeying before disconnecting with DB.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest007, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
     * @tc.expected: step1. Return 0.
     */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
     * @tc.steps:step3. Reset the key by invoking the sqlite3_rekey() with the password as g_newPasswd.
     * @tc.expected: step3. Return SQLITE_OK values.
     */
    EXPECT_EQ(sqlite3_rekey(g_db, static_cast<const void *>(g_newPasswd), strlen(g_newPasswd)), SQLITE_OK);

    /**
      * @tc.steps:step4/7. Add, Update, Get and Delete the data.
      * @tc.expected: step4/7. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step8. Close DB.
      * @tc.expected: step8. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest008
  * @tc.desc: Check if manipulating data possible after rekeying and reconnection with a wrong password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest008, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
     * @tc.expected: step1. Return 0.
    */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Reset the key by invoking the sqlite3_rekey() with the password as g_newPasswd.
      * @tc.expected: step3. Return SQLITE_OK values.
      */
    EXPECT_EQ(sqlite3_rekey(g_db, static_cast<const void *>(g_newPasswd), strlen(g_newPasswd)), SQLITE_OK);
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);

    /**
      * @tc.steps:step4. Open DB with the original password 'g_oldPasswd'.
      * @tc.expected: step4. Return SQLITE_OK values.
      */
    EXPECT_EQ(Reconnect(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step5/8. Add, Update, Get and Delete the data.
      * @tc.expected: step5/8. Return NOT_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_NE(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_NE(valueGet, valueUpdate);

    /**
      * @tc.steps:step9. Close DB.
      * @tc.expected: step9. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest009
  * @tc.desc: Check if manipulating data possible after rekeying and reconnection with a correct password.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
*/
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest009, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
      * @tc.expected: step1. Return 0.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Reset the key by invoking the sqlite3_rekey() with the password as g_newPasswd.
      * @tc.expected: step3. Return SQLITE_OK values.
      */
    EXPECT_EQ(sqlite3_rekey(g_db, static_cast<const void *>(g_newPasswd), strlen(g_newPasswd)), SQLITE_OK);
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);

    /**
      * @tc.steps:step4. Open DB with the new password 'g_newPasswd'.
      * @tc.expected: step4. Return SQLITE_OK values.
      */
    EXPECT_EQ(Reconnect(g_newPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step5/8. Add, Update, Get and Delete the data.
      * @tc.expected: step5/8. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step9. Close DB.
      * @tc.expected: step9. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest010
  * @tc.desc: Export DB when there is no encryption.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
*/
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest010, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without being encrypted.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2. Attach DB.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step3. export  DB.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Export(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest011
  * @tc.desc: Export DB when there is no encryption but decipherment is attempted.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest011, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without being encrypted.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Attach DB.
      * @tc.expected: step3. Return is not SQLITE_OK.
      */
    EXPECT_NE(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. export DB.
      * @tc.expected: step4. Return NOT_OK.
      */
    EXPECT_NE(Export(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Close DB.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest012
  * @tc.desc: Export DB when there is encryption but password is wrong.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest012, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_diffPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_diffPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Attach DB.
      * @tc.expected: step3. Return NOT_SQLITE_OK.
      */
    EXPECT_NE(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. export DB.
      * @tc.expected: step4. Return NOT_OK.
      */
    EXPECT_NE(Export(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Close DB.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest013
  * @tc.desc: Export DB when there is encryption and password matches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest013, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_diffPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Attach DB.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. export DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(Export(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Close DB.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest014
  * @tc.desc: Attach DB files when there is no encryption.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest014, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without being encrypted.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2. attach DB file STORE_ID2.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step3. Close DB.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest015
  * @tc.desc: Attach DB files when there is no encryption but decipherment is attempted.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest015, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without being encrypted.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. attach DB file STORE_ID2.
      * @tc.expected: step3. Return is not SQLITE_OK.
      */
    EXPECT_NE(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest016
  * @tc.desc: Attach DB files when there is encryption but password dismatches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest016, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_diffPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_diffPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. attach DB file STORE_ID2.
      * @tc.expected: step3. Return NOT_SQLITE_OK.
      */
    EXPECT_NE(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest017
  * @tc.desc: Attach DB files when there is encryption and password matches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest017, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. attach DB file STORE_ID2.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest018
  * @tc.desc: Export attached DB file failed if the file does not exist.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest018, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. attach DB file STORE_ID2.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step3. export DB.
      * @tc.expected: step3. Return NOT_OK.
      */
    EXPECT_NE(Export(STORE_ID3), SQLITE_OK);

    /**
      * @tc.steps:step4. Close DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest019
  * @tc.desc: Export attached DB file succeeded if the file exists.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
*/
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest019, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. attach DB file STORE_ID2.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step4. export DB.
      * @tc.expected: step4. Return NOT_OK.
      */
    EXPECT_EQ(Export(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Close DB.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest020
  * @tc.desc: Failed to manipulate the data if the parameter of number of iteration dismatches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest020, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd and choose not to save password.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION2, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_NE(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_NE(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest021
  * @tc.desc: Succeeded to manipulate the data if the parameter of number of iteration matches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest021, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd and choose not to save password.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest022
  * @tc.desc: Failed to manipulate the data if the parameter of encryption algorithm dismatches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest022, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd and choose not to save password.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG2), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_NE(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_NE(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest023
  * @tc.desc: Succeeded to manipulate the data if the parameter of encryption algorithm matches.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest023, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd and choose not to save password.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
  * @tc.name: EncryptTest024
  * @tc.desc: Export attached DB (no password) file and check the context.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT6
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest024, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database with password g_oldPasswd.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG1), SQLITE_OK);

    /**
      * @tc.steps:step3. Put key into DB
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(PutValue(KEY_1, VALUE_1), SQLITE_OK);

    /**
      * @tc.steps:step4. attach DB file STORE_ID2.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(Attach(STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Export DB.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(Export(STORE_ID2), SQLITE_OK);
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);

    /**
      * @tc.steps:step6. Open exported DB.
      * @tc.expected: step6. Return SQLITE_OK.
      */
    EXPECT_EQ(Open(g_db, STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step7. Get Value from exported DB and the value shall be the same as the original one.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    Value valueGet;
    GetValue(KEY_1, valueGet);
    EXPECT_EQ(valueGet, VALUE_1);

    /**
      * @tc.steps:step8. Close DB.
      * @tc.expected: step8. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
 * @tc.name: EncryptTest025
 * @tc.desc: Export attached DB (password) file and check the context.
 * @tc.type: FUNC
 * @tc.require: AR000CQDT6
 * @tc.author: maokeheng
 */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest025, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open a database without password.
      * @tc.expected: step1. Return SQLITE_OK.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG1, false), SQLITE_OK);

    EXPECT_EQ(PutValue(KEY_1, VALUE_1), SQLITE_OK);

    /**
      * @tc.steps:step2. attach DB file STORE_ID2 with password.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(AttachWithKey(STORE_ID2, g_oldPasswd), SQLITE_OK);

    /**
      * @tc.steps:step3. export DB.
      * @tc.expected: step3. Return SQLITE_OK.
      */
    EXPECT_EQ(Export(STORE_ID2), SQLITE_OK);
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);

    /**
      * @tc.steps:step4. Open exported DB.
      * @tc.expected: step4. Return SQLITE_OK.
      */
    EXPECT_EQ(Open(g_db, STORE_ID2), SQLITE_OK);

    /**
      * @tc.steps:step5. Input password to g_oldPasswd.
      * @tc.expected: step5. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_key(g_db, static_cast<const void *>(g_oldPasswd), strlen(g_oldPasswd)), SQLITE_OK);

    /**
      * @tc.steps:step6. Get Value from exported DB and the value shall be the same as the original one.
      * @tc.expected: step6. Return SQLITE_OK.
      */
    Value valueGet;
    GetValue(KEY_1, valueGet);
    EXPECT_EQ(valueGet, VALUE_1);

    /**
      * @tc.steps:step6. Close DB.
      * @tc.expected: step6. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}

/**
 * @tc.name: EncryptTest026
 * @tc.desc: Check if deciphering with a non-existing algorithm can be detected.
 * @tc.type: FUNC
 * @tc.require: AR000CQDT6
 * @tc.author: maokeheng
 */
HWTEST_F(DistributedDBStorageEncryptTest, EncryptTest026, TestSize.Level1)
{
    /**
      * @tc.steps:step1. Open an encrypted DB with password g_oldPasswd.
      * @tc.expected: step1. Return 0.
      */
    EXPECT_EQ(OpenWithKey(g_oldPasswd, ITERATION, ALG3, true), SQLITE_OK);

    /**
      * @tc.steps:step2. Set password to g_oldPasswd.
      * @tc.expected: step2. Return SQLITE_OK.
      */
    EXPECT_EQ(InputPasswd(g_oldPasswd, ITERATION, ALG3), SQLITE_OK);

    /**
      * @tc.steps:step3/6. Add, Update, Get and Delete the data.
      * @tc.expected: step3/6. Return SQLITE_OK values.
      */
    Value valueGet;
    Value valueUpdate = { VALUE_2 };
    EXPECT_EQ(MultipleOperation(valueGet, valueUpdate), SQLITE_OK);
    EXPECT_EQ(valueGet, valueUpdate);

    /**
      * @tc.steps:step7. Close DB.
      * @tc.expected: step7. Return SQLITE_OK.
      */
    EXPECT_EQ(sqlite3_close(g_db), SQLITE_OK);
}
#endif