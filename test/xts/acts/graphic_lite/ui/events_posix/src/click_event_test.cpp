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
#include "events/click_event.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ClickEventTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_CLICKEVENT_CONSTRUCTOR_0100
 * @tc.name     test click-event constructor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ClickEventTest, Graphic_ClickEvent_Test_Constructor_0100, Function | MediumTest | Level0)
{
    Point pos = {100, 200};
    ClickEvent *clickEvent = new ClickEvent(pos);
    EXPECT_EQ(clickEvent->GetCurrentPos().x, pos.x);
    EXPECT_EQ(clickEvent->GetCurrentPos().y, pos.y);
    delete clickEvent;
}
} // namespace OHOS
