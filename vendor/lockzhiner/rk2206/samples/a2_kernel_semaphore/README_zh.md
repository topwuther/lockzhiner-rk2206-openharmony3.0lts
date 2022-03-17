# 小凌派-RK2206开发板OpenHarmonyOS内核开发-信号量

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，通过信号量控制不同的线程，实现任务之间的同步。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

信号量是一种实现任务间通信的机制，可以实现任务间同步或共享资源的互斥访问。

一个信号量的数据结构中，通常有一个计数值，用于对有效资源数的计数，表示剩下的可被使用的共享资源数，其值的含义分两种情况：

* 0，表示该信号量当前不可获取，因此可能存在正在等待该信号量的任务。
* 正值，表示该信号量当前可被获取。

信号量是一种上锁机制，代码必须获得对应的钥匙才能继续执行。一旦获得了钥匙，就意味着该任务具有进入被锁部分代码的权限。一旦执行被锁代码段，则其它任务需要等待，直到被锁部分代码的钥匙被再次释放才能继续执行。任务可以利用信号量与其它任务、中断服务程序进行同步。

本例程设计3个任务，任务control_thread每隔1秒释放信号量，任务sem_one_thread和sem_two_thread则不断地尝试获取信号量。

### API分析

**头文件：**

/kernel/liteos_m/kernel/include/los_sem.h

#### LOS_SemCreate()

```c
UINT32 LOS_SemCreate(UINT16 count, UINT32 *semHandle);
```

**描述：**

创建信号量，返回信号量ID

**参数：**

| 名字      | 描述             |
| :-------- | :--------------- |
| count     | 可用的信号量数量 |
| semHandle | 申请的信号量指针 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SEM_PTR_NULL <br> LOS_ERRNO_SEM_OVERFLOW  <br> LOS_ERRNO_SEM_ALL_BUSY | 失败 |

#### LOS_SemDelete()

```c
UINT32 LOS_SemDelete(UINT32 semHandle);
```

**描述：**

删除指定的信号量

**参数：**

| 名字      | 描述             |
| :-------- | :--------------- |
| semHandle | 需要删除的信号量 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SEM_INVALID <br> LOS_ERRNO_SEM_UNAVAILABLE <br> LOS_ERRNO_SEM_PEND_INTERR <br> LOS_ERRNO_SEM_PEND_IN_LOCK <br> LOS_ERRNO_SEM_TIMEOUT | 失败 |

#### LOS_SemPend()

```c
UINT32 LOS_SemPend(UINT32 semHandle, UINT32 timeout);
```

**描述：**
申请指定的信号量，并设置超时时间

**参数：**

| 名字      | 描述             |
| :-------- | :--------------- |
| semHandle | 需要申请的信号量 |
| timeout   | 超时时间         |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SEM_INVALID <br> LOS_ERRNO_SEM_UNAVAILABLE <br> LOS_ERRNO_SEM_PEND_INTERR <br> LOS_ERRNO_SEM_PEND_IN_LOCK <br> LOS_ERRNO_SEM_TIMEOUT | 失败 |

#### LOS_SemPost()

```c
UINT32 LOS_SemPost(UINT32 semHandle);
```

**描述：**
释放指定的信号量

**参数：**

| 名字      | 描述             |
| :-------- | :--------------- |
| semHandle | 需要释放的信号量 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_SEM_INVALID <br> LOS_ERRNO_SEM_OVERFLOW | 失败 |

### 软件设计

**主要代码分析**

在semaphore_example函数中，通过LOS_SemCreate函数创建m_sem信号量，并通过LOS_TaskCreate函数创建三个线程：control_thread、sem_one_thread和sem_two_thread。

```c
void semaphore_example()
{
    unsigned int thread_crtl;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    TSK_INIT_PARAM_S task3 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_SemCreate(MAX_COUNT, &m_sem);
    if (ret != LOS_OK)
    {
        printf("Falied to create Semaphore\n");
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)control_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "control_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_crtl, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create control_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)sem_one_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "sem_one_thread";
    task2.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create sem_one_thread ret:0x%x\n", ret);
        return;
    }

    task3.pfnTaskEntry = (TSK_ENTRY_FUNC)sem_two_thread;
    task3.uwStackSize = 2048;
    task3.pcName = "sem_two_thread";
    task3.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id2, &task3);
    if (ret != LOS_OK)
    {
        printf("Falied to create sem_two_thread ret:0x%x\n", ret);
        return;
    }
}
```

control_thread函数中通过LOS_SemPost函数释放信号量，sem_one_thread和sem_two_thread函数中，则阻塞等待sem信号量。当Thread_Control函数中释放两次信号量，sem_one_thread和sem_two_thread同步运行；当control_thread函数只释放一次信号量，sem_one_thread和sem_two_thread轮流执行。

```c
void control_thread()
{
    unsigned int count = 0;

    while (1)
    {
        /*释放两次信号量，sem_one_thread和sem_two_thread同步执行;
        释放一次信号量，sem_one_thread和sem_two_thread交替执行*/
        if (count++%3)
        {
            LOS_SemPost(m_sem);
            printf("control_thread Release once Semaphore\n");
        }
        else
        {
            LOS_SemPost(m_sem);
            LOS_SemPost(m_sem);
            printf("control_thread Release twice Semaphore\n");
        }
      
        LOS_Msleep(1000);
    }
}

void sem_one_thread()
{
    while (1)
    {
        /*申请信号量*/
        LOS_SemPend(m_sem, LOS_WAIT_FOREVER);

        printf("sem_one_thread get Semaphore\n");
        LOS_Msleep(100);
    }
}

void sem_two_thread()
{
    while (1)
    {
        /*申请信号量*/
        LOS_SemPend(m_sem, LOS_WAIT_FOREVER);

        printf("sem_two_thread get Semaphore\n");
        LOS_Msleep(100);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a2_kernel_semaphore` 参与编译。

```r
"./a2_kernel_semaphore:semaphore_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lsemaphore_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lsemaphore_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，control_thread一次释放两个信号量，sem_one_thread和sem_two_thread同步执行；control_thread一次释放一个信号量，sem_one_thread和sem_two_thread交替执行。

```r
control_thread Release once Semaphore
sem_one_thread get Semaphore
control_thread Release once Semaphore
sem_two_thread get Semaphore
control_thread Release twice Semaphore
sem_two_thread get Semaphore
sem_one_thread get Semaphore
```

