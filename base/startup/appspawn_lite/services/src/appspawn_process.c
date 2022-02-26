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
#define _GNU_SOURCE
#include "appspawn_process.h"
#include <errno.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef OHOS_DEBUG
#include <time.h>
#endif // OHOS_DEBUG

#include <unistd.h>
#include "ability_main.h"
#include "appspawn_adapter.h"
#include "log.h"
#include "securec.h"

#ifdef __LINUX__
#include <sys/time.h>
#include <sys/resource.h>

static const rlim_t DEFAULT_RLIMIT = 40;
#endif // __LINUX__

#define DEFAULT_UMASK 022
#define CAP_NUM 2
#define ENV_TITLE "LD_LIBRARY_PATH="
#define UPPER_BOUND_GID   999
#define LOWER_BOUND_GID   100
#define GRP_NUM 2
#define DEVMGR_GRP 99

static int SetCapability(unsigned int capsCnt, const unsigned int* caps)
{
    struct __user_cap_header_struct capHeader;
    capHeader.version = _LINUX_CAPABILITY_VERSION_3;
    capHeader.pid = 0;

    // common user, clear all caps
    struct __user_cap_data_struct capData[CAP_NUM] = {0};
    for (unsigned int i = 0; i < capsCnt; ++i) {
        capData[CAP_TO_INDEX(caps[i])].effective |= CAP_TO_MASK(caps[i]);
        capData[CAP_TO_INDEX(caps[i])].permitted |= CAP_TO_MASK(caps[i]);
        capData[CAP_TO_INDEX(caps[i])].inheritable |= CAP_TO_MASK(caps[i]);
    }

    if (capset(&capHeader, capData) != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] capset failed, err: %{public}d.", errno);
        return -1;
    }
    for (unsigned int i = 0; i < capsCnt; ++i) {
        if (SetAmbientCapability(caps[i]) != 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] SetAmbientCapability failed, err: %{public}d.", errno);
            return -1;
        }
    }
    return 0;
}

static int SetPerms(uid_t uID, gid_t gID, unsigned int capsCnt, const unsigned int* caps)
{
    gid_t groups[GRP_NUM];

    if (KeepCapability() != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] KeepCapability failed, uID %{public}u, err: %{public}d.",\
            uID, errno);
        return -1;
    }

    if (setgid(gID) != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] setgid failed, gID %{public}u, err: %{public}d.",\
            gID, errno);
        return -1;
    }

    if (setuid(uID) != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] setuid failed, uID %{public}u, err: %{public}d.",\
            uID, errno);
        return -1;
    }

    // add device groups for system app
    if (gID >= LOWER_BOUND_GID && gID <= UPPER_BOUND_GID) {
        groups[0] = gID;
        groups[1] = DEVMGR_GRP;
        if (setgroups(GRP_NUM, groups)) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] setgroups failed, uID %{public}u, err: %{public}d.",\
                uID, errno);
            return -1;
        }
    }

    // umask call always succeeds and return the previous mask value which is not needed here
    (void)umask(DEFAULT_UMASK);

    // set rlimit
#ifdef __LINUX__
    struct rlimit rlim = {0};
    rlim.rlim_cur = DEFAULT_RLIMIT;
    rlim.rlim_max = DEFAULT_RLIMIT;
    if (setrlimit(RLIMIT_NICE, &rlim) != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] setrlimit failed, err: %{public}d.", errno);
        return -1;
    }

    unsigned int tmpCaps[] = {17};  // 17 means CAP_SYS_RAWIO
    unsigned int tmpsCapCnt = sizeof(tmpCaps) / sizeof(tmpCaps[0]);
    if (SetCapability(tmpsCapCnt, tmpCaps) != 0) {
        return -1;
    }
#else
    if (SetCapability(capsCnt, caps) != 0) {
        return -1;
    }
#endif // __LINUX__

    return 0;
}

pid_t CreateProcess(const MessageSt* msgSt)
{
    pid_t newPID = fork();
    if (newPID < 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] create process, fork failed! err %{public}d.", errno);
        return -1;
    }

    // in child process
    if (newPID == 0) {
#ifdef OHOS_DEBUG
        struct timespec tmStart = {0};
        if (clock_gettime(CLOCK_REALTIME, &tmStart) != 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] sub-process, pid %{public}d. get time err %{public}d.",\
                getpid(), errno);
        }
#endif // OHOS_DEBUG

        // set permissions
        if (SetPerms(msgSt->uID, msgSt->gID, msgSt->capsCnt, msgSt->caps) != 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] sub-process %{public}s exit!", msgSt->bundleName);
            exit(0x7f); // 0x7f: user specified
        }

        (void)prctl(PR_SET_NAME, msgSt->bundleName);

#ifdef OHOS_DEBUG
        struct timespec tmEnd = {0};
        if (clock_gettime(CLOCK_REALTIME, &tmEnd) != 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] sub-process, pid %{public}d. get time2 err %{public}d.",\
                getpid(), errno);
        }
        // 1s = 1000000000ns
        long timeUsed = (tmEnd.tv_sec - tmStart.tv_sec) * 1000000000L + (tmEnd.tv_nsec - tmStart.tv_nsec);
        HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] sub-process, pid %{public}d, timeused %ld ns.",\
            getpid(), timeUsed);
#endif // OHOS_DEBUG

        if (AbilityMain(msgSt->identityID) != 0) {
            HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] AbilityMain execute failed, pid %{public}d.", getpid());
            exit(0x7f); // 0x7f: user specified
        }

        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] sub-process exit, pid %{public}d.", getpid());
        exit(0x7f); // 0x7f: user specified
    }

    return newPID;
}

