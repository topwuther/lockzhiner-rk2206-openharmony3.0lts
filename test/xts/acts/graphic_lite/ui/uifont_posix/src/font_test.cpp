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
#include "components/root_view.h"
#include "components/ui_view_group.h"
#include "components/ui_label.h"
#include "font/ui_font.h"
#include "common/ui_text_language.h"

#include <climits>
#include <gtest/gtest.h>
#include <cstring>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class AbsatrctProgressTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};
/**
 * @tc.setup: define a setup for test suit
 * @return: true——setup success
 */
bool UIFontTestSetUp()
{
    return true;
}

/**
 * @tc.teardown: define a setup for test suit
 * @return: true——teardown success
 */
bool UIFontTestTearDown()
{
    return true;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_SETFONTID_0100
 * @tc.name     test font set-fondID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_SETFONTID_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    UIFont::GetInstance()->SetCurrentFontId(UITextLanguageFontId::F_ROBOTOCONDENSED_REGULAR_14_4);
    label->SetFontId(UITextLanguageFontId::F_ROBOTOCONDENSED_REGULAR_14_4);
    uint8_t fontId = UIFont::GetInstance()->GetCurrentFontId();
    EXPECT_EQ(fontId, 0); // 0: means fontId
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_SETFONT_0100
 * @tc.name     test font set-fond api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_SETFONT_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    uint8_t fontId = UIFont::GetInstance()->GetFontId("RobotoCondensed-Regular");
    UIFont::GetInstance()->SetCurrentFontId(fontId, 18); // 18: means font size
    label->SetFont("RobotoCondensed-Regular", 18); // 18: means font size
    EXPECT_EQ(fontId, 1); // 1: means fontId
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_SETFONTHEIGHT_0100
 * @tc.name     test font set-fondheight api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_SETFONTHEIGHT_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetFontId(UITextLanguageFontId::F_HYQIHEI_65S_18_4);
    uint16_t height = UIFont::GetInstance()->GetHeight();
    EXPECT_EQ(height, 21); // 21: means font Height
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETFONTVERSION_0100
 * @tc.name     test font get-font-version api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETFONTVERSION_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetFontId(UITextLanguageFontId::F_HYQIHEI_65S_14_4);
    char dVersion[OHOS::FONT_VERSION_LEN];
    char sVersion[OHOS::FONT_VERSION_LEN];
    UIFont::GetInstance()->GetFontVersion(dVersion, OHOS::FONT_VERSION_LEN, sVersion,
        OHOS::FONT_VERSION_LEN);
    int ret = strncmp(dVersion, "1.7",  OHOS::FONT_VERSION_LEN); // 1.7: means dfont version
    EXPECT_EQ(ret, 0); // 0: means equal
    ret = strncmp(sVersion, "1.10",  OHOS::FONT_VERSION_LEN); // 1.10: means sfont version
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETFONTID_0100
 * @tc.name     test font get-fontID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETFONTID_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetFont("HYQiHei-65S", 18); // 18: means font size
    uint8_t fontId = UIFont::GetInstance()->GetFontId("HYQiHei-65S", 18); // 18: means font size
    EXPECT_EQ(fontId, 11); // 11: means font Id
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETFONTWIDTH_0100
 * @tc.name     test font get-fontwidth api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETFONTWIDTH_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_RESOURCEID45);
    uint16_t width = UIFont::GetInstance()->GetWidth(0x31, 0); // 0x31: letter "1"
    EXPECT_EQ(width, 9); // 9: means font width
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETFONTHEADER_0100
 * @tc.name     test font get-fontheader api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETFONTHEADER_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetFontId(UITextLanguageFontId::F_HYQIHEI_65S_14_4);
    FontHeader fontHeader;
    FontHeader expectHeader = {1, 1520, 21, 17, -5, 3552, 3552, 34069, 49699, 0};
    UIFont::GetInstance()->GetCurrentFontHeader(fontHeader);
    int ret = memcmp(&fontHeader, &expectHeader, sizeof(FontHeader));
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETGLYPHNODE_0100
 * @tc.name     test font get-glyphnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETGLYPHNODE_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_RESOURCEID45);
    label->SetLongMode(UILabel::UI_LABEL_LONG_ROLL);
    GlyphNode glyphNode;
    UIFont::GetInstance()->GetGlyphNode(0x31, glyphNode); // 0x31: letter "1"
    GlyphNode expectNode = {49, 1, 13, 9, 5, 13, 605, 644, 0, 1};
    int ret = memcmp(&glyphNode, &expectNode, sizeof(GlyphNode));
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETTEXTPARAM_0100
 * @tc.name     test font get-textparam api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETTEXTPARAM_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_RESOURCEID45);
    label->SetLongMode(UILabel::UI_LABEL_LONG_ROLL);
    UITextLanguageTextParam param;
    UIFont::GetInstance()->GetTextParam(T_RESOURCEID45, param);
    UITextLanguageTextParam expectParam = {78, 7, TEXT_ALIGNMENT_RIGHT, TEXT_DIRECT_LTR, 1, 0, 607, 0, 607, 0, 607, 0};
    int ret = memcmp(&param, &expectParam, sizeof(UITextLanguageTextParam));
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETTEXTUTF8_0100
 * @tc.name     test font get-text-UTF8 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETTEXTUTF8_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_SCREEN3_CHINA4);
    uint8_t *utf8Addr = nullptr;
    uint16_t utf8Len;
    UIFont::GetInstance()->SetCurrentLangId(LANGUAGE_GB);
    UIFont::GetInstance()->GetTextUtf8(T_SCREEN3_CHINA4, &utf8Addr, utf8Len);
    int ret = strncmp("VO2max", reinterpret_cast<const char *>(utf8Addr), utf8Len);
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETCURLOCALE_0100
 * @tc.name     test font get-curlocale api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETCURLOCALE_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_RESOURCEID45);
    char *isoAddr = nullptr;
    uint8_t isoLen;
    UIFont::GetInstance()->GetCurrentLocale(&isoAddr, isoLen);
    int ret = strncmp("US,UM", reinterpret_cast<const char *>(isoAddr), isoLen);
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETLOCALEBYID_0100
 * @tc.name     test font get-locale-by-ID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETLOCALEBYID_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    label->SetText(T_RESOURCEID45);
    char *isoAddr = nullptr;
    uint8_t isoLen;
    UIFont::GetInstance()->GetLocaleByLangId(LANGUAGE_FR, &isoAddr, isoLen);
    int ret = strncmp("BE,CH,FR,LU,MC", reinterpret_cast<const char *>(isoAddr), isoLen);
    EXPECT_EQ(ret, 0); // 0: means equal
    UIFontTestTearDown();
    delete label;
}

void RunUIFontUnitTest()
{
    Font_FontEngine_FontConvert_Tdd_FontTestSetFontId_001();
    Font_FontEngine_FontConvert_Tdd_FontTestSetFont_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetFontHeight_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetFontVersion_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetFontId_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetFontWidth_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetFontHeader_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetGlyphNode_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetTextParam_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetTextUtf8_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetCurLocale_001();
    Font_FontEngine_FontConvert_Tdd_FontTestGetLocaleById_001();
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETCURLOCALE_0100
 * @tc.name     test font get-curlocale api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, SUB_GRAPHIC_FONT_GETCURLOCALE_0100, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    uintptr_t psramAddr = 0;
    uint32_t psramLen = 0;
    int8_t ret = label->SetPsramMemory(psramAddr, psramLen);
    EXPECT_EQ(ret, 0); 
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_FONT_GETCURLOCALE_0100
 * @tc.name     test font get-curlocale api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(AbsatrctProgressTest, Graphic_UIDumpDomTree_Test_DumpDomNode_001, Function | MediumTest | Level0)
{
    UIFontTestSetUp();
    auto label = std::unique_ptr<UILabel>(new UILabel());
    char* dpath = "src/bin";
    char* spath = "src/bin"
    int8_t ret = label->SetFontPath(dpath, spath);
    EXPECT_EQ(ret, 0); 
    delete label;
}
} // namespace OHOS