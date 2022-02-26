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
#include "events/drag_event.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class DragEventTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0100
 * @tc.name     test dray-event constructor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_Constructor_0100, Function | MediumTest | Level0)
{
    Point newPos = { 100, 200 };
    Point lastPos = { 150, 250 };
    Point totalLen = { 50, 150 };
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);
    EXPECT_EQ(daryEvent->GetCurrentPos().x, newPos.x);
    EXPECT_EQ(daryEvent->GetCurrentPos().y, newPos.y);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0200
 * @tc.name     test dray-event get-last-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_GetLastPoint_0200, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);
    EXPECT_EQ(daryEvent->GetLastPoint().x, lastPos.x);
    EXPECT_EQ(daryEvent->GetLastPoint().y, lastPos.y);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0300
 * @tc.name     test dray-event get-start-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest,  Graphic_DrayEvent_Test_GetStartPoint_0300, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);
    EXPECT_EQ(daryEvent->GetStartPoint().x, newPos.x - totalLen.x);
    EXPECT_EQ(daryEvent->GetStartPoint().y, newPos.y - totalLen.y);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0400
 * @tc.name     test dray-event set-prelast-point api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_SetPreLastPoint_0400, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);

    Point preLastPos = {20, 40};
    daryEvent->SetPreLastPoint(preLastPos);
    EXPECT_EQ(daryEvent->GetPreLastPoint().x, preLastPos.x);
    EXPECT_EQ(daryEvent->GetPreLastPoint().y, preLastPos.y);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0500
 * @tc.name     test dray-event get-drag-direction api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_GetDragDirection_0500, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);

    EXPECT_EQ(daryEvent->GetDragDirection(), 2);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0600
 * @tc.name     test dray-event get-deltaX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_GetDeltaX_0600, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);

    EXPECT_EQ(daryEvent->GetDeltaX(), newPos.x - lastPos.x);
    delete daryEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_DRAYEVENT_CONSTRUCTOR_0700
 * @tc.name     test dray-event get-deltaY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(DragEventTest, Graphic_DrayEvent_Test_GetDeltaY_0700, Function | MediumTest | Level0)
{
    Point newPos = {100, 200};
    Point lastPos = {150, 250};
    Point totalLen = {50, 150};
    DragEvent *daryEvent = new DragEvent(newPos, lastPos, totalLen);

    EXPECT_EQ(daryEvent->GetDeltaY(), newPos.y - lastPos.y);
    delete daryEvent;
}
} // namespace OHOS