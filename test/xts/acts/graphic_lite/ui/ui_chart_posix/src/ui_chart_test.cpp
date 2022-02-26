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

#include "components/ui_view_group.h"
#include "components/root_view.h"
#include "components/ui_chart.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIChartDataSerialTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETMAXDATACOUNT_0100
 * @tc.name     test chart set-max-data-count api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_SetMaxDataCount_0100, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    EXPECT_EQ(data->SetMaxDataCount(1), true);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ADDPOINTS_0200
 * @tc.name     test chart add-points api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_AddPoints_0200, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    bool ret = data->AddPoints(nullptr, 0);
    EXPECT_EQ(ret, false);
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 3200}, {4, 3500} };
    ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    ret = data->AddPoints(pointArray, 0);
    EXPECT_EQ(ret, false);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETPOINT_0300
 * @tc.name     test chart get-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_GetPoint_0300, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 3200}, {4, 3500} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    Point point;
    ret = data->GetPoint(1, point);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(point.x, 1);
    EXPECT_EQ(point.y, 2600);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_MODIFYPOINT_0400
 * @tc.name     test chart modify-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_ModifyPoint_0400, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 3200}, {4, 3500} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    Point point = { 1, 200 };
    ret = data->ModifyPoint(6, point);
    EXPECT_EQ(ret, false);

    ret = data->ModifyPoint(1, point);
    EXPECT_EQ(ret, true);

    Point point1;
    ret = data->GetPoint(1, point1);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(point1.x, 1);
    EXPECT_EQ(point1.y, 200);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_CLEARDATA_0500
 * @tc.name     test chart clear-data api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_ClearData_0500, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 3200}, {4, 3500} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    data->ClearData();

    Point point;
    ret = data->GetPoint(1, point);
    EXPECT_EQ(ret, false);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETDATACOUNT_0600
 * @tc.name     test chart get-data-count api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_GetDataCount_0600, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 3200}, {4, 3500} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    uint16_t count = data->GetDataCount();
    EXPECT_EQ(count, 5);

    data->ClearData();
    count = data->GetDataCount();
    EXPECT_EQ(count, 0);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLEGRADIENT_0700
 * @tc.name     test chart enable-grandient api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_EnableGradient_0700, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->EnableGradient(true);
    EXPECT_EQ(data->IsGradient(), true);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLESMOOTH_0800
 * @tc.name     test chart enable-smooth api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_EnableSmooth_0800, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->EnableSmooth(true);
    bool ret = data->IsSmooth();
    EXPECT_EQ(ret, true);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETPEAKINDEX_0900
 * @tc.name     test chart get-peak-index api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_GetPeakIndex_0900, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 200}, {4, 1654} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    uint16_t index = data->GetPeakIndex();
    EXPECT_EQ(index, 2);

    int16_t value = data->GetPeakData();
    EXPECT_EQ(value, 3000);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETVALLEYINDEX_1000
 * @tc.name     test chart get-valleyindex api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_GetValleyIndex_1000, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 200}, {4, 1654} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    uint16_t index = data->GetValleyIndex();
    EXPECT_EQ(index, 3);

    int16_t value = data->GetValleyData();
    EXPECT_EQ(value, 200);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETLATESTINDEX_1100
 * @tc.name     test chart get-latest-count api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_GetLatestIndex_1100, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 200}, {4, 1654} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    uint16_t index = data->GetLatestIndex();
    EXPECT_EQ(index, 4);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETLASTPOINTINDEX_1200
 * @tc.name     test chart set-LAST-POINT-INDEX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest,
    Graphic_UIChartDataSerialTest_Test_SetBottomPointStyle_1200,
    Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();
    data->SetMaxDataCount(5);
    Point pointArray[5] = { {0, 2478}, {1, 2600}, {2, 3000}, {3, 200}, {4, 1654} };
    bool ret = data->AddPoints(pointArray, 5);
    EXPECT_EQ(ret, true);

    uint16_t index = data->GetLastPointIndex();
    EXPECT_EQ(index, 0);

    data->SetLastPointIndex(4);
    EXPECT_EQ(data->GetLastPointIndex(), 4);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETLINECOLOR_1300
 * @tc.name     test chart set-linecolor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_SetLineColor_1300, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    ColorType color = Color::Red();
    data->SetLineColor(color);
    EXPECT_EQ(data->GetLineColor().full, color.full);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETFILLCOLOR_1400
 * @tc.name     test chart set-fillcolor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_SetFillColor_1400, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    ColorType color = Color::Red();
    data->SetFillColor(color);
    EXPECT_EQ(data->GetFillColor().full, color.full);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_HIDEPOINT_1500
 * @tc.name     test chart hide-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartDataSerialTest_Test_HidePoint_1500, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    uint16_t index = 3;
    uint16_t count = 5;
    data->HidePoint(index, count);
    EXPECT_EQ(data->GetHideIndex(), index);
    EXPECT_EQ(data->GetHideCount(), count);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETHEADPOINTSTYLE_1600
 * @tc.name     test chart set-head-point-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest,
    Graphic_UIChartDataSerialTest_Test_SetBottomPointStyle_1600,
    Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartDataSerial::PointStyle pointStyle;
    pointStyle.fillColor = Color::Red();
    pointStyle.radius = 5;
    pointStyle.strokeColor = Color::White();
    pointStyle.strokeWidth = 10;

    data->SetHeadPointStyle(pointStyle);

    UIChartDataSerial::PointStyle pointStyle1 = data->GetHeadPointStyle();

    EXPECT_EQ(pointStyle1.fillColor.full, pointStyle.fillColor.full);
    EXPECT_EQ(pointStyle1.strokeColor.full, pointStyle.strokeColor.full);
    EXPECT_EQ(pointStyle1.strokeWidth, pointStyle.strokeWidth);
    EXPECT_EQ(pointStyle1.radius, pointStyle.radius);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETTOPPOINTSTYLE_1700
 * @tc.name     test chart set-top-point-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest,
    Graphic_UIChartDataSerialTest_Test_SetBottomPointStyle_1700,
    Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartDataSerial::PointStyle pointStyle;
    pointStyle.fillColor = Color::Red();
    pointStyle.radius = 5;
    pointStyle.strokeColor = Color::White();
    pointStyle.strokeWidth = 10;

    data->SetTopPointStyle(pointStyle);

    UIChartDataSerial::PointStyle pointStyle1 = data->GetTopPointStyle();

    EXPECT_EQ(pointStyle1.fillColor.full, pointStyle.fillColor.full);
    EXPECT_EQ(pointStyle1.strokeColor.full, pointStyle.strokeColor.full);
    EXPECT_EQ(pointStyle1.strokeWidth, pointStyle.strokeWidth);
    EXPECT_EQ(pointStyle1.radius, pointStyle.radius);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_SETBOTTOMPOINTSTYLE_1800
 * @tc.name     test chart set-bottom-point-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest,
    Graphic_UIChartDataSerialTest_Test_SetBottomPointStyle_1800,
    Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartDataSerial::PointStyle pointStyle;
    pointStyle.fillColor = Color::Red();
    pointStyle.radius = 5;
    pointStyle.strokeColor = Color::White();
    pointStyle.strokeWidth = 10;

    data->SetBottomPointStyle(pointStyle);

    UIChartDataSerial::PointStyle pointStyle1 = data->GetBottomPointStyle();

    EXPECT_EQ(pointStyle1.fillColor.full, pointStyle.fillColor.full);
    EXPECT_EQ(pointStyle1.strokeColor.full, pointStyle.strokeColor.full);
    EXPECT_EQ(pointStyle1.strokeWidth, pointStyle.strokeWidth);
    EXPECT_EQ(pointStyle1.radius, pointStyle.radius);

    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ADDDATASERIAL_1900
 * @tc.name     test chart add-dataserial api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_AddDataSerial_1900, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();
    bool ret = chart->AddDataSerial(nullptr);
    EXPECT_EQ(ret, false);

    ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, false);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_DELETEDATASERIAL_2000
 * @tc.name     test chart delete-data-serial api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_DeleteDataSerial_2000, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    bool ret = chart->DeleteDataSerial(nullptr);
    EXPECT_EQ(ret, false);

    ret = chart->DeleteDataSerial(data);
    EXPECT_EQ(ret, false);

    ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    ret = chart->DeleteDataSerial(data);
    EXPECT_EQ(ret, true);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_CLEARDATASERIAL_2100
 * @tc.name     test chart clear-data-serial api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_ClearDataSerial_2100, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    bool ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    chart->ClearDataSerial();

    ret = chart->DeleteDataSerial(data);
    EXPECT_EQ(ret, false);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLEHEADPOINT_2200
 * @tc.name     test chart EnableHeadPoint api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_EnableHeadPoint_2200, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();
    bool ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    data->EnableHeadPoint(true);
    EXPECT_EQ(0, 0);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLETOPPOINT_2300
 * @tc.name     test chart EnableTopPoint api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_EnableTopPoint_2300, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    bool ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    data->EnableTopPoint(true);
    EXPECT_EQ(0, 0);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLEBOTTOMPOINT_2400
 * @tc.name     test chart enable-bottom-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_EnableBottomPoint_2400, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    bool ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    data->EnableBottomPoint(true);
    EXPECT_EQ(0, 0);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_ENABLEREVERSE_2500
 * @tc.name     test chart EnableReverse api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_EnableReverse_2500, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    bool ret = chart->AddDataSerial(data);
    EXPECT_EQ(ret, true);

    chart->EnableReverse(true);
    EXPECT_EQ(0, 0);

    delete chart;
    delete data;
}


/**
 * @tc.number   SUB_GRAPHIC_CHART_GETXAXIS_2600
 * @tc.name     test chart GetXAxis api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_GetXAxis_2600, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    ColorType color = Color::Red();
    UIXAxis &xAxis = chart->GetXAxis();

    xAxis.SetLineColor(color);

    EXPECT_EQ(xAxis.GetStyle(STYLE_LINE_COLOR), color.full);

    delete chart;
    delete data;
}

/**
 * @tc.number   SUB_GRAPHIC_CHART_GETYAXIS_2700
 * @tc.name     test chart GetYAxis api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIChartDataSerialTest, Graphic_UIChartTest_Test_GetYAxis_2700, Function | MediumTest | Level0)
{
    UIChartDataSerial* data = new UIChartDataSerial();

    UIChartPillar* chart = new UIChartPillar();

    ColorType color = Color::Red();

    UIYAxis &yAxis = chart->GetYAxis();

    yAxis.SetLineColor(color);
    EXPECT_EQ(yAxis.GetStyle(STYLE_LINE_COLOR), color.full);

    delete chart;
    delete data;
}
}