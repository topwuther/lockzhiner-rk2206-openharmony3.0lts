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

#ifndef BASE_STARTUP_INITLITE_LIST_H
#define BASE_STARTUP_INITLITE_LIST_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

#define ListEmpty(node)   ((node).next == &(node) && (node).prev == &(node))
#define ListEntry(ptr, type, member)   (type *)((char *)(ptr) - offsetof(type, member))
#define ForEachListEntry(list, node) \
    for (node = (list)->next; node != (list); node = node->next)

void ListInit(struct ListNode *list);
void ListAddTail(struct ListNode *list, struct ListNode *item);
void ListRemove(struct ListNode *item);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_LIST_H
