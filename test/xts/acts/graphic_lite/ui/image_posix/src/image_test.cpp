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

#include "common/image.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ImageTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        if (imageObject == nullptr) {
            imageObject = new Image();
        }
    }
    static void TearDownTestCase(void)
    {
        if (imageObject != nullptr) {
            delete imageObject;
            imageObject = nullptr;
        }
    }
    static  Image* imageObject;
};
Image* ImageTest::imageObject = nullptr;

/**
 * @tc.number   SUB_GRAPHIC_IMAGE_GETHEADER_0100
 * @tc.name     test image get-header api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ImageTest, Graphic_UIImage_Test_GetHeader_0100, Function | MediumTest | Level0)
{
    int colorMode = 1;
    ImageHeader header = {0};
    header.colorMode = colorMode;
    header.reserved = 2;
    header.width = 3;
    header.height = 4;
    imageObject->GetHeader(header);
    EXPECT_EQ(header.colorMode, colorMode);
}
}