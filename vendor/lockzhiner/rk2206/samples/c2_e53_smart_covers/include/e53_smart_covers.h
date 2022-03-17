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

#ifndef __E53_SMART_CITY_H__
#define __E53_SMART_CITY_H__

#include "lz_hardware.h"

typedef struct
{
    short   accel[3];
    unsigned int tilt_status;
} e53_sc_data_t;

typedef enum
{
    OFF = 0,
    ON
} SWITCH_STATUS_ENUM;

#define DELTA                       100

#define MPU6050_GYRO_OUT            0x43  //MPU6050陀螺仪数据寄存器地址
#define MPU6050_ACC_OUT             0x3B  //MPU6050加速度数据寄存器地址
#define MPU6050_SLAVE_ADDRESS       0x68  //MPU6050器件读地址
#define MPU6050_ADDRESS_AD0_LOW     0x68  //address pin low (GND), default for InvenSense evaluation board
#define MPU6050_RA_CONFIG           0x1A
#define MPU6050_RA_ACCEL_CONFIG     0x1C
#define MPU6050_RA_FF_THR           0x1D
#define MPU6050_RA_FF_DUR           0x1E
#define MPU6050_RA_MOT_THR          0x1F  //运动检测阀值设置寄存器
#define MPU6050_RA_MOT_DUR          0x20  //运动检测时间阀值
#define MPU6050_RA_ZRMOT_THR        0x21
#define MPU6050_RA_ZRMOT_DUR        0x22
#define MPU6050_RA_FIFO_EN          0x23
#define MPU6050_RA_INT_PIN_CFG      0x37   //中断/旁路设置寄存器
#define MPU6050_RA_INT_ENABLE       0x38   //中断使能寄存器
#define MPU6050_RA_TEMP_OUT_H       0x41
#define MPU6050_RA_USER_CTRL        0x6A
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_WHO_AM_I         0x75

void e53_sc_init();
void e53_sc_read_data(e53_sc_data_t *p_data);
void led_d1_set(SWITCH_STATUS_ENUM status);
void led_d2_set(SWITCH_STATUS_ENUM status);

#endif/*__E53_SMART_CITY_H__*/

