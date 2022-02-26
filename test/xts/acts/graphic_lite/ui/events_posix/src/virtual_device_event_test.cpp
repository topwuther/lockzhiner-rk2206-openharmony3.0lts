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
#include "events/virtual_device_event.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class VirtualDeviceEventTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_VIRTUALDEVICE_GETTYPE_0100
 * @tc.name     test virtual-device get-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(VirtualDeviceEventTest, Graphic_VirtualDeviceEvent_Test_GetType_0100, Function | MediumTest | Level0)
{
    uint16_t type = 10;
    uint16_t value = 20;
    VirtualDeviceEvent *virtualDeviceEvent = new VirtualDeviceEvent(type, value);
    EXPECT_EQ(virtualDeviceEvent->GetType(), type);
    delete virtualDeviceEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_VIRTUALDEVICE_GETSTATE_0200
 * @tc.name     test virtual-device get-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(VirtualDeviceEventTest, Graphic_VirtualDeviceEvent_Test_GetState_0200, Function | MediumTest | Level0)
{
    uint16_t type = 10;
    uint16_t value = 20;
    VirtualDeviceEvent *virtualDeviceEvent = new VirtualDeviceEvent(type, value);
    EXPECT_EQ(virtualDeviceEvent->GetState(), value);
    delete virtualDeviceEvent;
}
} // namespace OHOS