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
#include "utils_file.h"
#include "hal_file.h"

#define FILE_NAME           "test.txt"

/***************************************************************
* 函数名称: hal_file_thread
* 说    明: 文件读写线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void hal_file_thread()
{
    int fd;
    char buffer[1024];
    int read_length, write_length;
    int current = 0;

    /* 打开文件，如果没有该文件就创建，如有该文件则打开
     * O_TRUNC_FS => 清空文件内容
     */
    //fd = HalFileOpen(FILE_NAME, O_RDWR_FS | O_CREAT_FS, 0);
    fd = HalFileOpen(FILE_NAME, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1)
    {
        printf("%s HalFileOpen failed!\n", FILE_NAME);
        return;
    }

    while (1)
    {
        /* 文件位置移动到文件开始位置 */
        HalFileSeek(fd, 0, SEEK_SET);
        memset(buffer, 0, sizeof(buffer));
        /* 读取文件内容 */
        read_length = HalFileRead(fd, buffer, sizeof(buffer));
        printf("read: \n");
        printf("    length = %d\n", read_length);
        printf("    content = %s\n", buffer);

        /* 文件位置移动到文件开始位置 */
        HalFileSeek(fd, 0, SEEK_SET);
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Hello World(%d) => ", current);
        /* 写入文件 */
        write_length = HalFileWrite(fd, buffer, strlen(buffer));

        current++;
        LOS_Msleep(5000);
    }

    HalFileClose(fd);
}

/***************************************************************
* 函数名称: file_example
* 说    明: 文件读写入口函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void file_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_file_thread;
    task.uwStackSize = 1024 * 10;
    task.pcName = "hal_file_thread";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create hal_file_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(file_example);
