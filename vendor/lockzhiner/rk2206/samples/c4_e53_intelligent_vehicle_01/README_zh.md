# 小凌派-RK2206开发板E53模块开发——智慧车载

本示例将演示如何在小凌派-RK2206开发板上实现智慧车载的应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![智慧车载模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_iv01/e53_iv01_resource_map.jpg)

EEPROM 24C02的设备地址为：0x1010001* ；

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在智慧车载模块背面查看。

| 引脚名称 | 功能描述 |
| :-- | :------ | 
| ECHO | 测距脉宽输出，高电平的宽度代表超声波往返时间 |
| TRIG | 测距触发，输入10us的高电平脉冲，开始测距 | 
| BUZZER | 蜂鸣器控制线，推荐3KHz的方波信号 | 
| LED_Warning | LED控制线，低电平有效 | 
| I2C_SCL | I2C时钟信号线 | 
| I2C-SDA | I2C数据信号线 | 
| GND | 电源地引脚 | 
| 3V3 | 3.3V电源输入引脚 | 
| GND | 电源地引脚 |

## 硬件设计

硬件电路如下图所示：
![智慧车载模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_iv01/lz_e53_iv01_sch.jpg)

模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、超声波处理电路，声光报警电路。

超声波测距芯片，选用CS-100A，其是一款工业级超声波测距芯片，内部集成超声波发射电路，超声波接收电路，数字处理电路等，单片即可完成超声波测距，测距结果通过脉宽的方式进行输出。

CS100A配合使用40KHz的开放式超声波探头，在超声波发射端并联一个电阻R2到地和8MHz的晶振，即可实现高性能的测距功能，电阻R2的大小决定了超声波测量的距离。

三极管Q1为NPN管，基极为高电平时，三极管才能够导通，蜂鸣器需PWM波驱动，人耳可识别的频率范围为20Hz-20KHz，故PWM频率需在该范围内，我们默认使用3KHz的PWM波驱动。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![智慧车载模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_iv01/e53_iv01_connection_diagram.png)

## 程序设计

### API分析

### GPIO接口

[GPIO接口文档](/device/rockchip/hardware/docs/GPIO.md)

### PWM接口

[PWM接口文档](/device/rockchip/hardware/docs/PWM.md)

### LiteOS信号量

[LiteOS信号量接口文档](/vendor/lockzhiner/rk2206/samples/a2_kernel_semaphore/README_zh.md)

## 主要代码分析

### 初始化代码分析

**初始化GPIO**

初始化GPIO0_PC4和GPIO0_PA5为GPIO。首先通过`LzGpioInit()`初始化GPIO引脚，其次通过`LzGpioSetDir()`设置GPIO为输出模式，最后通过`LzGpioSetVal()`设置输出高电平/低电平。

```c
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
```

**初始化GPIO中断**

初始化GPIO0_PA2为GPIO。首先通过`LzGpioInit()`初始化GPIO引脚，其次通过`LzGpioSetDir()`设置GPIO为输入模式，最后通过`LzGpioRegisterIsrFunc()`设置该引脚为中断模式，触发模式为边沿触发（即上升沿+下降沿都可触发）。

```c
/***************************************************************
 * 函数名称: e53_iv01_init_interrupt
 * 说    明: 初始化GPIO中断
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
static void e53_iv01_init_interrupt()
{
    LzI2cInit(0, 400000);
    
    /* 创建信号量 */
    LOS_SemCreate(0, &m_task_sem);
    
    /* Echo引脚设置为GPIO输出模式 */
    PinctrlSet(E53_IV01_ECHO0_GPIO, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(E53_IV01_ECHO0_GPIO);
    LzGpioSetDir(E53_IV01_ECHO0_GPIO, LZGPIO_DIR_IN);
    LzGpioRegisterIsrFunc(E53_IV01_ECHO0_GPIO, GPIO_INT_EDGE_BOTH, e53_iv01_interrupt_echo_func, NULL);
}
```

**初始化PWM**

初始化GPIO0_PC7为GPIO。首先通过`PwmIoInit()`初始化PWM配置，最后通过`LzPwmInit()`初始化PWM配置id。

```c
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
    if (PwmIoInit(m_buzzer_config) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %d: PwmIoInit failed\n", __func__, __LINE__);
        return __LINE__;
    }
    if (LzPwmInit(E53_IV01_PWM_IO) != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %d: LzPwmInit failed\n", __func__, __LINE__);
        return __LINE__;
    }
    
    return 0;
}
```

### 车载测距流程代码分析

首先，初始化相关变量，使能GPIO中断，并控制TRIG引脚往E53模块发送一个至少10usec的高电平，通知E53模块开始工作。具体代码如下所示：

```c
/* 初始化采集相关信息 */
m_echo_info.flag = EECHO_FLAG_CAPTURE_RISE;
/* 使能GPIO中断 */
LzGpioEnableIsr(E53_IV01_ECHO0_GPIO);
/* 等待1msec */
LOS_Msleep(1);
/* 发送10Usec的高电平 */
e53_iv01_send_trig();
```

其次，等待信号量（该信号量最长等待时间为200msec）。具体代码如下所示：

```c
/* 等待200msec（最长等待时间），整个测距最长为66msec */
LOS_SemPend(m_task_sem, LOS_MS2Tick(200));
```

在GPIO中断处理函数中记录E53模块用ECHO引脚发送过来的一个高电平时间宽度（该高电平的时间宽度为超声波来回的时间宽度）。具体实现方式为：

* 上升沿触发时，记录系统时钟（即定时器5）的当前节拍数，修改触发状态；
* 下降沿出发时，记录系统时钟（即定时器5）的当前节拍数，修改触发状态，释放信号量。

具体代码如下：

```c
/***************************************************************
 * 函数名称: e53_iv01_interrupt_echo_func
 * 说    明: GPIO中断处理函数，通过中断捕获定时器计数器当前值
 * 参    数: 
 *      @arg，
 * 返 回 值: 无
 ***************************************************************/
static VOID e53_iv01_interrupt_echo_func(VOID *arg)
{
    if (m_echo_info.flag == EECHO_FLAG_CAPTURE_RISE)
    {
        /* 获取上升沿的定时器计数器当前值 */
        m_echo_info.time_rise = *m_ptimer5_current_value_low;
        m_echo_info.flag = EECHO_FLAG_CAPTURE_FALL;
    }
    else if (m_echo_info.flag == EECHO_FLAG_CAPTURE_FALL)
    {
        /* 获取下降沿的定时器计数器当前值 */
        m_echo_info.time_fall = *m_ptimer5_current_value_low;
        m_echo_info.flag = EECHO_FLAG_CAPTURE_SUCCESS;
        LOS_SemPost(m_task_sem);
    }
    else
    {
        /* 什么都不做 */
    }
}
```

然后禁用GPIO中断，判断中断状态判断E53模块是否采集成功。具体代码如下：

```c
/* 禁用GPIO中断 */
LzGpioDisableIsr(E53_IV01_ECHO0_GPIO);

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
```

最后，根据上升沿和下降沿的节拍数差计算距离。其中，系统时钟为40MHz，超声波速度为340米/秒，高电平时间宽度为超声波的往返之和，所以实际距离 = 节拍数差 / 40MHz / 340(米/秒) / 2(往返2次)。具体计算代码如下：

```c
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
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `e53_iv01_example` 参与编译。

```r
"./e53_intelligent_vehicle_01:e53_iv01_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_iv01_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_iv01_example,
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有LCD屏幕显示如下：

```c
========== E53 IV Example ==========
distance cm: 23.92
========== E53 IV Example ==========
distance cm: 23.89
========== E53 IV Example ==========
distance cm: 23.90
```

