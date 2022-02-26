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
#include "securec.h"
#include "sys_param.h"

#define SERVICE_START_NUMBER 2
#define SERVICE_CONTROL_NUMBER 3
#define CONTROL_SERVICE_POS 2
#define SERVICE_CONTROL_MAX_SIZE 50

static void ServiceControlUsage()
{
    printf("Please input correct params, example:\n");
    printf("    start_service serviceName\n");
    printf("    stop_service serviceName\n");
    printf("    service_control start serviceName\n");
    printf("    service_control stop serviceName\n");
    return;
}

static void ServiceControl(int argc, char** argv)
{
    if (argc != SERVICE_CONTROL_NUMBER) {
        ServiceControlUsage();
        return;
    }
    char serviceCtl[SERVICE_CONTROL_MAX_SIZE];
    if (strcmp(argv[1], "start") == 0) {
        if (strncpy_s(serviceCtl, sizeof(serviceCtl), "ohos.ctl.start", sizeof(serviceCtl) - 1) != EOK) {
            printf("strncpy_s failed.\n");
            return;
        }
    } else if (strcmp(argv[1], "stop") == 0) {
        if (strncpy_s(serviceCtl, sizeof(serviceCtl), "ohos.ctl.stop", sizeof(serviceCtl) - 1) != EOK) {
            printf("strncpy_s failed.\n");
            return;
        }
    } else {
        ServiceControlUsage();
        return;
    }
    if (SystemSetParameter(serviceCtl, argv[CONTROL_SERVICE_POS]) != 0) {
        printf("%s service:%s failed.\n", argv[1], argv[CONTROL_SERVICE_POS]);
        return;
    }
    return;
}

int main(int argc, char** argv)
{
    if (argc != SERVICE_START_NUMBER && argc != SERVICE_CONTROL_NUMBER) {
        ServiceControlUsage();
        return -1;
    }

    char serviceCtl[SERVICE_CONTROL_MAX_SIZE];
    if (strcmp(argv[0], "start_service") == 0) {
        if (strncpy_s(serviceCtl, sizeof(serviceCtl), "ohos.ctl.start", sizeof(serviceCtl) - 1) != EOK) {
            printf("strncpy_s failed.\n");
            return -1;
        }
    } else if (strcmp(argv[0], "stop_service") == 0) {
        if (strncpy_s(serviceCtl, sizeof(serviceCtl), "ohos.ctl.stop", sizeof(serviceCtl) - 1) != EOK) {
            printf("strncpy_s failed.\n");
            return -1;
        }
    } else {
        ServiceControl(argc, argv);
        return 0;
    }

    if (SystemSetParameter(serviceCtl, argv[1]) != 0) {
        printf("%s service:%s failed.\n", argv[0], argv[1]);
        return -1;
    }
    return 0;
}
