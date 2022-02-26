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

#ifndef INIT_LOG_H
#define INIT_LOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef OHOS_LITE
#include "hilog/log.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum InitLogLevel {
    INIT_DEBUG = 0,
    INIT_INFO,
    INIT_WARN,
    INIT_ERROR,
    INIT_FATAL
} InitLogLevel;

#ifdef LABEL
#define INIT_LOG_TAG LABEL
#endif

#ifndef INIT_LOG_TAG
#define INIT_LOG_TAG "Init"
#endif

#ifdef OHOS_LITE

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD000719

#define INIT_LOGD(fmt, ...) InitToHiLog(LOG_DEBUG, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define INIT_LOGI(fmt, ...) InitToHiLog(LOG_INFO, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define INIT_LOGW(fmt, ...) InitToHiLog(LOG_WARN, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define INIT_LOGE(fmt, ...) InitToHiLog(LOG_ERROR, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define INIT_LOGF(fmt, ...) InitToHiLog(LOG_FATAL, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)

#define STARTUP_LOGD(LABEL, fmt, ...) InitToHiLog(LABEL, LOG_DEBUG, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define STARTUP_LOGI(LABEL, fmt, ...) InitToHiLog(LABEL, LOG_INFO, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)
#define STARTUP_LOGE(LABEL, fmt, ...) InitToHiLog(LABEL, LOG_ERROR, "%s : "fmt, (__FUNCTION__), ##__VA_ARGS__)

void InitToHiLog(LogLevel logLevel, const char *fmt, ...);
void SetHiLogLevel(LogLevel logLevel);

#else
#define FILE_NAME   (strrchr((__FILE__), '/') ? strrchr((__FILE__), '/') + 1 : (__FILE__))
#define INIT_LOGD(fmt, ...) InitLog(INIT_DEBUG, (FILE_NAME), (__LINE__), "<7>", fmt"\n", ##__VA_ARGS__)
#define INIT_LOGI(fmt, ...) InitLog(INIT_INFO, (FILE_NAME), (__LINE__), "<6>", fmt"\n", ##__VA_ARGS__)
#define INIT_LOGW(fmt, ...) InitLog(INIT_WARN, (FILE_NAME), (__LINE__), "<4>", fmt"\n", ##__VA_ARGS__)
#define INIT_LOGE(fmt, ...) InitLog(INIT_ERROR, (FILE_NAME), (__LINE__), "<3>", fmt"\n", ##__VA_ARGS__)
#define INIT_LOGF(fmt, ...) InitLog(INIT_FATAL, (FILE_NAME), (__LINE__), "<3>", fmt"\n", ##__VA_ARGS__)

#define STARTUP_LOGD(LABEL, fmt, ...) InitLog(INIT_DEBUG, (FILE_NAME), (__LINE__), "<7>", fmt "\n", ##__VA_ARGS__)
#define STARTUP_LOGI(LABEL, fmt, ...) InitLog(INIT_INFO, (FILE_NAME), (__LINE__), "<6>", fmt "\n", ##__VA_ARGS__)
#define STARTUP_LOGE(LABEL, fmt, ...) InitLog(INIT_ERROR, (FILE_NAME), (__LINE__), "<3>", fmt "\n", ##__VA_ARGS__)


void InitLog(InitLogLevel logLevel, const char *fileName, int line, const char *kLevel, const char *fmt, ...);
void SetLogLevel(InitLogLevel logLevel);
void OpenLogDevice(void);
void EnableDevKmsg(void);
#endif

#define INIT_ERROR_CHECK(ret, statement, format, ...) \
    do {                                                  \
        if (!(ret)) {                                     \
            INIT_LOGE(format, ##__VA_ARGS__);             \
            statement;                                    \
        }                                                 \
    } while (0)

#define INIT_INFO_CHECK(ret, statement, format, ...) \
    do {                                                  \
        if (!(ret)) {                                    \
            INIT_LOGI(format, ##__VA_ARGS__);            \
            statement;                                   \
        }                                          \
    } while (0)

#define INIT_CHECK(ret, statement) \
    do {                                \
        if (!(ret)) {                  \
            statement;                 \
        }                         \
    } while (0)

#define INIT_CHECK_RETURN_VALUE(ret, result) \
    do {                                \
        if (!(ret)) {                            \
            return result;                       \
        }                                  \
    } while (0)

#define INIT_CHECK_ONLY_RETURN(ret) \
    do {                                \
        if (!(ret)) {                   \
            return;                     \
        } \
    } while (0)

#define INIT_CHECK_ONLY_ELOG(ret, format, ...) \
    do {                                       \
        if (!(ret)) {                          \
            INIT_LOGE(format, ##__VA_ARGS__);  \
        } \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // INIT_LOG_H
