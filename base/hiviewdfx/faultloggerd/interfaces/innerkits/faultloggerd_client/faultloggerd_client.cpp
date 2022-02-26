/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "faultloggerd_client.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <securec.h>

namespace {
static const int32_t SOCKET_BUFFER_SIZE = 256;
#if defined(USE_MUSL)
static const char FAULTLOGGERD_SOCK_PATH[] = "/dev/unix/socket/faultloggerd.server";
#else
static const char FAULTLOGGERD_SOCK_PATH[] = "/dev/socket/faultloggerd.server";
#endif
}

static int ReadFileDescriptorFromSocket(int socket)
{
    struct msghdr msg = { 0 };
    char messageBuffer[SOCKET_BUFFER_SIZE];
    struct iovec io = {
        .iov_base = messageBuffer,
        .iov_len = sizeof(messageBuffer)
    };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char controlBuffer[SOCKET_BUFFER_SIZE];
    msg.msg_control = controlBuffer;
    msg.msg_controllen = sizeof(controlBuffer);

    if (recvmsg(socket, &msg, 0) < 0) {
        perror("Failed to receive message\n");
        return -1;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    unsigned char *data = CMSG_DATA(cmsg);
    if (data == nullptr) {
        return -1;
    }
    return *(reinterpret_cast<int *>(data));
}

bool ReadStringFromFile(const char *path, char *buf, size_t len)
{
    if ((len <= 1) || (buf == nullptr) || (path == nullptr)) {
        return false;
    }

    char realPath[PATH_MAX];
    if (realpath(path, realPath) == nullptr) {
        return false;
    }

    FILE *fp = fopen(realPath, "r");
    if (fp == nullptr) {
        // log failure
        return false;
    }

    char *ptr = buf;
    for (size_t i = 0; i < len; i++) {
        int c = getc(fp);
        if (c == EOF) {
            *ptr++ = 0x00;
            break;
        } else {
            *ptr++ = c;
        }
    }
    fclose(fp);
    return false;
}

void FillRequest(int32_t type, FaultLoggerdRequest *request)
{
    if (request == nullptr) {
        perror("nullptr request");
        return;
    }

    request->type = type;
    request->pid = getpid();
    request->tid = gettid();
    request->uid = getuid();
    ReadStringFromFile("/proc/self/cmdline", request->module, sizeof(request->module));
}

int32_t RequestFileDescriptor(int32_t type)
{
    struct FaultLoggerdRequest request;
    memset_s(&request, sizeof(request), 0, sizeof(request));
    FillRequest(type, &request);
    return RequestFileDescriptorEx(&request);
}

int32_t RequestFileDescriptorEx(const struct FaultLoggerdRequest *request)
{
    if (request == nullptr) {
        perror("nullptr request");
        return -1;
    }

    int sockfd;
    if ((sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
        perror("client socket error");
        return -1;
    }

    struct sockaddr_un server;
    memset_s(&server, sizeof(server), 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    if (strncpy_s(server.sun_path, sizeof(server.sun_path),
        FAULTLOGGERD_SOCK_PATH, strlen(FAULTLOGGERD_SOCK_PATH)) != 0) {
        perror("Failed to set sock path.");
        close(sockfd);
        return -1;
    }

    int len = offsetof(struct sockaddr_un, sun_path) + strlen(server.sun_path) + 1;
    if (connect(sockfd, reinterpret_cast<struct sockaddr *>(&server), len) < 0) {
        perror("connect error");
        close(sockfd);
        return -1;
    }

    write(sockfd, request, sizeof(struct FaultLoggerdRequest));
    int fd = ReadFileDescriptorFromSocket(sockfd);
    close(sockfd);
    return fd;
}

int32_t LogThreadStacktraceToFile(const char *path, int32_t type, int32_t pid, int32_t tid, int32_t timeout)
{
    return 0;
}