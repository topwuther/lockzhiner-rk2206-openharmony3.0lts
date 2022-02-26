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

#include "param_manager.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LABEL "Manager"

#ifdef PARAM_SUPPORT_SELINUX
static int SelinuxAuditCallback(void *data,
    __attribute__((unused))security_class_t class, char *msgBuf, size_t msgSize)
{
    ParamAuditData *auditData = (ParamAuditData*)(data);
    PARAM_CHECK(auditData != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(auditData->name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(auditData->cr != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    if (snprintf_s(msgBuf, msgSize, msgSize - 1, "param=%s pid=%d uid=%d gid=%d",
        auditData->name, auditData->cr->pid, auditData->cr->uid, auditData->cr->gid) == -1) {
        return PARAM_CODE_INVALID_PARAM;
    }
    return 0;
}
#endif

int InitParamWorkSpace(ParamWorkSpace *workSpace, int onlyRead, const char *context)
{
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    u_int32_t flags = atomic_load_explicit(&workSpace->flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) == WORKSPACE_FLAGS_INIT) {
        return 0;
    }

#ifdef PARAM_SUPPORT_SELINUX
    union selinux_callback cb;
    cb.func_audit = SelinuxAuditCallback;
    selinux_set_callback(SELINUX_CB_AUDIT, cb);
#endif

#ifdef PARAM_SUPPORT_SELINUX
    if (context && fsetxattr(fd, XATTR_NAME_SELINUX, context, strlen(context) + 1, 0) != 0) {
        PARAM_LOGI("fsetxattr context %s fail", context);
    }
#endif
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_NAME, "Invalid param");
    workSpace->paramSpace.compareTrieNode = CompareTrieDataNode;
    workSpace->paramSpace.allocTrieNode = AllocateTrieDataNode;
    int ret = InitWorkSpace(PARAM_STORAGE_PATH, &workSpace->paramSpace, onlyRead);
    PARAM_CHECK(ret == 0, return ret, "InitWorkSpace failed.");

    workSpace->paramLabelSpace.compareTrieNode = CompareTrieNode; // 必须先设置
    workSpace->paramLabelSpace.allocTrieNode = AllocateTrieNode;
    ret = InitWorkSpace(PARAM_INFO_PATH, &workSpace->paramLabelSpace, onlyRead);
    PARAM_CHECK(ret == 0, return ret, "InitWorkSpace failed.");

    atomic_store_explicit(&workSpace->flags, WORKSPACE_FLAGS_INIT, memory_order_release);
    return ret;
}

void CloseParamWorkSpace(ParamWorkSpace *workSpace)
{
    PARAM_CHECK(workSpace != NULL, return, "Invalid work space");
    CloseWorkSpace(&workSpace->paramSpace);
    CloseWorkSpace(&workSpace->paramLabelSpace);
    atomic_store_explicit(&workSpace->flags, 0, memory_order_release);
}

int WriteParamInfo(ParamWorkSpace *workSpace, SubStringInfo *info, int subStrNumber)
{
    PARAM_CHECK(workSpace != NULL && info != NULL && subStrNumber > SUBSTR_INFO_NAME,
        return PARAM_CODE_INVALID_PARAM, "Failed to check param");
    const char *name = info[SUBSTR_INFO_NAME].value;
    char *label = NULL;
    char *type = NULL;
    if (subStrNumber >= SUBSTR_INFO_LABEL) {
        label = info[SUBSTR_INFO_LABEL].value;
    } else {
        label = "u:object_r:default_prop:s0";
    }
    if (subStrNumber >= SUBSTR_INFO_TYPE) {
        type = info[SUBSTR_INFO_TYPE].value;
    } else {
        type = "string";
    }

    int ret = CheckParamName(name, 1);
    PARAM_CHECK(ret == 0, return ret, "Illegal param name %s", name);

    // 先保存标签值
    TrieNode *node = AddTrieNode(&workSpace->paramLabelSpace,
        workSpace->paramLabelSpace.rootNode, label, strlen(label));
    PARAM_CHECK(node != NULL, return PARAM_CODE_REACHED_MAX, "Failed to add label");
    u_int32_t offset = GetTrieNodeOffset(&workSpace->paramLabelSpace, node);

    TrieDataNode *dataNode = AddTrieDataNode(&workSpace->paramSpace, name, strlen(name));
    PARAM_CHECK(dataNode != NULL, return PARAM_CODE_REACHED_MAX, "Failed to add node %s", name);
    TrieNode *entry = (TrieNode *)GetTrieNode(&workSpace->paramLabelSpace, &dataNode->labelIndex);
    if (entry != 0) { // 已经存在label
        PARAM_LOGE("Has been set label %s old label %s new label: %s", name, entry->key, label);
    }
    SaveIndex(&dataNode->labelIndex, offset);
    return 0;
}

int AddParam(WorkSpace *workSpace, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL && workSpace->area != NULL && name != NULL && value != NULL,
        return PARAM_CODE_INVALID_PARAM, "Failed to check param");

    TrieDataNode *node = AddTrieDataNode(workSpace, name, strlen(name));
    PARAM_CHECK(node != NULL, return PARAM_CODE_REACHED_MAX, "Failed to add node");
    DataEntry *entry = (DataEntry *)GetTrieNode(workSpace, &node->dataIndex);
    if (entry == NULL) {
        u_int32_t offset = AddData(workSpace, name, strlen(name), value, strlen(value));
        PARAM_CHECK(offset > 0, return PARAM_CODE_REACHED_MAX, "Failed to allocate name %s", name);
        SaveIndex(&node->dataIndex, offset);
    }
    atomic_store_explicit(&workSpace->area->serial,
        atomic_load_explicit(&workSpace->area->serial, memory_order_relaxed) + 1, memory_order_release);
    futex_wake(&workSpace->area->serial, INT_MAX);
    return 0;
}

int UpdateParam(WorkSpace *workSpace, u_int32_t *dataIndex, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL && workSpace->area != NULL && name != NULL && value != NULL,
        return PARAM_CODE_INVALID_PARAM, "Failed to check param");

    DataEntry *entry = (DataEntry *)GetTrieNode(workSpace, dataIndex);
    PARAM_CHECK(entry != NULL, return PARAM_CODE_REACHED_MAX,
        "Failed to update param value %s %u", name, *dataIndex);
    u_int32_t keyLen = DATA_ENTRY_KEY_LEN(entry);
    PARAM_CHECK(keyLen == strlen(name), return PARAM_CODE_INVALID_NAME, "Failed to check name len %s", name);

    u_int32_t serial = atomic_load_explicit(&entry->serial, memory_order_relaxed);
    serial |= 1;
    atomic_store_explicit(&entry->serial, serial | 1, memory_order_release);
    atomic_thread_fence(memory_order_release);
    int ret = UpdateDataValue(entry, value);
    if (ret != 0) {
        PARAM_LOGE("Failed to update param value %s %s", name, value);
    }
    atomic_store_explicit(&entry->serial, serial + 1, memory_order_release);
    futex_wake(&entry->serial, INT_MAX);
    atomic_store_explicit(&workSpace->area->serial,
        atomic_load_explicit(&workSpace->area->serial, memory_order_relaxed) + 1, memory_order_release);
    futex_wake(&workSpace->area->serial, INT_MAX);
    return ret;
}

DataEntry *FindParam(WorkSpace *workSpace, const char *name)
{
    PARAM_CHECK(workSpace != NULL, return NULL, "Failed to check param");
    PARAM_CHECK(name != NULL, return NULL, "Invalid param size");

    TrieDataNode *node = FindTrieDataNode(workSpace, name, strlen(name), 0);
    if (node != NULL) {
        return (DataEntry *)GetTrieNode(workSpace, &node->dataIndex);
    }
    return NULL;
}

int WriteParamWithCheck(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL && srcLabel != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(value != NULL && name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    u_int32_t flags = atomic_load_explicit(&workSpace->flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) != WORKSPACE_FLAGS_INIT) {
        return PARAM_CODE_NOT_INIT;
    }

    int ret = CheckParamName(name, 0);
    PARAM_CHECK(ret == 0, return ret, "Illegal param name %s", name);
    TrieDataNode *info = FindTrieDataNode(&workSpace->paramSpace, name, strlen(name), 1);
    ret = CanWriteParam(workSpace, srcLabel, info, name, value);
    PARAM_CHECK(ret == 0, return ret, "Permission to write param %s", name);
    return WriteParam(&workSpace->paramSpace, name, value);
}

int WriteParam(WorkSpace *workSpace, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(value != NULL && name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");

    TrieDataNode *node = FindTrieDataNode(workSpace, name, strlen(name), 0);
    int ret = CheckParamValue(workSpace, node, name, value);
    PARAM_CHECK(ret == 0, return ret, "Invalid value %s %s", name, value);
    if (node != NULL && node->dataIndex != 0) {
        return UpdateParam(workSpace, &node->dataIndex, name, value);
    }
    return AddParam(workSpace, name, value);
}

int ReadParamWithCheck(ParamWorkSpace *workSpace, const char *name, ParamHandle *handle)
{
    PARAM_CHECK(handle != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(workSpace != NULL && name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    u_int32_t flags = atomic_load_explicit(&workSpace->flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) != WORKSPACE_FLAGS_INIT) {
        return PARAM_CODE_NOT_INIT;
    }

    *handle = 0;
    // 取最长匹配
    TrieDataNode *paramInfo = FindTrieDataNode(&workSpace->paramSpace, name, strlen(name), 1);
    int ret = CanReadParam(workSpace, (paramInfo == NULL) ? 0 : paramInfo->labelIndex, name);
    PARAM_CHECK(ret == 0, return ret, "Permission to read param %s", name);

    // 查找结点
    TrieDataNode *node = FindTrieDataNode(&workSpace->paramSpace, name, strlen(name), 0);
    if (node != NULL && node->dataIndex != 0) {
        *handle = node->dataIndex;
        return 0;
    }
    return PARAM_CODE_NOT_FOUND_PROP;
}

int ReadParamValue(ParamWorkSpace *workSpace, ParamHandle handle, char *value, u_int32_t *len)
{
    PARAM_CHECK(workSpace != NULL && len != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    u_int32_t flags = atomic_load_explicit(&workSpace->flags, memory_order_relaxed);
    if ((flags & WORKSPACE_FLAGS_INIT) != WORKSPACE_FLAGS_INIT) {
        return PARAM_CODE_NOT_INIT;
    }
    DataEntry *entry = (DataEntry *)GetTrieNode(&workSpace->paramSpace, &handle);
    if (entry == NULL) {
        return -1;
    }

    if (value == NULL) {
        *len = DATA_ENTRY_DATA_LEN(entry) + 1;
        return 0;
    }

    while (1) {
        u_int32_t serial = GetDataSerial(entry);
        int ret = GetDataValue(entry, value, *len);
        PARAM_CHECK(ret == 0, return ret, "Failed to get value");
        atomic_thread_fence(memory_order_acquire);
        if (serial == atomic_load_explicit(&(entry->serial), memory_order_relaxed)) {
            return 0;
        }
    }
    return 0;
}

int ReadParamName(ParamWorkSpace *workSpace, ParamHandle handle, char *name, u_int32_t len)
{
    PARAM_CHECK(workSpace != NULL && name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    DataEntry *entry = (DataEntry *)GetTrieNode(&workSpace->paramSpace, &handle);
    if (entry == NULL) {
        return -1;
    }
    return GetDataName(entry, name, len);
}

u_int32_t ReadParamSerial(ParamWorkSpace *workSpace, ParamHandle handle)
{
    PARAM_CHECK(workSpace != NULL, return 0, "Invalid param");
    DataEntry *entry = (DataEntry *)GetTrieNode(&workSpace->paramSpace, &handle);
    if (entry == NULL) {
        return 0;
    }
    return GetDataSerial(entry);
}

int CheckControlParamPerms(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL && srcLabel != NULL && name != NULL && value != NULL,
        return PARAM_CODE_INVALID_PARAM, "Invalid param");
    int n = 0;
    const char *ctrlName[] = {
        "ctl.start",  "ctl.stop", "ctl.restart"
    };
    size_t size1 = strlen("ctl.") +  strlen(value);
    size_t size2 = strlen(name) +  strlen(value) + 1;
    size_t size = ((size1 > size2) ? size1 : size2) + 1;
    if (size > PARAM_VALUE_LEN_MAX) {
        return -1;
    }
    char *legacyName = (char*)malloc(size);
    PARAM_CHECK(legacyName != NULL, return PARAM_CODE_INVALID_PARAM, "Failed to alloc memory");

    // We check the legacy method first but these parameters are dontaudit, so we only log an audit
    // if the newer method fails as well.  We only do this with the legacy ctl. parameters.
    for (size_t i = 0; i < sizeof(ctrlName) / sizeof(char*); i++) {
        if (strcmp(name, ctrlName[i]) == 0) {
            // The legacy permissions model is that ctl. parameters have their name ctl.<action> and
            // their value is the name of the service to apply that action to.  Permissions for these
            // actions are based on the service, so we must create a fake name of ctl.<service> to
            // check permissions.
            int len = snprintf_s(legacyName, size, strlen("ctl.") + strlen(value) + 1, "ctl.%s", value);
            PARAM_CHECK(len > 0, free(legacyName);
                return PARAM_CODE_INVALID_PARAM, "Failed to snprintf value");
            legacyName[n] = '\0';

            TrieDataNode *node = FindTrieDataNode(&workSpace->paramSpace, legacyName, strlen(legacyName), 1);
            if (CheckMacPerms(workSpace, srcLabel, legacyName, (node == NULL) ? 0 : node->labelIndex) == 0) {
                free(legacyName);
                return 0;
            }
            break;
        }
    }
    int ret = snprintf_s(legacyName, size, size - 1, "%s$%s", name, value);
    PARAM_CHECK(ret > 0, free(legacyName);
        return PARAM_CODE_INVALID_PARAM, "Failed to snprintf value");

    TrieDataNode *node = FindTrieDataNode(&workSpace->paramSpace, name, strlen(name), 1);
    ret = CheckMacPerms(workSpace, srcLabel, name, (node == NULL) ? 0 : node->labelIndex);
    free(legacyName);
    return ret;
}

int CheckParamName(const char *name, int info)
{
    if (name == NULL) {
        return PARAM_CODE_INVALID_NAME;
    }
    size_t nameLen = strlen(name);
    if (nameLen >= PARAM_NAME_LEN_MAX) {
        return PARAM_CODE_INVALID_NAME;
    }

    if (nameLen < 1 || name[0] == '.' || (!info && name[nameLen - 1] == '.')) {
        PARAM_LOGE("CheckParamName %s %d", name, info);
        return PARAM_CODE_INVALID_NAME;
    }

    /* Only allow alphanumeric, plus '.', '-', '@', ':', or '_' */
    /* Don't allow ".." to appear in a param name */
    for (size_t i = 0; i < nameLen; i++) {
        if (name[i] == '.') {
            if (name[i - 1] == '.') {
                return PARAM_CODE_INVALID_NAME;
            }
            continue;
        }
        if (name[i] == '_' || name[i] == '-' || name[i] == '@' || name[i] == ':') {
            continue;
        }
        if (isalnum(name[i])) {
            continue;
        }
        return PARAM_CODE_INVALID_NAME;
    }
    return 0;
}

int CheckParamValue(WorkSpace *workSpace, const TrieDataNode *node, const char *name, const char *value)
{
    if (name == NULL || value == NULL) {
        return PARAM_CODE_INVALID_VALUE;
    }
    if (strncmp((name), "ro.", strlen("ro.")) == 0) {
        if (node != NULL && node->dataIndex != 0) {
            PARAM_LOGE("Read-only param was already set %s", name);
            return PARAM_CODE_READ_ONLY_PROPERTY;
        }
    } else {
        // 限制非read only的参数，防止参数值修改后，原空间不能保存
        PARAM_CHECK(strlen(value) < PARAM_VALUE_LEN_MAX,
            return PARAM_CODE_INVALID_VALUE, "Illegal param value");
    }
    return 0;
}

int CheckMacPerms(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, u_int32_t labelIndex)
{
#ifdef PARAM_SUPPORT_SELINUX
    ParamAuditData auditData;
    auditData.name = name;
    auditData.cr = &srcLabel->cred;

    int ret;
    TrieNode *node = (TrieNode *)GetTrieNode(&workSpace->paramLabelSpace, &labelIndex);
    if (node != 0) { // 已经存在label
        ret = selinux_check_access(srcLabel, node->key, "param_service", "set", &auditData);
    } else {
        ret = selinux_check_access(srcLabel, "u:object_r:default_prop:s0", "param_service", "set", &auditData);
    }
    return ((ret == 0) ? 0 : PARAM_CODE_PERMISSION_DENIED);
#else
    return 0;
#endif
}

int CanWriteParam(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const TrieDataNode *node, const char *name, const char *value)
{
    PARAM_CHECK(workSpace != NULL && name != NULL && value != NULL && srcLabel != NULL,
        return PARAM_CODE_INVALID_PARAM, "Invalid param");

    if (strncmp(name, "ctl.", strlen("ctl.")) == 0) {
        return CheckControlParamPerms(workSpace, srcLabel, name, value);
    }

    int ret = CheckMacPerms(workSpace, srcLabel, name, (node == NULL) ? 0 : node->labelIndex);
    PARAM_CHECK(ret == 0, return ret, "SELinux permission check failed");
    return 0;
}

int CanReadParam(ParamWorkSpace *workSpace, u_int32_t labelIndex, const char *name)
{
    PARAM_CHECK(workSpace != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
    PARAM_CHECK(name != NULL, return PARAM_CODE_INVALID_PARAM, "Invalid param");
#ifdef PARAM_SUPPORT_SELINUX
    ParamAuditData auditData;
    auditData.name = name;
    UserCred cr = {
        .pid = 0,
        .uid = 0,
        .gid = 0
    };
    auditData.cr = &cr;

    int ret;
    TrieNode *node = (TrieNode *)GetTrieNode(&workSpace->paramLabelSpace, &labelIndex);
    if (node != 0) { // 已经存在label
        ret = selinux_check_access(&workSpace->context, node->key, "param_service", "read", &auditData);
    } else {
        ret = selinux_check_access(&workSpace->context, "selinux_check_access", "file", "read", &auditData);
    }
    return (ret == 0) ? 0 : PARAM_CODE_PERMISSION_DENIED;
#else
    return 0;
#endif
}

int GetSubStringInfo(const char *buff, u_int32_t buffLen, char delimiter, SubStringInfo *info, int subStrNumber)
{
    if (buff == NULL || info == NULL) {
        return -1;
    }
    size_t i = 0;
    // 去掉开始的空格
    for (; i < strlen(buff); i++) {
        if (!isspace(buff[i])) {
            break;
        }
    }
    // 过滤掉注释
    if (buff[i] == '#') {
        return -1;
    }
    // 分割字符串
    int spaceIsValid = 0;
    int curr = 0;
    int valueCurr = 0;
    for (; i < buffLen; i++) {
        if (buff[i] == '\n' || buff[i] == '\r') {
            break;
        }
        if (buff[i] == delimiter && valueCurr != 0) {
            info[curr].value[valueCurr] = '\0';
            valueCurr = 0;
            curr++;
            spaceIsValid = 1;
        } else {
            if (!spaceIsValid && isspace(buff[i])) {
                continue;
            }
            if ((valueCurr + 1) >= (int)sizeof(info[curr].value)) {
                continue;
            }
            info[curr].value[valueCurr++] = buff[i];
        }
        if (curr >= subStrNumber) {
            break;
        }
    }
    if (valueCurr > 0) {
        info[curr].value[valueCurr] = '\0';
        valueCurr = 0;
        curr++;
    }
    return curr;
}

int BuildParamContent(char *content, u_int32_t contentSize, const char *name, const char *value)
{
    PARAM_CHECK(name != NULL && value != NULL && content != NULL, return -1, "Invalid param");
    u_int32_t nameLen = (u_int32_t)strlen(name);
    u_int32_t valueLen = (u_int32_t)strlen(value);
    PARAM_CHECK(contentSize >= (nameLen + valueLen + 2), return -1, "Invalid content size %u", contentSize);

    int offset = 0;
    int ret = memcpy_s(content + offset, contentSize - offset, name, nameLen);
    PARAM_CHECK(ret == 0, return -1, "Failed to copy porperty");
    offset += nameLen;
    ret = memcpy_s(content + offset, contentSize - offset, "=", 1);
    PARAM_CHECK(ret == 0, return -1, "Failed to copy porperty");
    offset += 1;
    ret = memcpy_s(content + offset, contentSize - offset, value, valueLen);
    offset += valueLen;
    content[offset] = '\0';
    PARAM_CHECK(ret == 0, return -1, "Failed to copy porperty");
    offset += 1;
    return offset;
}

int ProcessParamTraversal(WorkSpace *workSpace, TrieNode *node, void *cookie)
{
    ParamTraversalContext *context = (ParamTraversalContext *)cookie;
    if (context == NULL || context->traversalParamPtr == NULL) {
        return 0;
    }

    TrieDataNode *current = (TrieDataNode *)node;
    if (current == NULL) {
        return 0;
    }
    if (current->dataIndex == 0) {
        return 0;
    }
    context->traversalParamPtr(current->dataIndex, context->context);
    return 0;
}

int TraversalParam(ParamWorkSpace *workSpace, TraversalParamPtr walkFunc, void *cookie)
{
    PARAM_CHECK(workSpace != NULL, return -1, "Invalid param");
    ParamTraversalContext context = {
        walkFunc, cookie
    };
    return TraversalTrieDataNode(&workSpace->paramSpace,
        (TrieDataNode *)workSpace->paramSpace.rootNode, ProcessParamTraversal, &context);
}
