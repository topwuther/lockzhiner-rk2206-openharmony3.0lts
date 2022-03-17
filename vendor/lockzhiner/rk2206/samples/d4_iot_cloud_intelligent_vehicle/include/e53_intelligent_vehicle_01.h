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
#ifndef _E53_INTELLIGENT_VEHICLE_01_H_
#define _E53_INTELLIGENT_VEHICLE_01_H_

/***************************************************************
 * 函数名称: e53_iv01_init
 * 说    明: intelligent_vehicle01驱动初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int e53_iv01_init();


/***************************************************************
 * 函数名称: e53_iv01_deinit
 * 说    明: 智慧车载驱动销毁
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_deinit();


/***************************************************************
 * 函数名称: e53_iv01_get_distance
 * 说    明: 智慧车载发起1次超声波测距
 * 参    数: 
 *      @distance_meter：测距的距离，单位为厘米
 * 返 回 值: 返回1为成功，0为失败
 ***************************************************************/
unsigned int e53_iv01_get_distance(float *distance_cm);


/***************************************************************
 * 函数名称: e53_iv01_led_warning_set
 * 说    明: 智慧车载的Led灯控制
 * 参    数: 
 *      @is_on：Led灯控制，0为灭，1为亮
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_led_warning_set(unsigned char is_on);


/***************************************************************
 * 函数名称: e53_iv01_buzzer_set
 * 说    明: 智慧车载的PWM控制
 * 参    数: 
 *      @is_on：PWM控制，0为关闭，1为开启
 *      @duty_ns：高电平的占比，以nsec为单位
 *      @cycle_ns：PWM周期，以nsec为单位
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_buzzer_set(unsigned char is_on, unsigned int duty_ns, unsigned int cycle_ns);

#endif