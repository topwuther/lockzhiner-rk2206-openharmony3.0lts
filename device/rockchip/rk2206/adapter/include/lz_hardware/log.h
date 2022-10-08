/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

/**
 * @addtogroup Lockzhiner
 *
 * @file debug.h
 */


#ifndef LZ_HARDWARE_LOG_H
#define LZ_HARDWARE_LOG_H

#include "stdio.h"

enum {
    LZ_HARDWARE_NONE_LEVEL = 0,
    LZ_HARDWARE_LOGE_LEVEL,
    LZ_HARDWARE_LOGW_LEVEL,
    LZ_HARDWARE_LOGI_LEVEL,
    LZ_HARDWARE_LOGD_LEVEL,
    LZ_HARDWARE_LOGV_LEVEL,
};

#define NONE                        "\e[0m"
#define RED                         "\e[0;31m"
#define GREEN                       "\e[0;32m"
#define BROWN                       "\e[0;33m"
#define BLUE                        "\e[0;34m"

/* 定义LZ_HARDWARE_TRACE_LEVEL变量，定义在lz_hardware.c
 * 修改该值为LZ_HARDWARE_NONE_LEVEL表示不打印
 */
extern unsigned int LZ_HARDWARE_TRACE_LEVEL;

#define LZ_PRINTF(level, fmt, ...)  do { \
    if (LZ_HARDWARE_TRACE_LEVEL >= level) printf(fmt, ##__VA_ARGS__); \
} while (0)

#define LZ_HARDWARE_LOGV(tag, fmt, arg...)      LZ_PRINTF(LZ_HARDWARE_LOGV_LEVEL, "[" tag ":V]" fmt "\n", ##arg)
#define LZ_HARDWARE_LOGD(tag, fmt, arg...)      LZ_PRINTF(LZ_HARDWARE_LOGD_LEVEL, BLUE "[" tag ":D]" fmt "\n" NONE, ##arg)
#define LZ_HARDWARE_LOGI(tag, fmt, arg...)      LZ_PRINTF(LZ_HARDWARE_LOGI_LEVEL, BROWN "[" tag ":I]" fmt "\n" NONE, ##arg)
#define LZ_HARDWARE_LOGW(tag, fmt, arg...)      LZ_PRINTF(LZ_HARDWARE_LOGW_LEVEL, GREEN "[" tag ":W]" fmt "\n" NONE, ##arg)
#define LZ_HARDWARE_LOGE(tag, fmt, arg...)      LZ_PRINTF(LZ_HARDWARE_LOGE_LEVEL, RED "[" tag ":E]" fmt "\n" NONE, ##arg)


/* 定义PRINT_LEVEL等级 */
enum {
    PRINT_NONE_LEVEL = 0,
    PRINT_ERR_LEVEL,
    PRINT_WARN_LEVEL,
    PRINT_LOG_LEVEL,
};
/* 定义PRINT_LEVEL等级变量，定义在lz_hardware.c
 * 修改该值为PRINT_NONE_LEVEL，表示不打印
 */
extern unsigned int g_print_level;

/* 打印日志信息 */
#define PRINT_LOG(fmt, args...)     do { \
    if (g_print_level >= PRINT_LOG_LEVEL) printf("%s, %d, log: "fmt, __FILE__, __LINE__, ##args); \
} while (0)

/* 打印告警信息 */
#define PRINT_WARN(fmt, args...)     do { \
        if (g_print_level >= PRINT_WARN_LEVEL) printf("%s, %d, warn: "fmt, __FILE__, __LINE__, ##args); \
    } while (0)

/* 打印错误信息 */
#define PRINT_ERR(fmt, args...)     do { \
    if (g_print_level >= PRINT_ERR_LEVEL) printf("%s, %d, error: "fmt, __FILE__, __LINE__, ##args); \
} while (0)


#endif /* LZ_HARDWARE_LOG_H */
