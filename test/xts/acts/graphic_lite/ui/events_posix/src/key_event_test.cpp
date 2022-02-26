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
#include "events/key_event.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class KeyEventTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_KEYEVENT_GETKEYID_0100
 * @tc.name     test key-event get-keyID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(KeyEventTest, Graphic_KeyEvent_Test_GetKeyId_0100, Function | MediumTest | Level0)
{
    uint16_t keyId = 10;
    uint16_t state = 20;
    KeyEvent *keyEvent = new KeyEvent(keyId, state);

    EXPECT_EQ(keyEvent->GetKeyId(), keyId);
    delete keyEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_KEYEVENT_GETSTATE_0200
 * @tc.name     test key-event get-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(KeyEventTest, Graphic_KeyEvent_Test_GetState_0200, Function | MediumTest | Level0)
{
    uint16_t keyId = 10;
    uint16_t state = 20;
    KeyEvent *keyEvent = new KeyEvent(keyId, state);

    EXPECT_EQ(keyEvent->GetState(), state);
    delete keyEvent;
}
} // namespace OHOS