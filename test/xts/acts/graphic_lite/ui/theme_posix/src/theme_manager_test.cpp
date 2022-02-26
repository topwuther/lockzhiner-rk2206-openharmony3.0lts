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

#include <climits>
#include <gtest/gtest.h>
#include "themes/theme_manager.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ThemeManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void ThemeManagerTest::SetUpTestCase(void)
{
}

void ThemeManagerTest::TearDownTestCase(void)
{
}

/**
 * @tc.number   SUB_GRAPHIC_THEMESMANAGER_SETCURRENT_0100
 * @tc.name     test themes-manager set-current api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeManagerTest, Graphic_ThemesManager_Test_SetCurrent_0100, Function | MediumTest | Level0)
{
    Theme* theme = new Theme();
    ThemeManager::GetInstance().SetCurrent(theme);
    EXPECT_EQ(ThemeManager::GetInstance().GetCurrent(), theme);
}
} // namespace OHOS