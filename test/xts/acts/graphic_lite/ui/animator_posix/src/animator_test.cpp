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

#include "animator/animator.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class AnimatorTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        if (animator == nullptr) {
            animator = new Animator();
        }
    }
    static void TearDownTestCase(void)
    {
        if (animator != nullptr) {
            delete animator;
            animator = nullptr;
        }
    }
    static Animator* animator;
};
Animator* AnimatorTest::animator = nullptr;

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_START_0100
 * @tc.name     test animator start api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_Start_0100, Function | MediumTest | Level0)
{
    animator->Start();
    EXPECT_EQ(animator->GetState(), Animator::START);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_STOP_0200
 * @tc.name     test animator stop api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_Stop_0200, Function | MediumTest | Level0)
{
    animator->Stop();
    EXPECT_EQ(animator->GetState(), Animator::STOP);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_PAUSE_0300
 * @tc.name     test animator pause api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_Pause_0300, Function | MediumTest | Level0)
{
    animator->Pause();
    EXPECT_EQ(animator->GetState(), Animator::PAUSE);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_RESUME_0400
 * @tc.name     test animator resume api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_Resume_0400, Function | MediumTest | Level0)
{
    animator->Resume();
    EXPECT_EQ(animator->GetState(), Animator::START);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_SETSTATE_0500
 * @tc.name     test animator set-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_SetState_0500, Function | MediumTest | Level0)
{
    animator->SetState(Animator::START);
    EXPECT_EQ(animator->GetState(), Animator::START);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_SETTIME_0600
 * @tc.name     test animator set-time api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_SetTime_0600, Function | MediumTest | Level0)
{
    uint16_t time = 300;
    animator->SetTime(time);
    EXPECT_EQ(animator->GetTime(), time);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_SETRUNTIME_0700
 * @tc.name     test animator set-runtime api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_SetRunTime_0700, Function | MediumTest | Level0)
{
    uint16_t time = 300;
    animator->SetRunTime(time);
    EXPECT_EQ(animator->GetRunTime(), time);
}

/**
 * @tc.number   SUB_GRAPHIC_ANIMATOR_ISRPEAT_0800
 * @tc.name     test animator if-repeat api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AnimatorTest, Graphic_Animator_Test_IsRepeat_0800, Function | MediumTest | Level0)
{
    EXPECT_EQ(animator->IsRepeat(), false);
}
} // namespace OHOS