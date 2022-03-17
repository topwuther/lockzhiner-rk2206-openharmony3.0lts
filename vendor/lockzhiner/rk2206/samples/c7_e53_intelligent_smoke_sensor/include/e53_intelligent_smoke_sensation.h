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
#ifndef __E53_ISS_H__
#define __E53_ISS_H__


/***************************************************************
* 函数名称: e53_iss_status_e
* 说    明：枚举状态结构体
***************************************************************/
typedef enum
{
    OFF = 0,
    ON
} e53_iss_status_e;

/***************************************************************
* 函数名称: e53_iss_init
* 说    明: 初始化E53_Iss扩展板
* 参    数: 无
* 返 回 值: 0 成功，反之失败
***************************************************************/
uint32_t e53_iss_init(void);

/***************************************************************
* 函数名称: e53_iss_mq2_ppm_calibration
* 说    明: 传感器校准函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_iss_mq2_ppm_calibration(void);

/***************************************************************
* 函数名称: e53_iss_get_mq2_ppm
* 说    明: 获取PPM函数
* 参    数: 无
* 返 回 值: ppm
***************************************************************/
float e53_iss_get_mq2_ppm(void);

/***************************************************************
* 函数名称: e53_iss_get_mq2_alarm_value
* 说    明: 获取ppm报警阈值函数
* 参    数: 无
* 返 回 值: 报警阈值
***************************************************************/
uint16_t e53_iss_get_mq2_alarm_value(void);

/***************************************************************
* 函数名称: e53_iss_beep_status_set
* 说    明: 蜂鸣器报警与否
* 参    数: status,ENUM枚举的数据
*           OFF,关蜂鸣器
*           ON,开蜂鸣器
* 返 回 值: 无
***************************************************************/
void e53_iss_beep_status_set(e53_iss_status_e status);

/***************************************************************
* 函数名称: e53_iss_led_status_set
* 说    明: LED报警与否
* 参    数: status,ENUM枚举的数据
*           OFF,关led
*           ON,开led
* 返 回 值: 无
***************************************************************/
void e53_iss_led_status_set(e53_iss_status_e status);


#endif /* __E53_ISS_H__ */

