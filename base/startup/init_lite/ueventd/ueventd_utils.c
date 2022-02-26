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

#include "ueventd_utils.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include "securec.h"
#define INIT_LOG_TAG "ueventd"
#include "init_log.h"

int MakeDir(const char *dir, mode_t mode)
{
    int rc = -1;
    if (INVALIDSTRING(dir)) {
        errno = EINVAL;
        return rc;
    }
    rc = mkdir(dir, mode);
    if (rc < 0 && errno != EEXIST) {
        INIT_LOGE("Create directory \" %s \" failed, err = %d", dir, errno);
        return rc;
    }
    // create dir success or it already exist.
    return 0;
}

int MakeDirRecursive(const char *dir, mode_t mode)
{
    int rc = -1;
    char buffer[PATH_MAX] = {};
    const char *p = NULL;
    if (INVALIDSTRING(dir)) {
        errno = EINVAL;
        return rc;
    }
    char *slash = strchr(dir, '/');
    p = dir;
    while (slash != NULL) {
        int gap = slash - p;
        p = slash + 1;
        if (gap == 0) {
            slash = strchr(p, '/');
            continue;
        }
        if (gap < 0) { // end with '/'
            break;
        }
        if (memcpy_s(buffer, PATH_MAX, dir, p - dir - 1) != 0) {
            return -1;
        }
        rc = MakeDir(buffer, mode);
        if (rc < 0) {
            return rc;
        }
        slash = strchr(p, '/');
    }
    return MakeDir(dir, mode);
}

int StringToInt(const char *str, int defaultValue)
{
    if (INVALIDSTRING(str)) {
        return defaultValue;
    }
    errno = 0;
    int value = strtoul(str, NULL, DECIMALISM);
    return (errno != 0) ? defaultValue : value;
}
