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
#include "picture.h"
#include "lcd.h"

extern const unsigned char gImage_lingzhi[IMAGE_MAXSIZE_LINGZHI];


/***************************************************************
* 函数名称: lcd_process
* 说    明: lcd例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void lcd_process(void *arg)
{
    uint32_t ret = 0;
    float t = 0;
    uint8_t chinese_string[] = "小凌派";
    uint8_t cur_sizey = 12;
    
    ret = lcd_init();
    if (ret != 0)
    {
        printf("lcd_init failed(%d)\n", ret);
        return;
    }
    
    lcd_fill(0, 0, LCD_W, LCD_H, LCD_WHITE);
    
    while (1)
    {
        printf("************Lcd Example***********\n");
        lcd_show_picture(15, 0, 210, 62, &gImage_lingzhi[0]);
        lcd_show_string(0, 100, "Welcome to XiaoLingPai!", LCD_RED, LCD_WHITE, 16, 0);
        lcd_show_string(0, 130, "URL: http://www.fzlzda.com", LCD_RED, LCD_WHITE, 16, 0);
        lcd_show_string(0, 160, "LCD_W:", LCD_BLUE, LCD_WHITE, 16, 0);
        lcd_show_int_num(48, 160, LCD_W, 3, LCD_BLUE, LCD_WHITE, 16);
        lcd_show_string(80, 160, "LCD_H:", LCD_BLUE, LCD_WHITE, 16, 0);
        lcd_show_int_num(128, 160, LCD_H, 3, LCD_BLUE, LCD_WHITE, 16);
        lcd_show_string(80, 160, "LCD_H:", LCD_BLUE, LCD_WHITE, 16, 0);
        lcd_show_string(0, 190, "Increaseing Num:", LCD_BLACK, LCD_WHITE, 16, 0);
        lcd_show_float_num1(128, 190, t, 4, LCD_BLACK, LCD_WHITE, 16);
        t += 0.11;

        lcd_fill(0, 220, LCD_W, LCD_H, LCD_WHITE);
        lcd_show_chinese(0, 220, chinese_string, LCD_RED, LCD_WHITE, cur_sizey, 0);
        if (cur_sizey == 12)
            cur_sizey = 16;
        else if(cur_sizey == 16)
            cur_sizey = 24;
        else if(cur_sizey == 24)
            cur_sizey = 32;
        else
            cur_sizey = 12;

        printf("\n");
        LOS_Msleep(1000);
    }
}


/***************************************************************
* 函数名称: lcd_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void lcd_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)lcd_process;
    task.uwStackSize = 20480;
    task.pcName = "lcd process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}


APP_FEATURE_INIT(lcd_example);
