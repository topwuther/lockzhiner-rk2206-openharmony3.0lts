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
#include "dirent.h"


#define FILE_NAME1          "test.txt"
#define FILE_NAME2          "helloworld.c"
#define ENTRY_MAXSIZE       32
#define DIR_PATH            "./"

/***************************************************************
 * 函数名称: HalFileReadDir
 * 说    明: 列出某目录下所有文件和文件夹
 * 参    数: 
 *      @path           要求列表的目录
 *      @entry          struct dirent数组
 *      @entry_max      entry数组有多少个struct dirent
 *      @entry_length   当前目录下所有的文件和文件夹的数量
 * 返 回 值: 0为成功，反之为失败
***************************************************************/
extern int HalFileReadDir(const char *path, struct dirent *entry, int entry_max, int *entry_length);

/***************************************************************
* 函数名称: hal_dir_process
* 说    明: 目录列出线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void hal_dir_process()
{
    int fd;
    struct dirent entry[ENTRY_MAXSIZE];
    int entry_length = 0;
    int ret;
    int i;

    // 创建第1个文件
    fd = HalFileOpen(FILE_NAME1, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1) {
        printf("%s HalFileOpen failed!\n", FILE_NAME1);
        return;
    }
    HalFileClose(fd);

    // 创建第2个文件
    fd = HalFileOpen(FILE_NAME2, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1) {
        printf("%s HalFileOpen failed!\n", FILE_NAME2);
        return;
    }
    HalFileClose(fd);

    while (1) {
        memset(entry, 0, sizeof(entry));
        entry_length= 0;

        // 读取当前目录下所有的文件或文件夹
        ret = HalFileReadDir(DIR_PATH, &entry[0], ENTRY_MAXSIZE, &entry_length);
        if (ret != 0) {
            printf("%s, %d: HalFileReadDir failed(%d)\n", __FILE__, __LINE__, ret);
        } else {
            printf("entry_length = %d\n", entry_length);
            for (i = 0; i < entry_length; i++) {
                switch (entry[i].d_type) {
                    case DT_BLK:
                        printf("i(%d), name(%s), type(%d)(block device)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_CHR:
                        printf("i(%d), name(%s), type(%d)(character device)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_DIR:
                        printf("i(%d), name(%s), type(%d)(directory)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_FIFO:
                        printf("i(%d), name(%s), type(%d)(pipe or fifo)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_LNK:
                        printf("i(%d), name(%s), type(%d)(symbolic link)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_REG:
                        printf("i(%d), name(%s), type(%d)(regular file)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_SOCK:
                        printf("i(%d), name(%s), type(%d)(UNIX domain socket)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    default:
                        printf("i(%d), name(%s), type(%d)(could not be define)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                }
            }
        }
        
        printf("\n");
        LOS_Msleep(5000);
    }
}

/***************************************************************
* 函数名称: dir_example
* 说    明: 目录入口函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void dir_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;
    
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_dir_process;
    task.uwStackSize = 1024 * 1024;
    task.pcName = "hal_dir_thread";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK) {
        printf("Falied to create hal_file_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(dir_example);
