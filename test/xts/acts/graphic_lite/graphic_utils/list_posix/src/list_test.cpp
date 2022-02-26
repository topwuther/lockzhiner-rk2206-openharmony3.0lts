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

#include "gfx_utils/list.h"
#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void ListTest::SetUpTestCase()
{
    return;
}

void ListTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_FRONT_0100
 * @tc.name     test list front api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Front_0100, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Front(), 1);
    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_BACK_0200
 * @tc.name     test list back api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Back_0200, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Back(), 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_PUSHBACK_0300
 * @tc.name     test list pushback api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_PushBack_0300, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Size(), 2);
    EXPECT_EQ(list->Front(), 1);
    EXPECT_EQ(list->Back(), 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_PUSHFRONT_0400
 * @tc.name     test list pushfront api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_PushFront_0400, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushFront(1);
    list->PushFront(2);
    EXPECT_EQ(list->Size(), 2);
    EXPECT_EQ(list->Front(), 2);
    EXPECT_EQ(list->Back(), 1);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_POPBACK_0500
 * @tc.name     test list popback api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_PopBack_0500, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    list->PopBack();
    EXPECT_EQ(list->Size(), 1);
    EXPECT_EQ(list->Front(), 1);
    EXPECT_EQ(list->Back(), 1);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_POPFRONT_0600
 * @tc.name     test list popfront api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_PopFront_0600, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    list->PopFront();
    EXPECT_EQ(list->Size(), 1);
    EXPECT_EQ(list->Front(), 2);
    EXPECT_EQ(list->Back(), 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_INSERT_0700
 * @tc.name     test list insert api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Insert_0700, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    ListNode<uint16_t>* node = list->Tail();
    list->Insert(node, 3);
    EXPECT_EQ(list->Size(), 3);
    EXPECT_EQ(list->Front(), 1);
    EXPECT_EQ(list->Head()->next_->data_, 3);
    EXPECT_EQ(list->Back(), 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_REMOVE_0800
 * @tc.name     test list remove api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Remove_0800, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    ListNode<uint16_t>* node = list->Head();
    list->Remove(node);
    EXPECT_EQ(list->Size(), 1);
    EXPECT_EQ(list->Front(), 2);
    EXPECT_EQ(list->Back(), 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_CLEAR_0900
 * @tc.name     test list clear api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Clear_0900, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    list->Clear();
    EXPECT_EQ(list->Size(), 0);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_HEAD_1000
 * @tc.name     test list head api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Head_1000, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Head()->data_, 1);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_TAIL_1100
 * @tc.name     test list tail api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Tail_1100, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Tail()->data_, 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_BEGIN_1200
 * @tc.name     test list begin api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Begin_1200, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Begin()->data_, 1);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_NEXT_1300
 * @tc.name     test list next api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Next_1300, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    list->PushBack(2);
    EXPECT_EQ(list->Next(list->Head())->data_, 2);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_ISEMPTY_1400
 * @tc.name     test list isempty api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_IsEmpty_1400, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    EXPECT_EQ(list->IsEmpty(), false);
    list->Clear();
    EXPECT_EQ(list->IsEmpty(), true);

    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_SIZE_1500
 * @tc.name     test list size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListTest, Graphic_List_Test_Size_1500, Function | MediumTest | Level0)
{
    List<uint16_t>* list = new List<uint16_t>();
    list->PushBack(1);
    EXPECT_EQ(list->Size(), 1);
    list->Clear();
    EXPECT_EQ(list->Size(), 0);

    delete list;
}
} // namespace OHOS