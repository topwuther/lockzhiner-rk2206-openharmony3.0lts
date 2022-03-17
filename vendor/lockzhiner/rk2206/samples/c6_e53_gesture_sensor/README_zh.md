# 小凌派-RK2206开发板E53模块开发——智能手势

本示例将演示如何在小凌派-RK2206开发板上实现智能手势的应用案例。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 硬件资源

硬件资源图如下所示：
![智能手势模块硬件资源](/vendor/lockzhiner/rk2206/docs/figures/e53_sg01/e53_sg01_resource_map.jpg)

EEPROM 24C02的设备地址为：0x1010001* ；
手势传感器 PAJ7620U2 的设备地址为：0x0111001*

## 硬件接口说明

引脚名称开发者可在硬件资源图中查看，也可在人体感应模块背面查看。

| 引脚名称  | 功能描述                        |
| :-------- | :------------------------------ |
| LED_UP    | 向上，LED灯控制线，低电平有效   |
| LED_DOWN  | 向下，LED灯控制线，低电平有效   |
| LED_LEFT  | 向左，LED灯控制线，低电平有效   |
| LED_RIGHT | 向右，LED灯控制线，低电平有效   |
| INT_N     | 中断信号，低电平有效            |
| I2C_SCL   | I2C时钟信号线                   |
| I2C-SDA   | I2C数据信号线                   |
| LED_CW    | 顺时针，LED灯控制线，低电平有效 |
| LED_CCW   | 逆时针，LED灯控制线，低电平有效 |
| LED_WAVE  | 摆动，LED灯控制线，低电平有效   |
| GND       | 电源地引脚                      |
| 3V3       | 3.3V电源输入引脚                |
| GND       | 电源地引脚                      |
| 5V        | 5V电源输入引脚                  |

## 硬件设计

硬件电路如下图所示：
![智能手势模块硬件电路图](/vendor/lockzhiner/rk2206/docs/figures/e53_sg01/lz_e53_sg01_sch.jpg)

模块整体硬件电路如上图所示，电路中包含了E53接口连接器，EEPROM存储器、手势识别传感器电路，灯光指示电路。电路比较简单，本文不再过多说明。

### 硬件连接

小凌派开发板与模块均带有防呆设计，故很容易区分安装方向，直接将模块插入到开发板的E53母座接口上即可，安装图如下所示：
![智能手势模块硬件连接图](/vendor/lockzhiner/rk2206/docs/figures/e53_sg01/e53_sg01_connection_diagram.png)

## 程序设计

### API分析

#### GPIO接口

[GPIO接口文档](/device/rockchip/hardware/docs/GPIO.md)

#### I2C接口

[I2C接口文档](/device/rockchip/hardware/docs/I2C.md)

#### LiteOS任务管理

[LiteOS任务管理接口文档](/vendor/lockzhiner/rk2206/samples/a1_kernel_task/task.md)

### 主要代码分析

#### 初始化代码分析

**初始化GPIO**

本模块使用以下引脚作为LED灯。具体如下所示：

| 引脚      | 功能         |
| :-------- | :----------- |
| GPIO0_PB1 | LED_UP       |
| GPIO0_PB0 | LED_DOWN     |
| GPIO0_PA2 | LED_LEFT     |
| GPIO0_PC4 | LED_RIGHT    |
| GPIO0_PB4 | LED_FORWARD  |
| GPIO0_PB7 | LED_BACKWARD |
| GPIO0_PB6 | LED_CW       |
| GPIO0_PB5 | LED_CCW      |
| GPIO0_PB2 | LED_WAVE     |

初始化引脚为GPIO。首先通过 `LzGpioInit()`初始化GPIO引脚，其次通过 `LzGpioSetDir()`设置GPIO为输出模式，最后通过 `LzGpioSetVal()`设置输出高电平/低电平。

```c
/***************************************************************
* 函数名称: gesture_sensor_led_init
* 说    明: 初始化LED的GPIO引脚
* 参    数: 无
* 返 回 值: 无
***************************************************************/
static void gesture_sensor_led_init()
{
    PinctrlSet(GPIO_LED_UP, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_UP);
    LzGpioSetDir(GPIO_LED_UP, LZGPIO_DIR_OUT);
    gesture_sensor_led_up(0);

    PinctrlSet(GPIO_LED_DOWN, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_DOWN);
    LzGpioSetDir(GPIO_LED_DOWN, LZGPIO_DIR_OUT);
    gesture_sensor_led_down(0);

    PinctrlSet(GPIO_LED_LEFT, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_LEFT);
    LzGpioSetDir(GPIO_LED_LEFT, LZGPIO_DIR_OUT);
    gesture_sensor_led_left(0);

    PinctrlSet(GPIO_LED_RIGHT, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_RIGHT);
    LzGpioSetDir(GPIO_LED_RIGHT, LZGPIO_DIR_OUT);
    gesture_sensor_led_right(0);

    PinctrlSet(GPIO_LED_FORWARD, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_FORWARD);
    LzGpioSetDir(GPIO_LED_FORWARD, LZGPIO_DIR_OUT);
    gesture_sensor_led_forward(0);

    PinctrlSet(GPIO_LED_BACKWARD, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_BACKWARD);
    LzGpioSetDir(GPIO_LED_BACKWARD, LZGPIO_DIR_OUT);
    gesture_sensor_led_backward(0);

    PinctrlSet(GPIO_LED_CW, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_CW);
    LzGpioSetDir(GPIO_LED_CW, LZGPIO_DIR_OUT);
    gesture_sensor_led_cw(0);

    PinctrlSet(GPIO_LED_CCW, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_CCW);
    LzGpioSetDir(GPIO_LED_CCW, LZGPIO_DIR_OUT);
    gesture_sensor_led_ccw(0);

    PinctrlSet(GPIO_LED_WAVE, MUX_FUNC0, PULL_KEEP, DRIVE_KEEP);
    LzGpioInit(GPIO_LED_WAVE);
    LzGpioSetDir(GPIO_LED_WAVE, LZGPIO_DIR_OUT);
    gesture_sensor_led_wave(0);
}
```

**初始化i2c**

初始化GPIO0_PA1和GPIO0_PA0为I2C0。首先通过 `I2cIoInit()`初始化i2c通道，最后通过 `LzI2cInit()`设置i2c配置。具体代码如下所示：

```c
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
```

**配置PAJ7620U2**

通过i2c通信协议，配置PAJ7620U2。具体代码如下所示：

```c
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
```

**创建轮询任务**

通过 `LOS_TaskCreate()`创建任务，每隔100msec轮询PAJ7620U2的手势寄存器。具体代码如下所示：

```c
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
```

#### 查询手势感应代码分析

首先，通过 `LOS_TaskCreate()`创建任务，每隔100msec轮询PAJ7620U2的手势寄存器。如果寄存器有数据，则存储到FIFO队列中。具体代码如下所示：

```c
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
```

上层软件通过调用 `E53_GestureSensor_GetGestureState()`获知FIFO队列中的最新手势信息。具体代码如下：

```c
/***************************************************************
 * 函数名称: gesture_sensor_get_gesture_state
 * 说    明: 获取手势感应模块手势
 * 参    数: 
 *      @flag：获取当前手势
 * 返 回 值: 1为成功，0为失败
 ***************************************************************/
unsigned int gesture_sensor_get_gesture_state(unsigned short *flag)
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
```

## 5. 编译调试

### 5.1 修改 BUILD.gn 文件

修改 `vendor\lockzhiner\rk2206\sample` 路径下 BUILD.gn 文件，指定 `e53_gs_example` 参与编译。

```r
"./e53_gesture_sensor:e53_gs_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-le53_gs_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -le53_gs_example
```

### 5.2 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，并请使用带有LCD屏幕显示如下：

```c
========== E53 Gesture Sensor Example ==========
Get Gesture Statu: 0x1
   Down
========== E53 Gesture Sensor Example ==========
Get Gesture Statu: 0x5
   Down
   Left
```
