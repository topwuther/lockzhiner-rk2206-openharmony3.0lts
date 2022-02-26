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

#ifndef BASE_STARTUP_INIT_PARAM_H
#define BASE_STARTUP_INIT_PARAM_H
#include <stdio.h>
#include "cJSON.h"
#include "sys_param.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    EVENT_PROPERTY, // 参数修改事件
    EVENT_BOOT
} EventType;

/**
 * Init 接口
 * 初始化参数服务
 *
 */
void InitParamService();

/**
 * Init 接口
 * 启动参数服务，在main启动的最后调用，阻赛当前线程
 *
 */
int StartParamService();

/**
 * Init 接口
 * 停止参数服务
 *
 */
void StopParamService();

/**
 * Init 接口
 * 加载默认的参数值
 *
 */
int LoadDefaultParams(const char *fileName);

/**
 * Init 接口
 * 安全使用，加载参数的信息，包括selinux label 等
 *
 */
int LoadParamInfos(const char *fileName);

/**
 * Init 接口
 * 加载默认参数。
 *
 */
int LoadPersistParams();

/**
 * Init 接口
 * 设置参数，主要用于其他进程使用，通过管道修改参数
 *
 */
int SystemWriteParam(const char *name, const char *value);

/**
 * Init 接口
 * 查询参数。
 *
 */
int SystemReadParam(const char *name, char *value, unsigned int *len);

/**
 * 对Init接口
 * 触发一个trigger操作。
 *
 */
void PostTrigger(EventType type, const char *content, u_int32_t contentLen);

/**
 * 对Init接口
 * 触发一个参数trigger操作。
 *
 */
void PostParamTrigger(const char *name, const char *value);

/**
 * 对Init接口
 * 解析trigger文件。
 *
 */
int ParseTriggerConfig(const cJSON *fileRoot);

/**
 * 对Init接口
 * 按名字执行对应的trigger。
 *
 */
void DoTriggerExec(const char *content);

/**
 * 对Init接口
 * 按名字执行对应的trigger。
 *
 */
int SystemTraversalParam(void (*traversalParameter)(ParamHandle handle, void* cookie), void* cookie);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
