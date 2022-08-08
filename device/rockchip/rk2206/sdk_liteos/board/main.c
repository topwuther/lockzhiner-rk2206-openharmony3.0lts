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

#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_compiler.h"
#include "lz_hardware.h"
#include "config_network.h"

#define  MAIN_TAG              "MAIN"
int DeviceManagerStart();
void IotInit(void);

/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT int Main(void)
{
    int ret;
    LZ_HARDWARE_LOGD(MAIN_TAG, "%s: enter ...", __func__);
    
    HalInit();

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        OHOS_SystemInit();
        IotInit();
        /* 开启驱动管理服务 */
        //DeviceManagerStart();
        //ExternalTaskConfigNetwork();
        LZ_HARDWARE_LOGD(MAIN_TAG, "%s: LOS_Start ...", __func__);
        LOS_Start();
    }

    while (1) {
        __asm volatile("wfi");
    }
}

