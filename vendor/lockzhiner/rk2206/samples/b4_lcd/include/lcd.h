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
#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

/* 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏 */
#define USE_HORIZONTAL      0

/* 根据LCD是横屏或者竖屏，设置LCD的宽度和高度 */
#if ((USE_HORIZONTAL==0) || (USE_HORIZONTAL==1))
#define LCD_W 240
#define LCD_H 320
#else
#define LCD_W 320
#define LCD_H 240
#endif

/* 画笔颜色 */
#define LCD_WHITE           0xFFFF
#define LCD_BLACK           0x0000
#define LCD_BLUE            0x001F
#define LCD_BRED            0XF81F
#define LCD_GRED            0XFFE0
#define LCD_GBLUE           0X07FF
#define LCD_RED             0xF800
#define LCD_MAGENTA         0xF81F
#define LCD_GREEN           0x07E0
#define LCD_CYAN            0x7FFF
#define LCD_YELLOW          0xFFE0
#define LCD_BROWN           0XBC40 //棕色
#define LCD_BRRED           0XFC07 //棕红色
#define LCD_GRAY            0X8430 //灰色
#define LCD_DARKBLUE        0X01CF //深蓝色
#define LCD_LIGHTBLUE       0X7D7C //浅蓝色
#define LCD_GRAYBLUE        0X5458 //灰蓝色
#define LCD_LIGHTGREEN      0X841F //浅绿色
#define LCD_LGRAY           0XC618 //浅灰色(PANNEL),窗体背景色
#define LCD_LGRAYBLUE       0XA651 //浅灰蓝色(中间层颜色)
#define LCD_LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)


/***************************************************************
 * 函数名称: lcd_init
 * 说    明: Lcd初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int lcd_init();


/***************************************************************
 * 函数名称: lcd_deinit
 * 说    明: Lcd注销
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int lcd_deinit();


/***************************************************************
 * 函数名称: lcd_fill
 * 说    明: 指定区域填充颜色
 * 参    数:
 *       @xsta：指定区域的起始点X坐标
 *       @ysta：指定区域的起始点Y坐标
 *       @xend：指定区域的结束点X坐标
 *       @yend：指定区域的结束点Y坐标
 *       @color：指定区域的颜色
 * 返 回 值: 无
 ***************************************************************/
void lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);


/***************************************************************
 * 函数名称: lcd_draw_point
 * 说    明: 指定位置画一个点
 * 参    数:
 *       @x：指定点的X坐标
 *       @y：指定点的Y坐标
 *       @color：指定点的颜色
 * 返 回 值: 无
 ***************************************************************/
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);


/***************************************************************
 * 函数名称: lcd_draw_line
 * 说    明: 指定位置画一条线
 * 参    数:
 *       @x1：指定线的起始点X坐标
 *       @y1：指定线的起始点Y坐标
 *       @x2：指定线的结束点X坐标
 *       @y2：指定线的结束点Y坐标
 *       @color：指定点的颜色
 * 返 回 值: 无
 ***************************************************************/
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);


/***************************************************************
 * 函数名称: lcd_draw_rectangle
 * 说    明: 指定位置画矩形
 * 参    数:
 *       @x1：指定矩形的起始点X坐标
 *       @y1：指定矩形的起始点Y坐标
 *       @x2：指定矩形的结束点X坐标
 *       @y2：指定矩形的结束点Y坐标
 *       @color：指定点的颜色
 * 返 回 值: 无
 ***************************************************************/
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);


/***************************************************************
 * 函数名称: lcd_draw_circle
 * 说    明: 指定位置画圆
 * 参    数:
 *       @x0：指定圆的中心点X坐标
 *       @y0：指定圆的中心点Y坐标
 *       @r：指定圆的半径
 *       @color：指定点的颜色
 * 返 回 值: 无
 ***************************************************************/
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);


/***************************************************************
 * 函数名称: lcd_show_chinese
 * 说    明: 显示汉字串
 * 参    数:
 *       @x：指定汉字串的起始位置X坐标
 *       @y：指定汉字串的起始位置X坐标
 *       @s：指定汉字串（该汉字串为utf-8）
 *       @fc: 字的颜色
 *       @bc: 字的背景色
 *       @sizey: 字号，可选：12、16、24、32
 *       @mode: 0为非叠加模式；1为叠加模式
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_chinese(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);


/***************************************************************
 * 函数名称: lcd_show_char
 * 说    明: 显示一个字符
 * 参    数:
 *       @x：指定字符的起始位置X坐标
 *       @y：指定字串的起始位置X坐标
 *       @s：指定字串
 *       @fc: 字的颜色
 *       @bc: 字的背景色
 *       @sizey: 字号，可选：12、16、24、32
 *       @mode: 0为非叠加模式；1为叠加模式
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_char(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);


/***************************************************************
 * 函数名称: lcd_show_string
 * 说    明: 显示字符串
 * 参    数:
 *       @x：指定字符的起始位置X坐标
 *       @y：指定字串的起始位置X坐标
 *       @s：指定字串
 *       @fc: 字的颜色
 *       @bc: 字的背景色
 *       @sizey: 字号，可选：16、24、32
 *       @mode: 0为非叠加模式；1为叠加模式
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_string(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);


/***************************************************************
 * 函数名称: lcd_show_int_num
 * 说    明: 显示整数变量
 * 参    数:
 *       @x：指定整数变量的起始位置X坐标
 *       @y：指定整数变量的起始位置X坐标
 *       @num：指定整数变量
 *       @fc: 整数变量的颜色
 *       @bc: 整数变量的背景色
 *       @sizey: 字号，可选：16、24、32
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_int_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);


/***************************************************************
 * 函数名称: lcd_show_float_num1
 * 说    明: 显示两位小数变量
 * 参    数:
 *       @x：指定浮点变量的起始位置X坐标
 *       @y：指定浮点变量的起始位置X坐标
 *       @num：指定浮点变量
 *       @fc: 浮点变量的颜色
 *       @bc: 浮点变量的背景色
 *       @sizey: 字号，可选：16、24、32
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_float_num1(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);


/***************************************************************
 * 函数名称: lcd_show_picture
 * 说    明: 显示图片
 * 参    数:
 *       @x：指定图片的起始位置X坐标
 *       @y：指定图片的起始位置X坐标
 *       @length：指定图片的长度
 *       @width：指定图片的宽度
 *       @pic：指定图片的内容
 * 返 回 值: 无
 ***************************************************************/
void lcd_show_picture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t *pic);


#endif /* _LCD_H_ */
