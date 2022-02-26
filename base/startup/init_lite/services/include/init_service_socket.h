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

#ifndef INIT_SERVICE_SOCKET_
#define INIT_SERVICE_SOCKET_
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SOCK_NAME_LEN  16
#define SOCK_OPT_NUMS  6
enum SockOptionTab {
    SERVICE_SOCK_NAME = 0,
    SERVICE_SOCK_TYPE,
    SERVICE_SOCK_PERM,
    SERVICE_SOCK_UID,
    SERVICE_SOCK_GID,
    SERVICE_SOCK_SETOPT
};

struct ServiceSocket;
struct ServiceSocket {
    char *name;         // service name
    int type;           // socket type
    uid_t uid;        // uid
    gid_t gid;        // gid
    bool passcred;    // setsocketopt
    mode_t perm;      // Setting permissions
    int sockFd;
    struct ServiceSocket *next;
};

int DoCreateSocket(struct ServiceSocket *sockopt);

#endif
