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

#include "init_log.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include "securec.h"

#define UNUSED(x) \
    do { \
        (void)(x) \
    } while (0)

#define MAX_LOG_SIZE 1024
#define BASE_YEAR 1900
#define UNLIKELY(x)    __builtin_expect(!!(x), 0)

static InitLogLevel g_logLevel = INIT_INFO;
static const char *LOG_LEVEL_STR[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

void SetLogLevel(InitLogLevel logLevel)
{
    g_logLevel = logLevel;
}

#ifdef OHOS_LITE
static LogLevel g_hiLogLevel = LOG_INFO;

void SetHiLogLevel(LogLevel logLevel)
{
    g_hiLogLevel = logLevel;
}

void InitToHiLog(LogLevel logLevel, const char *fmt, ...)
{
    if (logLevel < g_hiLogLevel) {
        return;
    }

    va_list list;
    va_start(list, fmt);
    char tmpFmt[MAX_LOG_SIZE];
    if (vsnprintf_s(tmpFmt, MAX_LOG_SIZE, MAX_LOG_SIZE - 1, fmt, list) == -1) {
        va_end(list);
        return;
    }
    (void)HiLogPrint(LOG_CORE, logLevel, LOG_DOMAIN, INIT_LOG_TAG, "%{public}s", tmpFmt);
    va_end(list);
    return;
}
#endif

static int g_fd = -1;
void OpenLogDevice(void)
{
    int fd = open("/dev/kmsg", O_WRONLY | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP);
    if (fd >= 0) {
        g_fd = fd;
    }
    return;
}

void EnableDevKmsg(void)
{
    /* printk_devkmsg default value is ratelimit, We need to set "on" and remove the restrictions */
    int fd = open("/proc/sys/kernel/printk_devkmsg", O_WRONLY | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP);
    if (fd < 0) {
        return;
    }
    const char *kmsgStatus = "on";
    write(fd, kmsgStatus, strlen(kmsgStatus) + 1);
    close(fd);
    fd = -1;
    return;
}

void InitLog(InitLogLevel logLevel, const char *fileName, int line, const char *kLevel,
    const char *fmt, ...)
{
    if (logLevel < g_logLevel) {
        return;
    }

    if (UNLIKELY(g_fd < 0)) {
        OpenLogDevice();
        if (g_fd < 0) {
            return;
        }
    }
    va_list vargs;
    va_start(vargs, fmt);
    char tmpFmt[MAX_LOG_SIZE];
    if (vsnprintf_s(tmpFmt, MAX_LOG_SIZE, MAX_LOG_SIZE - 1, fmt, vargs) == -1) {
        close(g_fd);
        g_fd = -1;
        return;
    }

    char logInfo[MAX_LOG_SIZE];
    if (snprintf_s(logInfo, MAX_LOG_SIZE, MAX_LOG_SIZE - 1, "%s[pid=%d][%s:%d][%s][%s] %s",
        kLevel, getpid(), fileName, line, INIT_LOG_TAG, LOG_LEVEL_STR[logLevel], tmpFmt) == -1) {
        close(g_fd);
        g_fd = -1;
        return;
    }
    va_end(vargs);

    if (write(g_fd, logInfo, strlen(logInfo)) < 0) {
        close(g_fd);
        g_fd = -1;
    }
    return;
}
