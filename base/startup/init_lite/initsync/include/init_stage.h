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

#ifndef BASE_STARTUP_INITLITE_STAGE_H
#define BASE_STARTUP_INITLITE_STAGE_H

#include "init_sync.h"
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
    QS_STAGE1 = 1,   /* 1: start from stage1, 0 is already called in kernel process */
    QS_STAGE2,       /* system init stage No 2 */
    QS_STAGE3,       /* system init stage No 3 */
    QS_STAGE_LIMIT
} QuickstartStage;

typedef enum {
    QS_NOTIFY = QS_STAGE_LIMIT,    /* quickstart notify */
    QS_LISTEN,                     /* quickstart listen */
    QS_CTL_LIMIT
} QuickstartConctrl;

typedef struct {
    unsigned int events;
    unsigned int wait;
} QuickstartListenArgs;

#define QUICKSTART_IOC_MAGIC    'T'
#define QUICKSTART_NOTIFY       _IO(QUICKSTART_IOC_MAGIC, QS_NOTIFY)
#define QUICKSTART_LISTEN       _IOR(QUICKSTART_IOC_MAGIC, QS_LISTEN, QuickstartListenArgs)
#define QUICKSTART_STAGE(x)     _IO(QUICKSTART_IOC_MAGIC, (x))

#define QUICKSTART_NODE         "/dev/quickstart"

/* Simple sample Useage:
 *   INIT PROCESS
 * SystemInitStage(QS_STAGE1)----(1)fork----> key APP
 *      |(2)                                     |(3)
 * InitListen<-------------------(4)---------- NotifyInit
 *      |(5)
 * SystemInitStage(QS_STAGE2)----(6)fork---------------------> other APP
 *      |...
 * InitStageFinished
 */

/*
 * Listen the events of a specific pid process by Init process.
 * Only be called by Init process.
 * eventMask: There needs to be a consensus between the listener and the notifier.
 */
extern int InitListen(unsigned long eventMask, unsigned int wait);

/*
 * Trigger the SystemInit stage.
 * Only be called by Init process.
 */
extern int SystemInitStage(QuickstartStage stage);

/*
 * InitStage finished.
 * Only be called by Init process.
 */
extern int InitStageFinished(void);

/*
 * Listen event and trigger the SystemInit stage.
 * Only be called by Init process.
 */
extern void TriggerStage(unsigned int event, unsigned int wait, QuickstartStage stagelevel);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_STAGE_H
