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

#include "init_capability.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined OHOS_LITE && !defined __LINUX__
#include <sys/capability.h>
#else
#include <linux/capability.h>
#endif
#include <sys/types.h>
#include <unistd.h>

#include "init_log.h"
#include "init_perms.h"

#define MAX_CAPS_CNT_FOR_ONE_SERVICE 100

struct CapStrCapNum {
    char *capStr;
    int CapNum;
};

static const struct CapStrCapNum g_capStrCapNum[] = {
    {"CHOWN", CAP_CHOWN},
    {"DAC_OVERRIDE", CAP_DAC_OVERRIDE},
    {"DAC_READ_SEARCH", CAP_DAC_READ_SEARCH},
    {"FOWNER", CAP_FOWNER},
    {"FSETID", CAP_FSETID},
    {"KILL", CAP_KILL},
    {"SETGID", CAP_SETGID},
    {"SETUID", CAP_SETUID},
    {"SETPCAP", CAP_SETPCAP},
    {"LINUX_IMMUTABLE", CAP_LINUX_IMMUTABLE},
    {"NET_BIND_SERVICE", CAP_NET_BIND_SERVICE},
    {"NET_BROADCAST", CAP_NET_BROADCAST},
    {"NET_ADMIN", CAP_NET_ADMIN},
    {"NET_RAW", CAP_NET_RAW},
    {"IPC_LOCK", CAP_IPC_LOCK},
    {"IPC_OWNER", CAP_IPC_OWNER},
    {"SYS_MODULE", CAP_SYS_MODULE},
    {"SYS_RAWIO", CAP_SYS_RAWIO},
    {"SYS_CHROOT", CAP_SYS_CHROOT},
    {"SYS_PTRACE", CAP_SYS_PTRACE},
    {"SYS_PACCT", CAP_SYS_PACCT},
    {"SYS_ADMIN", CAP_SYS_ADMIN},
    {"SYS_BOOT", CAP_SYS_BOOT},
    {"SYS_NICE", CAP_SYS_NICE},
    {"SYS_RESOURCE", CAP_SYS_RESOURCE},
    {"SYS_TIME", CAP_SYS_TIME},
    {"SYS_TTY_CONFIG", CAP_SYS_TTY_CONFIG},
    {"MKNOD", CAP_MKNOD},
    {"LEASE", CAP_LEASE},
    {"AUDIT_WRITE", CAP_AUDIT_WRITE},
    {"AUDIT_CONTROL", CAP_AUDIT_CONTROL},
    {"SETFCAP", CAP_SETFCAP},
    {"MAC_OVERRIDE", CAP_MAC_OVERRIDE},
    {"MAC_ADMIN", CAP_MAC_ADMIN},
    {"SYSLOG", CAP_SYSLOG},
    {"WAKE_ALARM", CAP_WAKE_ALARM},
    {"BLOCK_SUSPEND", CAP_BLOCK_SUSPEND},
    {"AUDIT_READ", CAP_AUDIT_READ},
};

static int GetServiceStringCaps(const cJSON* filedJ, Service* curServ)          // string form
{
    unsigned int i = 0;
    for (; i < curServ->servPerm.capsCnt; ++i) {
        if (cJSON_GetArrayItem(filedJ, i) == NULL || !cJSON_GetStringValue(cJSON_GetArrayItem(filedJ, i))
            || strlen(cJSON_GetStringValue(cJSON_GetArrayItem(filedJ, i))) <= 0) {      // check all errors
            INIT_LOGE("service=%s, parse item[%d] as string, error.", curServ->name, i);
            break;
        }
        char* fieldStr = cJSON_GetStringValue(cJSON_GetArrayItem(filedJ, i));
        if (fieldStr == NULL) {
            INIT_LOGE("fieldStr is NULL");
            break;
        }
        int mapSize = sizeof(g_capStrCapNum) / sizeof(struct CapStrCapNum);     // search
        int j = 0;
        for (; j < mapSize; j++) {
            if (!strcmp(fieldStr, g_capStrCapNum[j].capStr)) {
                break;
            }
        }
        if (j < mapSize) {
            curServ->servPerm.caps[i] = g_capStrCapNum[j].CapNum;
        } else {
            INIT_LOGE("service=%s, CapbilityName=%s, error.", curServ->name, fieldStr);
            break;
        }
        if (curServ->servPerm.caps[i] > CAP_LAST_CAP && curServ->servPerm.caps[i] != FULL_CAP) {
            // resources will be released by function: ReleaseServiceMem
            INIT_LOGE("service=%s, cap = %d, error.", curServ->name, curServ->servPerm.caps[i]);
            return SERVICE_FAILURE;
        }
    }
    int ret = ((i == curServ->servPerm.capsCnt) ? SERVICE_SUCCESS : SERVICE_FAILURE);
    return ret;
}

int GetServiceCaps(const cJSON* curArrItem, Service* curServ)
{
    if (curServ == NULL || curArrItem == NULL) {
        INIT_LOGE("GetServiceCaps failed, curServ or curArrItem is NULL.");
        return SERVICE_FAILURE;
    }
    curServ->servPerm.capsCnt = 0;
    curServ->servPerm.caps = NULL;
    cJSON* filedJ = cJSON_GetObjectItem(curArrItem, "caps");
    if (filedJ == NULL) {
        return SERVICE_SUCCESS;
    }
    if (!cJSON_IsArray(filedJ)) {
        INIT_LOGE("service=%s, caps is not a list, error.", curServ->name);
        return SERVICE_FAILURE;
    }
    // caps array does not exist, means do not need any capability
    int capsCnt = cJSON_GetArraySize(filedJ);
    if (capsCnt <= 0) {
        return SERVICE_SUCCESS;
    }
    if (capsCnt > MAX_CAPS_CNT_FOR_ONE_SERVICE) {
        INIT_LOGE("service=%s, too many caps[cnt %d] for one service, max is %d.",
            curServ->name, capsCnt, MAX_CAPS_CNT_FOR_ONE_SERVICE);
        return SERVICE_FAILURE;
    }
    curServ->servPerm.caps = (unsigned int*)malloc(sizeof(unsigned int) * capsCnt);
    if (curServ->servPerm.caps == NULL) {
        INIT_LOGE("GetServiceCaps, service=%s, malloc error.", curServ->name);
        return SERVICE_FAILURE;
    }
    curServ->servPerm.capsCnt = capsCnt;
    int i = 0;
    for (; i < capsCnt; ++i) {          // number form
        cJSON* capJ = cJSON_GetArrayItem(filedJ, i);
        if (!cJSON_IsNumber(capJ) || cJSON_GetNumberValue(capJ) < 0) {
            // resources will be released by function: ReleaseServiceMem
            break;
        }
        curServ->servPerm.caps[i] = (unsigned int)cJSON_GetNumberValue(capJ);
        if (curServ->servPerm.caps[i] > CAP_LAST_CAP && curServ->servPerm.caps[i] != FULL_CAP) {
            // resources will be released by function: ReleaseServiceMem
            INIT_LOGE("service=%s, caps = %d, error.", curServ->name, curServ->servPerm.caps[i]);
            return SERVICE_FAILURE;
        }
    }
    if (i == capsCnt) {
        return SERVICE_SUCCESS;
    }
    int ret = GetServiceStringCaps(filedJ, curServ);
    return ret;
}

