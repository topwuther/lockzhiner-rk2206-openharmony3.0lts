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

#include "init_service_socket.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include "init_log.h"
#include "securec.h"

#define HOS_SOCKET_DIR    "/dev/unix/socket"
#define HOS_SOCKET_ENV_PREFIX    "OHOS_SOCKET_"
#define MAX_SOCKET_ENV_PREFIX_LEN 64
#define MAX_SOCKET_FD_LEN 16

static int CreateSocket(struct ServiceSocket *sockopt)
{
    if (sockopt == NULL || sockopt->name == NULL) {
        return -1;
    }
    if (sockopt->sockFd >= 0) {
        close(sockopt->sockFd);
        sockopt->sockFd = -1;
    }
    sockopt->sockFd = socket(PF_UNIX, sockopt->type, 0);
    INIT_ERROR_CHECK(sockopt->sockFd >= 0, return -1, "socket fail %d ", errno);
    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (snprintf_s(addr.sun_path, sizeof(addr.sun_path), sizeof(addr.sun_path) - 1, HOS_SOCKET_DIR"/%s",
        sockopt->name) < 0) {
        return -1;
    }
    if (access(addr.sun_path, F_OK) == 0) {
        INIT_LOGE("%s already exist, remove it", addr.sun_path);
        if (unlink(addr.sun_path) != 0) {
            INIT_LOGE("ulink fail err %d ", errno);
        }
    }
    if (sockopt->passcred) {
        int on = 1;
        if (setsockopt(sockopt->sockFd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on))) {
            unlink(addr.sun_path);
            close(sockopt->sockFd);
            return -1;
        }
    }
    if (bind(sockopt->sockFd, (struct sockaddr *)&addr, sizeof(addr))) {
        INIT_LOGE("Create socket for service %s failed: %d", sockopt->name, errno);
        unlink(addr.sun_path);
        close(sockopt->sockFd);
        return -1;
    }
    if (lchown(addr.sun_path, sockopt->uid, sockopt->gid)) {
        unlink(addr.sun_path);
        close(sockopt->sockFd);
        INIT_LOGE("lchown fail %d ", errno);
        return -1;
    }
    if (fchmodat(AT_FDCWD, addr.sun_path, sockopt->perm, AT_SYMLINK_NOFOLLOW)) {
        unlink(addr.sun_path);
        close(sockopt->sockFd);
        INIT_LOGE("fchmodat fail %d ", errno);
        return -1;
    }
    return sockopt->sockFd;
}

static int SetSocketEnv(int fd, const char *name)
{
    if (name == NULL) {
        return -1;
    }
    char pubName[MAX_SOCKET_ENV_PREFIX_LEN] = {0};
    char val[MAX_SOCKET_FD_LEN] = {0};
    if (snprintf_s(pubName, MAX_SOCKET_ENV_PREFIX_LEN, MAX_SOCKET_ENV_PREFIX_LEN - 1,
        HOS_SOCKET_ENV_PREFIX"%s", name) < 0) {
        return -1;
    }
    if (snprintf_s(val, MAX_SOCKET_FD_LEN, MAX_SOCKET_FD_LEN - 1, "%d", fd) < 0) {
        return -1;
    }
    int ret = setenv(pubName, val, 1);
    if (ret < 0) {
        INIT_LOGE("setenv fail %d ", errno);
        return -1;
    }
    fcntl(fd, F_SETFD, 0);
    return 0;
}

int DoCreateSocket(struct ServiceSocket *sockopt)
{
    if (sockopt == NULL) {
        return -1;
    }
    struct ServiceSocket *tmpSock = sockopt;
    while (tmpSock != NULL) {
        int fd = CreateSocket(tmpSock);
        if (fd < 0) {
            return -1;
        }
        if (tmpSock->name == NULL) {
            return -1;
        }
        int ret = SetSocketEnv(fd, tmpSock->name);
        if (ret < 0) {
            return -1;
        }
        tmpSock = tmpSock->next;
    }
    return 0;
}

