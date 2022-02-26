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

#include "components/root_view.h"
#include "components/ui_view_group.h"
#include "components/ui_image_view.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIImageTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};
/**
    * @tc.setup: define a setup for test suit, format:"UILabelTest + SetUp"
    * @return: true——setup success
    */
bool UIImageTestSetUp()
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(HORIZONTAL_RESOLUTION);
    viewGroup->SetHeight(VERTICAL_RESOLUTION);
    return true;
}

/**
    * @tc.teardown: define a setup for test suit, format:"UILabelTest + TearDown"
    * @return: true——teardown success
    */
bool UIImageTestTearDown()
{
    RootView* rootView = RootView::GetInstance();
    rootView->RemoveAll();
    return true;
}

/**
 * @tc.number   SUB_GRAPHIC_UIIMAGE_GETVIEWTYPE_0100
 * @tc.name     test UIimage get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageTest, Graphic_UIImage_Test_GetViewType_010, Function | MediumTest | Level0)
{
    UIImageView* imageView = new UIImageView();
    EXPECT_EQ(imageView->GetViewType(), UI_IMAGE_VIEW);
    delete imageView;
}

/**
 * @tc.number   SUB_GRAPHIC_UIIMAGE_SETIMAGE_0200
 * @tc.name     test UIimage set-image api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageTest, Graphic_UIImage_Test_SetImage_0200, Function | MediumTest | Level0)
{
    UIImageTestSetUp();
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    UIImageView* imageView = new UIImageView();
    imageView->SetPosition(10, 10);
    imageView->SetHeight(100);
    imageView->SetWidth(200);

    EXPECT_EQ(imageView->GetX(), 10);
    EXPECT_EQ(imageView->GetY(), 10);
    EXPECT_EQ(imageView->GetHeight(), 100);
    EXPECT_EQ(imageView->GetWidth(), 200);

    char* srcPath = nullptr;
    imageView->SetSrc(srcPath);
    EXPECT_EQ(imageView->GetPath(), srcPath);
    const char* strPath2 = static_cast<const char*>("..\\config\\images\\A021_001.bin");
    imageView->SetSrc(strPath2);
    EXPECT_NE(imageView->GetPath(), strPath2);
    ImageInfo* srcPath2 = nullptr;
    imageView->SetSrc(srcPath2);
    EXPECT_EQ(imageView->GetImageInfo(), srcPath2);

    imageView->SetAutoEnable(true);
    EXPECT_EQ(imageView->GetAutoEnable(), true);

    imageView->SetParent(nullptr);
    EXPECT_EQ(imageView->GetParent(), nullptr);
    UIView uiView;
    imageView->SetParent(&uiView);
    EXPECT_NE(imageView->GetParent(), nullptr);

    imageView->SetNextSibling(nullptr);
    EXPECT_EQ(imageView->GetNextSibling(), nullptr);
    imageView->SetNextSibling(&uiView);
    EXPECT_NE(imageView->GetNextSibling(), nullptr);

    imageView->SetVisible(true);
    EXPECT_EQ(imageView->IsVisible(), true);
    imageView->SetVisible(false);
    EXPECT_EQ(imageView->IsVisible(), false);

    viewGroup->Invalidate();
    UIImageTestTearDown();
}

/**
 * @tc.number   SUB_GRAPHIC_UIIMAGE_SETDRAGGABLE_0300
 * @tc.name     test UIimage set-draggable api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageTest, Graphic_UIImage_Test_SetDraggable_0300, Function | MediumTest | Level0)
{
    UIImageTestSetUp();
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    UIImageView* imageView = new UIImageView();
    imageView->SetTouchable(true);
    EXPECT_EQ(imageView->IsTouchable(), true);
    imageView->SetTouchable(false);
    EXPECT_EQ(imageView->IsTouchable(), false);

    imageView->SetPosition(30, 50, 70, 90);

    EXPECT_EQ(imageView->GetX(), 30);
    EXPECT_EQ(imageView->GetY(), 50);
    EXPECT_EQ(imageView->GetWidth(), 70);
    EXPECT_EQ(imageView->GetHeight(), 90);

    imageView->SetDraggable(true);
    EXPECT_EQ(imageView->IsDraggable(), true);
    imageView->SetDraggable(false);
    EXPECT_EQ(imageView->IsDraggable(), false);

    imageView->SetDragParentInstead(true);
    EXPECT_EQ(imageView->IsDragParentInstead(), true);
    imageView->SetDragParentInstead(false);
    EXPECT_EQ(imageView->IsDragParentInstead(), false);

    imageView->Resize(100, 300);
    EXPECT_EQ(imageView->GetWidth(), 100);
    EXPECT_EQ(imageView->GetHeight(), 300);

    imageView->SetOnDragListener(nullptr);
    EXPECT_EQ(imageView->GetOnDragListener(), nullptr);

    imageView->SetOnClickListener(nullptr);
    EXPECT_EQ(imageView->GetOnClickListener(), nullptr);

    imageView->SetOnLongPressListener(nullptr);
    EXPECT_EQ(imageView->GetOnLongPressListener(), nullptr);

    imageView->SetViewId(nullptr);
    EXPECT_EQ(imageView->GetViewId(), nullptr);

    imageView->SetViewIndex(101);
    EXPECT_EQ(imageView->GetViewIndex(), 101);

    viewGroup->Invalidate();
    UIImageTestTearDown();
}

/**
 * @tc.number   SUB_GRAPHIC_UIIMAGE_SETBLULEVEL_0400
 * @tc.name     test UIimage set-blur-level api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageTest, Graphic_UIImage_Test_SetBlurLevel_0400, Function | MediumTest | Level0)
{
    UIImageTestSetUp();
    UIImageView* imageView = new UIImageView();
    BlurLevel level = LEVEL0;
    imageView->SetBlurLevel(level);
    EXPECT_EQ(imageView->GetBlurLevel(), level);
    delete imageView;
    UIImageTestTearDown();
}

/**
 * @tc.number   SUB_GRAPHIC_UIIMAGE_SETTRANSFORMALGORITHM_0500
 * @tc.name     test UIimage set-trans-from-algorithm api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageTest, Graphic_UIImage_Test_SetTransformAlgorithm_0500, Function | MediumTest | Level0)
{
    UIImageTestSetUp();
    UIImageView* imageView = new UIImageView();
    TransformAlgorithm algorithm = NEAREST_NEIGHBOR;
    imageView->SetTransformAlgorithm(algorithm);
    EXPECT_EQ(imageView->GetTransformAlgorithm(), algorithm);
    delete imageView;
    UIImageTestTearDown();
} 
} // namespace OHOS