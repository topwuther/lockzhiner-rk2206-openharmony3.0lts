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

#ifndef BASE_STARTUP_PARAM_REQUEST_H
#define BASE_STARTUP_PARAM_REQUEST_H

#include <stdio.h>
#include "sys_param.h"
#include "param_manager.h"

#include "uv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum RequestType {
    SET_PARAM,
    GET_PARAM,
} RequestType;

typedef struct {
    ParamSecurityLabel securitylabel;
    RequestType type;
    int contentSize;
    char content[0];
} RequestMsg;

typedef struct {
    RequestType type;
    int result;
    int contentSize;
    char content[0];
} ResponseMsg;

typedef struct {
    uv_loop_t *loop;
    uv_connect_t connect;
    uv_pipe_t handle;
    uv_write_t wr;
    int result;
    RequestMsg msg;
} RequestNode;

typedef struct {
    uv_write_t writer;
    ResponseMsg msg;
} ResponseNode;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif