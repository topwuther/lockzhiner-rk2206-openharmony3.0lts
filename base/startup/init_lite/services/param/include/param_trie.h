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

#ifndef BASE_STARTUP_PARAM_TRIE_H
#define BASE_STARTUP_PARAM_TRIE_H
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

#include "sys_param.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PARAM_WORKSPACE_MAX  64*1024
#define TRIE_SERIAL_DIRTY_OFFSET         1
#define TRIE_SERIAL_KEY_LEN_OFFSET      24
#define TRIE_SERIAL_DATA_LEN_OFFSET     16

#define FILENAME_LEN_MAX  255
#define TRIE_DATA_LEN_MAX  128

#define TRIE_NODE_HEADER \
    atomic_uint_least32_t serial; \
    unsigned int left; \
    unsigned int right;

#define DATA_ENTRY_KEY_LEN(entry)   (entry)->dataLength >> TRIE_SERIAL_KEY_LEN_OFFSET
#define DATA_ENTRY_DATA_LEN(entry)  (((entry)->dataLength >> TRIE_SERIAL_DATA_LEN_OFFSET) & 0x00FF)
#define DATA_ENTRY_DIRTY(serial) ((serial) & 1)

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1
#define __futex(ftx, op, value, timeout, bitset) \
    syscall(__NR_futex, ftx, op, value, timeout, NULL, bitset)
#define futex_wake(ftx, count) __futex(ftx, FUTEX_WAKE, count, NULL, 0)
#define futex_wait(ftx, value, timeout) __futex(ftx, FUTEX_WAIT, value, timeout, 0)

typedef struct {
    TRIE_NODE_HEADER;
    char key[0];
} TrieNode;

typedef struct {
    TRIE_NODE_HEADER;
    unsigned int child;
    unsigned int labelIndex;
    unsigned int dataIndex;
    char key[0];
} TrieDataNode;

typedef struct {
    atomic_uint_least32_t serial;
    atomic_uint_least32_t dataLength;
    char data[0];
} DataEntry;

typedef struct {
    atomic_uint_least32_t serial;
    u_int32_t currOffset;
    u_int32_t firstNode;
    u_int32_t dataSize;
    u_int32_t reserved_[28];
    char data[0];
} WorkArea;

struct WorkSpace_;
typedef u_int32_t (*AllocTrieNodePtr)(struct WorkSpace_ *workSpace, const char *key, u_int32_t keyLen);
typedef int (*CompareTrieNodePtr)(const TrieNode *node, const char *key2, u_int32_t key2Len);

typedef struct WorkSpace_ {
    char fileName[FILENAME_LEN_MAX + 1];
    WorkArea *area;
    TrieNode *rootNode;
    AllocTrieNodePtr allocTrieNode;
    CompareTrieNodePtr compareTrieNode;
} WorkSpace;

u_int32_t GetWorkSpaceSerial(const WorkSpace *workSpace);
int CompareTrieNode(const TrieNode *node, const char *key, u_int32_t keyLen);
u_int32_t AllocateTrieNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen);
int CompareTrieDataNode(const TrieNode *node, const char *key, u_int32_t keyLen);
u_int32_t AllocateTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen);

u_int32_t GetTrieNodeOffset(const WorkSpace *workSpace, const TrieNode *current);
TrieNode *GetTrieNode(const WorkSpace *workSpace, const unsigned int *index);
void SaveIndex(unsigned int *index, u_int32_t offset);
TrieDataNode *AddTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen);
TrieDataNode *AddToSubTrie(WorkSpace *workSpace, TrieDataNode *dataNode, const char *key, u_int32_t keyLen);
TrieDataNode *FindSubTrie(WorkSpace *workSpace, const TrieDataNode *dataNode, const char *key, u_int32_t keyLen);
TrieDataNode *FindTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen, int matchPrefix);

TrieNode *AddTrieNode(WorkSpace *workSpace, TrieNode *root, const char *key, u_int32_t keyLen);
TrieNode *FindTrieNode(WorkSpace *workSpace, const TrieNode *tree, const char *key, u_int32_t keyLen);

int InitWorkSpace_(WorkSpace *workSpace, int mode, int prot, u_int32_t spaceSize, int readOnly);
int InitPersistWorkSpace(const char *fileName, WorkSpace *workSpace);
int InitWorkSpace(const char *fileName, WorkSpace *workSpace, int onlyRead);
void CloseWorkSpace(WorkSpace *workSpace);

typedef int (*TraversalTrieNodePtr)(WorkSpace *workSpace, const TrieNode *node, void *cookie);
int TraversalTrieNode(WorkSpace *workSpace, const TrieNode *root, TraversalTrieNodePtr walkFunc, void *cookie);
int TraversalTrieDataNode(WorkSpace *workSpace,
    const TrieDataNode *current, TraversalTrieNodePtr walkFunc, void *cookie);

u_int32_t AddData(WorkSpace *workSpace, const char *key, u_int32_t keyLen, const char *value, u_int32_t valueLen);
int UpdateDataValue(DataEntry *entry, const char *value);
int GetDataName(const DataEntry *entry, char *name, u_int32_t len);
int GetDataValue(const DataEntry *entry, char *value, u_int32_t len);
u_int32_t GetDataSerial(const DataEntry *entry);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // BASE_STARTUP_PARAM_TRIE_H
