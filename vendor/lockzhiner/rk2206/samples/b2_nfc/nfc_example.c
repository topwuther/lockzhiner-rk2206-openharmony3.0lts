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

#include "nfc.h"
#include "los_task.h"
#include "ohos_init.h"

#define TEXT        "XiaoZhiPai!"
#define WEB         "fzlzdz.com"

/***************************************************************
* 函数名称: nfc_process
* 说    明: nfc例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void nfc_process(void)
{
    unsigned int ret = 0;

    /* 初始化NFC设备 */
    nfc_init();

    ret = nfc_store_text(NDEFFirstPos, (uint8_t *)TEXT);
    if (ret != 1) {
        printf("NFC Write Text Failed: %d\n", ret);
    }

    ret = nfc_store_uri_http(NDEFLastPos, (uint8_t *)WEB);
    if (ret != 1) {
        printf("NFC Write Url Failed: %d\n", ret);
    }
    
    while (1) {
        printf("==============NFC Example==============\r\n");
        printf("Please use the mobile phone with NFC function close to the development board!\r\n");
        printf("\n\n");
        LOS_Msleep(1000);
    }
}


/***************************************************************
* 函数名称: nfc_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void nfc_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)nfc_process;
    task.uwStackSize = 10240;
    task.pcName = "nfc process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}


APP_FEATURE_INIT(nfc_example);
