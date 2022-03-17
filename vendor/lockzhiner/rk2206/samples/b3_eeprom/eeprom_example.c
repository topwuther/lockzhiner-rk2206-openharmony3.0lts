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
#include <stdio.h>
#include "los_task.h"
#include "ohos_init.h"
#include "eeprom.h"

void eeprom_proress(void *arg)
{
#define FOR_CHAR            30
#define FOR_ADDRESS         32
#define CHAR_START          0x21
#define CHAR_END            0x7F
    unsigned int ret = 0;
    unsigned char data_offset = CHAR_START;
    unsigned int addr_offset = 3;
    unsigned char data;
    unsigned char buffer[FOR_CHAR];
    
    eeprom_init();
    
    while (1)
    {
        printf("************ Eeprom Process ************\n");
        printf("BlockSize = 0x%x\n", eeprom_get_blocksize());
        
        /* 写EEPROM */
        memset(buffer, 0, sizeof(buffer));
        for (unsigned int i = 0; i < FOR_CHAR; i++)
        {
            buffer[i] = data_offset + i;
            printf("Write Byte: %d = %c\n", addr_offset + i, buffer[i]);
        }
        ret = eeprom_write(addr_offset, buffer, FOR_CHAR);
        if (ret != FOR_CHAR)
        {
            printf("EepromWrite failed(%d)\n", ret);
        }
        
        /* 读EEPROM */
        memset(buffer, 0, sizeof(buffer));
        ret = eeprom_read(addr_offset, buffer, FOR_CHAR);
        if (ret != FOR_CHAR)
        {
            printf("Read Bytes: failed!\n");
        }
        else
        {
            for (unsigned int i = 0; i < FOR_CHAR; i++)
            {
                printf("Read Byte: %d = %c\n", addr_offset + i, buffer[i]);
            }
        }
        
        data_offset++;
        if (data_offset >= CHAR_END)
        {
            data_offset = CHAR_START;
        }
        
        addr_offset += FOR_ADDRESS;
        if (addr_offset >= 200)
        {
            addr_offset = 0;
        }
        printf("\n");
        
        LOS_Msleep(5000);
    }
}

void eeprom_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)eeprom_proress;
    task.uwStackSize = 2048;
    task.pcName = "eeprom process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(eeprom_example);


