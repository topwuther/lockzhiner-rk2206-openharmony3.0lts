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

#ifndef __UEVENTD_READ_CFG_H
#define __UEVENTD_READ_CFG_H
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "list.h"

struct DeviceUdevConf {
    const char *name;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    struct ListNode list;
};

struct SysUdevConf {
    const char *sysPath;
    const char *attr;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    struct ListNode list;
};

struct FirmwareUdevConf {
    const char *fmPath;
    struct ListNode list;
};
void ParseUeventdConfigFile(const char *file);
void GetDeviceNodePermissions(const char *devNode, uid_t *uid, gid_t *gid, mode_t *mode);
void ChangeSysAttributePermissions(const char *sysPath);
#endif /* __UEVENTD_READ_CFG_H */
