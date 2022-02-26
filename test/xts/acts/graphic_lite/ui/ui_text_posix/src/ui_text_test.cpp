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

#include "common/text.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UITextTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        if (textVar == nullptr) {
            textVar = new Text();
        }
    }
    static void TearDownTestCase(void)
    {
        if (textVar != nullptr) {
            delete textVar;
            textVar = nullptr;
        }
    }
    static  Text* textVar;
};
Text* UITextTest::textVar = nullptr;

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETTEXT_0100
 * @tc.name     test text set-text api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_SetText_0100, Function | MediumTest | Level0)
{
    const char* text = "abc";
    textVar->SetText(text);
    EXPECT_EQ(strcmp(textVar->GetText(), text), 0);
    EXPECT_EQ(textVar->IsNeedRefresh(), true);
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETDIRECT_0200
 * @tc.name     test text set-direct api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_SetDirect_0200, Function | MediumTest | Level0)
{
    UITextLanguageDirect direct = TEXT_DIRECT_RTL;
    textVar->SetDirect(direct);
    EXPECT_EQ(textVar->GetDirect(), direct);
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETALIGN_0300
 * @tc.name     test text set-align api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_SetAlign_0300, Function | MediumTest | Level0)
{
    textVar->SetAlign(TEXT_ALIGNMENT_LEFT, TEXT_ALIGNMENT_TOP);
    EXPECT_EQ(textVar->GetHorAlign(), TEXT_ALIGNMENT_LEFT);
    EXPECT_EQ(textVar->GetVerAlign(), TEXT_ALIGNMENT_TOP);
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETEXPANDWIDTH_0500
 * @tc.name     test text set-expand-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_SetExpandWidth_0500, Function | MediumTest | Level0)
{
    bool expand = true;
    textVar->SetExpandWidth(expand);
    EXPECT_EQ(textVar->IsExpandWidth(), expand);
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETEXPANDHEIGHT_0600
 * @tc.name     test text set-expand-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_SetExpandHeight_0600, Function | MediumTest | Level0)
{
    bool expand = true;
    textVar->SetExpandHeight(expand);
    EXPECT_EQ(textVar->IsExpandHeight(), expand);
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXT_SETELLIPSISINDEX_0700
 * @tc.name     test text set-ellipsis-index api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextTest, Graphic_UITextTest_Test_GetEllipsisIndex_0700, Function | MediumTest | Level0)
{
    Rect rect;
    rect.SetX(1);
    rect.SetY(1);
    rect.SetWidth(2);
    rect.SetHeight(2);

    Style style;
    style.letterSpace_ = 3;
    style.lineHeight_ = 4;

    EXPECT_NE(textVar->GetEllipsisIndex(rect, style), 0);
}
}