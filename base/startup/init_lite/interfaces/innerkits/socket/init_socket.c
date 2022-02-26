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

#include "init_socket.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include "init_log.h"
#include "securec.h"

#define N_DEC 10
#define MAX_SOCKET_ENV_PREFIX_LEN 64
#define MAX_SOCKET_DIR_LEN 128

static int GetControlFromEnv(const char *path, int length)
{
    if (path == NULL || length <= 0) {
        return -1;
    }
    INIT_LOGI("GetControlFromEnv path is %s ", path);
    const char *val = getenv(path);
    if (val == NULL) {
        INIT_LOGE("GetControlFromEnv val is null %d", errno);
        return -1;
    }
    errno = 0;
    int fd = strtol(val, NULL, N_DEC);
    if (errno) {
        return -1;
    }
    INIT_LOGI("GetControlFromEnv fd is %d ", fd);
    if (fcntl(fd, F_GETFD) < 0) {
        INIT_LOGE("GetControlFromEnv errno %d ", errno);
        return -1;
    }
    return fd;
}

int GetControlSocket(const char *name)
{
    if (name == NULL) {
        return -1;
    }
    char path[MAX_SOCKET_ENV_PREFIX_LEN] = {0};
    if (snprintf_s(path, sizeof(path), sizeof(path) - 1, OHOS_SOCKET_ENV_PREFIX"%s", name) == -1) {
        return -1;
    }
    INIT_LOGI("GetControlSocket path is %s ", path);
    int fd = GetControlFromEnv(path, MAX_SOCKET_ENV_PREFIX_LEN);
    if (fd < 0) {
        INIT_LOGE("GetControlFromEnv fail ");
        return -1;
    }
    struct sockaddr_un addr;
    socklen_t addrlen = sizeof(addr);
    int ret = getsockname(fd, (struct sockaddr*)&addr, &addrlen);
    if (ret < 0) {
        INIT_LOGE("GetControlSocket errno %d ", errno);
        return -1;
    }
    char sockDir[MAX_SOCKET_DIR_LEN] = {0};
    if (snprintf_s(sockDir, sizeof(sockDir), sizeof(sockDir) - 1, OHOS_SOCKET_DIR"/%s", name) == -1) {
        return -1;
    }
    INIT_LOGI("sockDir %s ", sockDir);
    INIT_LOGI("addr.sun_path %s ", addr.sun_path);
    if (strncmp(sockDir, addr.sun_path, strlen(sockDir)) == 0) {
        return fd;
    }
    return -1;
}
