/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "list.h"

#include <stddef.h>

void ListInit(struct ListNode *node)
{
    if (node == NULL) {
        return;
    }
    node->next = node;
    node->prev = node;
}

void ListAddTail(struct ListNode *head, struct ListNode *item)
{
    if (head == NULL || item == NULL) {
        return;
    }
    item->next = head;
    item->prev = head->prev;
    head->prev->next = item;
    head->prev = item;
}

void ListRemove(struct ListNode *item)
{
    if (item == NULL) {
        return;
    }
    item->next->prev = item->prev;
    item->prev->next = item->next;
}
