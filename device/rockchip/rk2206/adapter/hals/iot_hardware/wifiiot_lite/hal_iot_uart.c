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


#include "iot_errno.h"
#include "iot_uart.h"
#include "lz_hardware.h"

#define TEST_UART_ID 0

unsigned int IoTUartInit(unsigned int id, const IotUartAttribute *param)
{
    id = TEST_UART_ID;
    return LzUartInit(id, (const UartAttribute *)param);
}

int IoTUartRead(unsigned int id, unsigned char *data, unsigned int dataLen)
{
    id = TEST_UART_ID;
    return LzUartRead(id, data, dataLen);
}

int IoTUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen)
{
    id = TEST_UART_ID;
    return LzUartWrite(id, data, dataLen);
}

unsigned int IoTUartDeinit(unsigned int id)
{
    id = TEST_UART_ID;
    return LzUartDeinit(id);
}

unsigned int IoTUartSetFlowCtrl(unsigned int id, IotFlowCtrl flowCtrl)
{
    id = TEST_UART_ID;
    return LzUartSetFlowCtrl(id, (FlowCtrl)flowCtrl);
}
