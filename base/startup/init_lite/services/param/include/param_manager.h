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

#ifndef BASE_STARTUP_PARAM_MANAGER_H
#define BASE_STARTUP_PARAM_MANAGER_H
#include <stdio.h>
#include <string.h>

#include "init_log.h"
#include "sys_param.h"
#include "param_trie.h"
#include "securec.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    PARAM_CODE_INVALID_PARAM = 100,
    PARAM_CODE_INVALID_NAME,
    PARAM_CODE_INVALID_VALUE,
    PARAM_CODE_REACHED_MAX,
    PARAM_CODE_PERMISSION_DENIED,
    PARAM_CODE_READ_ONLY_PROPERTY,
    PARAM_CODE_NOT_SUPPORT,
    PARAM_CODE_ERROR_MAP_FILE,
    PARAM_CODE_NOT_FOUND_PROP,
    PARAM_CODE_NOT_INIT
} PARAM_CODE;

#define LABEL_STRING_LEN 128

#ifdef STARTUP_LOCAL
#define PIPE_NAME "/tmp/paramservice"
#define PARAM_STORAGE_PATH "/media/sf_ubuntu/test/__parameters__/param_storage"
#define PARAM_PERSIST_PATH "/media/sf_ubuntu/test/param/persist_parameters"
#define PARAM_INFO_PATH "/media/sf_ubuntu/test/__parameters__/param_info"
#else
#define PIPE_NAME "/dev/unix/socket/paramservice"
#define PARAM_STORAGE_PATH "/dev/__parameters__/param_storage"
#define PARAM_PERSIST_PATH "/data/param/persist_parameters"
#define PARAM_INFO_PATH "/dev/__parameters__/param_info"
#endif

#define SUBSTR_INFO_NAME  0
#define SUBSTR_INFO_LABEL 1
#define SUBSTR_INFO_TYPE  2
#define SUBSTR_INFO_MAX   4

#define WORKSPACE_FLAGS_INIT    0x01
#define WORKSPACE_FLAGS_LOADED  0x02

#define PARAM_LOGI(fmt, ...) STARTUP_LOGI(LABEL, fmt, ##__VA_ARGS__)
#define PARAM_LOGE(fmt, ...) STARTUP_LOGE(LABEL, fmt, ##__VA_ARGS__)
#define PARAM_LOGD(fmt, ...) STARTUP_LOGD(LABEL, fmt, ##__VA_ARGS__)

#define PARAM_CHECK(retCode, exper, ...) \
    if (!(retCode)) {                     \
        PARAM_LOGE(__VA_ARGS__);         \
        exper;                            \
    }

#define futex(addr1, op, val, rel, addr2, val3) \
    syscall(SYS_futex, addr1, op, val, rel, addr2, val3)
#define futex_wait_always(addr1) \
    syscall(SYS_futex, addr1, FUTEX_WAIT, *(int*)(addr1), 0, 0, 0)
#define futex_wake_single(addr1) \
    syscall(SYS_futex, addr1, FUTEX_WAKE, 1, 0, 0, 0)

typedef struct UserCred {
    pid_t pid;
    uid_t uid;
    gid_t gid;
} UserCred;

typedef struct {
    char label[LABEL_STRING_LEN];
    UserCred cred;
} ParamSecurityLabel;

typedef struct ParamAuditData {
    const UserCred *cr;
    const char *name;
} ParamAuditData;

typedef struct {
    atomic_uint_least32_t flags;
    WorkSpace paramLabelSpace;
    WorkSpace paramSpace;
    ParamSecurityLabel label;
} ParamWorkSpace;

typedef struct {
    atomic_uint_least32_t flags;
    WorkSpace persistWorkSpace;
} ParamPersistWorkSpace;

typedef struct {
    char value[128];
} SubStringInfo;

int InitParamWorkSpace(ParamWorkSpace *workSpace, int onlyRead, const char *context);
void CloseParamWorkSpace(ParamWorkSpace *workSpace);

int ReadParamWithCheck(ParamWorkSpace *workSpace, const char *name, ParamHandle *handle);
int ReadParamValue(ParamWorkSpace *workSpace, ParamHandle handle, char *value, u_int32_t *len);
int ReadParamName(ParamWorkSpace *workSpace, ParamHandle handle, char *name, u_int32_t len);
u_int32_t ReadParamSerial(ParamWorkSpace *workSpace, ParamHandle handle);

int AddParam(WorkSpace *workSpace, const char *name, const char *value);
int WriteParam(WorkSpace *workSpace, const char *name, const char *value);
int WriteParamWithCheck(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, const char *value);
int WriteParamInfo(ParamWorkSpace *workSpace, SubStringInfo *info, int subStrNumber);

int CheckParamValue(WorkSpace *workSpace, const TrieDataNode *node, const char *name, const char *value);
int CheckParamName(const char *name, int paramInfo);
int CanReadParam(ParamWorkSpace *workSpace, u_int32_t labelIndex, const char *name);
int CanWriteParam(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const TrieDataNode *node, const char *name, const char *value);

int CheckMacPerms(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, u_int32_t labelIndex);
int CheckControlParamPerms(ParamWorkSpace *workSpace,
    const ParamSecurityLabel *srcLabel, const char *name, const char *value);

int GetSubStringInfo(const char *buff, u_int32_t buffLen, char delimiter, SubStringInfo *info, int subStrNumber);
int BuildParamContent(char *content, u_int32_t contentSize, const char *name, const char *value);
ParamWorkSpace *GetParamWorkSpace();

typedef void (*TraversalParamPtr)(ParamHandle handle, void* context);
typedef struct {
    TraversalParamPtr traversalParamPtr;
    void *context;
} ParamTraversalContext;
int TraversalParam(ParamWorkSpace *workSpace, TraversalParamPtr walkFunc, void *cookie);

const char *DetectParamChange(ParamWorkSpace *workSpace, ParamCache *cache,
    ParamEvaluatePtr evaluate, u_int32_t count, const char *parameters[][2]);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif