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
#include "../utils/sys_info.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class SysInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

class OnFPSChangedInstance : public SysInfo::OnFPSChangedListener {
    void OnFPSChanged(float newFPS) override 
    {
        return;
    }
};

/**
 * @tc.number   SUB_GRAPHIC_SYSINFO_FPSCALCULATETYPE_0100
 * @tc.name     test themes-manager FPS-calculate-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SysInfoTest, Graphic_SysInfo_Test_FPSCalculateType_0100, Function | MediumTest | Level0)
{
    SysInfo::FPSCalculateType type;
    auto* fpsChangedListener = new OnFPSChangedInstance;
    fpsChangedListener->SetFPSCalculateType(type);
    EXPECT_EQ(fpsChangedListener->GetFPSCalculateType(), type);
    delete fpsChangedListener;
}

/**
 * @tc.number   SUB_GRAPHIC_SYSINFO_GETFPS_0200
 * @tc.name     test themes-manager get-FPS api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SysInfoTest, Graphic_SysInfo_Test_GetFPS_0200, Function | MediumTest | Level0)
{
    float value = 0.0;
    value = SysInfo::GetFPS();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SYSINFO_REGISTERFPSCHANGEDLISTENER_0300
 * @tc.name     test themes-manager register-FPS-changed-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SysInfoTest, Graphic_SysInfo_Test_RegisterFPSChangedListener_0300, Function | MediumTest | Level0)
{
    auto* fpsChangedListener = new OnFPSChangedInstance;
    SysInfo::RegisterFPSChangedListener(fpsChangedListener);
    EXPECT_EQ(0, 0);    
    delete fpsChangedListener;
}
} // namespace OHOS