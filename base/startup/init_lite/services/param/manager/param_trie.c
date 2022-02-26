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

#include "param_trie.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "init_utils.h"
#include "sys_param.h"
#include "param_manager.h"

#define LABEL "Manager"

int InitWorkSpace(const char *fileName, WorkSpace *workSpace, int onlyRead)
{
    PARAM_CHECK(fileName != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    if (workSpace->area != NULL) {
        return 0;
    }

    int ret = memcpy_s(workSpace->fileName, FILENAME_LEN_MAX, fileName, strlen(fileName));
    PARAM_CHECK(ret == 0, return PARAM_CODE_INVALID_NAME, "Copy file %s fail ", fileName);
    int openMode = 0;
    int prot = PROT_READ;
    if (onlyRead) {
        openMode = O_RDONLY;
    } else {
        openMode = O_CREAT | O_RDWR | O_TRUNC;
        prot = PROT_READ | PROT_WRITE;
    }
    ret = InitWorkSpace_(workSpace, openMode, prot, PARAM_WORKSPACE_MAX, onlyRead);
    PARAM_CHECK(ret == 0, return ret, "Failed to init workspace  %s", workSpace->fileName);
    return ret;
}

int InitPersistWorkSpace(const char *fileName, WorkSpace *workSpace)
{
    PARAM_CHECK(fileName != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    if (workSpace->area != NULL) {
        return 0;
    }

    int ret = memcpy_s(workSpace->fileName, FILENAME_LEN_MAX, fileName, strlen(fileName));
    PARAM_CHECK(ret == 0, return PARAM_CODE_INVALID_NAME, "Copy file %s fail ", fileName);

    int flag = (access(fileName, F_OK) == 0) ? 1 : 0;
    int openMode = (flag == 0) ? (O_CREAT | O_RDWR | O_TRUNC) : O_RDWR;
    int prot = PROT_READ | PROT_WRITE;
    ret = InitWorkSpace_(workSpace, openMode, prot, PARAM_WORKSPACE_MAX, flag);
    PARAM_CHECK(ret == 0, return ret, "Failed to init workspace  %s", workSpace->fileName);
    return ret;
}

int InitWorkSpace_(WorkSpace *workSpace, int mode, int prot, u_int32_t spaceSize, int readOnly)
{
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid fileName");
    PARAM_CHECK(workSpace->allocTrieNode != NULL,
        return PARAM_CODE_INVALID_PARAM, "Invalid param %s", workSpace->fileName);
    PARAM_CHECK(workSpace->compareTrieNode != NULL,
        return PARAM_CODE_INVALID_PARAM, "Invalid param %s", workSpace->fileName);
    PARAM_LOGD("InitWorkSpace %s ", workSpace->fileName);
    CheckAndCreateDir(workSpace->fileName);
    int fd = open(workSpace->fileName, mode, S_IRWXU | S_IRWXG | S_IRWXO);
    PARAM_CHECK(fd >= 0, return PARAM_CODE_INVALID_NAME,
        "Open file %s fail error %d", workSpace->fileName, errno);

    if (!readOnly) {
        lseek(fd, spaceSize - 1, SEEK_SET);
        write(fd, "", 1);
    }
    void *areaAddr = (void *)mmap(NULL, spaceSize, prot, MAP_SHARED, fd, 0);
    PARAM_CHECK(areaAddr != MAP_FAILED, close(fd); return PARAM_CODE_ERROR_MAP_FILE,
        "Failed to map memory error %d", errno);
    close(fd);

    if (!readOnly) {
        workSpace->area = (WorkArea*)areaAddr;
        atomic_init(&workSpace->area->serial, 0);
        workSpace->area->dataSize = spaceSize;
        workSpace->area->currOffset = sizeof(WorkArea);
        // 创建一个key为#的节点
        u_int32_t offset = workSpace->allocTrieNode(workSpace, "#", 1);
        workSpace->area->firstNode = offset;
        workSpace->rootNode = GetTrieNode(workSpace, &offset);
    } else {
        workSpace->area = (WorkArea*)areaAddr;
        workSpace->rootNode = GetTrieNode(workSpace, &workSpace->area->firstNode);
    }
    PARAM_LOGD("InitWorkSpace success, readOnly %d currOffset %u firstNode %u dataSize %u",
        readOnly, workSpace->area->currOffset, workSpace->area->firstNode, workSpace->area->dataSize);
    return 0;
}

void CloseWorkSpace(WorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL && workSpace->area != NULL, return, "The workspace is null");
    munmap((char *)workSpace->area, workSpace->area->dataSize);
    workSpace->area = NULL;
}

u_int32_t GetWorkSpaceSerial(const WorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL && workSpace->area != NULL, return 0, "The workspace is null");
    return (u_int32_t)workSpace->area->serial;
}

u_int32_t AllocateTrieNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen)
{
    if (workSpace == NULL || workSpace->area == NULL || key == NULL) {
        return 0;
    }
    u_int32_t len = keyLen + sizeof(TrieNode) + 1;
    len = (len + 0x03) & (~0x03);
    PARAM_CHECK((workSpace->area->currOffset + len) < workSpace->area->dataSize, return 0,
        "Failed to allocate currOffset %d, dataSize %d", workSpace->area->currOffset, workSpace->area->dataSize);
    TrieNode *node = (TrieNode*)(workSpace->area->data + workSpace->area->currOffset + len);

    atomic_init(&node->serial, ATOMIC_VAR_INIT(keyLen << TRIE_SERIAL_KEY_LEN_OFFSET));
    int ret = memcpy_s(node->key, keyLen + 1, key, keyLen);
    PARAM_CHECK(ret == 0, return 0, "Failed to copy key");
    node->key[keyLen] = '\0';
    node->left = 0;
    node->right = 0;
    u_int32_t offset = workSpace->area->currOffset;
    workSpace->area->currOffset += len;
    return offset;
}

u_int32_t AllocateTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen)
{
    if (workSpace == NULL || workSpace->area == NULL || key == NULL) {
        return 0;
    }
    u_int32_t len = keyLen + sizeof(TrieDataNode) + 1;
    len = (len + 0x03) & (~0x03);
    PARAM_CHECK((workSpace->area->currOffset + len) < workSpace->area->dataSize, return 0,
        "Failed to allocate currOffset %d, dataSize %d", workSpace->area->currOffset, workSpace->area->dataSize);
    TrieDataNode *node = (TrieDataNode*)(workSpace->area->data + workSpace->area->currOffset);

    atomic_init(&node->serial, ATOMIC_VAR_INIT(keyLen << TRIE_SERIAL_KEY_LEN_OFFSET));
    int ret = memcpy_s(node->key, keyLen + 1, key, keyLen);
    PARAM_CHECK(ret == 0, return 0, "Failed to copy key");
    node->key[keyLen] = '\0';
    node->left = 0;
    node->right = 0;
    node->child = 0;
    node->dataIndex = 0;
    node->labelIndex = 0;
    u_int32_t offset = workSpace->area->currOffset;
    workSpace->area->currOffset += len;
    return offset;
}

TrieNode *GetTrieNode(const WorkSpace *workSpace, const unsigned int *index)
{
    if (index == NULL || workSpace == NULL || workSpace->area == NULL) {
        return NULL;
    }
    u_int32_t offset = *index;
    if (offset == 0 || offset > workSpace->area->dataSize) {
        return NULL;
    }
    return (TrieNode*)(workSpace->area->data + offset);
}

static u_int32_t GetTrieKeyLen(const TrieNode *current)
{
    PARAM_CHECK(current != NULL, return 0, "Invalid param");
    return (current)->serial >> TRIE_SERIAL_KEY_LEN_OFFSET;
}

u_int32_t GetTrieNodeOffset(const WorkSpace *workSpace, const TrieNode *current)
{
    PARAM_CHECK(current != NULL, return 0, "Invalid param");
    return (((char *)current) - workSpace->area->data);
}

void SaveIndex(unsigned int *index, u_int32_t offset)
{
    PARAM_CHECK(index != NULL, return, "Invalid param");
    *index = offset;
}

int CompareTrieDataNode(const TrieNode *node, const char *key, u_int32_t keyLen)
{
    if (node == NULL || key == NULL) {
        return -1;
    }
    TrieDataNode *data = (TrieDataNode *)node;
    u_int32_t len = GetTrieKeyLen((TrieNode *)data);
    if (len > keyLen) {
        return -1;
    } else if (len < keyLen) {
        return 1;
    }
    return strncmp(data->key, key, keyLen);
}

int CompareTrieNode(const TrieNode *node, const char *key, u_int32_t keyLen)
{
    if (node == NULL || key == NULL) {
        return -1;
    }
    u_int32_t len = GetTrieKeyLen(node);
    if (len > keyLen) {
        return -1;
    } else if (len < keyLen) {
        return 1;
    }
    return strncmp(node->key, key, keyLen);
}

static void GetNextKey(const char **remainingKey, char **subKey, u_int32_t *subKeyLen)
{
    *subKey = strchr(*remainingKey, '.');
    if (*subKey != NULL) {
        *subKeyLen = *subKey - *remainingKey;
    } else {
        *subKeyLen = strlen(*remainingKey);
    }
}

TrieDataNode *AddTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen)
{
    PARAM_CHECK(key != NULL, return NULL, "Invalid param");
    PARAM_CHECK(workSpace->allocTrieNode != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->compareTrieNode != NULL, return NULL, "Invalid param %s", key);

    const char *remainingKey = key;
    TrieDataNode *current = (TrieDataNode *)workSpace->rootNode;
    PARAM_CHECK(current != NULL, return NULL, "Invalid current param %s", key);
    while (1) {
        u_int32_t subKeyLen = 0;
        char *subKey = NULL;
        GetNextKey(&remainingKey, &subKey, &subKeyLen);
        if (!subKeyLen) {
            return NULL;
        }
        u_int32_t offset = ((subKey == NULL) ? strlen(key) : (subKey - key));

        if (current->child != 0) { // 如果child存在，则检查是否匹配
            TrieDataNode *next = (TrieDataNode*)GetTrieNode(workSpace, &current->child);
            if (next != NULL && workSpace->compareTrieNode((TrieNode*)next, remainingKey, subKeyLen) == 0) {
                current = next;
            } else { // 不匹配，需要建立子树
                current = (TrieDataNode*)AddToSubTrie(workSpace, current, key, offset);
            }
        } else {
            current = (TrieDataNode*)AddToSubTrie(workSpace, current, key, offset);
        }
        if (current == NULL) {
            return NULL;
        }
        if (subKey == NULL || strcmp(subKey, ".") == 0) {
            break;
        }
        remainingKey = subKey + 1;
    }
    return current;
}

TrieDataNode *AddToSubTrie(WorkSpace *workSpace, TrieDataNode *dataNode, const char *key, u_int32_t keyLen)
{
    PARAM_CHECK(workSpace != NULL && dataNode != NULL, return NULL,
        "Invalid param");
    TrieDataNode *root = NULL;
    int ret = workSpace->compareTrieNode((TrieNode *)dataNode, key, keyLen);
    if (ret <= 0) {
        root = (TrieDataNode *)GetTrieNode(workSpace, &dataNode->left);
        if (root == NULL) {
            u_int32_t offset = workSpace->allocTrieNode(workSpace, key, keyLen);
            PARAM_CHECK(offset != 0, return NULL, "Failed to allocate key %s", key);
            SaveIndex(&dataNode->left, offset);
            return (TrieDataNode *)GetTrieNode(workSpace, &dataNode->left);
        }
    } else {
        root = (TrieDataNode *)GetTrieNode(workSpace, &dataNode->right);
        if (root == NULL) {
            u_int32_t offset = workSpace->allocTrieNode(workSpace, key, keyLen);
            PARAM_CHECK(offset != 0, return NULL, "Failed to allocate key %s", key);
            SaveIndex(&dataNode->right, offset);
            return (TrieDataNode *)GetTrieNode(workSpace, &dataNode->right);
        }
    }
    return (TrieDataNode*)AddTrieNode(workSpace, (TrieNode*)root, key, keyLen);
}

TrieNode *AddTrieNode(WorkSpace *workSpace, TrieNode *root, const char *key, u_int32_t keyLen)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Invalid work space");
    PARAM_CHECK(workSpace->allocTrieNode != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->compareTrieNode != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(root != NULL, return NULL, "Invalid param %s", key);
    TrieNode *current = root;
    TrieNode *next = NULL;
    while (1) {
        if (current == NULL) {
            return NULL;
        }
        int ret = workSpace->compareTrieNode(current, key, keyLen);
        if (ret == 0) {
            return current;
        }
        if (ret < 0) {
            next = GetTrieNode(workSpace, &current->left);
            if (next == NULL) {
                u_int32_t offset = workSpace->allocTrieNode(workSpace, key, keyLen);
                PARAM_CHECK(offset != 0, return NULL, "Failed to allocate key %s", key);
                SaveIndex(&current->left, offset);
                return GetTrieNode(workSpace, &current->left);
            }
        } else {
            next = GetTrieNode(workSpace, &current->right);
            if (next == NULL) {
                u_int32_t offset = workSpace->allocTrieNode(workSpace, key, keyLen);
                PARAM_CHECK(offset != 0, return NULL, "Failed to allocate key %s", key);
                SaveIndex(&current->right, offset);
                return GetTrieNode(workSpace, &current->right);
            }
        }
        current = next;
    }
    return current;
}

TrieDataNode *FindTrieDataNode(WorkSpace *workSpace, const char *key, u_int32_t keyLen, int matchPrefix)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->allocTrieNode != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->compareTrieNode != NULL, return NULL, "Invalid param %s", key);

    const char *remainingKey = key;
    TrieDataNode *matchNode = (TrieDataNode *)workSpace->rootNode;
    TrieDataNode *current = (TrieDataNode *)workSpace->rootNode;
    PARAM_CHECK(current != NULL, return NULL, "Invalid current param %s", key);
    while (1) {
        u_int32_t subKeyLen = 0;
        char *subKey = NULL;
        GetNextKey(&remainingKey, &subKey, &subKeyLen);
        if (!subKeyLen) {
            return matchPrefix ? matchNode : NULL;
        }
        u_int32_t offset = ((subKey == NULL) ? strlen(key) : (subKey - key));

        if (current->child != 0) { // 如果child存在，则检查是否匹配
            TrieDataNode *next = (TrieDataNode*)GetTrieNode(workSpace, &current->child);
            if (next != NULL && workSpace->compareTrieNode((TrieNode*)next, remainingKey, subKeyLen) == 0) {
                current = next;
            } else { // 不匹配，搜索子树
                current = (TrieDataNode*)FindSubTrie(workSpace, current, key, offset);
            }
        } else {
            current = (TrieDataNode*)FindSubTrie(workSpace, current, key, offset);
        }
        if (current == NULL) {
            return matchPrefix ? matchNode : NULL;
        }
        matchNode = current;
        if (subKey == NULL || strcmp(subKey, ".") == 0) {
            break;
        }
        remainingKey = subKey + 1;
    }
    return matchPrefix ? matchNode : current;
}

TrieDataNode *FindSubTrie(WorkSpace *workSpace, const TrieDataNode *dataNode, const char *key, u_int32_t keyLen)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->compareTrieNode != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(dataNode != NULL, return NULL, "Invalid param %s", key);
    TrieDataNode *root = NULL;
    int ret = workSpace->compareTrieNode((TrieNode*)dataNode, key, keyLen);
    if (ret <= 0) {
        root = (TrieDataNode *)GetTrieNode(workSpace, &dataNode->left);
        if (root == NULL) {
            return NULL;
        }
    } else {
        root = (TrieDataNode *)GetTrieNode(workSpace, &dataNode->right);
        if (root == NULL) {
            return NULL;
        }
    }
    return (TrieDataNode*)FindTrieNode(workSpace, (TrieNode*)root, key, keyLen);
}

TrieNode *FindTrieNode(WorkSpace *workSpace, const TrieNode *root, const char *key, u_int32_t keyLen)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(root != NULL, return NULL, "Invalid param %s", key);
    PARAM_CHECK(workSpace->compareTrieNode != NULL, return NULL, "Invalid param %s", key);

    TrieNode *current = (TrieNode *)root;
    TrieNode *next = NULL;
    while (1) {
        if (current == NULL) {
            return NULL;
        }
        int ret = workSpace->compareTrieNode(current, key, keyLen);
        if (ret == 0) {
            return current;
        }
        if (ret < 0) {
            next = GetTrieNode(workSpace, &current->left);
        } else {
            next = GetTrieNode(workSpace, &current->right);
        }
        if (next == NULL) {
            return NULL;
        }
        current = next;
    }
    return current;
}

int TraversalTrieDataNode(WorkSpace *workSpace,
    const TrieDataNode *current, TraversalTrieNodePtr walkFunc, void *cookie)
{
    PARAM_CHECK(walkFunc != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    if (current == NULL) {
        return 0;
    }

    while (1) {
        TrieDataNode *child = NULL;
        // 显示子树
        TraversalTrieDataNode(workSpace, (TrieDataNode*)GetTrieNode(workSpace, &current->left), walkFunc, cookie);
        TraversalTrieDataNode(workSpace, (TrieDataNode*)GetTrieNode(workSpace, &current->right), walkFunc, cookie);
        walkFunc(workSpace, (TrieNode *)current, cookie);

        if (current->child != 0) { // 如果child存在，则检查是否匹配
            child = (TrieDataNode*)GetTrieNode(workSpace, &current->child);
        }
        if (child == NULL) {
            return 0;
        }
        current = child;
    }
    return 0;
}

int TraversalTrieNode(WorkSpace *workSpace, const TrieNode *root, TraversalTrieNodePtr walkFunc, void *cookie)
{
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(walkFunc != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    if (root == NULL) {
        return 0;
    }
    TraversalTrieNode(workSpace, GetTrieNode(workSpace, &root->left), walkFunc, cookie);
    TraversalTrieNode(workSpace, GetTrieNode(workSpace, &root->right), walkFunc, cookie);
    walkFunc(workSpace, (TrieNode *)root, cookie);
    return 0;
}

u_int32_t AddData(WorkSpace *workSpace, const char *key, u_int32_t keyLen, const char *value, u_int32_t valueLen)
{
    PARAM_CHECK(workSpace != NULL && workSpace->area != NULL, return 0, "Invalid param");
    PARAM_CHECK(key != NULL && value != NULL, return 0, "Invalid param");
    u_int32_t realLen = sizeof(DataEntry) + 1 + 1;
    if (valueLen > PARAM_VALUE_LEN_MAX) {
        realLen += keyLen + valueLen;
    } else {
        realLen += keyLen + PARAM_VALUE_LEN_MAX;
    }
    realLen = (realLen + 0x03) & (~0x03);
    PARAM_CHECK((workSpace->area->currOffset + realLen) < workSpace->area->dataSize, return 0,
        "Failed to allocate currOffset %d, dataSize %d", workSpace->area->currOffset, workSpace->area->dataSize);

    DataEntry *node = (DataEntry*)(workSpace->area->data + workSpace->area->currOffset);
    u_int32_t dataLength = keyLen << TRIE_SERIAL_KEY_LEN_OFFSET | valueLen << TRIE_SERIAL_DATA_LEN_OFFSET;
    atomic_init(&node->serial, ATOMIC_VAR_INIT(0));
    atomic_init(&node->dataLength, ATOMIC_VAR_INIT(dataLength));
    int ret = memcpy_s(node->data, keyLen + 1, key, keyLen);
    PARAM_CHECK(ret == 0, return 0, "Failed to copy key");
    ret = memcpy_s(node->data + keyLen + 1, valueLen + 1, value, valueLen);
    PARAM_CHECK(ret == 0, return 0, "Failed to copy key");
    node->data[keyLen] = '=';
    node->data[keyLen + 1 + valueLen] = '\0';
    u_int32_t offset = workSpace->area->currOffset;
    workSpace->area->currOffset += realLen;
    return offset;
}

int UpdateDataValue(DataEntry *entry, const char *value)
{
    PARAM_CHECK(entry != NULL && value != NULL, return PARAM_CODE_INVALID_PARAM, "Failed to check param");
    int ret = PARAM_CODE_INVALID_VALUE;
    u_int32_t keyLen = DATA_ENTRY_KEY_LEN(entry);
    u_int32_t valueLen = strlen(value);
    u_int32_t oldLen = DATA_ENTRY_DATA_LEN(entry);
    if (oldLen < PARAM_VALUE_LEN_MAX && valueLen < PARAM_VALUE_LEN_MAX) {
        PARAM_LOGD("Old value %s new value %s", entry->data + keyLen + 1, value);
        ret = memcpy_s(entry->data + keyLen + 1, PARAM_VALUE_LEN_MAX, value, valueLen + 1);
        PARAM_CHECK(ret == 0, return PARAM_CODE_INVALID_VALUE, "Failed to copy value");
        u_int32_t dataLength = keyLen << TRIE_SERIAL_KEY_LEN_OFFSET | valueLen << TRIE_SERIAL_DATA_LEN_OFFSET;
        atomic_store_explicit(&entry->dataLength, dataLength, memory_order_release);
    }
    return ret;
}

u_int32_t GetDataSerial(const DataEntry *entry)
{
    u_int32_t serial = atomic_load_explicit(&entry->serial, memory_order_acquire);
    while (DATA_ENTRY_DIRTY(serial)) {
        futex_wait(&entry->serial, serial, NULL);
        serial = atomic_load_explicit(&entry->serial, memory_order_acquire);
    }
    return serial;
}

int GetDataName(const DataEntry *entry, char *name, u_int32_t len)
{
    PARAM_CHECK(entry != NULL && name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    u_int32_t keyLen = DATA_ENTRY_KEY_LEN(entry);
    PARAM_CHECK(len > keyLen, return -1, "Invalid param size");
    int ret = memcpy_s(name, len, entry->data, keyLen);
    PARAM_CHECK(ret == 0, return PARAM_CODE_INVALID_PARAM, "Failed to copy name");
    name[keyLen] = '\0';
    return ret;
}

int GetDataValue(const DataEntry *entry, char *value, u_int32_t len)
{
    PARAM_CHECK(entry != NULL && value != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    u_int32_t keyLen = DATA_ENTRY_KEY_LEN(entry);
    u_int32_t valueLen = DATA_ENTRY_DATA_LEN(entry);
    PARAM_CHECK(len > valueLen, return PARAM_CODE_INVALID_PARAM, "Invalid value len %u %u", len, valueLen);
    int ret = memcpy_s(value, len, entry->data + keyLen + 1, valueLen);
    PARAM_CHECK(ret == 0, return PARAM_CODE_INVALID_PARAM, "Failed to copy value");
    value[valueLen] = '\0';
    return ret;
}
