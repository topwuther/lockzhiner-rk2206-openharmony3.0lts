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
#include "init_reboot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "init_log.h"
#include "securec.h"
#include "sys_param.h"

#define SYS_POWER_CTRL "sys.powerctrl="
#define MAX_REBOOT_NAME_SIZE  100
#define MAX_REBOOT_VAUE_SIZE  500

int DoReboot(const char *cmdContent)
{
    uid_t uid1 = getuid();
    uid_t uid2 = geteuid();
    if (uid1 != 0 || uid2 != 0) {
        INIT_LOGE("uid1=%d, uid2=%d, user MUST be root, error!", uid1, uid2);
        return -1;
    }
    char value[MAX_REBOOT_VAUE_SIZE];
    if (cmdContent == NULL || strlen(cmdContent) == 0) {
        if (snprintf_s(value, MAX_REBOOT_NAME_SIZE, strlen("reboot") + 1, "%s", "reboot") < 0) {
            INIT_LOGE("DoReboot api error, MAX_REBOOT_NAME_SIZE is not enough");
            return -1;
        }
        if (SystemSetParameter("sys.powerctrl", value) != 0) {
            INIT_LOGE("DoReboot Api SystemSetParameter error");
            return -1;
        }
        return 0;
    }
    size_t length = strlen(cmdContent);
    if (length > MAX_REBOOT_VAUE_SIZE) {
        INIT_LOGE("DoReboot api error, cmdContent = %s, length = %u.", cmdContent, length);
        return -1;
    }
    if (snprintf_s(value, MAX_REBOOT_NAME_SIZE, MAX_REBOOT_NAME_SIZE - 1, "%s%s", "reboot,", cmdContent) < 0) {
        INIT_LOGE("DoReboot api error, MAX_REBOOT_NAME_SIZE is not enough");
        return -1;
    }
    if (SystemSetParameter("sys.powerctrl", value) != 0) {
        INIT_LOGE("DoReboot Api SystemSetParameter error");
        return -1;
    }
    return 0;
}

