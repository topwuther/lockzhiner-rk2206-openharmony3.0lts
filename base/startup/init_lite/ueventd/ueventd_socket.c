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

#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "securec.h"
#define INIT_LOG_TAG "ueventd"
#include "init_log.h"

#define UEVENT_SOCKET_BUFF_SIZE (256 * 1024)

int UeventdSocketInit()
{
    struct sockaddr_nl addr;
    int sockfd = -1;
    int buffSize = UEVENT_SOCKET_BUFF_SIZE;
    int on = 1;

    if (memset_s(&addr, sizeof(addr), 0, sizeof(addr) != EOK)) {
        INIT_LOGE("Faild to clear socket address");
        return -1;
    }
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    sockfd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, NETLINK_KOBJECT_UEVENT);
    if (sockfd < 0) {
        INIT_LOGE("Create socket failed, err = %d", errno);
        return -1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUFFORCE, &buffSize, sizeof(buffSize));
    setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        INIT_LOGE("Bind socket failed, err = %d", errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

ssize_t ReadUeventMessage(int sockFd, char *buffer, size_t length)
{
    ssize_t n = -1;
    struct msghdr msghdr = {0};
    struct iovec iov;
    struct sockaddr_nl addr;
    char credMsg[CMSG_SPACE(sizeof(struct ucred))];

    // sanity check
    if (sockFd < 0 || buffer == NULL) {
        return n;
    }

    iov.iov_base = buffer;
    iov.iov_len = length;
    msghdr.msg_name = &addr;
    msghdr.msg_namelen = sizeof(addr);
    msghdr.msg_iov = &iov;
    msghdr.msg_iovlen = 1;
    msghdr.msg_control = credMsg;
    msghdr.msg_controllen = sizeof(credMsg);

    n = recvmsg(sockFd, &msghdr, 0);
    if (n <= 0) {
        return n;
    }
    struct cmsghdr *cmsghdr = CMSG_FIRSTHDR(&msghdr);
    if (cmsghdr == NULL || cmsghdr->cmsg_type != SCM_CREDENTIALS) {
        INIT_LOGE("Unexpected control message, ignored");
        // Drop this message
        *buffer = '\0';
        n = -1;
    }
    return n;
}
