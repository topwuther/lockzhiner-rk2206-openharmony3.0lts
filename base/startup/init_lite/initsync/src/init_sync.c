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

#include "init_stage.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "init_log.h"
static int SendCmd(int cmd, unsigned long arg)
{
    int fd = open(QUICKSTART_NODE, O_RDONLY);
    if (fd != -1) {
        int ret = ioctl(fd, cmd, arg);
        if (ret == -1) {
            INIT_LOGE("[Init] [ERR] %d!", errno);
        }
        close(fd);
        return ret;
    }
    INIT_LOGE("[Init] [ERR] %d!", errno);
    return fd;
}

int InitListen(unsigned long eventMask, unsigned int wait)
{
    QuickstartListenArgs args;
    args.wait = wait;
    args.events = eventMask;
    return SendCmd(QUICKSTART_LISTEN, (uintptr_t)&args);
}

int NotifyInit(unsigned long event)
{
    return SendCmd(QUICKSTART_NOTIFY, event);
}

int SystemInitStage(QuickstartStage stage)
{
    if (stage >= QS_STAGE_LIMIT || stage < QS_STAGE1) {
        INIT_LOGE("[Init] the stage(%d) is not expected!", stage);
        return -1;
    }
    return SendCmd(QUICKSTART_STAGE(stage), 0);
}

void TriggerStage(unsigned int event, unsigned int wait, QuickstartStage stagelevel)
{
    int ret = InitListen(event, wait);
    if (ret != 0) {
        SystemInitStage(stagelevel);
    }
}

int InitStageFinished(void)
{
    return unlink(QUICKSTART_NODE);
}
