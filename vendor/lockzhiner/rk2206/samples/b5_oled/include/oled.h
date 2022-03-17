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
#ifndef _OLED_H_
#define _OLED_H_

/* 定义OLED的行列数目 */
#define OLED_COLUMN_MAX         128
#define OLED_ROW_MAX            64

/***************************************************************
 * 函数名称: oled_init
 * 说    明: oled初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int oled_init();


/***************************************************************
 * 函数名称: oled_deinit
 * 说    明: oled销毁
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int oled_deinit();


/***************************************************************
 * 函数名称: oled_display_on
 * 说    明: oled显示开启
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_display_on(void);


/***************************************************************
 * 函数名称: oled_display_off
 * 说    明: oled显示关闭
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_display_off(void);


/***************************************************************
 * 函数名称: oled_clear
 * 说    明: oled清空
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void oled_clear(void);


/***************************************************************
 * 函数名称: oled_show_char
 * 说    明: oled显示字符
 * 参    数: 
 *      @x：字符的X轴坐标
 *      @y：字符的Y轴坐标
 *      @chr：字符
 *      @chr_size：字符的字体，包括12/16两种字体
 * 返 回 值: 无
 ***************************************************************/
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t chr_size);


/***************************************************************
 * 函数名称: oled_show_num
 * 说    明: oled显示数字
 * 参    数: 
 *      @x：数字的X轴坐标
 *      @y：数字的Y轴坐标
 *      @num：数字
 *      @len：数字的位数
 *      @size：字体大小
 * 返 回 值: 无
 ***************************************************************/
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);


/***************************************************************
 * 函数名称: oled_show_string
 * 说    明: oled显示字符串
 * 参    数: 
 *      @x：字符串的X轴坐标
 *      @y：字符串的Y轴坐标
 *      @p：字符串
 *      @chr_size：字符串的位数
 * 返 回 值: 无
 ***************************************************************/
void oled_show_string(uint8_t x, uint8_t y, uint8_t *p, uint8_t chr_size);


/***************************************************************
 * 函数名称: oled_draw_bmp
 * 说    明: oled显示图片
 * 参    数: 
 *      @x0：图片的起始点X轴坐标，取值为0~127
 *      @y0：图片的起始点Y轴坐标，取值为0~63
 *      @x1：图片的结束点X轴坐标，取值为0~127
 *      @y1：图片的结束点Y轴坐标，取值为0~63
 *      @bmp：图片
 * 返 回 值: 无
 ***************************************************************/
void oled_draw_bmp(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char bmp[]);


#endif
/** @} */
