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
#include "los_interrupt.h"
#include "lz_hardware.h"
#include "e53_intelligent_vehicle_01.h"

/* 引脚定义 */
#define E53_IV01_ECHO0_GPIO             GPIO0_PA2
#define E53_IV01_TRIG_GPIO              GPIO0_PC4
#define E53_IV01_BUZZER_GPIO            GPIO0_PC7
#define E53_IV01_LED_WARNING_GPIO       GPIO0_PA5

/* 系统时钟定时器主频40MHz,定时器5作为系统时钟源，每秒24MHz，从0一直累加 */
#define ECHO_TIMER_FREQ                 40000000UL
/* 定义每次中断对应的采集动作 */
typedef enum
{
    EECHO_FLAG_CAPTURE_RISE = 0,        /* 准备采集上升沿，即开始时间 */
    EECHO_FLAG_CAPTURE_FALL,            /* 准备采集下降沿，即结束时间 */
    EECHO_FLAG_CAPTURE_SUCCESS,         /* 采集成功 */
    EECHO_FLAG_MAX
} echo_flag_e;
/* 定义与中断相关的采集动作和采集时间相关信息 */
typedef struct
{
    echo_flag_e flag;   /* 中断采集动作 */
    uint32_t time_rise; /* 采集上升沿，即开始时间 */
    uint32_t time_fall; /* 采集下降沿，即结束时间 */
} e53_iv01_echo_info_s;
static e53_iv01_echo_info_s m_echo_info =
{
    .flag = EECHO_FLAG_CAPTURE_RISE,
    .time_rise = 0,
    .time_fall = 0,
};
/* 定义信号量，用于唤醒 */
static UINT32 m_task_sem;
/* 定义轮询任务id */
static UINT32 m_task_id = 0;
/* 定时器5的CURRENT_VALUE_LOW的基地址 */
static uint32_t *m_ptimer5_current_value_low = (uint32_t *)(0x400000A0U + 0x8U);

/* Trig引脚电平设置 */
#define E53_IV01_TRIG_Set()             LzGpioSetVal(E53_IV01_TRIG_GPIO, LZGPIO_LEVEL_HIGH)
#define E53_IV01_TRIG_Clr()             LzGpioSetVal(E53_IV01_TRIG_GPIO, LZGPIO_LEVEL_LOW)

/* LedWarning灯电平设置 */
#define E53_IV01_LED_WARNING_Set()      LzGpioSetVal(E53_IV01_LED_WARNING_GPIO, LZGPIO_LEVEL_HIGH)
#define E53_IV01_LED_WARNING_Clr()      LzGpioSetVal(E53_IV01_LED_WARNING_GPIO, LZGPIO_LEVEL_LOW)

/* Buzzer定义 */
#define E53_IV01_PWM_IO             7
static PwmBusIo m_buzzer_config  =
{
    .pwm = {.gpio = E53_IV01_BUZZER_GPIO, .func = MUX_FUNC2, .type = PULL_DOWN, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_PWM7,
    .mode = FUNC_MODE_NONE,
};

//////////////////////////////////////////////////

/***************************************************************
 * 函数名称: e53_iv01_delay_usec
 * 说    明: 延时函数
 * 参    数: 
 *      @usec，延时时间，以usec为单位
 * 返 回 值: 无
 ***************************************************************/
static inline void e53_iv01_delay_usec(uint32_t usec)
{
    HAL_DelayUs(usec);
}

/***************************************************************
 * 函数名称: e53_iv01_calc_cm
 * 说    明: 计算超声波测距
 * 参    数: 
 *      @time：节拍数
 *      @freq：节拍频率，以Hz为单位
 *      @meter：距离差，以厘米为单位
 * 返 回 值: 无
 ***************************************************************/
static inline void e53_iv01_calc_cm(uint32_t time, uint32_t freq, float *cmeter)
{
    float f_time = (float)time;
    float f_freq = (float)freq;
    
    /* 距离 = 时间差 * 340米/秒 / 2(超时波来回2次) * 100厘米/米 */
    *cmeter = f_time / f_freq * 170.0 * 100.0;
}


/***************************************************************
 * 函数名称: e53_iv01_task_echo_func
 * 说    明: 以轮询方式查询ECHO0引脚的高低电平变化
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static VOID e53_iv01_task_echo_func(VOID *arg)
{
    uint8_t value = 0;
    
    m_echo_info.flag = EECHO_FLAG_CAPTURE_RISE;

    while (1)
    {
        LzGpioGetVal(E53_IV01_ECHO0_GPIO, &value);
        if (value == LZGPIO_LEVEL_HIGH)
        {
            m_echo_info.time_rise = *m_ptimer5_current_value_low;
            m_echo_info.flag = EECHO_FLAG_CAPTURE_FALL;
            break;
        }
    }

    while (1)
    {
        LzGpioGetVal(E53_IV01_ECHO0_GPIO, &value);
        if (value == LZGPIO_LEVEL_LOW)
        {
            m_echo_info.time_fall = *m_ptimer5_current_value_low;
            m_echo_info.flag = EECHO_FLAG_CAPTURE_SUCCESS;
            break;
        }
    }

    /* 释放信号量 */
    LOS_SemPost(m_task_sem);
}


/***************************************************************
 * 函数名称: e53_iv01_task_create_echo
 * 说    明: 启动轮询方式查询ECHO0引脚的任务
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_task_create_echo()
{
    uint32_t ret = 0;
    TSK_INIT_PARAM_S task;
    
    /* 锁任务调度 */
    LOS_TaskLock();

    (VOID)memset_s(&task, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    task.pfnTaskEntry   = (TSK_ENTRY_FUNC)e53_iv01_task_echo_func;
    task.pcName         = "Cs100aTaskEcho";
    task.uwStackSize    = 0x400;
    /* 优先级比当前线程低 */
    task.usTaskPrio     = 30;
    ret = LOS_TaskCreate(&m_task_id, &task);
    if (ret != LOS_OK)
    {
        printf("%s, %d: LOS_TaskCreate failed(%d)\n", __func__, __LINE__, ret);
        /* 解锁任务调度 */
        LOS_TaskUnlock();
        return;
    }

    /* 解锁任务调度 */
    LOS_TaskUnlock();
}


/***************************************************************
 * 函数名称: e53_iv01_task_delete_echo
 * 说    明: 注销轮询方式查询ECHO0引脚的任务
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_task_delete_echo()
{
    /* 锁任务调度 */
    LOS_TaskLock();
    /* 删除该任务 */
    LOS_TaskDelete(m_task_id);
    /* 解锁任务调度 */
    LOS_TaskUnlock();
}


/***************************************************************
 * 函数名称: e53_iv01_send_trig
 * 说    明: 向智慧车载芯片发送开始测距信号，即发送至少10usec的高电平
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_send_trig()
{
    /* 发送至少10usec的高电平给智慧车载，触发其工作 */
    E53_IV01_TRIG_Set();
    e53_iv01_delay_usec(20);
    E53_IV01_TRIG_Clr();
}

/***************************************************************
 * 函数名称: e53_iv01_init_gpio
 * 说    明: 初始化GPIO
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_init_gpio()
{
    /* Trig引脚设置为GPIO输出模式 */
    PinctrlSet(E53_IV01_TRIG_GPIO, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(E53_IV01_TRIG_GPIO);
    LzGpioSetDir(E53_IV01_TRIG_GPIO, LZGPIO_DIR_OUT);
    E53_IV01_TRIG_Clr();
    
    /* LED_WARNING灯引脚设置为GPIO输出模式 */
    PinctrlSet(E53_IV01_LED_WARNING_GPIO, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(E53_IV01_LED_WARNING_GPIO);
    LzGpioSetDir(E53_IV01_LED_WARNING_GPIO, LZGPIO_DIR_OUT);
    e53_iv01_led_warning_set(0);
}

/***************************************************************
 * 函数名称: e53_iv01_deinit_gpio
 * 说    明: 释放GPIO
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_deinit_gpio()
{
    LzGpioDeinit(E53_IV01_TRIG_GPIO);
}

/***************************************************************
 * 函数名称: e53_iv01_init_pwm
 * 说    明: 初始化PWM
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static unsigned int e53_iv01_init_pwm()
{
    /* 初始化pwm */
    PinctrlSet(E53_IV01_BUZZER_GPIO, MUX_FUNC2, PULL_DOWN, DRIVE_KEEP);
    PwmIoInit(m_buzzer_config);
    LzPwmInit(E53_IV01_PWM_IO);
    
    return 0;
}

/***************************************************************
 * 函数名称: e53_iv01_deinit_pwm
 * 说    明: 释放PWM
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_deinit_pwm()
{
    LzPwmDeinit(E53_IV01_PWM_IO);
}

/***************************************************************
 * 函数名称: e53_iv01_init_interrupt
 * 说    明: 初始化GPIO中断
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_init_interrupt()
{
    //LzI2cInit(0, 400000);
    
    /* 创建信号量 */
    LOS_SemCreate(0, &m_task_sem);
    
    /* Echo引脚设置为GPIO输出模式 */
    PinctrlSet(E53_IV01_ECHO0_GPIO, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(E53_IV01_ECHO0_GPIO);
    LzGpioSetDir(E53_IV01_ECHO0_GPIO, LZGPIO_DIR_IN);
}


/***************************************************************
 * 函数名称: e53_iv01_deinit_interrupt
 * 说    明: 释放GPIO中断
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_deinit_interrupt()
{
    LzGpioDeinit(E53_IV01_ECHO0_GPIO);
}
//////////////////////////////////////////////////////////////////////

/***************************************************************
 * 函数名称: e53_iv01_init
 * 说    明: E53_IntelligentVehicle01驱动初始化
 * 参    数: 无
 * 返 回 值: 返回0为成功，反之为失败
 ***************************************************************/
unsigned int e53_iv01_init()
{
    unsigned int ret = 0;
    
    e53_iv01_init_gpio();
    
    ret = e53_iv01_init_pwm();
    if (ret != 0)
    {
        printf("%s, %s, %d: e53_iv01_init_pwm failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        e53_iv01_deinit_gpio();
        return __LINE__;
    }
    
    e53_iv01_init_interrupt();
    return 0;
}

/***************************************************************
 * 函数名称: e53_iv01_deinit
 * 说    明: E53_IntelligentVehicle01驱动销毁
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_deinit()
{
    e53_iv01_deinit_interrupt();
    e53_iv01_deinit_pwm();
    e53_iv01_deinit_gpio();
}

/***************************************************************
 * 函数名称: e53_iv01_get_distance
 * 说    明: E53_IntelligentVehicle01发起1次超声波测距
 * 参    数: 
 *      @distance_meter：测距的距离，单位为厘米
 * 返 回 值: 返回1为成功，0为失败
 ***************************************************************/
unsigned int e53_iv01_get_distance(float *distance_cm)
{
    uint32_t time_diff = 0;

    e53_iv01_task_create_echo();
    /* 发送10Usec的高电平 */
    e53_iv01_send_trig();
    /* 等待200msec，整个测距最长为66msec */
    LOS_SemPend(m_task_sem, LOS_MS2Tick(200));
    e53_iv01_task_delete_echo();

    if (m_echo_info.flag == EECHO_FLAG_CAPTURE_SUCCESS)
    {/* 如果是采集成功，则计算距离 */
        if (m_echo_info.time_rise <= m_echo_info.time_fall)
        {
            time_diff = m_echo_info.time_fall - m_echo_info.time_rise;
        }
        else
        {
            time_diff = 0xFFFFFFFF - m_echo_info.time_rise + m_echo_info.time_fall + 1;
        }

        e53_iv01_calc_cm(time_diff, ECHO_TIMER_FREQ, distance_cm);
        return 1;
    }
    else
    {
        printf("%s, %d: flag(%d) is error!\n", __func__, __LINE__, m_echo_info.flag);
        return 0;
    }
}

/***************************************************************
 * 函数名称: e53_iv01_led_warning_set
 * 说    明: E53_IntelligentVehicle01的Led灯控制
 * 参    数: 
 *      @is_on：Led灯控制，0为灭，1为亮
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_led_warning_set(unsigned char is_on)
{
    if (is_on == 0)
    {
        /* LED Warning灯低电平有效 */
        E53_IV01_LED_WARNING_Set();
    }
    else
    {
        E53_IV01_LED_WARNING_Clr();
    }
}

/***************************************************************
 * 函数名称: e53_iv01_buzzer_set
 * 说    明: 智慧车载的PWM控制
 * 参    数: 
 *      @is_on：PWM控制，0为关闭，1为开启
 *      @duty_ns：高电平的占比，以nsec为单位
 *      @cycle_ns：PWM周期，以nsec为单位
 * 返 回 值: 无
 ***************************************************************/
void e53_iv01_buzzer_set(unsigned char is_on, unsigned int duty_ns, unsigned int cycle_ns)
{
    if (is_on == 0)
    {
        /* 关闭蜂鸣器 */
        LzPwmStop(E53_IV01_PWM_IO);
    }
    else
    {
        /* 打开蜂鸣器 */
        LzPwmStart(E53_IV01_PWM_IO, duty_ns, cycle_ns);
    }
}



