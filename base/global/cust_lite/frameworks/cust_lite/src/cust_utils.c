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

#include "cust_utils.h"

#include <securec.h>
#include <unistd.h>

void FreeCfgFiles(CfgFiles *res)
{
    if (res == NULL) {
        return;
    }
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        if (res->paths[i] != NULL) {
            free(res->paths[i]);
            res->paths[i] = NULL;
        }
    }
    free(res);
}

void FreeCfgDirList(CfgDir *res)
{
    if (res == NULL) {
        return;
    }
    if (res->realPolicyValue != NULL) {
        free(res->realPolicyValue);
        res->realPolicyValue = NULL;
    }
    free(res);
}

static void GetCfgDirRealPolicyValue(CfgDir *res)
{
    if (res == NULL) {
        return;
    }
    res->realPolicyValue = strdup("/system/res:/user/data/ecota");
}

char *GetOneCfgFile(const char *pathSuffix, int type, char *buf, unsigned int bufLength)
{
    if (pathSuffix == NULL || buf == NULL || bufLength < MAX_PATH_LEN) {
        return NULL;
    }
    *buf = '\0';
    CfgDir *dirs = GetCfgDirListType(type);
    if (dirs == NULL) {
        return NULL;
    }
    for (size_t i = MAX_CFG_POLICY_DIRS_CNT; i > 0; i--) {
        if (dirs->paths[i - 1] == NULL) {
            continue;
        }
        if (snprintf_s(buf, bufLength, bufLength - 1, "%s/%s", dirs->paths[i - 1], pathSuffix) == -1) {
            *buf = '\0';
            continue;
        }
        if (access(buf, F_OK) == 0) {
            break;
        }
        *buf = '\0';
    }
    FreeCfgDirList(dirs);
    return (*buf != '\0') ? buf : NULL;
}

CfgFiles *GetCfgFiles(const char *pathSuffix, int type)
{
    if (pathSuffix == NULL) {
        return NULL;
    }
    char buf[MAX_PATH_LEN];
    CfgDir *dirs = GetCfgDirListType(type);
    if (dirs == NULL) {
        return NULL;
    }
    CfgFiles *files = (CfgFiles *)(malloc(sizeof(CfgFiles)));
    if (files == NULL) {
        FreeCfgDirList(dirs);
        return NULL;
    }
    (void)memset_s(files, sizeof(CfgFiles), 0, sizeof(CfgFiles));
    int j = 0;
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        if (dirs->paths[i] == NULL) {
            continue;
        }
        if (snprintf_s(buf, MAX_PATH_LEN, MAX_PATH_LEN - 1, "%s/%s", dirs->paths[i], pathSuffix) == -1) {
            continue;
        }
        if (access(buf, F_OK) == 0) {
            files->paths[j++] = strdup(buf);
        }
    }
    FreeCfgDirList(dirs);
    return files;
}

CfgDir *GetCfgDirListType(int type)
{
    CfgDir *res = (CfgDir *)(malloc(sizeof(CfgDir)));
    if (res == NULL) {
        return NULL;
    }
    (void)memset_s(res, sizeof(CfgDir), 0, sizeof(CfgDir));
    GetCfgDirRealPolicyValue(res);
    char *next = res->realPolicyValue;
    if (next == NULL) {
        free(res);
        return NULL;
    }
    for (size_t i = 0; i < MAX_CFG_POLICY_DIRS_CNT; i++) {
        res->paths[i] = next;
        next = strchr(next, ':');
        if (next == NULL) {
            break;
        }
        *next = 0;
        next += 1;
    }
    return res;
}

CfgDir *GetCfgDirList(void)
{
    return GetCfgDirListType(CUST_TYPE_CONFIG);
}
