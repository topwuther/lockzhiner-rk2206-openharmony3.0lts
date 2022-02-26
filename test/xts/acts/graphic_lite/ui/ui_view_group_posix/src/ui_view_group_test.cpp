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
#include "components/root_view.h"
#include "components/ui_view_group.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIViewGroupTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_ADD_0100
 * @tc.name     test UIview-group add api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_Add_0100, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    UIView* view = new UIView();
    group->Add(view);
    EXPECT_EQ(view, group->GetChildrenHead());
    EXPECT_EQ(view->GetParent(), group);
    delete view;
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_INSERT_0200
 * @tc.name     test UIview-group insert api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_Insert_0200, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    UIView* preView = new UIView();
    UIView* view = new UIView();
    group->Add(preView);
    group->Insert(preView, view);
    EXPECT_EQ(view, preView->GetNextSibling());

    delete preView;
    delete view;
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_REMOVE_0300
 * @tc.name     test UIview-group remove api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_Remove_0300, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    UIView* view = new UIView();
    group->Add(view);
    group->Remove(view);
    EXPECT_EQ(nullptr, group->GetChildrenHead());
    EXPECT_EQ(nullptr, view->GetParent());
    delete view;
    delete group;
}


/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_REMOVEALL_0400
 * @tc.name     test UIview-group removeall api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_RemoveAll_0400, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    UIView* view = new UIView();
    UIView* view2 = new UIView();
    group->Add(view);
    group->RemoveAll();
    EXPECT_EQ(nullptr, group->GetChildrenHead());
    delete view;
    delete view2;
    delete group;
}


/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_GETVIEWTYPE_0500
 * @tc.name     test UIview-group get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_GetViewType_0500, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    EXPECT_EQ(group->GetViewType(), UI_VIEW_GROUP);
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_GETCHILDRENTAIL_0600
 * @tc.name     test UIview-group get-children-tail api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_GetChildrenTail_0600, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    UIView*  childrenTail = group->GetChildrenTail();
    EXPECT_EQ(0, 0);
    delete childrenTail;
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_SetDisallowIntercept_0700
 * @tc.name     test UIview-group set-disallow-intercept api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_SetDisallowIntercept_0700, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    bool flag = false;
    group->SetDisallowIntercept(flag);
    EXPECT_EQ(0, 0);
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEWGROUP_SetAutoSize_0800
 * @tc.name     test UIview-group set-auto-size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewGroupTest, Graphic_UIViewGroup_Test_SetAutoSize_0800, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    bool state = false;
    group->SetDisallowIntercept(state);
    EXPECT_EQ(0, 0);
    delete group;
}
} // namespace OHOS