# 小凌派-RK2206开发板OpenHarmonyOS内核开发-队列

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行队列编程开发。例程创建一个队列，两个任务；任务1调用写队列接口发送消息，任务2调用读队列接口接收消息。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

队列，是一种常用于任务间通信的数据结构。队列可以在任务与任务、任务与中断之间传递消息，实现任务接收来自其他任务或者中断的不固定长度的消息。任务能够从队列里面读取消息，当队列中的消息为空时，读取消息的任务将被阻塞，阻塞的时间可以由用户自行定义，任务会一直等待队列里新消息的到来或者直到等待的时间超过阻塞时间，任务就会从阻塞态转为就绪态。

队列中可以存储有限的、大小固定的数据结构。任务与任务、任务与中断之间要传递的数据保存在队列中；队列所能保存的最大数据数量叫做队列的长度，创建队列的时候需要指定数据的大小和队列的长度。

通过队列，任务或中断可以将一条或者多条消息放入队列中，这样其他的任务就可以通过队列获取消息。当多个消息发送到队列时，通常是将先进队列的消息先传出去，即队列支持先进先出的原则（FIFO）。

### API分析

#### LOS_QueueCreate()

```c
UINT32 LOS_QueueCreate(CHAR *queueName,
                        UINT16 len,
                        UINT32 *queueID,
                        UINT32 flags,
                        UINT16 maxMsgSize);
```

**描述：**

创建一个队列，由系统动态申请队列空间。

**参数：**

| 名字       | 描述             |
| :--------- | :-----------     |
| queueName  | 需要创建的队列名  |
| len        | 需要创建队列的长度|
| queueID    | 创建生成的队列ID  |
| flags      | 队列模式         |
| maxMsgSize | 队列节点大小     |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_QUEUE_CB_UNAVAILABLE <br> LOS_ERRNO_QUEUE_CREATE_NO_MEMORY <br> LOS_ERRNO_QUEUE_CREAT_PTR_NULL <br> LOS_ERRNO_QUEUE_PARA_ISZERO <br> LOS_ERRNO_QUEUE_SIZE_TOO_BIG | 失败 |

#### LOS_QueueDelete()

```c
UINT32 LOS_QueueDelete(UINT32 queueID);
```

**描述：**

根据队列ID删除一个指定队列。

**参数：**

| 名字    | 描述             |
| :------ | :--------------- |
| queueID | 需要删除的队列ID |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_QUEUE_PTR_NULL <br> LOS_ERRNO_QUEUE_INVALID <br> LOS_ERRNO_QUEUE_NOT_CREATE | 失败 |

#### LOS_QueueRead()

```c
UINT32 LOS_QueueRead(UINT32 queueID,
                    VOID *bufferAddr,
                    UINT32 bufferSize,
                    UINT32 timeOut);
```

**描述：**

读取指定队列头节点中的数据（队列节点中的数据实际上是一个地址）。

**参数：**

| 名字       | 描述                 |
| :--------- | :------------------- |
| queueID    | 需要读取的队列ID |
| bufferAddr | 缓冲区指针           |
| bufferSize | 缓冲区大小           |
| timeout    | 超时时间             |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_QUEUE_INVALID <br> LOS_ERRNO_QUEUE_READ_PTR_NULL <br> LOS_ERRNO_QUEUE_READSIZE_ISZERO <br> LOS_ERRNO_QUEUE_READ_IN_INTERRUPT <br> LOS_ERRNO_QUEUE_NOT_CREATE <br> LOS_ERRNO_QUEUE_ISEMPTY <br> LOS_ERRNO_QUEUE_PEND_IN_LOCK <br> LOS_ERRNO_QUEUE_TIMEOUT | 失败 |

### 软件设计

**主要代码分析**

在queue_example函数中，通过LOS_QueueCreate函数创建一个队列；通过LOS_TaskCreate函数创建两个线程：msg_write_thread和msg_read_thread。

```c
void queue_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)msg_write_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "msg_write_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create msg_write_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)msg_read_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "msg_read_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create msg_read_thread ret:0x%x\n", ret);
        return;
    }
}
```

msg_write_thread函数每隔1s，通过LOS_QueueWrite函数向队列中发送消息。

```c
void msg_write_thread(void *arg)
{
    unsigned int data = 0;
    unsigned int ret = LOS_OK;

    while (1)
    {
        data++;
        ret = LOS_QueueWrite(m_msg_queue, (void *)&data, sizeof(data), LOS_WAIT_FOREVER);
        if (LOS_OK != ret)
        {
            printf("%s write Message Queue msg fail ret:0x%x\n", __func__, ret);
        }
        else
        {
            printf("%s write Message Queue msg:%u\n", __func__, data);
        }

        /*delay 1s*/
        LOS_Msleep(1000);
    }
}
```

msg_read_thread函数通过LOS_QueueRead函数读取队列中的消息，当队列中没有消息的时候，msg_read_thread函数阻塞等待消息。

```c
void msg_read_thread(void *arg)
{
    unsigned int addr;
    unsigned int ret = LOS_OK;
    unsigned int *pData = NULL;

    while (1)
    {
        /*wait for message*/
        ret = LOS_QueueRead(m_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            pData = addr;
            printf("%s read Message Queue msg:%u\n", __func__, *pData);
        }
        else
        {
            printf("%s read Message Queue fail ret:0x%x\n", __func__, ret);
        }
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a5_kernel_queue` 参与编译。

```r
"./a5_kernel_queue:queue_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lqueue_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lqueue_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，Thread_Msg_Write函数1S写入一次数据，此时Thread_Msg_Read函数退出阻塞读取数据并打印接收的数据。

```r
msg_write_thread write Message Queue msg:1
msg_read_thread read Message Queue msg:1
msg_write_thread write Message Queue msg:2
msg_read_thread read Message Queue msg:2
msg_write_thread write Message Queue msg:3
msg_read_thread read Message Queue msg:3
msg_write_thread write Message Queue msg:4
msg_read_thread read Message Queue msg:4
```

