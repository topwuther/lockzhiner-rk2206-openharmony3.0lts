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

#include "event_log_wrapper.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

class EventFwkLogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void EventFwkLogTest::SetUpTestCase()
{}

void EventFwkLogTest::TearDownTestCase()
{}

void EventFwkLogTest::SetUp()
{}

void EventFwkLogTest::TearDown()
{}

/*
 * Feature: EventLogWrapper
 * Function: EventLogWrapper
 * SubFunction: NA
 * FunctionPoints: Dynamically control the log print level
 * EnvConditions: NA
 * CaseDescription: set log print level
 */
HWTEST_F(EventFwkLogTest, EventLogWrapper_001, TestSize.Level0)
{
    EXPECT_EQ(EventLogLevel::DEBUG, EventLogWrapper::GetLogLevel());
    EventLogWrapper::SetLogLevel(EventLogLevel::INFO);
    EXPECT_EQ(EventLogLevel::INFO, EventLogWrapper::GetLogLevel());
}

/*
 * Feature: EventLogWrapper
 * Function: EventLogWrapper
 * SubFunction: NA
 * FunctionPoints: splice filename
 * EnvConditions: NA
 * CaseDescription: normal valid filename
 */
HWTEST_F(EventFwkLogTest, EventLogWrapper_002, TestSize.Level0)
{
    std::string fileName = "../function/EventFwk/test.cpp";
    std::string exceptStr = "test.cpp";

    std::string result = EventLogWrapper::GetBriefFileName(fileName.c_str());
    EXPECT_EQ(exceptStr, result);
}

/*
 * Feature: EventLogWrapper
 * Function: EventLogWrapper
 * SubFunction: NA
 * FunctionPoints: splice filename
 * EnvConditions: NA
 * CaseDescription: invalid filename
 */
HWTEST_F(EventFwkLogTest, EventLogWrapper_003, TestSize.Level0)
{
    std::string fileName = "test.cpp";
    std::string exceptStr = "";

    std::string result = EventLogWrapper::GetBriefFileName(fileName.c_str());
    EXPECT_EQ(exceptStr, result);

    fileName = "";
    result = EventLogWrapper::GetBriefFileName(fileName.c_str());
    EXPECT_EQ(exceptStr, result);

    result = EventLogWrapper::GetBriefFileName(nullptr);
    EXPECT_EQ(exceptStr, result);
}