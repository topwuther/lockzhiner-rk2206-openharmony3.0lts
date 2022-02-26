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
#include "dfx_log.h"

#include <stdarg.h>
#include <stdio.h>

#include <hilog/log_c.h>
#include <securec.h>

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#define LOG_DOMAIN 0x2D11
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "ProcessDump"
#endif

#define LOG_BUF_LEN 1024

int DfxLogDebug(const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_DEBUG(LOG_CORE, "%{public}s", buf);
    return ret;
}

int DfxLogInfo(const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_INFO(LOG_CORE, "%{public}s", buf);
    return ret;
}

int DfxLogWarn(const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_WARN(LOG_CORE, "%{public}s", buf);
    return ret;
}

int DfxLogError(const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_ERROR(LOG_CORE, "%{public}s", buf);
    return ret;
}

int DfxLogFatal(const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_FATAL(LOG_CORE, "%{public}s", buf);
    return ret;
}

int WriteLog(int32_t fd, const char *format, ...)
{
    int ret;
    char buf[LOG_BUF_LEN] = {0};
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(buf, LOG_BUF_LEN, LOG_BUF_LEN - 1, format, args);
    va_end(args);
    HILOG_ERROR(LOG_CORE, "%{public}s", buf);
    dprintf(fd, "%s", buf);
    return ret;
}
