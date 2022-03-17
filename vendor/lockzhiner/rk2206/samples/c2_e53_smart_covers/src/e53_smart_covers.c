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

#include "e53_smart_covers.h"

#define SC_I2C0                                    0


static I2cBusIo m_sc_i2c0m2 = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

/***************************************************************
* 函数名称: e53_sc_io_init
* 说    明: E53_SC初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_io_init()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;

    /*led1 gpio init*/
    LzGpioInit(GPIO0_PA5);
    /*led2 gpio init*/
    LzGpioInit(GPIO1_PD0);

    /*设置GPIO0_PA5为输出模式*/
    ret = LzGpioSetDir(GPIO0_PA5, LZGPIO_DIR_OUT);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("set GPIO0_PA5 Direction fail ret:%d\n", ret);
        return;
    }

    /*设置GPIO1_PD0为输出模式*/
    ret = LzGpioSetDir(GPIO1_PD0, LZGPIO_DIR_OUT);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("set GPIO1_PD0 Direction fail ret:%d\n", ret);
        return;
    }

    if (I2cIoInit(m_sc_i2c0m2) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 io fail\n");
        return;
    }
    /*I2c时钟频率400K*/
    if (LzI2cInit(SC_I2C0, 400000) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 fail\n");
        return;
    }
}

/***************************************************************
* 函数名称: MPU6050_Read_Buffer
* 说    明: I2C读取一段寄存器内容存放到指定的缓冲区
* 参    数:  reg：目标寄存器
*          p_buffer：缓冲区
*          value：值
* 返 回 值: 操作结果
***************************************************************/
uint8_t MPU6050_Read_Buffer(uint8_t reg, uint8_t *p_buffer, uint16_t length)
{
  
    uint32_t status = 0;
    uint8_t  buffer[1] = {reg};

    status = LzI2cWrite(SC_I2C0, MPU6050_SLAVE_ADDRESS, buffer, 1);
    if (status != 0)
    {
        printf("Error: I2C write status:0x%x\n", status);
        return status;
    }

    LzI2cRead(SC_I2C0, MPU6050_SLAVE_ADDRESS, p_buffer, length);
    return 0;  
}

/***************************************************************
* 函数名称: mpu6050_write_reg
* 说    明: 写数据到MPU6050寄存器
* 参    数:  reg：目标寄存器
*          data
* 返 回 值: 无
***************************************************************/
void mpu6050_write_reg(uint8_t reg, uint8_t data)
{
    uint8_t send_data[2] = {reg, data};

    LzI2cWrite(SC_I2C0, MPU6050_SLAVE_ADDRESS, send_data, 2);
}

/***************************************************************
* 函数名称: mpu6050_read_data
* 说    明: 从MPU6050寄存器读取数据
* 参    数:  reg：目标寄存器
*          buf：缓冲区
*          length：长度
* 返 回 值: 无
***************************************************************/
void mpu6050_read_data(uint8_t reg, unsigned char *buf, uint8_t length)
{
    MPU6050_Read_Buffer(reg, buf, length);
}

/***************************************************************
* 函数名称: mpu6050_read_acc
* 说    明: 读取MPU6050的加速度数据
* 参    数:  acc_data：加速度数据
* 返 回 值: 无
***************************************************************/
void mpu6050_read_acc(short *acc_data)
{
    uint8_t buf[6];
    mpu6050_read_data(MPU6050_ACC_OUT, buf, 6);
    acc_data[0] = (buf[0] << 8) | buf[1];
    acc_data[1] = (buf[2] << 8) | buf[3];
    acc_data[2] = (buf[4] << 8) | buf[5];
}

/***************************************************************
* 函数名称: action_interrupt
* 说    明: 运动中断设置
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void action_interrupt()
{
    mpu6050_write_reg(MPU6050_RA_MOT_THR,0x03);//运动阈值
    mpu6050_write_reg(MPU6050_RA_MOT_DUR,0x14);//检测时间20ms 单位1ms
}

/***************************************************************
* 函数名称: mpu6050_init
* 说    明: 初始化MPU6050芯片
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void mpu6050_init()
{
    int i = 0, j = 0;
    /*在初始化之前要延时一段时间，若没有延时，则断电后再上电数据可能会出错*/
    for(i=0;i<1000;i++)
    {
        for(j=0;j<1000;j++)
        {

        }
    }
    mpu6050_write_reg(MPU6050_RA_PWR_MGMT_1, 0X80);  //复位MPU6050
    usleep(20000);
    mpu6050_write_reg(MPU6050_RA_PWR_MGMT_1, 0X00);  //唤醒MPU6050
    mpu6050_write_reg(MPU6050_RA_INT_ENABLE, 0X00);  //关闭所有中断
    mpu6050_write_reg(MPU6050_RA_USER_CTRL, 0X00);   //I2C主模式关闭
    mpu6050_write_reg(MPU6050_RA_FIFO_EN, 0X00);     //关闭FIFO
    mpu6050_write_reg(MPU6050_RA_INT_PIN_CFG, 0X80); //中断的逻辑电平模式,设置为0，中断信号为高电；设置为1，中断信号为低电平时。
    action_interrupt();                              //运动中断
    mpu6050_write_reg(MPU6050_RA_CONFIG, 0x04);      //配置外部引脚采样和DLPF数字低通滤波器
    mpu6050_write_reg(MPU6050_RA_ACCEL_CONFIG, 0x1C);//加速度传感器量程和高通滤波器配置
    mpu6050_write_reg(MPU6050_RA_INT_PIN_CFG, 0X1C); //INT引脚低电平平时
    mpu6050_write_reg(MPU6050_RA_INT_ENABLE, 0x40);  //中断使能寄存器
}

/***************************************************************
  * 函数功能: 读取MPU6050的ID
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  ***************************************************************/ 
uint8_t mpu6050_read_id()
{
    unsigned char buff = 0;
    mpu6050_read_data(MPU6050_RA_WHO_AM_I, &buff, 1);
    if(buff != 0x68)
    {
        printf("MPU6050 dectected error Re:%u\n", buff);
        return 0;
    }
    else
    {
        return 1;
    }
}

/***************************************************************
* 函数名称: e53_sc_init
* 说    明: 初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_init()
{
    e53_sc_io_init();
    mpu6050_init();
    usleep(1000000);
}

/***************************************************************
* 函数名称: e53_sc_read_data
* 说    明: 读取数据
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_read_data(e53_sc_data_t *p_data)
{
    short accel[3];
    short temp;
    if (mpu6050_read_id() == 0)
    {
        while(1);
    }
    mpu6050_read_acc(accel);
    p_data->accel[0] = accel[0];
    p_data->accel[1] = accel[1];
    p_data->accel[2] = accel[2];
    usleep(50000);
}

/***************************************************************
* 函数名称: led_d1_set
* 说    明: LED_D1控制
* 参    数: OFF,关
*          ON,开
* 返 回 值: 无
***************************************************************/
void led_d1_set(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {   
        /*设置GPIO0_PA5输出低电平点亮灯*/
        LzGpioSetVal(GPIO0_PA5, LZGPIO_LEVEL_LOW);
    }
    else if(status == OFF)
    {   
        /*设置GPIO0_PA5输出高电平关闭灯*/
        LzGpioSetVal(GPIO0_PA5, LZGPIO_LEVEL_HIGH);
    }
}

/***************************************************************
* 函数名称: led_d2_set
* 说    明: LED_D2控制
* 参    数: OFF,关
*          ON,开
* 返 回 值: 无
***************************************************************/
void led_d2_set(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {   
        /*设置GPIO1_PD0输出低电平点亮灯*/
        LzGpioSetVal(GPIO1_PD0, LZGPIO_LEVEL_LOW);
    }
    else if(status == OFF)
    {   
        /*设置GPIO1_PD0输出高电平关闭灯*/
        LzGpioSetVal(GPIO1_PD0, LZGPIO_LEVEL_HIGH);
    }
}

