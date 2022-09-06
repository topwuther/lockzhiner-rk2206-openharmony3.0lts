# 小凌派-RK2206开发板OpenHarmonyOS内核开发-文件

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行文件读写开发。

例程：

（1）创建一个文件；

（2）每5秒进行1次文件读写操作；

（3）文件标识移动到文件起始处，读文件内容，并打印；

（4）文件标识移动到文件起始处，写文件内容；

（5）循环上述的第2～4步骤。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

在本章节中，我们将了解OpenHarmony标准文件操作接口，如文件如何创建、打开、读写和关闭文件。

### API分析

#### 头文件

```shell
//utils/native/lite/hals/file/hal_file.h
```

#### HalFileOpen()

```c
int HalFileOpen(const char *path, int oflag, int mode);
```

**描述：**

打开/创建文件，类似于Linux的open函数。

**参数：**

|名字|描述|
|:--|:------|
| path | 文件路径 |
| oflag | 参考///utils/native/lite/include/utils_file.h<br>O_RDONLY_FS：只读<br>O_WRONLY_FS：只写<br>O_RDWR_FS：读写<br>O_CREAT_FS：如果没有则创建<br>O_EXCL_FS：如果没有则创建；如有则不能打开<br>O_TRUNC_FS：如果文件存在，则清空文件内容<br>O_APPEND_FS：如果文件存在，则标记位置移动到文件最后 |
| mode | 0 |

**返回值：**

|返回值|描述|
|:--|:------|
| LOS_OK | 成功 |
| 其它 | 失败 |

#### HalFileClose()

```c
int HalFileClose(int fd);
```

**描述：**

关闭文件。

**参数：**

|名字|描述|
|:--|:------|
| fd | 文件句柄 |

**返回值：**
|返回值|描述|
|:--|:------|
| LOS_OK | 成功 |
| 其它 | 失败 |

#### HalFileRead()

```c
int HalFileRead(int fd, char* buf, unsigned int len);
```

**描述：**

从文件中读取一段内容。

**参数：**

|名字|描述|
|:--|:------|
| fd | 文件句柄 |
| buf | 从文件读取内容的缓冲区 |
| len | 从文件读取内容的大小 |

**返回值：**
|返回值|描述|
|:--|:------|
| >= 0 | 成功，表示成功从文件读取内容的大小 |
| < 0 | 失败 |

#### HalFileWrite()

```c
int HalFileWrite(int fd, const char* buf, unsigned int len);
```

**描述：**

往文件写入一段内容。

**参数：**

|名字|描述|
|:--|:------|
| fd | 文件句柄 |
| buf | 需要写入到文件的内容缓冲区 |
| len | 需要写入到文件的内容大小 |

**返回值：**
|返回值|描述|
|:--|:------|
| >= 0 | 成功，表示成功写入到文件的内容大小 |
| < 0 | 失败 |

#### HalFileDelete()

```c
int HalFileDelete(const char* path);
```

**描述：**

删除文件。

**参数：**

| 名字 | 描述     |
| :--- | :------- |
| path | 文件路径 |

**返回值：**

| 返回值 | 描述 |
| :----- | :--- |
| 0      | 成功 |
| 其它   | 失败 |

#### HalFileStat()

```c
int HalFileStat(const char* path, unsigned int* fileSize);
```

**描述：**

删除文件。

**参数：**

| 名字     | 描述         |
| :------- | :----------- |
| path     | 文件路径     |
| fileSize | 文件内容大小 |

**返回值：**

| 返回值 | 描述 |
| :----- | :--- |
| 0      | 成功 |
| 其它   | 失败 |

#### HalFileSeek()

```c
int HalFileSeek(int fd, int offset, unsigned int whence);
```

**描述：**

文件标记移动。

**参数：**

| 名字   | 描述                                                         |
| :----- | :----------------------------------------------------------- |
| fd     | 文件句柄                                                     |
| offset | 文件位置移动位数                                             |
| whence | SEEK_SET_FS：从文件开头移动<br>SEEK_CUR_FS：从文件当前位置移动<br/>SEEK_END_FS：从文件结尾移动 |

**返回值：**

| 返回值 | 描述 |
| :----- | :--- |
| 0      | 成功 |
| 其它   | 失败 |

### 软件设计

**主要代码分析**

在file_example函数中通过LOS_TaskCreate函数创建一个线程：hal_file_thread。

```c
void file_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_file_thread;
    task.uwStackSize = 1024 * 10;
    task.pcName = "hal_file_thread";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create hal_file_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(file_example);
```

hal_file_thread函数负责打开文件，每5秒移动到文件头读取数据，再移动到文件头写入一段内容，重复以上流程。

```c
void hal_file_thread()
{
    int fd;
    char buffer[1024];
    int read_length, write_length;
    int current = 0;

    /* 打开文件，如果没有该文件就创建，如有该文件则打开
     * O_TRUNC_FS => 清空文件内容
     */
    //fd = HalFileOpen(FILE_NAME, O_RDWR_FS | O_CREAT_FS, 0);
    fd = HalFileOpen(FILE_NAME, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1)
    {
        printf("%s HalFileOpen failed!\n", FILE_NAME);
        return;
    }

    while (1)
    {
        /* 文件位置移动到文件开始位置 */
        HalFileSeek(fd, 0, SEEK_SET);
        memset(buffer, 0, sizeof(buffer));
        /* 读取文件内容 */
        read_length = HalFileRead(fd, buffer, sizeof(buffer));
        printf("read: \n");
        printf("    length = %d\n", read_length);
        printf("    content = %s\n", buffer);

        /* 文件位置移动到文件开始位置 */
        HalFileSeek(fd, 0, SEEK_SET);
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Hello World(%d) => ", current);
        /* 写入文件 */
        write_length = HalFileWrite(fd, buffer, strlen(buffer));

        current++;
        LOS_Msleep(5000);
    }

    HalFileClose(fd);
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a7_hal_file` 参与编译。

```r
"./a7_hal_file:hal_file_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lhal_file_example` 参与编译。

```r
app_LIBS = -lhal_file_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志。

```r
HalFileInit: Flash Init Successful!
read:
    length = 0
    content =
read:
    length = 18
    content = Hello World(0) =>
read:
    length = 18
    content = Hello World(1) =>
```

## 注意事项

（1）如果出现内存检测错误，则需要加大任务的堆栈大小，即：

```c
void file_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_file_thread;
    task.uwStackSize = 1024 * 10;						/* 如果出现内存堆栈错误，则加大任务堆栈大小 */
    task.pcName = "hal_file_thread";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create hal_file_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(file_example);
```



