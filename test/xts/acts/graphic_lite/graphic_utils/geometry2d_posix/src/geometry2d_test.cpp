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

#include "gfx_utils/geometry2d.h"
#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class Geometry2dTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void Geometry2dTest::SetUpTestCase()
{
    return;
}

void Geometry2dTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_LINE_OPERATOR_0100
 * @tc.name     test line of geometry2d operator api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Line_Test_operator_0100, Function | MediumTest | Level0)
{
    Vector2<int16_t> pt1 = { 100, 200 };
    Vector2<int16_t> pt2 = { 300, 400 };
    Line* line = new Line(pt1, pt2);

    EXPECT_EQ(line->operator[](0).x_, 100);
    EXPECT_EQ(line->operator[](0).y_, 200);
    EXPECT_EQ(line->operator[](1).x_, 300);
    EXPECT_EQ(line->operator[](1).y_, 400);
    delete line;

    line = new Line(100, 200, 300, 400);

    EXPECT_EQ(line->operator[](0).x_, 100);
    EXPECT_EQ(line->operator[](0).y_, 200);
    EXPECT_EQ(line->operator[](1).x_, 300);
    EXPECT_EQ(line->operator[](1).y_, 400);
    delete line;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_POLYGON_MAKEAABB_0200
 * @tc.name     test polygon of geometry2d makeAABB api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Polygon_Test_MakeAABB_0200, Function | MediumTest | Level0)
{
    Vector2<int16_t> vertexes[4] = { {0, 0}, {500, 100}, {300, 500}, {100, 300} };
    Polygon* polygon = new Polygon(vertexes, 4);
    Rect rect = polygon->MakeAABB();

    EXPECT_EQ(rect.GetLeft(), 0);
    EXPECT_EQ(rect.GetTop(), 0);
    EXPECT_EQ(rect.GetRight(), 500);
    EXPECT_EQ(rect.GetBottom(), 500);
    delete polygon;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_POLYGON_GETVERTEXNUM_0300
 * @tc.name     test polygon of geometry2d get-vertexNum api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Polygon_Test_GetVertexNum_0300, Function | MediumTest | Level0)
{
    Vector2<int16_t> vertexes[4] = { { 0, 0 }, { 500, 100 }, { 300, 500 }, { 100, 300 } };
    Polygon* polygon = new Polygon(vertexes, 4);

    EXPECT_EQ(polygon->GetVertexNum(), 4);
    delete polygon;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_POLYGON_SETVERTEXNUM_0400
 * @tc.name     test polygon of geometry2d set-vertexNum api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Polygon_Test_SetVertexNum_0400, Function | MediumTest | Level0)
{
    Polygon* polygon = new Polygon();
    polygon->SetVertexNum(8);

    EXPECT_EQ(polygon->GetVertexNum(), 8);
    delete polygon;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_POLYGON_OPERATOR_0500
 * @tc.name     test polygon of geometry2d operator api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Polygon_Test_operator_0500, Function | MediumTest | Level0)
{
    Vector2<int16_t> vertexes[4] = { { 0, 0 }, { 500, 100 }, { 300, 500 }, { 100, 300 } };
    Polygon* polygon = new Polygon(vertexes, 4);

    EXPECT_EQ(polygon->operator[](1).x_, 500);
    EXPECT_EQ(polygon->operator[](2).y_, 500);
    delete polygon;

    Rect rect(100, 200, 300, 400);
    polygon = new Polygon(rect);

    EXPECT_EQ(polygon->operator[](0).x_, 100);
    EXPECT_EQ(polygon->operator[](0).y_, 200);
    EXPECT_EQ(polygon->operator[](1).x_, 300);
    EXPECT_EQ(polygon->operator[](1).y_, 200);
    EXPECT_EQ(polygon->operator[](2).x_, 300);
    EXPECT_EQ(polygon->operator[](2).y_, 400);
    EXPECT_EQ(polygon->operator[](3).x_, 100);
    EXPECT_EQ(polygon->operator[](3).y_, 400);
    EXPECT_EQ(polygon->GetVertexNum(), 4);
    delete polygon;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_INTERSECT_0600
 * @tc.name     test geometry2d intersect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Geometry2d_Test_Intersect_0600, Function | MediumTest | Level0)
{
    Vector2<int16_t> pt11 = { 0, 200 };
    Vector2<int16_t> pt12 = { 400, 200 };
    Line* line1 = new Line(pt11, pt12);
    Vector2<int16_t> pt21 = { 200, 0 };
    Vector2<int16_t> pt22 = { 200, 400 };
    Line* line2 = new Line(pt21, pt22);
    Vector2<int16_t> pt31 = { 0, 0 };
    Vector2<int16_t> pt32 = { 500, 0 };
    Line* line3 = new Line(pt31, pt32);
    Vector2<int16_t> out;

    EXPECT_EQ(Intersect(*line1, *line2, out), true);
    EXPECT_EQ(out.x_, 200);
    EXPECT_EQ(out.y_, 200);
    EXPECT_EQ(Intersect(*line1, *line3, out), false);

    delete line1;
    delete line2;
    delete line3;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_ISINTERSECT_0700
 * @tc.name     test geometry2d if-intersected api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Geometry2d_Test_IsIntersect_0700, Function | MediumTest | Level0)
{
    Vector2<int16_t> pt11 = { 0, 200 };
    Vector2<int16_t> pt12 = { 400, 200 };
    Line* line1 = new Line(pt11, pt12);
    Vector2<int16_t> pt21 = { 200, 0 };
    Vector2<int16_t> pt22 = { 200, 400 };
    Line* line2 = new Line(pt21, pt22);
    Vector2<int16_t> pt31 = { 0, 0 };
    Vector2<int16_t> pt32 = { 500, 0 };
    Line* line3 = new Line(pt31, pt32);

    EXPECT_EQ(IsIntersect(*line1, *line2), true);
    EXPECT_EQ(IsIntersect(*line1, *line3), false);

    delete line1;
    delete line2;
    delete line3;
}

/**
 * @tc.number   SUB_GRAPHIC_GEOMETRY2D_CLIP_0800
 * @tc.name     test geometry2d clip api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(Geometry2dTest, Graphic_Geometry2d_Test_Clip_0800, Function | MediumTest | Level0)
{
    const Vector2<int16_t> vertexes[4] = { { 0, 0 }, { 500, 100 }, { 300, 500 }, { 100, 300 } };
    Polygon* polygon = new Polygon(vertexes, 4);
    const Vector2<int16_t> pt11 = { 100, 300 };
    const Vector2<int16_t> pt12 = { 500, 100 };
    Line* line = new Line(pt11, pt12);
    Clip(*polygon, *line);

    EXPECT_EQ(polygon->GetVertexNum(), 3);
    EXPECT_EQ(polygon->operator[](0).x_, 500);
    EXPECT_EQ(polygon->operator[](0).y_, 100);
    EXPECT_EQ(polygon->operator[](1).x_, 100);
    EXPECT_EQ(polygon->operator[](1).y_, 300);
    EXPECT_EQ(polygon->operator[](2).x_, 0);
    EXPECT_EQ(polygon->operator[](2).y_, 0);

    delete polygon;
    delete line;

    polygon = new Polygon(vertexes, 4);
    Rect rect(1, 4, 3, 1);
    Polygon polygon1 = SuthHodgClip(rect, *polygon);
    EXPECT_EQ(polygon1[0].x_, 1);
    EXPECT_EQ(polygon1[0].y_, 1);
    EXPECT_EQ(polygon1[1].x_, 3);
    EXPECT_EQ(polygon1[1].y_, 1);
    EXPECT_EQ(polygon1[2].x_, 3);
    EXPECT_EQ(polygon1[2].y_, 4);
    EXPECT_EQ(polygon1[3].x_, 1);
    EXPECT_EQ(polygon1[3].y_, 4);
    delete polygon;
}
} // namespace OHOS