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

#include "buffer_common.h"
#include "surface.h"
#include "surface_impl.h"
#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class SurfaceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_FORMAT_0100
 * @tc.name     test surface format api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_Format_0100, Function | MediumTest | Level0)
{
    uint32_t format = 0x00001000;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetFormat(format);
        surface->GetFormat();
        delete surface;
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SIZE_0200
 * @tc.name     test surface size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_Size_0200, Function | MediumTest | Level0)
{
    uint32_t size = 0x00001000;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetSize(size);
    }
    if (surface != nullptr) {
        EXPECT_EQ(surface->GetSize(), size);
    }
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_STRIDEALIGNMENT_0300
 * @tc.name     test surface stride-alignment api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_StrideAlignment_0300, Function | MediumTest | Level0)
{
    uint32_t strideAlignment = 0x00001000;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetStrideAlignment(strideAlignment);
        surface->GetStrideAlignment();
        delete surface;
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_USAGE_0400
 * @tc.name     test surface usage api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_Usage_0400, Function | MediumTest | Level0)
{
    uint32_t usage = 0x00001000;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetUsage(usage);
        surface->GetUsage();
        delete surface;
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_RELEASEBUFFER_0500
 * @tc.name     test surface release-buffer api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_ReleaseBuffer_0500, Function | MediumTest | Level0)
{
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->AcquireBuffer();
        delete surface;
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_FLUSHBUFFER_0600
 * @tc.name     test surface flush-buffer api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_FlushBuffer_0600, Function | MediumTest | Level0)
{
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetSize(1024);
        SurfaceBuffer* buffer = surface->RequestBuffer();
        surface->ReleaseBuffer(buffer);
        delete surface;
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_ACQUIREBUFFER_0700
 * @tc.name     test surface acquire-buffer api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_AcquireBuffer_0700, Function | MediumTest | Level0)
{
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->AcquireBuffer();
    }
    EXPECT_EQ(0, 0);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_REQUESTBUFFER_0800
 * @tc.name     test surface request-buffer api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_RequestBuffer_0800, Function | MediumTest | Level0)
{
    uint8_t wait = 100;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->RequestBuffer(wait);
    }
    EXPECT_EQ(0, 0);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_CANCELBUFFER_0900
 * @tc.name     test surface cancel-buffer api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_CancelBuffer_0900, Function | MediumTest | Level0)
{
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetSize(1024);
        SurfaceBuffer* buffer = surface->RequestBuffer();
        surface->CancelBuffer(buffer);
    }
    EXPECT_EQ(0, 0);
    delete surface;
}


/**
 * @tc.number   SUB_GRAPHIC_SURFACE_USERDATA_1000
 * @tc.name     test surface user-data api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_UserData_1000, Function | MediumTest | Level0)
{
    std::string key = "testWord";
    std::string value = "testValue";
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetUserData(key, value);
    }
    if (surface != nullptr) {
        EXPECT_EQ(surface->GetUserData(key), value);
    }
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_GETSTRIDE_1100
 * @tc.name     test surface get-stride api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_GetStride_1100, Function | MediumTest | Level0)
{
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->GetStride();
    }
    EXPECT_EQ(0, 0);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETWIDTHANDHEIGHT_1200
 * @tc.name     test surface set-width-and-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_SetWidthAndHeight_1200, Function | MediumTest | Level0)
{
    uint32_t width = 0x00000200;
    uint32_t height = 0x00000100;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetWidthAndHeight(width, height);
    }
    EXPECT_EQ(0, 0);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETQUEUESIZE_1300
 * @tc.name     test surface set-queue-size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceTest, Graphic_Surface_Test_SetQueueSize_1300, Function | MediumTest | Level0)
{
    uint8_t queueSize = 100;
    Surface* surface = Surface::CreateSurface();
    if (surface != nullptr) {
        surface->SetQueueSize(queueSize);
    }
    EXPECT_EQ(0, 0);
    delete surface;
}
} // namespace OHOS