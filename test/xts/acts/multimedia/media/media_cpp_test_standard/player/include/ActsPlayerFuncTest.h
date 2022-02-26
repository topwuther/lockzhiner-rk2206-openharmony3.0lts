/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PLAYER_FUNC_TEST_H
#define PLAYER_FUNC_TEST_H

#include "TestPlayer.h"

namespace OHOS {
namespace Media {
class ActsPlayerFuncTest : public testing::Test {
public:
    // SetUpTestCase: before all testcases
    static void SetUpTestCase(void)
    {
        MEDIA_DEBUG_LOG("ActsPlayerFuncTest::SetUpTestCase");
        TestPlayerBasic::TestParamsConfig::GetInstance().InitPlayProtocol();
    };
    // TearDownTestCase: after all testcases
    static void TearDownTestCase(void)
    {
        MEDIA_DEBUG_LOG("ActsPlayerFuncTest::TearDownTestCase");
    };
    // SetUp
    void SetUp(void)
    {
        const ::testing::TestInfo * const testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        MEDIA_DEBUG_LOG("ActsPlayerFuncTest::SetUp:%s", testInfo->name());
    };
    // TearDown
    void TearDown(void)
    {
        const ::testing::TestInfo * const testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        MEDIA_DEBUG_LOG("ActsPlayerFuncTest::TearDown:%s", testInfo->name());
    };
    static void LocalPlayFunc(const std::string uri, bool isAsync, WindowConfig config = g_sub_config);
};
}
}
#endif
