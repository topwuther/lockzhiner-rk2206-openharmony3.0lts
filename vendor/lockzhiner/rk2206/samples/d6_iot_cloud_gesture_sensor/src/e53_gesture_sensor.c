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
#include <stdint.h>
    
#include "los_sem.h"
#include "lz_hardware.h"

/* LED对应RK2206芯片的GPIO引脚 */
#define GPIO_LED_UP         GPIO0_PB1
#define GPIO_LED_DOWN       GPIO0_PB0
#define GPIO_LED_LEFT       GPIO0_PA2
#define GPIO_LED_RIGHT      GPIO0_PC4
#define GPIO_LED_FORWARD    GPIO0_PB4
#define GPIO_LED_BACKWARD   GPIO0_PB7
#define GPIO_LED_CW         GPIO0_PB6
#define GPIO_LED_CCW        GPIO0_PB5
#define GPIO_LED_WAVE       GPIO0_PB2

#define E53_I2C_BUS         0
static I2cBusIo m_i2cBus = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};
static unsigned int m_i2c_freq = 400000;

/* PAJ7620U2 从设备地址 */
#define PAJ7620U2_I2C_SLAVE_ADDRESS         0x73
/* PAJ7620U2的寄存器Bank */
typedef enum {
    BANK0 = 0,  /* Bank0寄存器 */
    BANK1,      /* Bank1寄存器 */
}BankId;

/* BANK的寄存器地址和BANK0~1 */
#define PAJ_REG_BANK_SEL                0xEF //BANK选择寄存器
#define PAJ_REB_BANK_SEL_BANK0          0x00 //BANK0
#define PAJ_REB_BANK_SEL_BANK1          0x01 //BANK1

/* BANK0 寄存器组 */
#define PAJ_REG_SUSPEND_CMD             0x03 //设置设备挂起
#define PAJ_REG_SET_INT_FLAG1           0x41 //设置手势检测中断寄存器1
#define PAJ_REG_SET_INT_FLAG2           0x42 //设置手势检测中断寄存器2
#define PAJ_REG_GET_INT_FLAG1           0x43 //获取手势检测中断标志寄存器1(获取手势结果)
#define PAJ_REG_GET_INT_FLAG2           0x44 //获取手势检测中断标志寄存器2(获取手势结果)
#define PAJ_REG_GET_STATE               0x45 //获取手势检测工作状态
#define PAJ_REG_SET_HIGH_THRESHOLD      0x69 //设置滞后高阀值（仅在接近检测模式下）
#define PAJ_REG_SET_LOW_THRESEHOLD      0x6A //设置滞后低阀值
#define PAJ_REG_GET_APPROACH_STATE      0x6B //获取接近状态 （1：PS data>= PS threshold ,0:PS data<= Low threshold）
#define PAJ_REG_GET_GESTURE_DATA        0x6C //获取接近数据
#define PAJ_REG_GET_OBJECT_BRIGHTNESS   0xB0 //获取被照物体亮度（最大255）
#define PAJ_REG_GET_OBJECT_SIZE_1       0xB1 //获取被照物体大小低八位（bit7:0）(最大900)
#define PAJ_REG_GET_OBJECT_SIZE_2       0xB2 //获取被照物体大小高四位（bit3:0）

/* BANK1 寄存器组 */
#define PAJ_REG_SET_PS_GAIN             0x44 //设置检测增益大小 (0:1x gain 1:2x gain)
#define PAJ_REG_SET_IDLE_S1_STEP_0      0x67 //设置S1的响应因子
#define PAJ_REG_SET_IDLE_S1_STEP_1      0x68 
#define PAJ_REG_SET_IDLE_S2_STEP_0      0x69 //设置S2的响应因子
#define PAJ_REG_SET_IDLE_S2_STEP_1      0x6A 
#define PAJ_REG_SET_OP_TO_S1_STEP_0     0x6B //设置OP到S1的过度时间
#define PAJ_REG_SET_OP_TO_S1_STEP_1     0x6C
#define PAJ_REG_SET_S1_TO_S2_STEP_0     0x6D //设置S1到S2的过度时间
#define PAJ_REG_SET_S1_TO_S2_STEP_1     0x6E
#define PAJ_REG_OPERATION_ENABLE        0x72 //设置PAJ7620U2使能寄存器

static uint8_t m_Paj7620u2_InitRegisterConfig[][2] = {
    {0xEF,0x00}, //切换bank0
    {0x37,0x07}, //
    {0x38,0x17},
    {0x39,0x06},
    {0x41,0x00},
    {0x42,0x00},
    {0x46,0x2D},
    {0x47,0x0F},
    {0x48,0x3C},
    {0x49,0x00},
    {0x4A,0x1E},
    {0x4C,0x20},
    {0x51,0x10},
    {0x5E,0x10},
    {0x60,0x27},
    {0x80,0x42},
    {0x81,0x44},
    {0x82,0x04},
    {0x8B,0x01},
    {0x90,0x06},
    {0x95,0x0A},
    {0x96,0x0C},
    {0x97,0x05},
    {0x9A,0x14},
    {0x9C,0x3F},
    {0xA5,0x19},
    {0xCC,0x19},
    {0xCD,0x0B},
    {0xCE,0x13},
    {0xCF,0x64},
    {0xD0,0x21},
    {0xEF,0x01},
    {0x02,0x0F},
    {0x03,0x10},
    {0x04,0x02},
    {0x25,0x01},
    {0x27,0x39},
    {0x28,0x7F},
    {0x29,0x08},
    {0x3E,0xFF},
    {0x5E,0x3D},
    {0x65,0x96},
    {0x67,0x97},
    {0x69,0xCD},
    {0x6A,0x01},
    {0x6D,0x2C},
    {0x6E,0x01},
    {0x72,0x01},
    {0x73,0x35},
    {0x74,0x00},
    {0x77,0x01},
};

static uint8_t m_Paj7620u2_SetPSModeConfig[][2] = {
    {0xEF,0x00},
    {0x41,0x00},
    {0x42,0x02},
    {0x48,0x20},
    {0x49,0x00},
    {0x51,0x13},
    {0x83,0x00},
    {0x9F,0xF8},
    {0x69,0x96},
    {0x6A,0x02},
    {0xEF,0x01},
    {0x01,0x1E},
    {0x02,0x0F},
    {0x03,0x10},
    {0x04,0x02},
    {0x41,0x50},
    {0x43,0x34},
    {0x65,0xCE},
    {0x66,0x0B},
    {0x67,0xCE},
    {0x68,0x0B},
    {0x69,0xE9},
    {0x6A,0x05},
    {0x6B,0x50},
    {0x6C,0xC3},
    {0x6D,0x50},
    {0x6E,0xC3},
    {0x74,0x05},
};

static uint8_t m_Paj7620u2_SetGestureModeConfig[][2] = {
    {0xEF,0x00},
    {0x41,0x00},
    {0x42,0x00},
    {0xEF,0x00},
    {0x48,0x3C},
    {0x49,0x00},
    {0x51,0x10},
    {0x83,0x20},
    {0x9F,0xF9},
    {0xEF,0x01},
    {0x01,0x1E},
    {0x02,0x0F},
    {0x03,0x10},
    {0x04,0x02},
    {0x41,0x40},
    {0x43,0x30},
    {0x65,0x96},
    {0x66,0x00},
    {0x67,0x97},
    {0x68,0x01},
    {0x69,0xCD},
    {0x6A,0x01},
    {0x6B,0xB0},
    {0x6C,0x04},
    {0x6D,0x2C},
    {0x6E,0x01},
    {0x74,0x00},
    {0xEF,0x00},
    {0x41,0xFF},
    {0x42,0x01},
};

/* 定义先进先出队列 */
#define FIFO_MAXSIZE    12
typedef struct {
    uint16_t buffer[FIFO_MAXSIZE]; /* 相关bit定义参考paj7620u2.h的“手势识别效果” */
    uint16_t offset_read;
    uint16_t offset_write;
}fifo_s;
static fifo_s m_fifo_intflags;

/* 轮询方式访问 */
static UINT32 m_pollTaskId;

////////////////////////////////////////////

static inline void FifoInit(fifo_s *fifo)
{
    fifo->offset_read = fifo->offset_write = 0;
}

static inline void FifoPut(fifo_s *fifo, uint16_t flag)
{
    fifo->buffer[fifo->offset_write] = flag;
    fifo->offset_write = (fifo->offset_write + 1) % FIFO_MAXSIZE;
}

static inline uint16_t FifoGet(fifo_s *fifo, uint16_t *flag)
{
    if (fifo->offset_read == fifo->offset_write)
        return 0;

    *flag = fifo->buffer[fifo->offset_read];
    fifo->offset_read = (fifo->offset_read + 1) % FIFO_MAXSIZE;
    return 1;
}

/***************************************************************
* 函数名称: e53_gs_led_init
* 说    明: 初始化LED的GPIO引脚
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void e53_gs_led_init()
{
    PinctrlSet(GPIO_LED_UP, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_UP);
    LzGpioSetDir(GPIO_LED_UP, LZGPIO_DIR_OUT);
    e53_gs_led_up_set(0);

    PinctrlSet(GPIO_LED_DOWN, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_DOWN);
    LzGpioSetDir(GPIO_LED_DOWN, LZGPIO_DIR_OUT);
    e53_gs_led_down_set(0);

    PinctrlSet(GPIO_LED_LEFT, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_LEFT);
    LzGpioSetDir(GPIO_LED_LEFT, LZGPIO_DIR_OUT);
    e53_gs_led_left_set(0);

    PinctrlSet(GPIO_LED_RIGHT, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_RIGHT);
    LzGpioSetDir(GPIO_LED_RIGHT, LZGPIO_DIR_OUT);
    e53_gs_led_right_set(0);

    PinctrlSet(GPIO_LED_FORWARD, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_FORWARD);
    LzGpioSetDir(GPIO_LED_FORWARD, LZGPIO_DIR_OUT);
    e53_gs_led_forward_set(0);

    PinctrlSet(GPIO_LED_BACKWARD, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_BACKWARD);
    LzGpioSetDir(GPIO_LED_BACKWARD, LZGPIO_DIR_OUT);
    e53_gs_led_backward_set(0);

    PinctrlSet(GPIO_LED_CW, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_CW);
    LzGpioSetDir(GPIO_LED_CW, LZGPIO_DIR_OUT);
    e53_gs_led_cw_set(0);

    PinctrlSet(GPIO_LED_CCW, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_CCW);
    LzGpioSetDir(GPIO_LED_CCW, LZGPIO_DIR_OUT);
    e53_gs_led_ccw_set(0);

    PinctrlSet(GPIO_LED_WAVE, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_WAVE);
    LzGpioSetDir(GPIO_LED_WAVE, LZGPIO_DIR_OUT);
    e53_gs_led_wave_set(0);
}


/***************************************************************
* 函数名称: paj7620u2_delay_usec
* 说    明: 忙等待
* 参    数: 
*           @usec: 忙等待的微秒数
* 返 回 值: 无
***************************************************************/
static inline void paj7620u2_delay_usec(uint32_t usec)
{
    HAL_DelayUs(usec);
}


/***************************************************************
* 函数名称: paj7620U2_write_null
* 说    明: 使用i2c确认PAJ7620U2是否存在，不做任何事情
* 参    数: 无
* 返 回 值: 返回1为成功
***************************************************************/
static uint8_t paj7620U2_write_null()
{
    unsigned int ret = 0;
    
    ret = LzI2cWrite(E53_I2C_BUS, PAJ7620U2_I2C_SLAVE_ADDRESS, NULL, 0);
    if (ret != LZ_HARDWARE_SUCCESS) 
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return 0;
    }

    return 1;
}

/***************************************************************
* 函数名称: paj7620u2_write_data
* 说    明: 使用i2c往PAJ7620U2写入数据
* 参    数: 
*           @addr: 寄存器地址
*           @data: 数值
* 返 回 值: 返回1为成功
***************************************************************/
static uint8_t paj7620u2_write_data(uint8_t addr, uint8_t data)
{
    unsigned int ret = 0;
    unsigned char buffer[2];

    /* write value to reg */
    buffer[0] = addr;
    buffer[1] = data;
    ret = LzI2cWrite(E53_I2C_BUS, PAJ7620U2_I2C_SLAVE_ADDRESS, buffer, 2);
    if (ret != LZ_HARDWARE_SUCCESS) 
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return 0;
    }

    return 1;
}

/***************************************************************
* 函数名称: paj7620u2_read_data
* 说    明: 使用i2c读取PAJ7620U2的寄存器数据
* 参    数: 
*           @addr: 寄存器地址
*           @data: 数值
* 返 回 值: 返回1为成功
***************************************************************/
static uint8_t paj7620u2_read_data(uint8_t addr, uint8_t *data)
{
    unsigned int ret = 0;
    unsigned char buffer[1];

    /* 发送地址给PAJ7620U2 */
    buffer[0] = addr;
    ret = LzI2cWrite(E53_I2C_BUS, PAJ7620U2_I2C_SLAVE_ADDRESS, buffer, 1);
    if (ret != LZ_HARDWARE_SUCCESS) 
    {
        printf("%s, %s, %d: LzI2cWrite failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return 0;
    }

    ret = LzI2cRead(E53_I2C_BUS, PAJ7620U2_I2C_SLAVE_ADDRESS, data, 1);
    if (ret != LZ_HARDWARE_SUCCESS) 
    {
        printf("%s, %s, %d: LzI2cRead failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return 0;
    }

    return 1;
}

/***************************************************************
* 函数名称: paj7620u2_select_bank
* 说    明: 选择PAJ7620U2 BANK区域
* 参    数: 
*           @bank：bank区域[0,1]
* 返 回 值: 无
***************************************************************/
static void paj7620u2_select_bank(BankId bank)
{
    switch (bank)
    {
    case BANK0:
        paj7620u2_write_data(PAJ_REG_BANK_SEL, PAJ_REB_BANK_SEL_BANK0);
        break;
    case BANK1:
        paj7620u2_write_data(PAJ_REG_BANK_SEL, PAJ_REB_BANK_SEL_BANK1);
        break;
    default:
        printf("%s, %s, %d: bank(%d) out of the range!\n", __FILE__, __func__, __LINE__, bank);
        break;
    }
}

/***************************************************************
 * 函数名称: paj7620u2_get_bank_id
 * 说    明: 获取当前BankId
 * 参    数: 无
 * 返 回 值: BankId
 ***************************************************************/
static uint8_t paj7620u2_get_bank_id()
{
    uint8_t bankId = 0;
    
    paj7620u2_read_data(PAJ_REG_BANK_SEL, &bankId);
    return bankId;
}

/***************************************************************
* 函数名称: paj7620u2_wake_up
* 说    明: 唤醒PAJ7620U2
* 参    数: 无
* 返 回 值: 返回0为成功
***************************************************************/
static uint32_t paj7620u2_wake_up()
{
    uint8_t ret = 0;
    uint8_t data = 0;
    
    /* 查询PAJ7620U2，用于唤醒它 */
    paj7620U2_write_null();
    paj7620u2_delay_usec(1000);
    /* 多唤醒1次 */
    paj7620U2_write_null();
    paj7620u2_delay_usec(1000);

    /* 读取bank0 addr 0x0，返回一定是0x20 */
    paj7620u2_select_bank(BANK0);
    ret = paj7620u2_read_data(0x00, &data);
    if (ret != 1)
    {
        printf("%s, %s, %d: paj7620u2_read_data failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return __LINE__;
    }
    if (data != 0x20)
    {
        printf("%s, %s, %d: paj7620u2_read_data 0x%x != 0x20\n", __FILE__, __func__, __LINE__, data);
        return __LINE__;
    }

    return 0;
}

/***************************************************************
* 函数名称: paj7620u2_suspend
* 说    明: 挂起PAJ7620U2
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void paj7620u2_suspend()
{
    /* 禁用PAJ7620U2，往bank1     addr 0x72寄存器写0x00 */
    paj7620u2_select_bank(BANK1);
    paj7620u2_write_data(PAJ_REG_OPERATION_ENABLE, 0x0);
    /* PAJ7620U2设置挂起状态，往bank0 addr 0x03寄存器写0x01 */
    paj7620u2_select_bank(BANK0);
    paj7620u2_write_data(PAJ_REG_SUSPEND_CMD, 0x1);
}

/***************************************************************
* 函数名称: paj7620u2_poll_task
* 说    明: 轮询任务，每隔100msec访问PAJ7620U2的手势中断寄存器，
*       如果有手势数据，则填写到m_fifo_intflags队列中。
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static VOID paj7620u2_poll_task(VOID *args)
{
    uint8_t int_flag1 = 0;
    uint8_t int_flag2 = 0;
    uint16_t value = 0;
    
    while (1)
    {
        /* 读取Paj7620U2的手势中断寄存器 */
        paj7620u2_select_bank(BANK0);
        paj7620u2_read_data(PAJ_REG_GET_INT_FLAG1, &int_flag1);
        paj7620u2_read_data(PAJ_REG_GET_INT_FLAG2, &int_flag2);

        value = 0;
        if (int_flag1 != 0)
        {
            value |= (uint16_t)(int_flag1);
        }
        if (int_flag2 != 0)
        {
            value |= (uint16_t)(int_flag2 << 8);
        }

        if (value != 0)
            FifoPut(&m_fifo_intflags, value);

        LOS_Msleep(100);
    }
}

/***************************************************************
* 函数名称: paj7620u2_i2c_init
* 说    明: 初始化与PAJ7620U2通信的i2c
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void paj7620u2_i2c_init()
{
    if (I2cIoInit(m_i2cBus) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
        return;
    }
    if (LzI2cInit(E53_I2C_BUS, m_i2c_freq) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
        return;
    }

    PinctrlSet(GPIO0_PA1, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
    PinctrlSet(GPIO0_PA0, MUX_FUNC3, PULL_KEEP, DRIVE_KEEP);
}

/***************************************************************
* 函数名称: paj7620u2_poll_task_init
* 说    明: 初始化轮询PAJ7620U2手势的任务
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void paj7620u2_poll_task_init()
{
    TSK_INIT_PARAM_S task;
    uint8_t int_flag1, int_flag2;
    UINT32 ret;
    
    /* 初始化先进先出缓冲区 */
    m_fifo_intflags.offset_read = 0;
    m_fifo_intflags.offset_write = 0;

    /* 锁任务调度 */
    LOS_TaskLock();

    /* 创建中断之后的i2c读取中断标记寄存器的任务 */
    (VOID)memset_s(&task, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    task.pfnTaskEntry   = (TSK_ENTRY_FUNC)paj7620u2_poll_task;
    task.pcName         = "InterruptSemTask";
    task.uwStackSize    = 0x400;
    task.usTaskPrio     = 10;
    ret = LOS_TaskCreate(&m_pollTaskId, &task);
    if (ret != LOS_OK)
    {
        printf("%s, %d: LOS_TaskCreate failed(%d)\n", __func__, __LINE__, ret);
        /* 解锁任务调度 */
        LOS_TaskUnlock();
        return;
    }

    /* 解锁任务调度 */
    LOS_TaskUnlock();

    /* 先清空PAJ7620U2的中断标记寄存器 */
    paj7620u2_select_bank(BANK0);
    paj7620u2_read_data(PAJ_REG_GET_INT_FLAG1, &int_flag1);
    paj7620u2_read_data(PAJ_REG_GET_INT_FLAG2, &int_flag2);
}

/***************************************************************
* 函数名称: paj7620u2_init_config
* 说    明: 配置PAJ7620U2为手势感应模式
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void paj7620u2_init_config()
{
    uint8_t ret = 0;
    uint32_t size;
    
    ret = paj7620u2_wake_up();
    if (ret != 0)
    {
        printf("%s, %s, %d: paj7620u2_wake_up failed(%d)\n", __FILE__, __func__, __LINE__, ret);
    }

    /* 初始化PAJ7620U2 */
    size = sizeof(m_Paj7620u2_InitRegisterConfig) / (sizeof(uint8_t) * 2);
    for (uint32_t i = 0; i < size; i++) 
    {
        paj7620u2_write_data(m_Paj7620u2_InitRegisterConfig[i][0], m_Paj7620u2_InitRegisterConfig[i][1]);
    }

    /* 设置为手势识别模式 */
    size = sizeof(m_Paj7620u2_SetGestureModeConfig) / (sizeof(uint8_t) * 2);
    for (uint32_t i = 0; i < size; i++)
    {
        paj7620u2_write_data(m_Paj7620u2_SetGestureModeConfig[i][0], m_Paj7620u2_SetGestureModeConfig[i][1]);
    }

    paj7620u2_select_bank(BANK0);
}


////////////////////////////////////////////

/***************************************************************
 * 函数名称: e53_gs_init
 * 说    明: 手势感应模块初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int e53_gs_init()
{
    /* 初始化LED */
    e53_gs_led_init();
    /* 上电后，等待PAJ7620U2 700usec */
    paj7620u2_delay_usec(700);
    /* 初始化i2c */    
    paj7620u2_i2c_init();
    /* 初始化寄存器配置和工作模式 */
    paj7620u2_init_config();
    /* 初始化采集任务 */
    paj7620u2_poll_task_init();
}

/***************************************************************
 * 函数名称: e53_gs_led_up_set
 * 说    明: 手势感应模块向上LED设置
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_up_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_UP, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_UP, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_down_set
 * 说    明: 手势感应模块向上LED设置
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_down_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_DOWN, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_DOWN, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_left_set
 * 说    明: 手势感应模块向左LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_left_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_LEFT, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_LEFT, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_right_set
 * 说    明: 手势感应模块向右LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_right_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_RIGHT, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_RIGHT, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_forward_set
 * 说    明: 手势感应模块向前LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_forward_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_FORWARD, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_FORWARD, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_backward_set
 * 说    明: 手势感应模块向后LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_backward_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_BACKWARD, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_BACKWARD, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_cw_set
 * 说    明: 手势感应模块顺时针转圈LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_cw_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_CW, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_CW, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_ccw_set
 * 说    明: 手势感应模块逆时针转圈LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_ccw_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_CCW, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_CCW, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_led_wave_set
 * 说    明: 手势感应模块挥手LED亮起
 * 参    数: 
 *          @is_on：1为开启，0为关闭
 * 返 回 值: 无
 ***************************************************************/
void e53_gs_led_wave_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        LzGpioSetVal(GPIO_LED_WAVE, LZGPIO_LEVEL_HIGH);
    }
    else
    {
        LzGpioSetVal(GPIO_LED_WAVE, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
 * 函数名称: e53_gs_get_gesture_state
 * 说    明: 获取手势感应模块手势
 * 参    数: 
 *      @flag：获取当前手势
 * 返 回 值: 1为成功，0为失败
 ***************************************************************/
unsigned int e53_gs_get_gesture_state(unsigned short *flag)
{
    *flag = 0;

    if (FifoGet(&m_fifo_intflags, flag) != 0)
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}


