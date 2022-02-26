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

#include "gfx_utils/rect.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class RectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void RectTest::SetUpTestCase()
{
    return;
}

void RectTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_OPERATOR_EQUAL_0100
 * @tc.name     test rect operator-equal api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_operator_Equal_0100, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect();
    rect1->SetRect(100, 200, 300, 400);
    Rect* rect = new Rect();
    rect->operator=(*rect1);

    EXPECT_EQ(rect->GetLeft(), 100);
    EXPECT_EQ(rect->GetTop(), 200);
    EXPECT_EQ(rect->GetRight(), 300);
    EXPECT_EQ(rect->GetBottom(), 400);

    delete rect1;
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETRECT_0200
 * @tc.name     test rect set-rect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetRect_0200, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetLeft(), 100);
    EXPECT_EQ(rect->GetTop(), 200);
    EXPECT_EQ(rect->GetRight(), 300);
    EXPECT_EQ(rect->GetBottom(), 400);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETWIDTH_0300
 * @tc.name     test rect get-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetWidth_0300, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetWidth(), 201);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETHEIGHT_0400
 * @tc.name     test rect get-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetHeight_0400, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetHeight(), 201);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETX_0500
 * @tc.name     test rect getX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetX_0500, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetX(), 100);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETY_0600
 * @tc.name     test rect getY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetY_0600, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetY(), 200);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETLEFT_0700
 * @tc.name     test rect get-left api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetLeft_0700, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetLeft(), 100);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETTOP_0800
 * @tc.name     test rect get-top api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetTop_0800, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetTop(), 200);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETRIGHT_0900
 * @tc.name     test rect get-right api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetRight_0900, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetRight(), 300);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETBOTTOM_1000
 * @tc.name     test rect get-bottom api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetBottom_1000, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetBottom(), 400);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETX_1100
 * @tc.name     test rect setX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetX_1100, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->SetX(200);

    EXPECT_EQ(rect->GetLeft(), 200);
    EXPECT_EQ(rect->GetRight(), 400);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETY_1200
 * @tc.name     test rect setY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetY_1200, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->SetY(300);

    EXPECT_EQ(rect->GetTop(), 300);
    EXPECT_EQ(rect->GetBottom(), 500);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETPOSITION_1300
 * @tc.name     test rect set-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetPosition_1300, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->SetPosition(200, 300);

    EXPECT_EQ(rect->GetLeft(), 200);
    EXPECT_EQ(rect->GetRight(), 400);
    EXPECT_EQ(rect->GetTop(), 300);
    EXPECT_EQ(rect->GetBottom(), 500);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETWIDTH_1400
 * @tc.name     test rect set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetWidth_1400, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->SetWidth(300);

    EXPECT_EQ(rect->GetRight(), 399);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETHEIGHT_1500
 * @tc.name     test rect set-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetHeight_1500, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->SetHeight(300);

    EXPECT_EQ(rect->GetBottom(), 499);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETLEFT_1600
 * @tc.name     test rect set-left api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetLeft_1600, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetLeft(200);

    EXPECT_EQ(rect->GetLeft(), 200);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETTOP_1700
 * @tc.name     test rect set-top api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetTop_1700, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetTop(200);

    EXPECT_EQ(rect->GetTop(), 200);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETRIGHT_1800
 * @tc.name     test rect set-right api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetRight_1800, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRight(200);

    EXPECT_EQ(rect->GetRight(), 200);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_SETBOTTOM_1900
 * @tc.name     test rect set-bottom api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_SetBottom_1900, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetBottom(200);

    EXPECT_EQ(rect->GetBottom(), 200);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_RESIZE_2000
 * @tc.name     test rect resize api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_Resize_2000, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);
    rect->Resize(300, 300);

    EXPECT_EQ(rect->GetLeft(), 100);
    EXPECT_EQ(rect->GetTop(), 200);
    EXPECT_EQ(rect->GetRight(), 399);
    EXPECT_EQ(rect->GetBottom(), 499);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_GETSIZE_2100
 * @tc.name     test rect get-size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_GetSize_2100, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->SetRect(100, 200, 300, 400);

    EXPECT_EQ(rect->GetSize(), 40401);

    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_INTERSECT_2200
 * @tc.name     test rect intersect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_Intersect_2200, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Rect* rect2 = new Rect(0, 0, 500, 600);
    Rect* rect3 = new Rect(100, 500, 300, 600);

    EXPECT_EQ(rect1->Intersect(*rect1, *rect2), true);
    EXPECT_EQ(rect1->Intersect(*rect1, *rect3), false);

    delete rect1;
    delete rect2;
    delete rect3;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_JOIN_2300
 * @tc.name     test rect join api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_Join_2300, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Rect* rect2 = new Rect(0, 0, 500, 600);
    Rect* rect3 = new Rect(100, 500, 300, 700);

    rect1->Join(*rect1, *rect2);
    EXPECT_EQ(rect1->GetLeft(), 0);
    EXPECT_EQ(rect1->GetTop(), 0);
    EXPECT_EQ(rect1->GetRight(), 500);
    EXPECT_EQ(rect1->GetBottom(), 600);

    rect1->Join(*rect1, *rect3);
    EXPECT_EQ(rect1->GetLeft(), 0);
    EXPECT_EQ(rect1->GetTop(), 0);
    EXPECT_EQ(rect1->GetRight(), 500);
    EXPECT_EQ(rect1->GetBottom(), 700);


    delete rect1;
    delete rect2;
    delete rect3;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_ISCONTAINS_2400
 * @tc.name     test rect judge-whether-contains api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_IsContains_2400, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Vector2<int16_t> pt1 = { 200, 300 };
    Vector2<int16_t> pt2 = { 0, 0 };

    EXPECT_EQ(rect1->IsContains(pt1), true);
    EXPECT_EQ(rect1->IsContains(pt2), false);

    delete rect1;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_ISCONTAINS_2500
 * @tc.name     test rect judge-whether-contains2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_IsContains_2500, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Point pt1;
    pt1.x = 200;
    pt1.y = 300;
    Point pt2;
    pt2.x = 0;
    pt2.y = 0;

    EXPECT_EQ(rect1->IsContains(pt1), true);
    EXPECT_EQ(rect1->IsContains(pt2), false);

    delete rect1;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_ISINTERSECT_2600
 * @tc.name     test rect judge-whether-intersect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_IsIntersect_2600, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Rect* rect2 = new Rect(0, 0, 500, 600);
    Rect* rect3 = new Rect(100, 500, 300, 700);

    EXPECT_EQ(rect1->IsIntersect(*rect2), true);
    EXPECT_EQ(rect1->IsIntersect(*rect3), false);

    delete rect1;
    delete rect2;
    delete rect3;
}

/**
 * @tc.number   SUB_GRAPHIC_RECT_ISCONTAINS_2700
 * @tc.name     test rect judge-whether-contains3 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(RectTest, Graphic_Rect_Test_IsContains_2700, Function | MediumTest | Level0)
{
    Rect* rect1 = new Rect(100, 200, 300, 400);
    Rect* rect2 = new Rect(0, 0, 500, 600);
    Rect* rect3 = new Rect(100, 500, 300, 700);

    EXPECT_EQ(rect2->IsContains(*rect1), true);
    EXPECT_EQ(rect2->IsContains(*rect3), false);

    delete rect1;
    delete rect2;
    delete rect3;
}
} // namespace OHOS