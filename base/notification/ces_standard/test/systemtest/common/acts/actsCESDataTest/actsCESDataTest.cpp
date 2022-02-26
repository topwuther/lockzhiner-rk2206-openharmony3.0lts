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

#include <thread>
#include <chrono>
#include <sys/time.h>

#include "common_event.h"
#include "common_event_manager.h"

#define private public
#define protected public
#include "common_event_control_manager.h"
#undef private
#undef protected

#include "datetime_ex.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "system_ability_helper.h"
#include "want.h"

#include <gtest/gtest.h>
#include <time.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

namespace {
const std::string EVENTCASE1 = "com.common.event.data.case1";
const std::string EVENTCASE2 = "com.common.event.data.case2";
const std::string EVENTCASE3 = "com.common.event.data.case3";
const std::string EVENTCASE4 = "com.common.event.data.case4";
const std::string EVENTCASE5 = "com.common.event.data.case5";
const std::string EVENTCASE6 = "com.common.event.data.case6";
const std::string EVENTCASE7 = "com.common.event.data.case7";
const std::string EVENTCASE8 = "com.common.event.data.case8";
const std::string EVENTCASE9 = "com.common.event.data.case9";
const std::string EVENTCASE10 = "com.common.event.data.case10";
const std::string EVENTCASE11 = "com.common.event.data.case11";
const std::string EVENTCASE12 = "com.common.event.data.case12";
const std::string EVENTCASE13 = "com.common.event.data.case13";
const std::string EVENTCASE14 = "com.common.event.data.case14";
const std::string EVENTCASE15 = "com.common.event.data.case15";
const std::string EVENTCASE16 = "com.common.event.data.case16";
const std::string EVENTCASE17 = "com.common.event.data.case17";
const std::string EVENTCASE18 = "com.common.event.data.case18";
const std::string EVENTCASE19 = "com.common.event.data.case19";
const std::string EVENTCASE20 = "com.common.event.data.case20";
const std::string EVENTCASE21 = "com.common.event.data.case21";
const std::string EVENTCASE22 = "com.common.event.data.case22";
const std::string EVENTCASE23 = "com.common.event.data.case23";

const int CODECASE1 = 0;
const std::string DATACASE1 = "com.data.case1";

const int CODECASE2 = 1;
const std::string DATACASE2 = "com.data.case2";

const int CODECASE3 = 100;
const std::string DATACASE3 = "com.data.case3";

const int CODECASE4 = -1;
const std::string DATACASE4 = "com.data.case4";

const int CODECASE5 = -100;
const std::string DATACASE5 = "com.data.case5";

const int CODECASE6 = 200;
const std::string DATACASE6 = "com.data.case6";

const int CODECASE7 = -200;
const std::string DATACASE7 = "com.data.case7";

const int CODECASE8 = 300;
const std::string DATACASE8 = "com.data.case8";

const int CODECASE9 = -300;
const std::string DATACASE9 = "com.data.case10";

const int CODECASE10 = 1000;
const std::string DATACASE10 = "com.data.case10";

const int CODECASE11 = -1000;
const std::string DATACASE11 = "com.data.case11";

const int CODECASE12 = 10000;
const std::string DATACASE12 = "com.data.case12";

const int CODECASE13 = 10000;
const std::string DATACASE13 = "com.data.case13";

const int CODECASE14 = -10000;
const std::string DATACASE14 = "com.data.case14";

const int CODECASE15 = 50000;
const std::string DATACASE15 = "com.data.case15";

const int CODECASE16 = -50000;
const std::string DATACASE16 = "com.data.case16";

const int CODECASE17 = 100000;
const std::string DATACASE17 = "com.data.case17";

const int CODECASE18 = -100000;
const std::string DATACASE18 = "com.data.case18";

const int CODECASE19 = -10000;
const std::string DATACASE19 = "com.data.case19";

const int CODECASE20 = 50000;
const std::string DATACASE20 = "com.data.case20";

const int CODECASE21 = -50000;
const std::string DATACASE21 = "com.data.case21";

const int CODECASE22 = 100000;
const std::string DATACASE22 = "com.data.case22";

const int CODECASE23 = -100000;
const std::string DATACASE23 = "com.data.case23";

const int LOWPRIORITY = 0;
const int MIDPRIORITY = 50;
const int HIGHPRIORITY = 100;
void Delay(int sec)
{
    time_t starttimeDelay, curtime;
    time(&starttimeDelay);
    do {
        time(&curtime);
    } while ((curtime - starttimeDelay) < sec);
}
}  // namespace

class ActsCESDateTest : public testing::Test {
public:
    ActsCESDateTest()
    {}
    ~ActsCESDateTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class SubscriberFirstTest : public CommonEventSubscriber {
public:
    explicit SubscriberFirstTest(const CommonEventSubscribeInfo &subscriberInfo) : CommonEventSubscriber(subscriberInfo)
    {}

    ~SubscriberFirstTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            SubscriberFirstTestCase1();
        } else if (action == EVENTCASE2) {
            SubscriberFirstTestCase2();
        } else if (action == EVENTCASE3) {
            SubscriberFirstTestCase3();
        } else if (action == EVENTCASE4) {
            SubscriberFirstTestCase4();
        } else if (action == EVENTCASE5) {
            SubscriberFirstTestCase5();
        } else if (action == EVENTCASE6) {
            SubscriberFirstTestCase6();
        } else if (action == EVENTCASE7) {
            SubscriberFirstTestCase7();
        } else if (action == EVENTCASE8) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE9) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE10) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE11) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE12) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE13) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE14) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE15) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE16) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE17) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE18) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE19) {
            SubscriberFirstTestCase19();
        } else if (action == EVENTCASE20) {
            SubscriberFirstTestCase20();
        } else if (action == EVENTCASE21) {
            SubscriberFirstTestCase21();
        } else if (action == EVENTCASE22) {
            SubscriberFirstTestCase22();
        } else if (action == EVENTCASE23) {
            SubscriberFirstTestCase23();
        } else {
            GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest  OnReceiveEvent do nothing";
        }
    }

private:
    void SubscriberFirstTestCase1()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase1 ";

        EXPECT_EQ(GetCode() == CODECASE1, true);
        EXPECT_EQ(GetData() == DATACASE1, true);
    }

    void SubscriberFirstTestCase2()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase2 ";

        EXPECT_EQ(GetCode() == CODECASE2, true);
        EXPECT_EQ(GetData() == DATACASE2, true);
    }

    void SubscriberFirstTestCase3()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase3 ";

        EXPECT_EQ(GetCode() == CODECASE3, true);
        EXPECT_EQ(GetData() == DATACASE3, true);
    }

    void SubscriberFirstTestCase4()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase4 ";

        EXPECT_EQ(GetCode() == CODECASE4, true);
        EXPECT_EQ(GetData() == DATACASE4, true);
    }

    void SubscriberFirstTestCase5()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase5 ";

        EXPECT_EQ(GetCode() == CODECASE5, true);
        EXPECT_EQ(GetData() == DATACASE5, true);
    }

    void SubscriberFirstTestCase6()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase6 ";

        EXPECT_EQ(GetCode() == CODECASE6, true);
        EXPECT_EQ(GetData() == DATACASE6, true);
    }

    void SubscriberFirstTestCase7()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase7 ";

        EXPECT_EQ(GetCode() == CODECASE7, true);
        EXPECT_EQ(GetData() == DATACASE7, true);
    }

    void SubscriberFirstTestCaseEight()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase8_12 ";
        EXPECT_TRUE(AbortCommonEvent());
        EXPECT_TRUE(GetAbortCommonEvent());
        EXPECT_TRUE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_TRUE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCaseThirteen()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberFirstTestcase13_18 ";
        EXPECT_FALSE(AbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCase19()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberThirdTestcase19 ";

        SetCode(CODECASE1);
        SetData(DATACASE1);
        EXPECT_EQ(GetCode() == CODECASE1, true);
        EXPECT_EQ(GetData() == DATACASE1, true);
        SetCodeAndData(CODECASE2, DATACASE2);
        EXPECT_EQ(GetCode() == CODECASE2, true);
        EXPECT_EQ(GetData() == DATACASE2, true);
        GoAsyncCommonEvent();
    }

    void SubscriberFirstTestCase20()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberThirdTestcase20 ";

        SetCode(CODECASE3);
        SetData(DATACASE3);
        EXPECT_EQ(GetCode() == CODECASE3, true);
        EXPECT_EQ(GetData() == DATACASE3, true);
        SetCodeAndData(CODECASE4, DATACASE4);
        EXPECT_EQ(GetCode() == CODECASE4, true);
        EXPECT_EQ(GetData() == DATACASE4, true);
        GoAsyncCommonEvent();
    }

    void SubscriberFirstTestCase21()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberThirdTestcase21 ";

        SetCode(CODECASE5);
        SetData(DATACASE5);
        EXPECT_EQ(GetCode() == CODECASE5, true);
        EXPECT_EQ(GetData() == DATACASE5, true);
        SetCodeAndData(CODECASE6, DATACASE6);
        EXPECT_EQ(GetCode() == CODECASE6, true);
        EXPECT_EQ(GetData() == DATACASE6, true);
        GoAsyncCommonEvent();
    }

    void SubscriberFirstTestCase22()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberThirdTestcase22 ";

        SetCode(CODECASE7);
        SetData(DATACASE7);
        EXPECT_EQ(GetCode() == CODECASE7, true);
        EXPECT_EQ(GetData() == DATACASE7, true);
        SetCodeAndData(CODECASE9, DATACASE9);
        EXPECT_EQ(GetCode() == CODECASE9, true);
        EXPECT_EQ(GetData() == DATACASE9, true);
        GoAsyncCommonEvent();
    }

    void SubscriberFirstTestCase23()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberFirstTest OnReceiveEvent SubscriberThirdTestcase23 ";

        SetCode(CODECASE10);
        SetData(DATACASE10);
        EXPECT_EQ(GetCode() == CODECASE10, true);
        EXPECT_EQ(GetData() == DATACASE10, true);
        SetCodeAndData(CODECASE11, DATACASE11);
        EXPECT_EQ(GetCode() == CODECASE11, true);
        EXPECT_EQ(GetData() == DATACASE11, true);
        GoAsyncCommonEvent();
    }
};

class SubscriberSecondTest : public CommonEventSubscriber {
public:
    explicit SubscriberSecondTest(const CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo)
    {}

    ~SubscriberSecondTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            SubscriberFirstTestCase1();
        } else if (action == EVENTCASE2) {
            SubscriberFirstTestCase2();
        } else if (action == EVENTCASE3) {
            SubscriberFirstTestCase3();
        } else if (action == EVENTCASE4) {
            SubscriberFirstTestCase4();
        } else if (action == EVENTCASE5) {
            SubscriberFirstTestCase5();
        } else if (action == EVENTCASE6) {
            SubscriberFirstTestCase6();
        } else if (action == EVENTCASE7) {
            SubscriberFirstTestCase7();
        } else if (action == EVENTCASE8) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE9) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE10) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE11) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE12) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE13) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE14) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE15) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE16) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE17) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE18) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE19) {
            SubscriberFirstTestCase19();
        } else if (action == EVENTCASE20) {
            SubscriberFirstTestCase20();
        } else if (action == EVENTCASE21) {
            SubscriberFirstTestCase21();
        } else if (action == EVENTCASE22) {
            SubscriberFirstTestCase22();
        } else if (action == EVENTCASE23) {
            SubscriberFirstTestCase23();
        } else {
            GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest  OnReceiveEvent do nothing";
        }
    }

private:
    void SubscriberFirstTestCase1()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase1 ";

        EXPECT_EQ(GetCode() == CODECASE1, true);
        EXPECT_EQ(GetData() == DATACASE1, true);
    }
    void SubscriberFirstTestCase2()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase2 ";

        EXPECT_EQ(GetCode() == CODECASE2, true);
        EXPECT_EQ(GetData() == DATACASE2, true);
    }
    void SubscriberFirstTestCase3()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase3 ";

        EXPECT_EQ(GetCode() == CODECASE3, true);
        EXPECT_EQ(GetData() == DATACASE3, true);
    }
    void SubscriberFirstTestCase4()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase4 ";

        EXPECT_EQ(GetCode() == CODECASE4, true);
        EXPECT_EQ(GetData() == DATACASE4, true);
    }
    void SubscriberFirstTestCase5()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase5 ";

        EXPECT_EQ(GetCode() == CODECASE5, true);
        EXPECT_EQ(GetData() == DATACASE5, true);
    }
    void SubscriberFirstTestCase6()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase6 ";

        EXPECT_EQ(GetCode() == CODECASE6, true);
        EXPECT_EQ(GetData() == DATACASE6, true);
    }
    void SubscriberFirstTestCase7()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberSecondTestcase7 ";

        EXPECT_EQ(GetCode() == CODECASE7, true);
        EXPECT_EQ(GetData() == DATACASE7, true);
    }
    void SubscriberFirstTestCaseEight()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberFirstTestcase8_12 ";
        EXPECT_TRUE(AbortCommonEvent());
        EXPECT_TRUE(GetAbortCommonEvent());
        EXPECT_TRUE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_TRUE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCaseThirteen()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberFirstTestcase13_18 ";
        EXPECT_FALSE(AbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCase19()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberThirdTestcase19 ";
    }

    void SubscriberFirstTestCase20()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberThirdTestcase20 ";
    }

    void SubscriberFirstTestCase21()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberThirdTestcase21 ";
    }

    void SubscriberFirstTestCase22()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberThirdTestcase22 ";
    }

    void SubscriberFirstTestCase23()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberSecondTest OnReceiveEvent SubscriberThirdTestcase23 ";
    }
};

class SubscriberThirdTest : public CommonEventSubscriber {
public:
    explicit SubscriberThirdTest(const CommonEventSubscribeInfo &subscriberInfo) : CommonEventSubscriber(subscriberInfo)
    {}

    ~SubscriberThirdTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            SubscriberFirstTestCase1();
        } else if (action == EVENTCASE2) {
            SubscriberFirstTestCase2();
        } else if (action == EVENTCASE3) {
            SubscriberFirstTestCase3();
        } else if (action == EVENTCASE4) {
            SubscriberFirstTestCase4();
        } else if (action == EVENTCASE5) {
            SubscriberFirstTestCase5();
        } else if (action == EVENTCASE6) {
            SubscriberFirstTestCase6();
        } else if (action == EVENTCASE7) {
            SubscriberFirstTestCase7();
        } else if (action == EVENTCASE8) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE9) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE10) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE11) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE12) {
            SubscriberFirstTestCaseEight();
        } else if (action == EVENTCASE13) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE14) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE15) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE16) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE17) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE18) {
            SubscriberFirstTestCaseThirteen();
        } else if (action == EVENTCASE19) {
            SubscriberFirstTestCase19();
        } else if (action == EVENTCASE20) {
            SubscriberFirstTestCase20();
        } else if (action == EVENTCASE21) {
            SubscriberFirstTestCase21();
        } else if (action == EVENTCASE22) {
            SubscriberFirstTestCase22();
        } else if (action == EVENTCASE23) {
            SubscriberFirstTestCase23();
        } else {
            GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdtTest  OnReceiveEvent do nothing";
        }
    }

private:
    void SubscriberFirstTestCase1()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase1 ";

        EXPECT_EQ(GetCode() == CODECASE1, true);
        EXPECT_EQ(GetData() == DATACASE1, true);
    }
    void SubscriberFirstTestCase2()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase2 ";

        EXPECT_EQ(GetCode() == CODECASE2, true);
        EXPECT_EQ(GetData() == DATACASE2, true);
    }
    void SubscriberFirstTestCase3()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase3 ";

        EXPECT_EQ(GetCode() == CODECASE3, true);
        EXPECT_EQ(GetData() == DATACASE3, true);
    }
    void SubscriberFirstTestCase4()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase4 ";

        EXPECT_EQ(GetCode() == CODECASE4, true);
        EXPECT_EQ(GetData() == DATACASE4, true);
    }
    void SubscriberFirstTestCase5()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase5 ";

        EXPECT_EQ(GetCode() == CODECASE5, true);
        EXPECT_EQ(GetData() == DATACASE5, true);
    }
    void SubscriberFirstTestCase6()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase6 ";

        EXPECT_EQ(GetCode() == CODECASE6, true);
        EXPECT_EQ(GetData() == DATACASE6, true);
    }
    void SubscriberFirstTestCase7()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase7 ";

        EXPECT_EQ(GetCode() == CODECASE7, true);
        EXPECT_EQ(GetData() == DATACASE7, true);
    }
    void SubscriberFirstTestCaseEight()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberFirstTestcase8_12 ";
        EXPECT_TRUE(AbortCommonEvent());
        EXPECT_TRUE(GetAbortCommonEvent());
        EXPECT_TRUE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_TRUE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCaseThirteen()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberFirstTestcase13_18 ";
        EXPECT_FALSE(AbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(ClearAbortCommonEvent());
        EXPECT_FALSE(GetAbortCommonEvent());
        EXPECT_FALSE(IsOrderedCommonEvent());
    }

    void SubscriberFirstTestCase19()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase19 ";
    }

    void SubscriberFirstTestCase20()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase20 ";
    }

    void SubscriberFirstTestCase21()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase21 ";
    }

    void SubscriberFirstTestCase22()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase22 ";
    }

    void SubscriberFirstTestCase23()
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberThirdTest OnReceiveEvent SubscriberThirdTestcase23 ";
    }
};

class SubscriberPublishTest : public CommonEventSubscriber {
public:
    explicit SubscriberPublishTest(const CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo)
    {}

    ~SubscriberPublishTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "ActsCESDateTest::SubscriberPublishTest OnReceiveEvent ";
        std::string action = data.GetWant().GetAction();
    }
};

void ActsCESDateTest::SetUpTestCase(void)
{}

void ActsCESDateTest::TearDownTestCase(void)
{}

void ActsCESDateTest::SetUp(void)
{}

void ActsCESDateTest::TearDown(void)
{}

/*
 * @tc.number: CES_CommonEventData_CodeData_0100
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE1);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE1);
    data.SetData(DATACASE1);

    EXPECT_EQ(data.GetCode() == CODECASE1, true);
    EXPECT_EQ(data.GetData() == DATACASE1, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE1, true);
    EXPECT_EQ(data.GetData() == DATACASE1, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0200
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE2);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE2);
    data.SetData(DATACASE2);

    EXPECT_EQ(data.GetCode() == CODECASE2, true);
    EXPECT_EQ(data.GetData() == DATACASE2, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE2, true);
    EXPECT_EQ(data.GetData() == DATACASE2, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0300
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE3);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE3);
    data.SetData(DATACASE3);

    EXPECT_EQ(data.GetCode() == CODECASE3, true);
    EXPECT_EQ(data.GetData() == DATACASE3, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE3, true);
    EXPECT_EQ(data.GetData() == DATACASE3, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0400
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE4);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE4);
    data.SetData(DATACASE4);

    EXPECT_EQ(data.GetCode() == CODECASE4, true);
    EXPECT_EQ(data.GetData() == DATACASE4, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE4, true);
    EXPECT_EQ(data.GetData() == DATACASE4, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0500
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE5);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE5);
    data.SetData(DATACASE5);

    EXPECT_EQ(data.GetCode() == CODECASE5, true);
    EXPECT_EQ(data.GetData() == DATACASE5, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE5, true);
    EXPECT_EQ(data.GetData() == DATACASE5, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0600
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0600, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE6);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE6);
    data.SetData(DATACASE6);

    EXPECT_EQ(data.GetCode() == CODECASE6, true);
    EXPECT_EQ(data.GetData() == DATACASE6, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE6, true);
    EXPECT_EQ(data.GetData() == DATACASE6, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeData_0700
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 * GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeData_0700, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE7);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE7);
    data.SetData(DATACASE7);

    EXPECT_EQ(data.GetCode() == CODECASE7, true);
    EXPECT_EQ(data.GetData() == DATACASE7, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE7, true);
    EXPECT_EQ(data.GetData() == DATACASE7, true);
}

/*
 * @tc.number: CES_CommonEventData_Want_0100
 * @tc.name: CommonEventData : SetWant and GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant and GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION";
    bool resultWant = false;

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData;
    commonEventData.SetWant(wantTest);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberPublishTest> subscriber = std::make_shared<SubscriberPublishTest>(subscribeInfo);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo, subscriber);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0200
 * @tc.name: CommonEventData : GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION2";
    int codeTest = 1;
    std::string dataTest = "DATA_GET_CODE_TEST";
    bool resultWant = false;

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest, codeTest, dataTest);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberPublishTest> subscriber = std::make_shared<SubscriberPublishTest>(subscribeInfo);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo, subscriber);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0300
 * @tc.name: CommonEventData : GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0300, Function | MediumTest | Level1)
{
    Want wantTest;
    bool resultWant = false;
    CommonEventData commonEventData;
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0400
 * @tc.name: CommonEventData : SetWant and GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant and GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0400, Function | MediumTest | Level1)
{
    bool resultWant = false;
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION1";
    std::string eventAction2 = "TESTEVENT_PUBLISH_ACTION2";
    std::string dataTest = "DATA_GET_CODE_TEST2";
    Want wantTest;
    wantTest.SetAction(eventAction);
    Want wantTest2;
    wantTest2.SetAction(eventAction2);
    CommonEventData commonEventData(wantTest);
    commonEventData.SetWant(wantTest2);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest2.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0500
 * @tc.name: CommonEventData : GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0500, Function | MediumTest | Level1)
{
    int codeTest = 2;
    bool resultWant = false;
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION3";
    std::string dataTest = "DATA_GET_CODE_TEST3";
    CommonEventData commonEventData;
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData2(wantTest, codeTest, dataTest);
    resultWant = (commonEventData2.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0600
 * @tc.name: CommonEventData : SetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0600, Function | MediumTest | Level1)
{
    bool resultWant = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction2 = "TESTEVENT_PUBLISH_ACTION3";
    int codeTest = 1;
    std::string dataTest = "DATA_GET_CODE_TEST";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    Want wantTest2;
    wantTest2.SetAction(eventAction2);
    CommonEventData commonEventData(wantTest, codeTest, dataTest);
    commonEventData.SetWant(wantTest2);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<SubscriberPublishTest> subscriber = std::make_shared<SubscriberPublishTest>(subscribeInfo);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo, subscriber);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest2.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0700
 * @tc.name: CommonEventData : SetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0700, Function | MediumTest | Level1)
{
    bool resultWant = false;
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION2";
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData;
    commonEventData.SetWant(wantTest);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_Want_0800
 * @tc.name: CommonEventData : SetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_Want_0800, Function | MediumTest | Level1)
{
    int codeTest = 2;
    bool resultWant = false;
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION3";
    std::string eventAction2 = "TESTEVENT_PUBLISH_ACTION4";
    std::string dataTest = "DATA_GET_CODE_TEST3";
    CommonEventData commonEventData;
    Want wantTest;
    wantTest.SetAction(eventAction);
    Want wantTest2;
    wantTest2.SetAction(eventAction2);
    CommonEventData commonEventData2(wantTest, codeTest, dataTest);
    commonEventData2.SetWant(wantTest2);
    resultWant = (commonEventData2.GetWant().GetAction() == wantTest2.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0100
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE8);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE8);
    data.SetData(DATACASE8);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0200
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE9);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE9);
    data.SetData(DATACASE9);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0300
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE10);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE10);
    data.SetData(DATACASE10);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0400
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE11);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE11);
    data.SetData(DATACASE11);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0500
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE12);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE12);
    data.SetData(DATACASE12);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0600
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0600, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE13);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE13);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE13);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE13);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE13);
    data.SetData(DATACASE13);

    EXPECT_EQ(data.GetCode() == CODECASE13, true);
    EXPECT_EQ(data.GetData() == DATACASE13, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE13, true);
    EXPECT_EQ(data.GetData() == DATACASE13, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0700
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0700, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE14);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE14);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE14);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE14);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE14);
    data.SetData(DATACASE14);

    EXPECT_EQ(data.GetCode() == CODECASE14, true);
    EXPECT_EQ(data.GetData() == DATACASE14, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE14, true);
    EXPECT_EQ(data.GetData() == DATACASE14, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0800
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0800, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE15);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE15);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE15);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE15);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE15);
    data.SetData(DATACASE15);

    EXPECT_EQ(data.GetCode() == CODECASE15, true);
    EXPECT_EQ(data.GetData() == DATACASE15, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE15, true);
    EXPECT_EQ(data.GetData() == DATACASE15, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_0900
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_0900, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE16);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE16);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE16);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE16);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE16);
    data.SetData(DATACASE16);

    EXPECT_EQ(data.GetCode() == CODECASE16, true);
    EXPECT_EQ(data.GetData() == DATACASE16, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE16, true);
    EXPECT_EQ(data.GetData() == DATACASE16, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_1000
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_1000, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE17);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE17);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE17);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE17);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE17);
    data.SetData(DATACASE17);

    EXPECT_EQ(data.GetCode() == CODECASE17, true);
    EXPECT_EQ(data.GetData() == DATACASE17, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE17, true);
    EXPECT_EQ(data.GetData() == DATACASE17, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEvent_1100
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish no order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEvent_1100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE18);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE18);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE18);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE18);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE18);
    data.SetData(DATACASE18);

    EXPECT_EQ(data.GetCode() == CODECASE18, true);
    EXPECT_EQ(data.GetData() == DATACASE18, true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE18, true);
    EXPECT_EQ(data.GetData() == DATACASE18, true);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsync_0100
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsync_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE19);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE19);
    data.SetData(DATACASE19);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(2);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsync_0200
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsync_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE20);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE20);
    data.SetData(DATACASE20);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsync_0300
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsync_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE21);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE21);
    data.SetData(DATACASE21);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsync_0400
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsync_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE22);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE22);
    data.SetData(DATACASE22);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsync_0500
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsync_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE23);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE23);
    data.SetData(DATACASE23);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    std::shared_ptr<SubscriberPublishTest> subscriberPublishTest =
        std::make_shared<SubscriberPublishTest>(subscriberInfo);
    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo, subscriberPublishTest);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0100
 * @tc.name: CommonEventData : SetCode SetData etCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE1);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE1);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE1);
    data.SetData(DATACASE1);

    EXPECT_EQ(data.GetCode() == CODECASE1, true);
    EXPECT_EQ(data.GetData() == DATACASE1, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE1, true);
    EXPECT_EQ(data.GetData() == DATACASE1, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0200
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE2);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE2);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE2);
    data.SetData(DATACASE2);

    EXPECT_EQ(data.GetCode() == CODECASE2, true);
    EXPECT_EQ(data.GetData() == DATACASE2, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE2, true);
    EXPECT_EQ(data.GetData() == DATACASE2, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0300
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE3);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE3);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE3);
    data.SetData(DATACASE3);

    EXPECT_EQ(data.GetCode() == CODECASE3, true);
    EXPECT_EQ(data.GetData() == DATACASE3, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE3, true);
    EXPECT_EQ(data.GetData() == DATACASE3, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0400
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE4);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE4);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE4);
    data.SetData(DATACASE4);

    EXPECT_EQ(data.GetCode() == CODECASE4, true);
    EXPECT_EQ(data.GetData() == DATACASE4, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE4, true);
    EXPECT_EQ(data.GetData() == DATACASE4, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0500
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE5);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE5);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE5);
    data.SetData(DATACASE5);

    EXPECT_EQ(data.GetCode() == CODECASE5, true);
    EXPECT_EQ(data.GetData() == DATACASE5, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE5, true);
    EXPECT_EQ(data.GetData() == DATACASE5, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0600
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0600, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE6);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE6);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE6);
    data.SetData(DATACASE6);

    EXPECT_EQ(data.GetCode() == CODECASE6, true);
    EXPECT_EQ(data.GetData() == DATACASE6, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE6, true);
    EXPECT_EQ(data.GetData() == DATACASE6, true);
}

/*
 * @tc.number: CES_CommonEventData_CodeDataInfo_0700
 * @tc.name: CommonEventData : SetCode SetData GetCode and GetData
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetCode SetData GetCode and
 *           GetData
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_CodeDataInfo_0700, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(LOWPRIORITY);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);
    subscriberInfoSecond.SetPriority(MIDPRIORITY);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE7);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);
    subscriberInfoThird.SetPriority(HIGHPRIORITY);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE7);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE7);
    data.SetData(DATACASE7);

    EXPECT_EQ(data.GetCode() == CODECASE7, true);
    EXPECT_EQ(data.GetData() == DATACASE7, true);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);

    EXPECT_EQ(data.GetCode() == CODECASE7, true);
    EXPECT_EQ(data.GetData() == DATACASE7, true);
}

/*
 * @tc.number: CES_CommonEventData_WantInfo_0100
 * @tc.name: CommonEventData : SetWant and GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant and GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_WantInfo_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION";
    bool resultWant = false;

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData;
    commonEventData.SetWant(wantTest);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_WantInfo_0200
 * @tc.name: CommonEventData : GetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: GetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_WantInfo_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION2";
    int codeTest = 1;
    std::string dataTest = "DATA_GET_CODE_TEST";
    bool resultWant = false;

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest, codeTest, dataTest);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventData_WantInfo_0300
 * @tc.name: CommonEventData : SetWant
 * @tc.desc: 1.Set Subscriber
 *           2.Verify the function of CommonEventData when publish order event: SetWant
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventData_WantInfo_0300, Function | MediumTest | Level1)
{
    bool resultWant = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION2";
    std::string eventAction2 = "TESTEVENT_PUBLISH_ACTION3";
    int codeTest = 1;
    std::string dataTest = "DATA_GET_CODE_TEST";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    Want wantTest2;
    wantTest2.SetAction(eventAction2);
    CommonEventData commonEventData(wantTest, codeTest, dataTest);
    commonEventData.SetWant(wantTest2);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    resultWant = (commonEventData.GetWant().GetAction() == wantTest2.GetAction());
    EXPECT_TRUE(resultWant);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEventInfo_0100
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEventInfo_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE8);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE8);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE8);
    data.SetData(DATACASE8);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEventInfo_0200
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEventInfo_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE9);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE9);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE9);
    data.SetData(DATACASE9);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEventInfo_0300
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEventInfo_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE10);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE10);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE10);
    data.SetData(DATACASE10);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEventInfo_0400
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEventInfo_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE11);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE11);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE11);
    data.SetData(DATACASE11);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CommonEventInfo_0500
 * @tc.name: CommonEventSubscriber : AbortCommonEvent GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: AbortCommonEvent
 *           GetAbortCommonEvent ClearAbortCommonEvent and IsOrderedCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CommonEventInfo_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE12);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE12);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE12);
    data.SetData(DATACASE12);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsyncInfo_0100
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsyncInfo_0100, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE19);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE19);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE19);
    data.SetData(DATACASE19);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(2);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsyncInfo_0200
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsyncInfo_0200, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE20);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE20);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE20);
    data.SetData(DATACASE20);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsyncInfo_0300
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsyncInfo_0300, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE21);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE21);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE21);
    data.SetData(DATACASE21);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsyncInfo_0400
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsyncInfo_0400, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE22);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE22);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE22);
    data.SetData(DATACASE22);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}

/*
 * @tc.number: CES_CommonEventSubscriber_CodeDataAsyncInfo_0500
 * @tc.name: CommonEventSubscriber : SetCode SetData GetCode GetData and GoAsyncCommonEvent
 * @tc.desc: 1.Set three Subscriber
 *           2.Verify the function of CommonEventSubscriber when publish order event: SetCode
 *           SetData GetCode GetData and GoAsyncCommonEvent
 */
HWTEST_F(ActsCESDateTest, CES_CommonEventSubscriber_CodeDataAsyncInfo_0500, Function | MediumTest | Level1)
{
    // the first subscriber
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberFirstTest> subscriberFirstTest = std::make_shared<SubscriberFirstTest>(subscriberInfo);
    bool subscribeFirstResult = CommonEventManager::SubscribeCommonEvent(subscriberFirstTest);
    EXPECT_TRUE(subscribeFirstResult);

    // the second subscriber
    MatchingSkills matchingSkillsSecond;
    matchingSkillsSecond.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfoSecond(matchingSkillsSecond);

    std::shared_ptr<SubscriberSecondTest> subscriberSecondTest =
        std::make_shared<SubscriberSecondTest>(subscriberInfoSecond);
    bool subscribeResultSecond = CommonEventManager::SubscribeCommonEvent(subscriberSecondTest);
    EXPECT_TRUE(subscribeResultSecond);

    // the third subscriber
    MatchingSkills matchingSkillsThird;
    matchingSkillsThird.AddEvent(EVENTCASE23);

    CommonEventSubscribeInfo subscriberInfoThird(matchingSkillsThird);

    std::shared_ptr<SubscriberThirdTest> subscriberThirdTest =
        std::make_shared<SubscriberThirdTest>(subscriberInfoThird);
    bool subscribeResultThird = CommonEventManager::SubscribeCommonEvent(subscriberThirdTest);
    EXPECT_TRUE(subscribeResultThird);

    // publish
    Want want;
    want.SetAction(EVENTCASE23);

    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODECASE23);
    data.SetData(DATACASE23);

    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    bool publishResult = CommonEventManager::PublishCommonEvent(data, publishInfo);
    Delay(1);
    EXPECT_TRUE(publishResult);
}
