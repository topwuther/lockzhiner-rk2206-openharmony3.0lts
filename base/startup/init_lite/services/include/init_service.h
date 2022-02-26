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

#ifndef BASE_STARTUP_INITLITE_SERVICE_H
#define BASE_STARTUP_INITLITE_SERVICE_H

#include <sys/types.h>
#include "init_cmds.h"
#include "init_service_socket.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// return value
#define SERVICE_FAILURE (-1)
#define SERVICE_SUCCESS 0

// service attributes
#define SERVICE_ATTR_INVALID      0x001  // option invalid
#define SERVICE_ATTR_ONCE         0x002  // do not restart when it exits
#define SERVICE_ATTR_NEED_RESTART 0x004  // will restart in the near future
#define SERVICE_ATTR_NEED_STOP    0x008  // will stop in reap
#define SERVICE_ATTR_IMPORTANT    0x010  // will reboot if it crash
#define SERVICE_ATTR_CRITICAL     0x020  // critical, will reboot if it crash 4 times in 4 minutes
#define SERVICE_ATTR_DISABLED     0x040  // disabled
#define SERVICE_ATTR_CONSOLE      0x080  // console

#define MAX_SERVICE_NAME 32
#define MAX_WRITEPID_FILES 100

#define CAP_NUM 2

#define SERVICES_ARR_NAME_IN_JSON "services"

typedef struct {
    uid_t uID;
    gid_t *gIDArray;
    int gIDCnt;
    unsigned int *caps;
    unsigned int capsCnt;
} Perms;

struct OnRestartCmd {
    CmdLine *cmdLine;
    int cmdNum;
};

typedef struct {
    char   name[MAX_SERVICE_NAME + 1];
    char** pathArgs;
    int    pathArgsCnt;
    int    pid;
    int    crashCnt;
    time_t firstCrashTime;
    int    criticalCrashCnt;            // count for critical
    time_t firstCriticalCrashTime;      // record for critical
    char   *writepidFiles[MAX_WRITEPID_FILES];
    unsigned int attribute;
    int    importance;
    Perms  servPerm;
    struct OnRestartCmd *onRestart;
    struct ServiceSocket *socketCfg;
} Service;

int ServiceStart(Service *service);
int ServiceStop(Service *service);
void ServiceReap(Service *service);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_SERVICE_H
