/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <string.h>
#include <stdio.h>
#include "sys_param.h"

#define HELP_PARAM "--help"
#define BUFFER_SIZE 256
#define PARAM_GET_ARG_NUM 2

static void ProcessParam(ParamHandle handle, void* cookie)
{
    if (cookie == NULL) {
        printf("ProcessParam cookie is NULL\n");
        return;
    }
    SystemGetParameterName(handle, (char*)cookie, BUFFER_SIZE);
    u_int32_t size = BUFFER_SIZE;
    SystemGetParameterValue(handle, ((char*)cookie) + BUFFER_SIZE, &size);
    printf("\t%s = %s \n", (char*)cookie, ((char*)cookie) + BUFFER_SIZE);
}

int main(int argc, char* argv[])
{
    if (argc == 1) { // 显示所有的记录
        char value[BUFFER_SIZE + BUFFER_SIZE] = {0};
        SystemTraversalParameter(ProcessParam, (void*)value);
        return 0;
    }
    if (argc == PARAM_GET_ARG_NUM && argv[1] != NULL && strncmp(argv[1], HELP_PARAM, strlen(HELP_PARAM)) == 0) {
        printf("usage: getparam NAME VALUE\n");
        return 0;
    }
    if (argc != PARAM_GET_ARG_NUM) {
        printf("usage: getparam NAME VALUE\n");
        return 0;
    }
    char value[BUFFER_SIZE] = {0};
    u_int32_t size = BUFFER_SIZE;
    int ret = SystemGetParameter(argv[1], value, &size);
    if (ret == 0) {
        printf("%s \n", value);
    } else {
        printf("getparam %s %s fail\n", argv[1], value);
    }
}
