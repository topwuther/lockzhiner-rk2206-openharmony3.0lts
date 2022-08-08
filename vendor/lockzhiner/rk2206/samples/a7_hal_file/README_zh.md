# 小凌派-RK2206开发板OpenHarmonyOS内核开发-事件

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行事件编程开发。例程创建一个事件，两个任务；任务1调用读事件接口等待事件通知，任务2调用写事件接口通知任务1事件到达。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

事件是一种实现任务间通信的机制，可用于实现任务间的同步，但是仅仅作为事件类型的通信，不提供数据传输功能。一个任务可以等待多个事件的发生：可以是任意一个事件发生时唤醒任务进行事件处理;也可以是几个事件都发生后才唤醒任务进行事件处理。事件集合用32位无符号整型变量来表示，每一位代表一个事件。

多任务环境下，任务之间往往需要同步操作，一个等待即是一个同步。事件可以提供一对多、多对多的同步操作。一对多同步模型：一个任务等待多个事件的触发；多对多同步模型：多个任务等待多个事件的触发。任务可以通过创建事件控制块来实现对事件的触发和等待操作。

### API分析

#### LOS_EventInit()

```c
UINT32 LOS_EventInit(PEVENT_CB_S eventCB);
```

**描述：**

事件控制块初始化。

**参数：**

|名字|描述|
|:--|:------| 
| eventCB | 事件控制块指针 |

**返回值：**
|返回值|描述|
|:--|:------|
| LOS_OK | 成功 |
| LOS_ERRNO_EVENT_PTR_NULL | 失败 |

#### LOS_EventRead()

```c
UINT32 LOS_EventRead(PEVENT_CB_S eventCB, UINT32 eventMask, UINT32 mode, UINT32 timeOut);
```

**描述：**

读事件（等待事件），任务会根据timeOut（单位：tick）进行阻塞等待；
未读取到事件时，返回值为0；正常读取到事件时，返回正值（事件发生的集合）；其他情况返回特定错误码。

**参数：**

|名字|描述|
|:--|:------| 
| eventCB | 事件控制块指针 |
| eventMask | 事件掩码 |
| mode | 事件读取的模式 |
| timeOut | 超时时间 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_EVENT_SETBIT_INVALID <br> LOS_ERRNO_EVENT_EVENTMASK_INVALID <br> LOS_ERRNO_EVENT_READ_IN_INTERRUPT <br> LOS_ERRNO_EVENT_FLAGS_INVALID <br> LOS_ERRNO_EVENT_READ_IN_LOCK <br> LOS_ERRNO_EVENT_PTR_NULL | 失败 |

#### LOS_EventWrite()

```c
UINT32 LOS_EventWrite(PEVENT_CB_S eventCB, UINT32 events);
```

**描述：**

写一个特定的事件到事件控制块。

**参数：**

|名字|描述|
|:--|:------| 
| eventCB | 事件控制块指针 |
| events | 要写入的事件掩码 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_EVENT_SETBIT_INVALID <br> LOS_ERRNO_EVENT_PTR_NULL | 失败 |

#### LOS_EventClear()

```c
UINT32 LOS_EventClear(PEVENT_CB_S eventCB, UINT32 eventMask);
```

**描述：**

根据events掩码，清除事件控制块中的事件。

**参数：**

|名字|描述|
|:--|:------| 
| eventCB | 事件控制块指针 |
| eventMask| 要清除的事件掩码 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_EVENT_PTR_NULL | 失败 |

### 软件设计

**主要代码分析**

在event_example函数中，通过LOS_EventInit函数创建事件，并通过LOS_TaskCreate函数创建两个线程：event_master_thread和event_slave_thread。

```c
void event_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_EventInit(&m_event);
    if (ret != LOS_OK)
    {
        printf("Falied to create EventFlags\n");
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)event_master_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "event_master_thread";
    task1.usTaskPrio = 5;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create event_master_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)event_slave_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "event_slave_thread";
    task2.usTaskPrio = 5;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create event_slave_thread ret:0x%x\n", ret);
        return;
    }
}
```

event_slave_thread线程函数中通过LOS_EventRead函数将线程置于阻塞状态，等待事件到达；在event_master_thread函数中通过LOS_EventWrite函数每隔2S写入事件，实现线程的同步，2s后清除事件，重复以上流程。

```c
void event_master_thread()
{
    unsigned int ret = LOS_OK;

    LOS_Msleep(1000);

    while (1)
    {
        printf("%s write event:0x%x\n", __func__, EVENT_WAIT);
        ret = LOS_EventWrite(&m_event, EVENT_WAIT);
        if (ret != LOS_OK) {
            printf("%s write event failed ret:0x%x\n", __func__, ret);
        }

        /*delay 1s*/
        LOS_Msleep(2000);
        LOS_EventClear(&m_event, ~m_event.uwEventID);
    }
}

void event_slave_thread()
{
    unsigned int event;

    while (1)
    {
        /* 阻塞方式读事件，等待事件到达*/
        event = LOS_EventRead(&m_event, EVENT_WAIT, LOS_WAITMODE_AND, LOS_WAIT_FOREVER);
        printf("%s read event:0x%x\n", __func__, event);
        LOS_Msleep(1000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a6_kernel_event` 参与编译。

```r
"./a6_kernel_event:event_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-levent_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -levent_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志，event_master_thread函数每隔2s写入事件，2s后清除事件；event_slave_thread函数阻塞等待事件达到，事件到达后，每1s打印一次读事件信息。

```r
event_master_thread write event:0x1
event_slave_thread read event:0x1
event_slave_thread read event:0x1
event_master_thread write event:0x1
event_slave_thread read event:0x1
event_slave_thread read event:0x1
```

