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
#ifndef _E53_GESTURE_SENSOR_H_
#define _E53_GESTURE_SENSOR_H_

/* 手势识别效果 */
#define GES_UP                          (0x1 << 0) //向上
#define GES_DOWM                        (0x1 << 1) //向下
#define GES_LEFT                        (0x1 << 2) //向左
#define GES_RIGHT                       (0x1 << 3) //向右
#define GES_FORWARD                     (0x1 << 4) //向前
#define GES_BACKWARD                    (0x1 << 5) //向后
#define GES_CLOCKWISE                   (0x1 << 6) //顺时针
#define GES_COUNT_CLOCKWISE             (0x1 << 7) //逆时针
#define GES_WAVE                        (0x1 << 8) //挥动

/***************************************************************
 * 函数名称: e53_gs_init
 * 说    明: 手势感应模块初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int e53_gs_init();

/***************************************************************
 * 函数名称: gs_led_up_set
 * 说    明: 手势感应模块向上LED设置
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_up_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_down_set
 * 说    明: 手势感应模块向下LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_down_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_left_set
 * 说    明: 手势感应模块向左LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_left_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_right_set
 * 说    明: 手势感应模块向右LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_right_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_forward_set
 * 说    明: 手势感应模块向前LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_forward_set(unsigned char is_on);

/***************************************************************
 * 函数名称: gesture_sensor_led_backward_set
 * 说    明: 手势感应模块向后LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_backward_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_cw_set
 * 说    明: 手势感应模块顺时针转圈LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_cw_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_ccw_set
 * 说    明: 手势感应模块逆时针转圈LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_ccw_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_led_wave_set
 * 说    明: 手势感应模块挥手LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_wave_set(unsigned char is_on);

/***************************************************************
 * 函数名称: e53_gs_get_gesture_state
 * 说    明: 获取手势感应模块手势
 * 参    数: 
 *      @flag：获取当前手势
 * 返 回 值: 1为成功，0为失败
 ***************************************************************/
unsigned int e53_gs_get_gesture_state(unsigned short *flag);

#endif