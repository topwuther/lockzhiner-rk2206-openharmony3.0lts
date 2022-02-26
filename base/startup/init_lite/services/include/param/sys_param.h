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

#ifndef BASE_STARTUP_SYS_PARAM_H
#define BASE_STARTUP_SYS_PARAM_H
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PARAM_VALUE_LEN_MAX  96
#define PARAM_NAME_LEN_MAX  96
typedef u_int32_t ParamHandle;

typedef struct {
    u_int32_t serial;
    ParamHandle handle;
    char value[PARAM_VALUE_LEN_MAX];
} ParamCacheNode;

typedef const char *(*ParamEvaluatePtr)(u_int32_t cacheCount, ParamCacheNode *node);

typedef struct {
    pthread_mutex_t lock;
    u_int32_t serial;
    u_int32_t cacheCount;
    ParamEvaluatePtr evaluate;
    ParamCacheNode *cacheNode;
} ParamCache;

/**
 * 对外接口
 * 设置参数，主要用于其他进程使用，通过管道修改参数。
 *
 */
int SystemSetParameter(const char *name, const char *value);

/**
 * 对外接口
 * 查询参数，主要用于其他进程使用，需要给定足够的内存保存参数。
 * 如果 value == null，获取value的长度
 * 否则value的大小认为是len
 *
 */
int SystemGetParameter(const char *name, char *value, unsigned int *len);

/**
 * 外部接口
 * 遍历参数。
 *
 */
int SystemTraversalParameter(void (*traversalParameter)(ParamHandle handle, void* cookie), void* cookie);

/**
 * 外部接口
 * 查询参数，主要用于其他进程使用，需要给定足够的内存保存参数。
 * 如果 value == null，获取value的长度
 * 否则value的大小认为是len
 *
 */
int SystemGetParameterName(ParamHandle handle, char *name, unsigned int len);

/**
 * 外部接口
 * 获取参数值。
 *
 */
int SystemGetParameterValue(ParamHandle handle, char *value, unsigned int *len);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif