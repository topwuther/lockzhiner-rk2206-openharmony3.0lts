# 小凌派-RK2206开发板OpenHarmonyOS内核开发-任务

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行任务编程开发。例程创建两个任务，任务1 1s执行一次，任务2 2s执行一次。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

从系统的角度看，任务是竞争系统资源的最小运行单元。任务可以使用或等待CPU、使用内存空间等系统资源，并且独立于其它任务运行。鸿蒙LiteOS的任务模块可以给用户提供多个任务，实现了任务之间的切换和通信，帮助用户管理任务程序流程。鸿蒙LiteOS是一个支持多任务的操作系统，一个任务表示一个线程。

鸿蒙LiteOS操作系统的任务管理是抢占式调度机制，同时支持时间片轮转调度方式。高优先级的任务可打断低优先级任务，低优先级任务只能在高优先级任务阻塞或结束后才能得到调度。鸿蒙LiteOS操作系统的任务一共有32个优先级(0-31)，最高优先级为0，最低优先级为31（其中0和31为系统占用）。

### API分析

**头文件：**

/kernel/liteos_m/kernel/include/los_task.h

#### LOS_TaskCreate()

```c
UINT32 LOS_TaskCreate(UINT32 *taskID, TSK_INIT_PARAM_S *taskInitParam);
```

**描述：**
创建任务，并使该任务进入ready状态，如果就绪队列中没有更高优先级的任务，则运行该任务。

**参数：**

| 名字          | 描述           |
| :------------ | :------------- |
| taskID        | 任务ID         |
| taskInitParam | 线程初始化参数 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_TSK_ID_INVALID <br> LOS_ERRNO_TSK_PTR_NULL <br> LOS_ERRNO_TSK_NAME_EMPTY <br> LOS_ERRNO_TSK_ENTRY_NULL <br> LOS_ERRNO_TSK_PRIOR_ERROR <br> LOS_ERRNO_TSK_STKSZ_TOO_LARGE <br> LOS_ERRNO_TSK_STKSZ_TOO_SMALL <br> LOS_ERRNO_TSK_TCB_UNAVAILABLE <br> LOS_ERRNO_TSK_NO_MEMORY | 失败 |

#### LOS_TaskDelete()

```c
UINT32 LOS_TaskDelete(UINT32 taskID);
```

**描述：**
删除指定的任务。

**参数：**

| 名字   | 描述             |
| :----- | :--------------- |
| taskID | 需要删除的任务ID |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_TSK_OPERATE_IDLE <br> LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED <br> LOS_ERRNO_TSK_ID_INVALID <br> LOS_ERRNO_TSK_NOT_CREATED | 失败 |


### 软件设计

**主要代码分析**

在task_example函数中，通过LOS_TaskCreate函数创建task_one和task_two两个任务。

```c
void task_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)task_one;
    task1.uwStackSize = 2048;
    task1.pcName = "Task_One";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create Task_One ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)task_two;
    task2.uwStackSize = 2048;
    task2.pcName = "Task_Two";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create Task_Two ret:0x%x\n", ret);
        return;
    }
}
```

task_one函数每1秒执行一次打印日志。

```c
void task_one()
{
    while (1)
    {
        printf("This is %s\n", __func__);
        LOS_Msleep(1000);
    }
}
```

task_two函数每2秒执行一次打印日志。

```c
void task_two()
{
    while (1)
    {
        printf("This is %s\n", __func__);
        LOS_Msleep(2000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a1_kernal_task` 参与编译。

```r
"./a1_kernal_task:task_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-ltask_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -ltask_example
```

### 运行结果 

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，task_one和task_two会交替打印信息，task_one 1s打印一次，task_two 2s打印一次。

```c
This is task_one
This is task_one
This is task_two
This is task_one
This is task_one
This is task_two
```
