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

#include <chrono>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "hilog/log.h"
#include "hisysevent.h"

#ifndef SYS_EVENT_PARAMS
#define SYS_EVENT_PARAMS(A) "key"#A, 0 + (A), "keyA"#A, 1 + (A), "keyB"#A, 2 + (A), "keyC"#A, 3 + (A), \
    "keyD"#A, 4 + (A), "keyE"#A, 5 + (A), "keyF"#A, 6 + (A), "keyG"#A, 7 + (A), "keyH"#A, 8 + (A), \
    "keyI"#A, 9 + (A)
#endif
using namespace testing::ext;
using OHOS::HiviewDFX::HiLogLabel;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiSysEvent;
static constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D08, "HISYSEVENTTEST" };

class HiSysEventNativeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};


void HiSysEventNativeTest::SetUpTestCase(void)
{
}

void HiSysEventNativeTest::TearDownTestCase(void)
{
}

void HiSysEventNativeTest::SetUp(void)
{
}

void HiSysEventNativeTest::TearDown(void)
{
}

/**
 * @tc.name: TestHiSysEventNormal001
 * @tc.desc: Test normal write.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventNormal001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure SystemAbilityManager is started.
     */
    std::string domain = "DEMO";
    std::string eventName = "NORMAL001";

    bool bValue = true;
    char cValue = 'a';
    short sValue = -100;
    int iValue = -200;
    long lValue = -300;
    long long llValue = -400;

    unsigned char ucValue = 'a';
    unsigned short usValue = 100;
    unsigned int uiValue = 200;
    unsigned long ulValue = 300;
    unsigned long long ullValue = 400;

    float fValue = 1.1;
    double dValue = 2.2;
    std::string strValue = "abc";

    std::vector<bool> bValues;
    bValues.push_back(true);
    bValues.push_back(true);
    bValues.push_back(false);

    std::vector<char> cValues;
    cValues.push_back('a');
    cValues.push_back('b');
    cValues.push_back('c');

    std::vector<unsigned char> ucValues;
    ucValues.push_back('a');
    ucValues.push_back('b');
    ucValues.push_back('c');

    std::vector<short> sValues;
    sValues.push_back(-100);
    sValues.push_back(-200);
    sValues.push_back(-300);

    std::vector<unsigned short> usValues;
    usValues.push_back(100);
    usValues.push_back(200);
    usValues.push_back(300);

    std::vector<int> iValues;
    iValues.push_back(-1000);
    iValues.push_back(-2000);
    iValues.push_back(-3000);

    std::vector<unsigned int> uiValues;
    uiValues.push_back(1000);
    uiValues.push_back(2000);
    uiValues.push_back(3000);

    std::vector<long> lValues;
    lValues.push_back(-10000);
    lValues.push_back(-20000);
    lValues.push_back(-30000);

    std::vector<unsigned long> ulValues;
    ulValues.push_back(10000);
    ulValues.push_back(20000);
    ulValues.push_back(30000);

    std::vector<long long> llValues;
    llValues.push_back(-100000);
    llValues.push_back(-200000);
    llValues.push_back(-300000);

    std::vector<unsigned long long> ullValues;
    ullValues.push_back(100000);
    ullValues.push_back(200000);
    ullValues.push_back(300000);

    std::vector<float> fValues;
    fValues.push_back(1.1);
    fValues.push_back(2.2);
    fValues.push_back(3.3);

    std::vector<double> dValues;
    dValues.push_back(10.1);
    dValues.push_back(20.2);
    dValues.push_back(30.3);

    std::vector<std::string> strValues;
    strValues.push_back(std::string("a"));
    strValues.push_back(std::string("b"));
    strValues.push_back(std::string("c"));

    HiLog::Info(LABEL, "test hisysevent normal write");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT,
        "keyBool", bValue, "keyChar", cValue, "keyShort", sValue, "keyInt", iValue,
        "KeyLong", lValue, "KeyLongLong", llValue,
        "keyUnsignedChar", ucValue, "keyUnsignedShort", usValue, "keyUnsignedInt", uiValue,
        "keyUnsignedLong", ulValue, "keyUnsignedLongLong", ullValue,
        "keyFloat", fValue, "keyDouble", dValue, "keyString1", strValue, "keyString2", "efg",
        "keyBools", bValues, "keyChars", cValues, "keyUnsignedChars", ucValues,
        "keyShorts", sValues, "keyUnsignedShorts", usValues, "keyInts", iValues, "keyUnsignedInts", uiValues,
        "keyLongs", lValues, "keyUnsignedLongs", ulValues, "keyLongLongs", llValues, "keyUnsignedLongLongs", ullValues,
        "keyFloats", fValues, "keyDoubles", dValues, "keyStrings", strValues);
    HiLog::Info(LABEL, "normal write, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventDomainSpecialChar002
 * @tc.desc: Test domain has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainSpecialChar002, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "_demo";
    std::string eventName = "DOMAIN_SPECIAL_CHAR";
    HiLog::Info(LABEL, "test hisysevent domain has special char");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "domain has special char, retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TestHiSysEventDomainEmpty003
 * @tc.desc: Test domain is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainEmpty003, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "";
    std::string eventName = "DOMAIN_EMPTY";
    HiLog::Info(LABEL, "test hisysevent domain is empty");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "domain is empty, retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TestHiSysEventDomainTooLong004
 * @tc.desc: Test domain is too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventDomainTooLong004, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "";
    std::string eventName = "DOMAIN_TOO_LONG_16";
    HiLog::Info(LABEL, "test hisysevent domain is too long, normal length");
    int normal = 16;
    for (int index = 0; index < normal; index++) {
        domain.append("A");
    }
    int result = 0;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "domain too long, equal 16 retCode=%{public}d", result);

    HiLog::Info(LABEL, "test hisysevent domain is too long");
    domain.append("L");
    eventName = "DOMAIN_TOO_LONG_17";
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "domain is too long, more than 16 retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TesetHiSysEventSpecailEventName005
 * @tc.desc: Test event name has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TesetHiSysEventSpecailEventName005, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "SPEC_EVT_NAME";
    std::string eventName = "_SPECIAL_CHAR";
    HiLog::Info(LABEL, "test hisysevent event name has special char");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "event name has special char, retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TestHiSysEventNameEmpty006
 * @tc.desc: Test event name is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventNameEmpty006, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "EMPTY";
    std::string eventName = "";
    HiLog::Info(LABEL, "test hisysevent event name is empty");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "event name is empty, retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TesetHiSysEventNameTooLong007
 * @tc.desc: Test event name too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TesetHiSysEventNameTooLong007, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "NAME_32";
    std::string eventName = "";
    HiLog::Info(LABEL, "test hisysevent event name is too long, normal length");
    int normal = 32;
    for (int index = 0; index < normal; index++) {
        eventName.append("N");
    }
    int result = 0;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "event name is too long, equal 32, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);

    HiLog::Info(LABEL, "test hisysevent event name is too long");
    domain = "NAME_33";
    eventName.append("L");
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT);
    HiLog::Info(LABEL, "event name is too long, more than 32, retCode=%{public}d", result);
    ASSERT_TRUE(result < 0);
}

/**
 * @tc.name: TestHiSysEventKeySpecialChar008
 * @tc.desc: Test key has specail char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeySpecialChar008, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "HiSysEvent006";
    std::string key1 = "_key1";
    std::string key2 = "key2";
    int result = 0;
    HiLog::Info(LABEL, "test hisysevent key has special char");
    bool value1 = true;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value1, key2, value1);
    ASSERT_TRUE(result > 0);

    short value2 = 2;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value2, key2, value2);
    ASSERT_TRUE(result > 0);

    unsigned short value3 = 3;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value3, key2, value3);
    ASSERT_TRUE(result > 0);

    int value4 = 4;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value4, key2, value4);
    ASSERT_TRUE(result > 0);

    unsigned int value5 = 5;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value5, key2, value5);
    ASSERT_TRUE(result > 0);

    long value6 = 6;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value6, key2, value6);
    ASSERT_TRUE(result > 0);

    unsigned long value7 = 7;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value7, key2, value7);
    ASSERT_TRUE(result > 0);

    long long value8 = 8;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value8, key2, value8);
    ASSERT_TRUE(result > 0);

    unsigned long long value9 = 9;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value9, key2, value9);
    ASSERT_TRUE(result > 0);

    char value10 = 'a';
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value10, key2, value10);
    ASSERT_TRUE(result > 0);

    unsigned char value11 = 'b';
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value11, key2, value11);
    ASSERT_TRUE(result > 0);

    float value12 = 12.12;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value12, key2, value12);
    ASSERT_TRUE(result > 0);

    double value13 = 13.13;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key1, value13, key2, value13);
    ASSERT_TRUE(result > 0);
}


/**
 * @tc.name: TestHiSysEventEscape009
 * @tc.desc: Test key's value need escape.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventEscape009, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "ESCAPE";
    HiLog::Info(LABEL, "test hisysevent escape char");
    std::string value = "\"escapeByCpp\"";
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HiLog::Info(LABEL, "key's value has espcae char, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventKeyEmpty010
 * @tc.desc: Test key is empty.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeyEmpty010, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "KEY_EMPTY";
    HiLog::Info(LABEL, "test hisysevent key is empty");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT,
        "", "valueIsEmpty", "key2", "notEmpty");
    HiLog::Info(LABEL, "key is empty, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventKeySpecialChar011
 * @tc.desc: Test key has special char.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeySpecialChar011, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "KEY_SPECIAL_CHAR";
    HiLog::Info(LABEL, "test hisysevent key is special");
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT,
        "_key1", "special", "key2", "normal");
    HiLog::Info(LABEL, "key has special char, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventKeyTooLong012
 * @tc.desc: Test key is too long.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventKeyTooLong012, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "KEY_48";
    HiLog::Info(LABEL, "test hisysevent key 48 char");
    std::string key = "";
    int normal = 48;
    for (int index = 0; index < normal; index++) {
        key.append("V");
    }
    int result = 0;
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key, "48length", "key2", "normal");
    HiLog::Info(LABEL, "key equal 48 char, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);

    HiLog::Info(LABEL, "test hisysevent key 49 char");
    eventName = "KEY_49";
    key.append("V");
    result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, key, "49length", "key2", "normal");
    HiLog::Info(LABEL, "key more than 48 char, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEvent128Keys013
 * @tc.desc: Test 128 key.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEvent128Keys013, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "TEST";
    std::string eventName = "KEY_EQUAL_128";
    HiLog::Info(LABEL, "test hisysevent 128 keys");
    std::string k = "k";
    bool v = true;
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT,
        SYS_EVENT_PARAMS(10), SYS_EVENT_PARAMS(20), SYS_EVENT_PARAMS(30), SYS_EVENT_PARAMS(40), SYS_EVENT_PARAMS(50),
        SYS_EVENT_PARAMS(60), SYS_EVENT_PARAMS(70), SYS_EVENT_PARAMS(80), SYS_EVENT_PARAMS(90), SYS_EVENT_PARAMS(100),
        SYS_EVENT_PARAMS(110), SYS_EVENT_PARAMS(120),
        k, v, k, v, k, v, k, v, k, v, k, v, k, v, k, v);
    HiLog::Info(LABEL, "has 128 key, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEvent129Keys014
 * @tc.desc: Test 129 key.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEvent129Keys014, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "TEST";
    std::string eventName = "KEY_EQUAL_129";
    HiLog::Info(LABEL, "test hisysevent 129 key");
    std::string k = "k";
    bool v = true;
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT,
        SYS_EVENT_PARAMS(10), SYS_EVENT_PARAMS(20), SYS_EVENT_PARAMS(30), SYS_EVENT_PARAMS(40), SYS_EVENT_PARAMS(50),
        SYS_EVENT_PARAMS(60), SYS_EVENT_PARAMS(70), SYS_EVENT_PARAMS(80), SYS_EVENT_PARAMS(90), SYS_EVENT_PARAMS(100),
        SYS_EVENT_PARAMS(110), SYS_EVENT_PARAMS(120),
        k, v, k, v, k, v, k, v, k, v, k, v, k, v, k, v, k, v);
    HiLog::Info(LABEL, "has 129 key, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventStringValueEqual256K015
 * @tc.desc: Test 256K string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventStringValueEqual256K015, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "TEST";
    std::string eventName = "EQUAL_256K";
    HiLog::Info(LABEL, "test key's value 256K string");
    std::string value;
    int length = 256 * 1024;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HiLog::Info(LABEL, "string length is 256K, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventStringValueMoreThan256K016
 * @tc.desc: Test 256K + 1 string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventStringValueMoreThan256K016, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "MORETHAN256K";
    HiLog::Info(LABEL, "test more than 256K string");
    std::string value;
    int length = 256 * 1024 + 1;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", value);
    HiLog::Info(LABEL, "string length is more than 256K, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventArray100Item017
 * @tc.desc: Test bool array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100Item017, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "BOOL_ARRAY_100";
    HiLog::Info(LABEL, "test bool array 100 item");
    std::vector<bool> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back(true);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array bool list 100, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventArray101Item018
 * @tc.desc: Test bool array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101Item018, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "BOOL_ARRAY_101";
    HiLog::Info(LABEL, "test bool array 101 item");
    std::vector<bool> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back(true);
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array bool list 101, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventArray100CharItem019
 * @tc.desc: Test char array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100CharItem019, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "CHAR_ARRAY_100";
    HiLog::Info(LABEL, "test char array 100 item");
    std::vector<char> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('a');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array char list 100, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventArray101CharItem020
 * @tc.desc: Test char array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101CharItem020, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "CHAR_ARRAY_101";
    HiLog::Info(LABEL, "test char array 101 item");
    std::vector<char> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('z');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array char list 101, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventArray100UnsignedCharItem021
 * @tc.desc: Test unsigned char array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100UnsignedCharItem021, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "UCHAR_ARRAY_100";
    HiLog::Info(LABEL, "test unsigned char array 100 item");
    std::vector<unsigned char> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('a');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array unsigned char list 100, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventArray101UnsignedCharItem022
 * @tc.desc: Test unsigned char array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101UnsignedCharItem022, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "UCHAR_ARRAY_101";
    HiLog::Info(LABEL, "test unsigned char array 101 item");
    std::vector<unsigned char> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back('z');
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array unsigned char list 101, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}


/**
 * @tc.name: TestHiSysEventArray100StringItem023
 * @tc.desc: Test string array item 100.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray100StringItem023, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "STR_ARRAY_100";
    HiLog::Info(LABEL, "test string array 100 item");
    std::vector<std::string> values;
    int maxItem = 100;
    for (int index = 0; index < maxItem; index++) {
        values.push_back("a");
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array string list 100, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventArray101StringItem024
 * @tc.desc: Test string array item 101.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArray101StringItem024, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "STR_ARRAY_101";
    HiLog::Info(LABEL, "test string array 101 item");
    std::vector<std::string> values;
    int maxItem = 101;
    for (int index = 0; index < maxItem; index++) {
        values.push_back("z");
    }
    sleep(1); // make sure hiview read all data before send large data
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array string list 101, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}

/**
 * @tc.name: TestHiSysEventArrayStringValueEqual256K025
 * @tc.desc: Test array item 256K string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArrayStringValueEqual256K025, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "TEST";
    std::string eventName = "EQUAL_256K";
    HiLog::Info(LABEL, "test array item value 256K string");
    std::string value;
    int length = 256 * 1024;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    std::vector<std::string> values;
    values.push_back("c");
    values.push_back(value);
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array item value length is 256K, retCode=%{public}d", result);
    ASSERT_TRUE(result == 0);
}

/**
 * @tc.name: TestHiSysEventArrayStringValueMoreThan256K016
 * @tc.desc: Test 256K + 1 string.
 * @tc.type: FUNC
 * @tc.require: AR000G2QKU AR000FT2Q1
 */
HWTEST_F(HiSysEventNativeTest, TestHiSysEventArrayStringValueMoreThan256K026, TestSize.Level1)
{
    /**
     * @tc.steps: step1.make sure write sys event.
     */
    std::string domain = "DEMO";
    std::string eventName = "MORETHAN256K";
    HiLog::Info(LABEL, "test array item value more than 256K string");
    std::string value;
    int length = 256 * 1024 + 1;
    for (int index = 0; index < length; index++) {
        value.push_back('1' + index % 10);
    }
    sleep(1); // make sure hiview read all data before send large data
    std::vector<std::string> values;
    values.push_back("c");
    values.push_back(value);
    int result = HiSysEvent::Write(domain, eventName, HiSysEvent::EventType::FAULT, "key1", values);
    HiLog::Info(LABEL, "array item value length is more than 256K, retCode=%{public}d", result);
    ASSERT_TRUE(result > 0);
}