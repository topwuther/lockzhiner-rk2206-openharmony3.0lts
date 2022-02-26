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

#include "zone_util_performance_test.h"
#include <chrono>
#include <ctime>
#include <gtest/gtest.h>
#include "zone_util.h"

using namespace OHOS::Global::I18n;
using testing::ext::TestSize;
using namespace std;

namespace {
class ZoneUtilPerformanceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ZoneUtilPerformanceTest::SetUpTestCase(void)
{}

void ZoneUtilPerformanceTest::TearDownTestCase(void)
{}

void ZoneUtilPerformanceTest::SetUp(void)
{
}

void ZoneUtilPerformanceTest::TearDown(void)
{}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest001
 * @tc.desc: Test ZoneUtil GetDefaultZone
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest001, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    string expects[] = { "Asia/Shanghai", "America/New_York" };
    string countries[] = { "JP", "KR" };
    int count = 2;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        for (int i = 0; i < count; ++i) {
            auto t1= std::chrono::high_resolution_clock::now();
            string out = util.GetDefaultZone(countries[i].c_str());
            auto t2 = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        }
    }
    average = total / (1000.0 * 2);
    EXPECT_LT(average, 9300);
}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest002
 * @tc.desc: Test ZoneUtil GetDefaultZone with offset
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest002, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    string expects[] = { "Asia/Shanghai", "America/Detroit" };
    int32_t offsets[] = { 3600 * 1000 * 8, -3600 * 1000 * 5 };
    string countries[] = { "CN", "US" };
    int count = 2;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        for (int i = 0; i < count; ++i) {
            auto t1= std::chrono::high_resolution_clock::now();
            string out = util.GetDefaultZone(countries[i].c_str(), offsets[i]);
            auto t2 = std::chrono::high_resolution_clock::now();
            total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        }
    }
    average = total / (1000.0 * 2);
    EXPECT_LT(average, 10800);
}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest003
 * @tc.desc: Test ZoneUtil GetDefaultZoneList for CN
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest003, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    vector<string> expects = { "Asia/Shanghai", "Asia/Urumqi"};
    string country = "CN";
    vector<string> out;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        auto t1= std::chrono::high_resolution_clock::now();
        util.GetZoneList(country, out);
        auto t2 = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }
    average = total / 1000.0;
    EXPECT_LT(average, 9300);
}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest004
 * @tc.desc: Test ZoneUtil GetDefaultZoneList for GB
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest004, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    vector<string> expects = { "Europe/London" };
    string country = "GB";
    vector<string> out;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        auto t1= std::chrono::high_resolution_clock::now();
        util.GetZoneList(country, out);
        auto t2 = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }
    average = total / 1000.0;
    EXPECT_LT(average, 9300);
}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest003
 * @tc.desc: Test ZoneUtil GetDefaultZoneList for de
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest005, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    vector<string> expects = { "Europe/Berlin", "Europe/Busingen" };
    string country = "DE";
    vector<string> out;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        auto t1= std::chrono::high_resolution_clock::now();
        util.GetZoneList(country, out);
        auto t2 = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }
    average = total / 1000.0;
    EXPECT_LT(average, 9300);
}

/**
 * @tc.name: ZoneUtilPerformanceFuncTest006
 * @tc.desc: Test ZoneUtil GetDefaultZoneList for CN with offset 8 hours
 * @tc.type: FUNC
 */
HWTEST_F(ZoneUtilPerformanceTest,  ZoneUtilPerformanceFuncTest006, TestSize.Level1)
{
    unsigned long long total = 0;
    double average = 0;
    vector<string> expects = { "Asia/Shanghai" };
    string country = "CN";
    vector<string> out;
    ZoneUtil util;
    for (int k = 0; k < 1000; ++k) {
        auto t1= std::chrono::high_resolution_clock::now();
        util.GetZoneList(country, 3600 * 1000 * 8, out);
        auto t2 = std::chrono::high_resolution_clock::now();
        total += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    }
    average = total / 1000.0;
    EXPECT_LT(average, 10300);
}
}
