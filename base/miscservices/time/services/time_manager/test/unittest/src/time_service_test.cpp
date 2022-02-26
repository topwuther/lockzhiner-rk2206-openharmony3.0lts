/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "timer_info_test.h"
#include "time_service_test.h"


using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;

class TimeServiceTest : public testing::Test
{
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TimeServiceTest::SetUpTestCase(void)
{
}

void TimeServiceTest::TearDownTestCase(void)
{
}

void TimeServiceTest::SetUp(void)
{
}

void TimeServiceTest::TearDown(void)
{
}

/**
* @tc.name: SetTime001
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, SetTime001, TestSize.Level0)
{
    struct timeval getTime;
    gettimeofday(&getTime, NULL);
    int64_t time = (getTime.tv_sec + 1000) * 1000 + getTime.tv_usec / 1000;
    if (time < 0) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Time now invalid : %{public}" PRId64 "",time);
        time = 1627307312000;
    }
    TIME_HILOGI(TIME_MODULE_CLIENT, "Time now : %{public}" PRId64 "",time);
    bool result = TimeServiceClient::GetInstance()->SetTime(time);
    EXPECT_TRUE(result);
}

/**
* @tc.name: SetTimeZone001
* @tc.desc: set system time zone.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, SetTimeZone001, TestSize.Level0)
{   
    struct timezone tz = {};
    gettimeofday(NULL, &tz);
    TIME_HILOGD(TIME_MODULE_CLIENT, "Before set timezone, GMT offset in kernel : %{public}d", tz.tz_minuteswest);
    std::string timeZoneSet("Asia/Shanghai");
    bool result = TimeServiceClient::GetInstance()->SetTimeZone(timeZoneSet);
    EXPECT_TRUE(result);
    auto ret = gettimeofday(NULL, &tz);
    TIME_HILOGD(TIME_MODULE_CLIENT, "gettimeofday result : %{public}d", ret);
    TIME_HILOGD(TIME_MODULE_CLIENT, "After set timezone, GMT offset minutes in kernel : %{public}d", tz.tz_minuteswest);
    auto timeZoneRes = TimeServiceClient::GetInstance()->GetTimeZone();
    EXPECT_EQ(timeZoneRes, timeZoneSet);
}

/**
* @tc.name: SetTimeZone002
* @tc.desc: set system time zone.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, SetTimeZone002, TestSize.Level0)
{
    struct timezone tz = {};
    gettimeofday(NULL, &tz);
    TIME_HILOGD(TIME_MODULE_CLIENT, "Before set timezone, GMT offset in kernel : %{public}d", tz.tz_minuteswest);
    std::string timeZoneSet("Asia/Ulaanbaatar");

    bool result = TimeServiceClient::GetInstance()->SetTimeZone(timeZoneSet);
    EXPECT_TRUE(result);
    auto ret = gettimeofday(NULL, &tz);
    TIME_HILOGD(TIME_MODULE_CLIENT, "gettimeofday result : %{public}d", ret);
    TIME_HILOGD(TIME_MODULE_CLIENT, "After set timezone, GMT offset minutes in kernel : %{public}d", tz.tz_minuteswest);
    auto timeZoneRes = TimeServiceClient::GetInstance()->GetTimeZone();
    EXPECT_EQ(timeZoneRes, timeZoneSet);
}

/**
* @tc.name: GetTime001
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime001, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetWallTimeMs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetWallTimeMs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime002
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime002, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetWallTimeNs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetWallTimeNs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime003
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime003, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetBootTimeMs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetBootTimeMs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime004
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime004, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime005
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime005, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetBootTimeNs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetBootTimeNs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime006
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime006, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetMonotonicTimeNs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetMonotonicTimeNs();
    EXPECT_TRUE(time2 != -1);
}

/**
* @tc.name: GetTime007
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime007, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetThreadTimeMs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetThreadTimeMs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: GetTime008
* @tc.desc: get system time.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, GetTime008, TestSize.Level0)
{
    auto time1 = TimeServiceClient::GetInstance()->GetThreadTimeNs();
    EXPECT_TRUE(time1 != -1);
    auto time2 = TimeServiceClient::GetInstance()->GetThreadTimeNs();
    EXPECT_TRUE(time2 >= time1);
}

/**
* @tc.name: CreateTimer01 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer001, TestSize.Level0)
{
    g_data1 = 0;
    auto timerInfo = std::make_shared<TimerInfoTest>();
    timerInfo->SetType(1);
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(0);
    timerInfo->SetWantAgent(nullptr);
    timerInfo->SetCallbackInfo(TimeOutCallback1);
    auto timerId1 = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    EXPECT_TRUE(timerId1 > 0);

    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_data1 == 1);
    ret = TimeServiceClient::GetInstance()->StopTimer(timerId1);
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId1);
    EXPECT_TRUE(ret);
}

/**
* @tc.name: CreateTimer02 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer002, TestSize.Level0)
{
    g_data1 = 0;
    auto timerInfo = std::make_shared<TimerInfoTest>();
    timerInfo->SetType(1);
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(0);
    timerInfo->SetWantAgent(nullptr);
    timerInfo->SetCallbackInfo(TimeOutCallback1);
    auto timerId1 = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    EXPECT_TRUE(timerId1 > 0);

    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, 5000);
    std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_data1 == 1);
    ret = TimeServiceClient::GetInstance()->StopTimer(timerId1);
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, 5000);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_data1 == 1);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId1);
    EXPECT_TRUE(ret);
}

/**
* @tc.name: CreateTimer03 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer003, TestSize.Level0)
{
    uint64_t timerId = 0;
    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId, 5);
    EXPECT_FALSE(ret);
    ret = TimeServiceClient::GetInstance()->StopTimer(timerId);
    EXPECT_FALSE(ret);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId);
    EXPECT_FALSE(ret);
}

/**
* @tc.name: CreateTimer04 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer004, TestSize.Level0)
{
    auto timerInfo = std::make_shared<TimerInfoTest>();
    timerInfo->SetType(1);
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(0);
    timerInfo->SetWantAgent(nullptr);
    timerInfo->SetCallbackInfo(TimeOutCallback1);
    auto timerId1 = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    EXPECT_TRUE(timerId1 > 0);
    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, 2000);
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->StopTimer(timerId1);
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId1);
    EXPECT_TRUE(ret);
}

/**
* @tc.name: CreateTimer05 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer005, TestSize.Level0)
{
    g_data1 = 0;
    auto timerInfo = std::make_shared<TimerInfoTest>();
    timerInfo->SetType(1);
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(0);
    timerInfo->SetWantAgent(nullptr);
    timerInfo->SetCallbackInfo(TimeOutCallback1);
    auto timerId1 = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    EXPECT_TRUE(timerId1 > 0);

    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, 2000);
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId1);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_data1 == 0);

    ret = TimeServiceClient::GetInstance()->StopTimer(timerId1);
    EXPECT_FALSE(ret);
}

/**
* @tc.name: CreateTimer06 
* @tc.desc: Create system timer.
* @tc.type: FUNC
*/
HWTEST_F(TimeServiceTest, CreateTimer006, TestSize.Level0)
{
    g_data1 = 1;
    auto timerInfo = std::make_shared<TimerInfoTest>();
    timerInfo->SetType(0);
    timerInfo->SetRepeat(false);
    timerInfo->SetInterval(0);
    timerInfo->SetWantAgent(nullptr);
    timerInfo->SetCallbackInfo(TimeOutCallback1);

    struct timeval getTime;
    gettimeofday(&getTime, NULL);
    int64_t current_time = (getTime.tv_sec + 100) * 1000 + getTime.tv_usec / 1000;
    if (current_time < 0) {
        current_time = 0;
    }
    auto timerId1 = TimeServiceClient::GetInstance()->CreateTimer(timerInfo);
    EXPECT_TRUE(timerId1 > 0);

    auto ret = TimeServiceClient::GetInstance()->StartTimer(timerId1, static_cast<uint64_t>(current_time));
    EXPECT_TRUE(ret);
    ret = TimeServiceClient::GetInstance()->DestroyTimer(timerId1);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(g_data1 == 1);

    ret = TimeServiceClient::GetInstance()->StopTimer(timerId1);
    EXPECT_FALSE(ret);
}
