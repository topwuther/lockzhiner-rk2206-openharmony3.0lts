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

#include "init_service.h"

#include <bits/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MUSL__
#include <stropts.h>
#endif
#include <sys/param.h>
#ifndef OHOS_LITE
#include <sys/resource.h>
#endif
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "init_adapter.h"
#include "init_cmds.h"
#include "init_log.h"
#ifndef OHOS_LITE
#include "init_param.h"
#endif
#include "init_perms.h"
#include "init_service_socket.h"
#include "init_utils.h"
#include "securec.h"
#ifndef OHOS_LITE
#include "sys_param.h"
#endif

#define CAP_NUM 2
#define WAIT_MAX_COUNT 10

#ifndef TIOCSCTTY
#define TIOCSCTTY 0x540E
#endif
// 240 seconds, 4 minutes
static const int CRASH_TIME_LIMIT  = 240;
// maximum number of crashes within time CRASH_TIME_LIMIT for one service
static const int CRASH_COUNT_LIMIT = 4;

// 240 seconds, 4 minutes
static const int CRITICAL_CRASH_TIME_LIMIT  = 240;
// maximum number of crashes within time CRITICAL_CRASH_TIME_LIMIT for one service
static const int CRITICAL_CRASH_COUNT_LIMIT = 4;
static const int MAX_PID_STRING_LENGTH = 50;


static int SetAllAmbientCapability()
{
    for (int i = 0; i <= CAP_LAST_CAP; ++i) {
        if (SetAmbientCapability(i) != 0) {
            return SERVICE_FAILURE;
        }
    }
    return SERVICE_SUCCESS;
}

static int SetPerms(const Service *service)
{
    INIT_CHECK_RETURN_VALUE(KeepCapability() == 0, SERVICE_FAILURE);
    if (service->servPerm.gIDCnt > 0) {
        INIT_ERROR_CHECK(setgid(service->servPerm.gIDArray[0]) == 0, return SERVICE_FAILURE,
            "SetPerms, setgid for %s failed. %d", service->name, errno);
    }
    if (service->servPerm.gIDCnt > 1) {
        INIT_ERROR_CHECK(setgroups(service->servPerm.gIDCnt - 1, &service->servPerm.gIDArray[1]) == 0,
             return SERVICE_FAILURE,
            "SetPerms, setgroups failed. errno = %d, gIDCnt=%d", errno, service->servPerm.gIDCnt);
    }
    if (service->servPerm.uID != 0) {
        if (setuid(service->servPerm.uID) != 0) {
            INIT_LOGE("setuid of service: %s failed, uid = %d", service->name, service->servPerm.uID);
            return SERVICE_FAILURE;
        }
    }

    // umask call always succeeds and return the previous mask value which is not needed here
    (void)umask(DEFAULT_UMASK_INIT);

    struct __user_cap_header_struct capHeader;
    capHeader.version = _LINUX_CAPABILITY_VERSION_3;
    capHeader.pid = 0;

    struct __user_cap_data_struct capData[CAP_NUM] = {};
    for (unsigned int i = 0; i < service->servPerm.capsCnt; ++i) {
        if (service->servPerm.caps[i] == FULL_CAP) {
            for (int i = 0; i < CAP_NUM; ++i) {
                capData[i].effective = FULL_CAP;
                capData[i].permitted = FULL_CAP;
                capData[i].inheritable = FULL_CAP;
            }
            break;
        }
        capData[CAP_TO_INDEX(service->servPerm.caps[i])].effective |= CAP_TO_MASK(service->servPerm.caps[i]);
        capData[CAP_TO_INDEX(service->servPerm.caps[i])].permitted |= CAP_TO_MASK(service->servPerm.caps[i]);
        capData[CAP_TO_INDEX(service->servPerm.caps[i])].inheritable |= CAP_TO_MASK(service->servPerm.caps[i]);
    }

    if (capset(&capHeader, capData) != 0) {
        INIT_LOGE("capset faild for service: %s, error: %d", service->name, errno);
        return SERVICE_FAILURE;
    }
    for (unsigned int i = 0; i < service->servPerm.capsCnt; ++i) {
        if (service->servPerm.caps[i] == FULL_CAP) {
            return SetAllAmbientCapability();
        }
        if (SetAmbientCapability(service->servPerm.caps[i]) != 0) {
            INIT_LOGE("SetAmbientCapability faild for service: %s", service->name);
            return SERVICE_FAILURE;
        }
    }
    return SERVICE_SUCCESS;
}

static void OpenConsole()
{
    setsid();
    WaitForFile("/dev/console", WAIT_MAX_COUNT);
    int fd = open("/dev/console", O_RDWR);
    if (fd >= 0) {
        ioctl(fd, TIOCSCTTY, 0);
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2); // Redirect fd to 0:stdin, 1:stdout, 2:stderr
        close(fd);
    } else {
        INIT_LOGE("Open /dev/console failed. err = %d", errno);
    }
    return;
}

static void WriteServicePid(const Service *service, pid_t pid)
{
    char pidString[MAX_PID_STRING_LENGTH];
    INIT_ERROR_CHECK(snprintf_s(pidString, MAX_PID_STRING_LENGTH, MAX_PID_STRING_LENGTH - 1, "%d", pid) >= 0,
        _exit(0x7f), "Build pid string failed");

    for (int i = 0; i < MAX_WRITEPID_FILES; i++) {
        if (service->writepidFiles[i] == NULL) {
            break;
        }
        char *realPath = realpath(service->writepidFiles[i], NULL);
        if (realPath == NULL) {
            continue;
        }
        FILE *fd = fopen(realPath, "wb");
        free(realPath);
        realPath = NULL;
        INIT_ERROR_CHECK(fd != NULL, continue, "Open file %s failed, err = %d", service->writepidFiles[i], errno);
        INIT_CHECK_ONLY_ELOG(fwrite(pidString, 1, strlen(pidString), fd) == strlen(pidString),
            "write pid %s to file %s failed, err = %d", pidString, service->writepidFiles[i], errno);
        (void)fclose(fd);
    }
}

int ServiceStart(Service *service)
{
    INIT_ERROR_CHECK(service != NULL, return SERVICE_FAILURE, "start service failed! null ptr.");
    INIT_INFO_CHECK(service->pid <= 0, return SERVICE_SUCCESS, "service : %s had started already.", service->name);
    if (service->attribute & SERVICE_ATTR_INVALID) {
        INIT_LOGE("start service %s invalid.", service->name);
        return SERVICE_FAILURE;
    }
    INIT_ERROR_CHECK(service->pathArgs != NULL, return SERVICE_FAILURE, "start service pathArgs is NULL.");
    struct stat pathStat = {0};
    service->attribute &= (~(SERVICE_ATTR_NEED_RESTART | SERVICE_ATTR_NEED_STOP));
    INIT_ERROR_CHECK(stat(service->pathArgs[0], &pathStat) == 0, service->attribute |= SERVICE_ATTR_INVALID;
        return SERVICE_FAILURE, "start service %s invalid, please check %s.", service->name, service->pathArgs[0]);
    pid_t pid = fork();
    if (pid == 0) {
        if (service->socketCfg != NULL) {    // start socket service
            INIT_ERROR_CHECK(DoCreateSocket(service->socketCfg) >= 0, _exit(0x7f), "Create Socket failed. ");
        }
        if (service->attribute & SERVICE_ATTR_CONSOLE) {
            OpenConsole();
        }
        INIT_ERROR_CHECK(SetPerms(service) == SERVICE_SUCCESS, _exit(0x7f),
            "service %s exit! set perms failed! err %d.", service->name, errno);
        WriteServicePid(service, getpid());
        INIT_LOGI("service->name is %s ", service->name);
#ifndef OHOS_LITE
        if (service->importance != 0) {
            INIT_ERROR_CHECK(setpriority(PRIO_PROCESS, 0, service->importance) == 0, _exit(0x7f),
                "setpriority failed for %s, importance = %d", service->name, service->importance);
        }
        // L2 Can not be reset env
        INIT_CHECK_ONLY_ELOG(execv(service->pathArgs[0], service->pathArgs) == 0,
            "service %s execve failed! err %d.", service->name, errno);
#else
        char* env[] = {"LD_LIBRARY_PATH=/storage/app/libs", NULL};
        INIT_CHECK_ONLY_ELOG(execve(service->pathArgs[0], service->pathArgs, env) == 0,
            "service %s execve failed! err %d.", service->name, errno);
#endif
        _exit(0x7f); // 0x7f: user specified
    } else if (pid < 0) {
        INIT_LOGE("start service %s fork failed!", service->name);
        return SERVICE_FAILURE;
    }
    service->pid = pid;
#ifndef OHOS_LITE
    char paramName[PARAM_NAME_LEN_MAX] = {0};
    int ret = snprintf_s(paramName, PARAM_NAME_LEN_MAX, PARAM_NAME_LEN_MAX - 1, "init.svc.%s", service->name);
    INIT_CHECK_ONLY_ELOG(ret >= 0, "snprintf_s paramName error %d ", errno);
    SystemWriteParam(paramName, "running");
#endif
    return SERVICE_SUCCESS;
}

int ServiceStop(Service *service)
{
    if (service == NULL) {
        INIT_LOGE("stop service failed! null ptr.");
        return SERVICE_FAILURE;
    }

    service->attribute &= ~SERVICE_ATTR_NEED_RESTART;
    service->attribute |= SERVICE_ATTR_NEED_STOP;
    if (service->pid <= 0) {
        return SERVICE_SUCCESS;
    }

    if (kill(service->pid, SIGKILL) != 0) {
        INIT_LOGE("stop service %s pid %d failed! err %d.", service->name, service->pid, errno);
        return SERVICE_FAILURE;
    }
#ifndef OHOS_LITE
    char paramName[PARAM_NAME_LEN_MAX] = {0};
    if (snprintf_s(paramName, PARAM_NAME_LEN_MAX, PARAM_NAME_LEN_MAX - 1, "init.svc.%s", service->name) < 0) {
        INIT_LOGE("snprintf_s paramName error %d ", errno);
    }
    SystemWriteParam(paramName, "stopping");
#endif
    INIT_LOGI("stop service %s, pid %d.", service->name, service->pid);
    return SERVICE_SUCCESS;
}

// the service need to be restarted, if it crashed more than 4 times in 4 minutes
void CheckCritical(Service *service)
{
    if (service == NULL) {
        return;
    }
    if (service->attribute & SERVICE_ATTR_CRITICAL) {            // critical
        // crash time and count check
        time_t curTime = time(NULL);
        if (service->criticalCrashCnt == 0) {
            service->firstCriticalCrashTime = curTime;
            ++service->criticalCrashCnt;
        } else if (difftime(curTime, service->firstCriticalCrashTime) > CRITICAL_CRASH_TIME_LIMIT) {
            service->firstCriticalCrashTime = curTime;
            service->criticalCrashCnt = 1;
        } else {
            ++service->criticalCrashCnt;
            if (service->criticalCrashCnt > CRITICAL_CRASH_COUNT_LIMIT) {
                INIT_LOGE("reap critical service %s, crash too many times! Need reboot!", service->name);
                RebootSystem();
            }
        }
    }
}

static int ExecRestartCmd(const Service *service)
{
    INIT_LOGI("ExecRestartCmd ");
    if ((service == NULL) || (service->onRestart == NULL) || (service->onRestart->cmdLine == NULL)) {
        return SERVICE_FAILURE;
    }

    for (int i = 0; i < service->onRestart->cmdNum; i++) {
        INIT_LOGI("SetOnRestart cmdLine->name %s  cmdLine->cmdContent %s ", service->onRestart->cmdLine[i].name,
            service->onRestart->cmdLine[i].cmdContent);
        DoCmd(&service->onRestart->cmdLine[i]);
    }
    free(service->onRestart->cmdLine);
    service->onRestart->cmdLine = NULL;
    free(service->onRestart);
    return SERVICE_SUCCESS;
}

void ServiceReap(Service *service)
{
    if (service == NULL) {
        INIT_LOGE("reap service failed! null ptr.");
        return;
    }
    service->pid = -1;
#ifndef OHOS_LITE
    char paramName[PARAM_NAME_LEN_MAX] = {0};
    if (snprintf_s(paramName, PARAM_NAME_LEN_MAX, PARAM_NAME_LEN_MAX - 1, "init.svc.%s", service->name) < 0) {
        INIT_LOGE("snprintf_s paramName error %d ", errno);
    }
    SystemWriteParam(paramName, "stopped");
#endif
    if (service->attribute & SERVICE_ATTR_INVALID) {
        INIT_LOGE("ServiceReap service %s invalid.", service->name);
        return;
    }
    // stopped by system-init itself, no need to restart even if it is not one-shot service
    if (service->attribute & SERVICE_ATTR_NEED_STOP) {
        service->attribute &= (~SERVICE_ATTR_NEED_STOP);
        service->crashCnt = 0;
        return;
    }
    // for one-shot service
    if (service->attribute & SERVICE_ATTR_ONCE) {
        // no need to restart
        if (!(service->attribute & SERVICE_ATTR_NEED_RESTART)) {
            service->attribute &= (~SERVICE_ATTR_NEED_STOP);
            return;
        }
    }
    // the service that does not need to be restarted restarts, indicating that it has crashed
    if (!(service->attribute & SERVICE_ATTR_NEED_RESTART)) {
        // crash time and count check
        time_t curTime = time(NULL);
        if (service->crashCnt == 0) {
            service->firstCrashTime = curTime;
            ++service->crashCnt;
        } else if (difftime(curTime, service->firstCrashTime) > CRASH_TIME_LIMIT) {
            service->firstCrashTime = curTime;
            service->crashCnt = 1;
        } else {
            ++service->crashCnt;
            if (service->crashCnt > CRASH_COUNT_LIMIT) {
                INIT_LOGE("reap service %s, crash too many times!", service->name);
                return;
            }
        }
    }
    CheckCritical(service);
    if (service->onRestart != NULL) {
        INIT_CHECK_ONLY_ELOG(ExecRestartCmd(service) == SERVICE_SUCCESS, "SetOnRestart fail ");
    }
    INIT_CHECK_ONLY_ELOG(ServiceStart(service) == SERVICE_SUCCESS, "reap service %s start failed!", service->name);
    service->attribute &= (~SERVICE_ATTR_NEED_RESTART);
}

