# 小凌派-RK2206开发板OpenHarmonyOS内核开发-PWM控制

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用LiteOS-M内核接口，进行PWM编程开发。例程将创建一个任务，每隔5秒将PWM0~10依次启用，输出1000Hz。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

PWM在日常设备中非常常见，以下我们将演示LiteOS-M的PWM接口如何进行PWM控制。

### API分析

#### 头文件

```c
//base/iot_hardware/peripheral/interfaces/kits/iot_pwm.h
```

#### IoTPwmInit()

```c
unsigned int IoTPwmInit(unsigned int port);
```

**描述：**

PWM初始化。

**参数：**

|名字|描述|
|:--|:------| 
| port | PWM端口号 |

其中，port对应于如下表所示：

| 端口号 | 描述             |
| :----- | :--------------- |
| 0      | GPIO_PB4 输出PWM |
| 1      | GPIO_PB5 输出PWM |
| 2      | GPIO_PB6 输出PWM |
| 3      | GPIO_PC0 输出PWM |
| 4      | GPIO_PC1 输出PWM |
| 5      | GPIO_PC2 输出PWM |
| 6      | GPIO_PC3 输出PWM |
| 7      | GPIO_PC4 输出PWM |
| 8      | GPIO_PC5 输出PWM |
| 9      | GPIO_PC6 输出PWM |
| 10     | GPIO_PC7 输出PWM |

**返回值：**

|返回值|描述|
|:--|:------|
| IOT_SUCCESS | 成功 |
| IOT_FAILURE | 失败 |

#### IoTPwmDeinit()

```c
unsigned int IoTPwmDeinit(unsigned int port);
```

**描述：**

销毁PWM设备。

**参数：**

|名字|描述|
|:--|:------| 
| port | PWM端口号 |

**返回值：**

|返回值|描述|
|:--|:------|
| IOT_SUCCESS | 成功 |
| IOT_FAILURE | 失败 |

#### IoTPwmStart()

```c
unsigned int IoTPwmStart(unsigned int port, unsigned short duty, unsigned int freq);
```

**描述：**

使能PWM，输出波形。

**参数：**

|名字|描述|
|:--|:------| 
| port | PWM端口号 |
| duty | 高电平的占空比，范围为1~99 |
| freq | 频率 |

**返回值：**

|返回值|描述|
|:--|:------|
| IOT_SUCCESS | 成功 |
| IOT_FAILURE | 失败 |

#### IoTPwmStop()

```c
unsigned int IoTPwmStop(unsigned int port);
```

**描述：**

禁用PWM设备。

**参数：**

|名字|描述|
|:--|:------| 
| port | PWM端口号 |

**返回值：**

|返回值|描述|
|:--|:------|
| IOT_SUCCESS | 成功 |
| IOT_FAILURE | 失败 |

### 软件设计

**主要代码分析**

在pwm_example函数中，创建一个任务。

```c
void pwm_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_pw_thread;
    task.uwStackSize = 2048;
    task.pcName = "hal_pwm_thread";
    task.usTaskPrio = 20;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create hal_pw_thread ret:0x%x\n", ret);
        return;
    }
}
```

hal_pw_thread任务中调用OpenHarmony公共接口控制PWM。

```c
void hal_pw_thread()
{
    unsigned int ret;
    /* PWM端口号对应于参考文件：
     * device/rockchip/rk2206/adapter/hals/iot_hardware/wifiiot_lite/hal_iot_pwm.c
     */
    unsigned int port = 0;

    while (1)
    {
        printf("===========================\n");
        ret = IoTPwmInit(port);
        if (ret != 0) {
            printf("IoTPwmInit failed(%d)\n");
            return;
        }
        
        printf("PWM(%d) start\n", port);
        ret = IoTPwmStart(port, 50, 1000);
        if (ret != 0) {
            printf("IoTPwmStart failed(%d)\n");
            return;
        }
        
        LOS_Msleep(5000);
        
        printf("PWM(%d) end\n", port);
        ret = IoTPwmStop(port);
        if (ret != 0) {
            printf("IoTPwmStop failed(%d)\n");
            return;
        }
        
        ret = IoTPwmDeinit(port);
        if (ret != 0) {
            printf("IoTPwmInit failed(%d)\n");
            return;
        }
        
        printf("\n");

        port++;
        if (port >= 11) {
            port = 0;
        }
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `hal_pwm_example` 参与编译。

```r
"./a8_hal_pwm:hal_pwm_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lhal_pwm_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lhal_pwm_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，任务每隔5s控制不同PWM输出。

```r
===========================
[HAL INFO] setting GPIO0-12 to 1
[HAL INFO] setting route 41050204 = 00100010, 00000010
[HAL INFO] setting route for GPIO0-12
[HAL INFO] setting GPIO0-12 pull to 2
[GPIO:D]LzGpioInit: id 12 is initialized successfully
[HAL INFO] setting GPIO0-12 to 1
[HAL INFO] setting route 41050204 = 00100010, 00000010
[HAL INFO] setting route for GPIO0-12
[HAL INFO] setting GPIO0-12 pull to 2
[HAL INFO] PINCTRL Write before set reg val=0x10
[HAL INFO] PINCTRL Write after  set reg val=0x10
PWM(0) start
[HAL INFO] channel=0, period_ns=1000000, duty_ns=500000
[HAL INFO] channel=0, period=40000, duty=20000, polarity=0
[HAL INFO] Enable channel=0
IotProcess: sleep 5 sec!
PWM(0) end
[HAL INFO] Disable channel=0

===========================
[HAL INFO] setting GPIO0-13 to 1
[HAL INFO] setting route 41050204 = 00200020, 00000030
[HAL INFO] setting route for GPIO0-13
[HAL INFO] setting GPIO0-13 pull to 2
[GPIO:D]LzGpioInit: id 13 is initialized successfully
[HAL INFO] setting GPIO0-13 to 1
[HAL INFO] setting route 41050204 = 00200020, 00000030
[HAL INFO] setting route for GPIO0-13
[HAL INFO] setting GPIO0-13 pull to 2
[HAL INFO] PINCTRL Write before set reg val=0x30
[HAL INFO] PINCTRL Write after  set reg val=0x30
PWM(1) start
[HAL INFO] channel=1, period_ns=1000000, duty_ns=500000
[HAL INFO] channel=1, period=40000, duty=20000, polarity=0
[HAL INFO] Enable channel=1
IotProcess: sleep 5 sec!
PWM(1) end
[HAL INFO] Disable channel=1

...
```

