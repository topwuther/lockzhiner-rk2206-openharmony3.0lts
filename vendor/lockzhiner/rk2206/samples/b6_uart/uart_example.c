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
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"

/* E53模块中
 * UART0_RX_M0 -> GPIO0_B6
 * UART0_TX_M0 -> GPIO0_B7
 */
#define UART_ID                 0

#define STRING_MAXSIZE          128

void uart_process(void)
{
    unsigned int ret;
    UartAttribute attr;
    unsigned char str[] = "HelloWorld!";
    unsigned char recv_buffer[STRING_MAXSIZE];
    unsigned int recv_length = 0;
    unsigned int i;

    LzUartDeinit(UART_ID);
    
    attr.baudRate = 115200;
    attr.dataBits = UART_DATA_BIT_8;
    attr.pad = FLOW_CTRL_NONE;
    attr.parity = UART_PARITY_NONE;
    attr.rxBlock = UART_BLOCK_STATE_NONE_BLOCK;
    attr.stopBits = UART_STOP_BIT_1;
    attr.txBlock = UART_BLOCK_STATE_NONE_BLOCK;
    
    PinctrlSet(GPIO0_PB6, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);
    PinctrlSet(GPIO0_PB7, MUX_FUNC2, PULL_KEEP, DRIVE_LEVEL2);
    
    ret = LzUartInit(UART_ID, &attr);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %d: LzUartInit(%d) failed!\n", __FILE__, __LINE__, ret);
        return;
    }
    LOS_Msleep(1000);

    while (1)
    {
        printf("%s, %d: uart write and str(%s), len(%d)!\n", __FILE__, __LINE__, str, strlen(str));
        // LzUartWrite是异步发送，非阻塞发送
        LzUartWrite(UART_ID, str, strlen(str));
        // 等待发送完毕
        LOS_Msleep(1000);

        recv_length = 0;
        memset(recv_buffer, 0, sizeof(recv_buffer));
        recv_length = LzUartRead(UART_ID, recv_buffer, sizeof(recv_buffer));
        printf("%s, %d: uart recv and str(%s), len(%d)\n", __FILE__, __LINE__, recv_buffer, recv_length);
        printf("");

        LOS_Msleep(2000);
    }
    
    return;
}

/***************************************************************
* 函数名称: uart_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void uart_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)uart_process;
    task.uwStackSize = 1024 * 1024;
    task.pcName = "uart process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(uart_example);
