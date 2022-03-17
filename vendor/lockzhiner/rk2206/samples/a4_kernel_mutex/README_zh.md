# 小凌派-RK2206开发板OpenHarmonyOS内核开发-互斥锁

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行互斥锁编程开发。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

互斥锁也叫做互斥型信号量，是一种特殊的二值性信号量，用于实现对共享资源的独占式处理。

在任意时刻互斥锁的状态只有两种，开锁或闭锁。当有任务持有时，互斥锁处于闭锁状态，这个任务获得该互斥锁的所有权。当该任务释放它时，该互斥锁被开锁，任务失去该互斥锁的所有权。当一个任务持有互斥锁时，其他任务将不能再对该互斥锁进行开锁或持有。

多任务环境下往往存在多个任务竞争同一共享资源的应用场景，互斥锁可被用于对共享资源的保护从而实现独占式访问；另外，互斥锁可以解决信号量存在的优先级翻转问题。需要注意的是，互斥锁不能在中断服务程序中使用。

### API分析

#### LOS_MuxCreate()

```c
UINT32 LOS_MuxCreate(UINT32 *muxHandle);
```

**描述：**

创建互斥锁

**参数：**

| 名字      | 描述             |
| :---------- | :----------------- |
| muxHandle | 创建的互斥锁指针 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_MUX_PTR_NULL <br> LOS_ERRNO_MUX_ALL_BUSY | 失败 |

#### LOS_MuxDelete()

```c
UINT32 LOS_MuxDelete(UINT32 muxHandle);
```

**描述：**

删除指定的互斥锁

**参数：**

| 名字      | 描述             |
| :---------- | :----------------- |
| muxHandle | 需要删除的互斥锁 |

**返回值：**
|返回值|描述|
|:--|:------|
| LOS_OK | 成功 |
| LOS_ERRNO_MUX_INVALID <br> LOS_ERRNO_MUX_PENDED | 失败 |

#### LOS_MuxPend()

```c
UINT32 LOS_MuxPend(UINT32 muxHandle, UINT32 timeout);
```

**描述：**

申请指定的互斥锁

**参数：**

| 名字      | 描述             |
| :---------- | :----------------- |
| muxHandle | 需要申请的互斥锁 |
| timeout   | 超时时间         |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_MUX_INVALID <br> LOS_ERRNO_MUX_UNAVAILABLE <br> LOS_ERRNO_MUX_PEND_INTERR <br> LOS_ERRNO_MUX_PEND_IN_LOCK <br> LOS_ERRNO_MUX_TIMEOUT | 失败 |

#### LOS_MuxPost()

```c
UINT32 LOS_MuxPost(UINT32 muxHandle);
```

**描述：**

释放指定的互斥锁

**参数：**

| 名字      | 描述             |
| :---------- | :----------------- |
| muxHandle | 需要释放的互斥锁 |

**返回值：**
|返回值|描述|
|:--|:------| 
| LOS_OK | 成功 |
| LOS_ERRNO_MUX_INVALID <br> LOS_ERRNO_MUX_PEND_INTERR | 失败 |

### 软件设计

**主要代码分析**

在mutex_example函数中，通过LOS_MuxCreate函数创建了互斥锁，并创建的两个线程write_thread和read_thread。

```c
void mutex_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    unsigned int ret = LOS_OK;

    ret = LOS_MuxCreate(&m_mutex_id);
    if (ret != LOS_OK)
    {
        printf("Falied to create Mutex\n");
    }

    ret = CreateThread(&thread_id1, write_thread, NULL, "write_thread");
    if (ret != LOS_OK)
    {
        printf("Falied to create write_thread\n");
        return;
    }

    ret = CreateThread(&thread_id2, read_thread, NULL, "read_thread");
    if (ret != LOS_OK)
    {
        printf("Falied to create read_thread\n");
        return;
    }
}
```

在write_thread线程函数中，先获得互斥锁，写入数据，并持有它时延迟3s。在read_thread线程函数中，延时1s后，申请互斥锁等待，线程被阻塞；3S后write_thread线程释放互斥锁，read_thread线程获得互斥锁，读取数据。

```c
void write_thread()
{
    while (1)
    {
        LOS_MuxPend(m_mutex_id, LOS_WAIT_FOREVER);

        m_data++;
        printf("write_thread write data:%u\n", m_data);

        LOS_Msleep(3000);
        LOS_MuxPost(m_mutex_id);
    }
}

void read_thread()
{
    /*delay 1s*/
    LOS_Msleep(1000);

    while (1)
    {
        LOS_MuxPend(m_mutex_id, LOS_WAIT_FOREVER);
        printf("read_thread read data:%u\n", m_data);
  
        LOS_Msleep(1000);
        LOS_MuxPost(m_mutex_id);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a4_kernel_mutex` 参与编译。

```r
"./a4_kernel_mutex:mutex_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lmutex_example` 参与编译。

```r
hardware_LIBS = -lhal_iothardware -lhardware -lmutex_example
```

### 运行结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，write_thread线程函数先写入数据阻塞3s，read_thread线程函数阻塞3s后读取数据。

```r
write_thread write data:1
read_thread read data:1
write_thread write data:2
read_thread read data:2
write_thread write data:3
read_thread read data:3
```

