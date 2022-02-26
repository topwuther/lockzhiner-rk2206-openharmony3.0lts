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

#include "distributeddb_tools_unit_test.h"

#include <gtest/gtest.h>

#include "parcel.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

class DistributedDBParcelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBParcelTest::SetUpTestCase(void)
{
}

void DistributedDBParcelTest::TearDownTestCase(void)
{
}

void DistributedDBParcelTest::SetUp(void)
{
}

void DistributedDBParcelTest::TearDown(void)
{
}

/**
 * @tc.name: WriteInt001
 * @tc.desc: write and read a integer.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteInt001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    int writeData1 = INT_MAX;
    uint32_t writeData2 = UINT32_MAX;
    uint64_t writeData3 = 0;

    uint32_t len = Parcel::GetIntLen() + Parcel::GetUInt32Len() + Parcel::GetUInt64Len();
    len = Parcel::GetEightByteAlign(len);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteInt(writeData1);
    EXPECT_TRUE(ret == E_OK);
    ret = writeParcel.WriteUInt32(writeData2);
    EXPECT_TRUE(ret == E_OK);
    ret = writeParcel.WriteUInt64(writeData3);
    EXPECT_TRUE(ret == E_OK);
    // write overflow
    ret = writeParcel.WriteUInt64(writeData3);
    EXPECT_TRUE(ret != E_OK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    int readData1;
    uint32_t readData2;
    uint64_t readData3;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadInt(readData1);
    EXPECT_TRUE(readLen == Parcel::GetIntLen());
    readLen += readParcel.ReadUInt32(readData2);
    EXPECT_TRUE(readLen == Parcel::GetIntLen() + Parcel::GetUInt32Len());
    readLen += readParcel.ReadUInt64(readData3);
    EXPECT_TRUE(readLen == Parcel::GetIntLen() + Parcel::GetUInt32Len() + Parcel::GetUInt64Len());
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readData1 == writeData1);
    EXPECT_TRUE(readData2 == writeData2);
    EXPECT_TRUE(readData3 == writeData3);
    // read overflow
    readLen = readParcel.ReadUInt64(readData3);
    EXPECT_TRUE(readParcel.IsError());
    EXPECT_TRUE(readLen == 0);
    delete []buf;
}

/**
 * @tc.name: WriteVector001
 * @tc.desc: write and read a vector<uint8_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData = {1, 2, 5, 7, 20, 30, 99};
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint8_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint8_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint8_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector002
 * @tc.desc: write and read an empty vector<uint8_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create an empty vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData;
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint8_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint8_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint8_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector003
 * @tc.desc: write and read a vector<uint32_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint32_t> writeData = {1, 2, 5, 7, 20, 30, 99, 0xffffffff, 0x5678, 0x98765432, 0xabcdef12};
    uint32_t len = Parcel::GetVectorLen<uint32_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint32_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint32_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint32_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint32_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector004
 * @tc.desc: write and read an empty vector<uint32_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create an empty vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint32_t> writeData;
    uint32_t len = Parcel::GetVectorLen<uint32_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint32_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint32_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint32_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint32_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector005
 * @tc.desc: write and read a vector<uint64_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint64_t> writeData = {1, 2, 5, 7, 20, 30, 99, 0xffffffffffffffff,
        0x5678, 0x98765432ffffffff, 0xabcdef1212345678};
    uint32_t len = Parcel::GetVectorLen<uint64_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint64_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint64_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint64_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint64_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector006
 * @tc.desc: write and read an empty vector<uint64_t>.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create an empty vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint64_t> writeData;
    uint32_t len = Parcel::GetVectorLen<uint64_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint64_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. read the vector, the vector should same as written vector;
     * @tc.expected: step1. read out vector same as written vector;
     */
    vector<uint64_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint64_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData.size() == writeData.size());
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVector<uint64_t>(writeData, readData));
    delete []buf;
}

/**
 * @tc.name: WriteVector007
 * @tc.desc: write and read a vector<uint8_t>, insert a wrong len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData = {1, 2, 5, 7, 20, 30, 99};
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is INT_MAX;
     * @tc.expected: the vector should be empty, and isError should be true.
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(INT32_MAX));

    /**
     * @tc.steps: step3. read the vector
     * @tc.expected: the vector should be empty, and isError should be true.
     */
    vector<uint8_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint8_t>(readData);
    EXPECT_TRUE(readParcel.IsError());
    EXPECT_TRUE(readLen == 0);
    EXPECT_TRUE(readData.size() == 0);
    delete []buf;
}

/**
 * @tc.name: WriteVector008
 * @tc.desc: write and read a vector<uint8_t>, insert a wrong len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData = {1, 2, 5, 7, 20, 30, 99, 0xff, 0xff, 0x1f, 0xab, 0x45};
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is bigger than it should be;
     * @tc.expected: the vector should be empty, and isError should be true.
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(writeData.size()) + 1);

    /**
     * @tc.steps: step3. read the vector
     * @tc.expected: the vector should be empty, and isError should be true.
     */
    vector<uint8_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint8_t>(readData);
    EXPECT_TRUE(readParcel.IsError());
    EXPECT_TRUE(readLen == 0);
    EXPECT_TRUE(readData.size() == 0);
    delete []buf;
}

/**
 * @tc.name: WriteVector009
 * @tc.desc: write and read a vector<uint8_t>, insert a wrong len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector009, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData = {1, 2, 5, 7, 20, 30, 99};
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is smaller than it should be;
     * @tc.expected: the vector should be empty, and isError should be true;
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(writeData.size()) - 1);

    /**
     * @tc.steps: step3. read the vector
     * @tc.expected: the vector should be same as writeData.sub(0, len - 1);
     */
    vector<uint8_t> readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadVector<uint8_t>(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen != 0);
    EXPECT_TRUE(readData.size() == writeData.size() - 1);
    EXPECT_TRUE(DistributedDBToolsUnitTest::CompareVectorN<uint8_t>(readData, writeData, writeData.size() - 1));
    delete []buf;
}

#ifndef LOW_LEVEL_MEM_DEV
/**
 * @tc.name: WriteVector010
 * @tc.desc: write and read a vector<uint8_t>, vector len is INT_MAX.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteVector010, TestSize.Level2)
{
    /**
     * @tc.steps: step1. create a vector, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    vector<uint8_t> writeData(INT32_MAX, 0xff);
    uint32_t len = Parcel::GetVectorLen<uint8_t>(writeData);
    EXPECT_TRUE(len == 0);
    len = Parcel::GetEightByteAlign(static_cast<uint32_t>(INT32_MAX) + 4);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteVector<uint8_t>(writeData);
    delete []buf;
    EXPECT_TRUE(ret != EOK);
}
#endif

/**
 * @tc.name: WriteString001
 * @tc.desc: write and read a string normally.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData("abcd1234ffff234");
    uint32_t len = Parcel::GetStringLen(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step3. read the string
     * @tc.expected: the string should be read correctly;
     */
    string readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadString(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen == len);
    EXPECT_TRUE(readData == writeData);
    delete []buf;
}

/**
 * @tc.name: WriteString002
 * @tc.desc: write and read a string, insert a wrong len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData("abcd1234ffff234");
    uint32_t len = Parcel::GetStringLen(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is smaller than it should be;
     * @tc.expected: the vector should be empty, and isError should be true;
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(writeData.size()) - 1);

    /**
     * @tc.steps: step3. read the string
     * @tc.expected: the string should be read correctly;
     */
    string readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadString(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen != 0);
    EXPECT_TRUE(readData == writeData.substr(0, writeData.size() - 1));
    delete []buf;
}

/**
 * @tc.name: WriteString003
 * @tc.desc: write and read a string, insert a wrong len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData("abcd1234ffff2349poff");
    uint32_t len = Parcel::GetStringLen(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is bigger than it should be;
     * @tc.expected: the string should be empty, and isError should be true;
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(writeData.size()) + 1);

    /**
     * @tc.steps: step3. read the string
     * @tc.expected: the string should be read with error;
     */
    string readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadString(readData);
    EXPECT_TRUE(readParcel.IsError());
    EXPECT_TRUE(readLen == 0);
    EXPECT_TRUE(readData.size()== 0);
    delete []buf;
}

/**
 * @tc.name: WriteString004
 * @tc.desc: write and read a string, string is empty.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData;
    uint32_t len = Parcel::GetStringLen(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step3. read the string
     * @tc.expected: the string should be read with error;
     */
    string readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadString(readData);
    EXPECT_TRUE(!readParcel.IsError());
    EXPECT_TRUE(readLen != 0);
    EXPECT_TRUE(readData.size()== 0);
    delete []buf;
}

/**
 * @tc.name: WriteString005
 * @tc.desc: write and read a string, insert a INT_MAX len.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData;
    uint32_t len = Parcel::GetStringLen(writeData);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    EXPECT_TRUE(ret == EOK);

    /**
     * @tc.steps: step2. set a wrong len, the len is INT32_MAX;
     * @tc.expected: the string should be empty, and isError should be true;
     */
    *(reinterpret_cast<uint32_t *>(buf)) = HostToNet(static_cast<uint32_t>(INT32_MAX));

    /**
     * @tc.steps: step3. read the string
     * @tc.expected: the string should be read with error;
     */
    string readData;
    Parcel readParcel(buf, len);
    uint32_t readLen = readParcel.ReadString(readData);
    EXPECT_TRUE(readParcel.IsError());
    EXPECT_TRUE(readLen == 0);
    EXPECT_TRUE(readData.size() == 0);
    delete []buf;
}
#ifndef LOW_LEVEL_MEM_DEV
/**
 * @tc.name: WriteString006
 * @tc.desc: write and read a string, string size is INT_MAX.
 * @tc.type: FUNC
 * @tc.require: AR000CQE0U
 * @tc.author: weifeng
 */
HWTEST_F(DistributedDBParcelTest, WriteString006, TestSize.Level2)
{
    /**
     * @tc.steps: step1. create a string, and write it into a buffer;
     * @tc.expected: step1. write ok;
     */
    string writeData(INT32_MAX, 'z');
    uint32_t len = Parcel::GetStringLen(writeData);
    EXPECT_TRUE(len == 0);
    len = Parcel::GetEightByteAlign(static_cast<uint32_t>(INT32_MAX) + 4);
    uint8_t *buf = new (nothrow) uint8_t[len];
    ASSERT_NE(buf, nullptr);
    Parcel writeParcel(buf, len);
    int ret = writeParcel.WriteString(writeData);
    delete []buf;
    EXPECT_TRUE(ret != EOK);
}
#endif