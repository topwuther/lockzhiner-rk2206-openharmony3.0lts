# 小凌派-RK2206开发板OpenHarmonyOS内核开发-定时器

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行定时器编程开发。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

软件定时器，是基于系统Tick时钟中断，由软件来模拟的定时器，当经过设定的Tick时钟计数值后会触发用户定义的回调函数。定时器的精度与系统Tick时钟的周期有关。硬件定时器受硬件的限制，数量上不足以满足用户的实际需求。因此为了满足用户需求，提供更多的定时器，鸿蒙LiteOS操作系统提供软件定时器功能。

软件定时器是系统资源，在模块初始化的时候已经分配了一块连续的内存。软件定时器使用了系统的一个队列和一个任务资源，软件定时器的触发遵循队列规则，先进先出。定时时间短的定时器总是比定时时间长的靠近队列头，满足优先被触发的准则。

### API分析

**头文件：**

/kernel/liteos_m/kernel/include/los_task.h/los_swtmr.h

#### LOS_SwtmrCreate()

```c
UINT32 LOS_SwtmrCreate(UINT32 interval,
                        UINT8 mode,
                        SWTMR_PROC_FUNC handler,
                        UINT32 *swtmrID,
                        UINT32 arg);
```

**描述：**

创建定时器

**参数：**

| 名字     | 描述               |
| :------- | :----------------- |
| interval | 定时器超时时间     |
| mode     | 定时器模式         |
| handler  | 定时器回调函数     |
| swtmrID  | 定时器ID指针       |
| arg      | 定时器回调函数参数 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SWTMR_INTERVAL_NOT_SUITED <br> LOS_ERRNO_SWTMR_MODE_INVALID <br> LOS_ERRNO_SWTMR_PTR_NULL <br> LOS_ERRNO_SWTMR_RET_PTR_NULL <br> LOS_ERRNO_SWTMR_MAXSIZE | 失败 |

#### LOS_SwtmrDelete()

```c
UINT32 LOS_SwtmrDelete(UINT32 swtmrID);
```

**描述：**

删除定时器

**参数：**

| 名字    | 描述     |
| :------ | :------- |
| swtmrID | 定时器ID |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SWTMR_ID_INVALID <br> LOS_ERRNO_SWTMR_NOT_CREATED <br> LOS_ERRNO_SWTMR_STATUS_INVALID | 失败 |

#### LOS_SwtmrStart()

```c
UINT32 LOS_SwtmrStart(UINT32 swtmrID);
```

**描述：**

启动定时器

**参数：**

| 名字    | 描述     |
| :------ | :------- |
| swtmrID | 定时器ID |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SWTMR_ID_INVALID <br> LOS_ERRNO_SWTMR_NOT_CREATED <br> LOS_ERRNO_SWTMR_STATUS_INVALID | 失败 |

#### LOS_SwtmrStop()

```c
UINT32 LOS_SwtmrStop(UINT32 swtmrID);
```

**描述：**

停止定时器

**参数：**

| 名字    | 描述     |
| :------ | :------- |
| swtmrID | 定时器ID |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SWTMR_ID_INVALID <br> LOS_ERRNO_SWTMR_NOT_CREATED <br> LOS_ERRNO_SWTMR_NOT_STARTED <br> LOS_ERRNO_SWTMR_STATUS_INVALID | 失败 |

### 软件设计

**主要代码分析**

在timer_example函数中，通过LOS_SwtmrCreate函数创建定时器1和定时器2，并通过LOS_SwtmrStart函数启动定时器1和定时器2。1S超时后会触发Timer1_Timeout函数并打印日志；2S超时后会触发Timer2_Timeout函数并打印日志。

```c
void timer_example()
{
    unsigned int timer_id1, timer_id2;
    unsigned int ret;

    ret = LOS_SwtmrCreate(1000, LOS_SWTMR_MODE_PERIOD, timer1_timeout, &timer_id1, NULL);
    if (ret == LOS_OK)
    {
        ret = LOS_SwtmrStart(timer_id1);
        if (ret != LOS_OK)
        {
            printf("start timer1 fail ret:0x%x\n", ret);
            return;
        }
    }
    else
    {
        printf("create timer1 fail ret:0x%x\n", ret);
        return;
    }

    ret = LOS_SwtmrCreate(2000, LOS_SWTMR_MODE_PERIOD, timer2_timeout, &timer_id2, NULL);
    if (ret == LOS_OK)
    {
        ret = LOS_SwtmrStart(timer_id2);
        if (ret != LOS_OK)
        {
            printf("start timer2 fail ret:0x%x\n", ret);
            return;
        }
    }
    else
    {
        printf("create timer2 fail ret:0x%x\n"), ret;
        return;
    }
}

void timer1_timeout(void *arg)
{
    printf("This is Timer1 Timeout function\n");
}

void timer2_timeout(void *arg)
{
    printf("This is Timer2 Timeout function\n");
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a3_kernel_timer` 参与编译。

```r
"./a3_kernel_timer:timer_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-ltimer_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -ltimer_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，Timer1_Timeout函数会1秒打印一次日志，Timer2_Timeout函数会2秒打印一次日志。

```r
This is Timer1 Timeout function
This is Timer1 Timeout function
This is Timer2 Timeout function
This is Timer1 Timeout function
This is Timer1 Timeout function
This is Timer2 Timeout function
```

