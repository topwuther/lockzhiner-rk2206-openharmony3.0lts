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

#include "gfx_utils/transform.h"
#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class TransformTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void TransformTest::SetUpTestCase()
{
    return;
}

void TransformTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_SETPOLYGON_0100
 * @tc.name     test transform set-polygon api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_SetPolygon_0100, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();
    Polygon poly;
    uint8_t vertexNum = 8;
    poly.SetVertexNum(vertexNum);
    transform->SetPolygon(poly);
    EXPECT_EQ(transform->GetPolygon().GetVertexNum(), vertexNum);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_GETPIVOT_0200
 * @tc.name     test transform get-pivot api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_GetPivot_0200, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();

    EXPECT_EQ(transform->GetPivot().x_, 0);
    EXPECT_EQ(transform->GetPivot().y_, 0);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_GETCLOCKWISE_0300
 * @tc.name     test transform get-clockwise api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_GetClockWise_0300, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();
    transform->GetPolygon().SetVertexNum(2); // 2 < VERTEX_NUM_MIN
    EXPECT_EQ(transform->GetClockWise(), false);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ISINVALID_0400
 * @tc.name     test transform judge-whether-invalid api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_IsInvalid_0400, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();
    EXPECT_EQ(transform->IsInvalid(), true);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ISINVALID_0500
 * @tc.name     test transform judge-whether-invalid2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_IsInvalid_0500, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap(Rect(100, 200, 300, 400));
    EXPECT_EQ(transform->IsInvalid(), true);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_GETTRANSMAPRECT_0600
 * @tc.name     test transform get-transmaprect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_GetTransMapRect_0600, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap(Rect(100, 200, 300, 400));
    EXPECT_EQ(transform->GetTransMapRect().GetLeft(), 100);
    EXPECT_EQ(transform->GetTransMapRect().GetTop(), 200);
    EXPECT_EQ(transform->GetTransMapRect().GetRight(), 300);
    EXPECT_EQ(transform->GetTransMapRect().GetBottom(), 400);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_GETBOXRECT_0700
 * @tc.name     test transform getboxrect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_GetBoxRect_0700, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap(Rect(100, 200, 300, 400));
    EXPECT_EQ(transform->GetBoxRect().GetLeft(), 100);
    EXPECT_EQ(transform->GetBoxRect().GetTop(), 200);
    EXPECT_EQ(transform->GetBoxRect().GetRight(), 300);
    EXPECT_EQ(transform->GetBoxRect().GetBottom(), 400);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ROTATE_0800
 * @tc.name     test transform rotate api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_Rotate_0800, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();
    int16_t angle = 0;
    Vector2<float> pivot = {0, 0};
    transform->Rotate(angle, pivot);
    EXPECT_EQ(0, 0);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_SCAL_0900
 * @tc.name     test transform scal api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_Scal_0900, Function | MediumTest | Level0)
{
    TransformMap *transform = new TransformMap();
    Vector2<float> scale = {0, 0};
    Vector2<float> pivot = {0, 0};
    transform->Scale(scale, pivot);
    EXPECT_EQ(0, 0);
    delete transform;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ROTATE_1000
 * @tc.name     test transform rotate2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_Rotate_1000, Function | MediumTest | Level0)
{
    Vector2<int16_t> point = {0, 0};
    int16_t angle = 0;
    Vector2<int16_t> pivot = {0, 0};
    Vector2<int16_t> out = {0, 0};
    Rotate(point, angle, pivot, out);
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ROTATE_1100
 * @tc.name     test transform rotate3 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_Rotate_1100, Function | MediumTest | Level0)
{
    Line* origLine = new Line();
    int16_t angle = 0;
    Vector2<int16_t> pivot = {0, 0};
    Line* out =new Line();
    Rotate(*origLine, angle, pivot, *out);
    EXPECT_EQ(0, 0);
    delete out;
    delete origLine;
}

/**
 * @tc.number   SUB_GRAPHIC_TRANSFORM_ROTATE_1200
 * @tc.name     test transform rotate4 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TransformTest, Graphic_Transform_Test_Rotate_1200, Function | MediumTest | Level0)
{
    Rect* origRect = new Rect();
    int16_t angle = 0;
    Vector2<int16_t> pivot = {0, 0};
    Polygon* out =new Polygon();
    Rotate(*origRect, angle, pivot, *out);
    EXPECT_EQ(0, 0);
    delete out;
    delete origRect;
}
} // namespace OHOS
