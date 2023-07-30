# 小凌派-RK2206开发板OpenHarmonyOS内核开发-目录

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行文件读写开发。

例程：

（1）创建2个文件；

（2）每5秒进行列出当前目录所有文件以及文件夹；

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

#### HalFileReadDir()

```c
int HalFileReadDir(const char *path, struct dirent *entry, int entry_max, int *entry_length);
```

**描述：**

查看path目录下的所有文件以及文件夹。

**参数：**

| 名字         | 描述                      |
| :----------- | :------------------------ |
| path         | 目录路径                  |
| entry        | struct dirent数组         |
| entry_max    | struct dirent数组最大长度 |
| entry_length | struct dirent数组实际长度 |

**返回值：**

| 返回值 | 描述 |
| :----- | :--- |
| 0      | 成功 |
| 其它   | 失败 |

### 软件设计

**主要代码分析**

在dir_example函数中通过LOS_TaskCreate函数创建一个线程：hal_dir_process。

```c
void dir_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;
    
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_dir_process;
    task.uwStackSize = 1024 * 1024;
    task.pcName = "hal_dir_thread";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK) {
        printf("Falied to create hal_file_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(dir_example);
```

hal_dir_process函数负责新建2个文件，每5秒查看当前目录的所有文件以及文件夹，重复以上流程。

```c
void hal_dir_process()
{
    int fd;
    struct dirent entry[ENTRY_MAXSIZE];
    int entry_length = 0;
    int ret;
    int i;
    
    // 创建第1个文件
    fd = HalFileOpen(FILE_NAME1, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1) {
        printf("%s HalFileOpen failed!\n", FILE_NAME1);
        return;
    }
    HalFileClose(fd);

    // 创建第2个文件
    fd = HalFileOpen(FILE_NAME2, O_RDWR_FS | O_CREAT_FS | O_TRUNC_FS, 0);
    if (fd == -1) {
        printf("%s HalFileOpen failed!\n", FILE_NAME2);
        return;
    }
    HalFileClose(fd);
    
    while (1) {
        memset(entry, 0, sizeof(entry));
        entry_length= 0;

        // 读取当前目录下所有的文件或文件夹
        ret = HalFileReadDir(DIR_PATH, &entry[0], ENTRY_MAXSIZE, &entry_length);
        if (ret != 0) {
            printf("%s, %d: HalFileReadDir failed(%d)\n", __FILE__, __LINE__, ret);
        } else {
            printf("entry_length = %d\n", entry_length);
            for (i = 0; i < entry_length; i++) {
                switch (entry[i].d_type) {
                    case DT_BLK:
                        printf("i(%d), name(%s), type(%d)(block device)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_CHR:
                        printf("i(%d), name(%s), type(%d)(character device)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_DIR:
                        printf("i(%d), name(%s), type(%d)(directory)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_FIFO:
                        printf("i(%d), name(%s), type(%d)(pipe or fifo)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_LNK:
                        printf("i(%d), name(%s), type(%d)(symbolic link)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_REG:
                        printf("i(%d), name(%s), type(%d)(regular file)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    case DT_SOCK:
                        printf("i(%d), name(%s), type(%d)(UNIX domain socket)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                    default:
                        printf("i(%d), name(%s), type(%d)(could not be define)\n", i, entry[i].d_name, entry[i].d_type);
                        break;
                }
            }
        }
        
        printf("\n");
        LOS_Msleep(5000);
    }
}
```

## 编译调试

### 修改 BUILD.gn 文件

修改 `vendor/lockzhiner/rk2206/sample` 路径下 BUILD.gn 文件，指定 `a9_hal_dir` 参与编译。

```r
"./a9_hal_dir:hal_dir_example",
```

修改 `device/lockzhiner/rk2206/sdk_liteos` 路径下 Makefile 文件，添加 `-lhal_dir_example` 参与编译。

```r
app_LIBS = -lhal_dir_example
```

### 运行结果

示例代码编译烧录代码后，按下开发板的RESET按键，通过串口助手查看日志。

```r
HalFileInit: Flash Init Successful!
entry_length = 5
i(0), name(.), type(4)(directory)
i(1), name(..), type(4)(directory)
i(2), name(deviceid), type(8)(regular file)
i(3), name(helloworld.c), type(8)(regular file)
i(4), name(test.txt), type(8)(regular file)

entry_length = 5
i(0), name(.), type(4)(directory)
i(1), name(..), type(4)(directory)
i(2), name(deviceid), type(8)(regular file)
i(3), name(helloworld.c), type(8)(regular file)
i(4), name(test.txt), type(8)(regular file)

```





