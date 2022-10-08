# 小凌派-RK2206开发板OpenHarmony内核开发-Hello World

## 实验内容

本例程演示如何在小凌派-RK2206开发板上使用鸿蒙LiteOS-M内核接口，进行编程开发。例程创建两个任务，任务1每隔1s执行一次，打印Hello World；任务2每隔2s执行一次，打印Hello OpenHarmony。

![小凌派-RK2206开发板](/vendor/lockzhiner/rk2206/docs/figures/lockzhiner-rk2206.jpg)

## 程序设计

### 创建a0_hello_world文件夹

在OpenHarmony源代码主目录vendor/lockzhiner/rk2206/samples创建a0_hello_world文件夹。

```shell
mkdir -p a0_hello_world
```

### 创建hello_world.c文件

在a0_hello_world文件夹下创建hello_world.c文件。其中，task_example函数负责创建2个任务，分别为task_helloworld和task_openharmony任务，task_helloworld函数负责每隔1秒打印Hello World，task_openharmony任务负责每隔2秒打印Hello OpenHarmony。具体代码如下所示：

```c
#include "los_task.h"   // OpenHarmony LiteOS的任务管理头文件

/***************************************************************
* 函数名称: task_helloworld
* 说    明: 线程函数helloworld
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_helloworld()
{
    while (1)
    {
        printf("Hello World\n");
        /* 睡眠1秒。该函数为OpenHarmony LiteoS内核睡眠函数，单位为：毫秒 */
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: task_openharmony
* 说    明: 线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_openharmony()
{
    while (1)
    {
        printf("Hello OpenHarmony\n");
        /* 睡眠1秒。该函数为OpenHarmony内核睡眠函数，单位为：毫秒 */
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: task_example
* 说    明: 内核任务创建例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_example()
{
    /* 任务id */
    unsigned int thread_id1;
    unsigned int thread_id2;
    /* 任务参数 */
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    /* 返回值 */
    unsigned int ret = LOS_OK;

    /* 创建HelloWorld任务 */
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)task_helloworld;   // 运行函数入口
    task1.uwStackSize = 2048;                               // 堆栈大小
    task1.pcName = "task_helloworld";                       // 函数注册名称
    task1.usTaskPrio = 24;                                  // 任务的优先级，从0~63
    ret = LOS_TaskCreate(&thread_id1, &task1);              // 创建任务
    if (ret != LOS_OK)
    {
        printf("Falied to create task_helloworld ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)task_openharmony;  // 运行函数入口
    task2.uwStackSize = 2048;                               // 堆栈大小
    task2.pcName = "task_openharmony";                      // 函数注册名称
    task2.usTaskPrio = 25;                                  // 任务的优先级，从0~63
    ret = LOS_TaskCreate(&thread_id2, &task2);              // 创建任务
    if (ret != LOS_OK)
    {
        printf("Falied to create task_openharmony ret:0x%x\n", ret);
        return;
    }
}
```

### 创建BUILD.gn

在a0_hello_world文件夹下创建BUILD.gn文件。BUILD.gn负责将hello_world.c文件编译成静态库libtask_helloworld.a。BUILD.gn的语法为gn语法，对gn语法感兴趣的同学可以上gn官网阅读相关文档。BUILD.gn具体内容如下：

```gn
static_library("task_helloworld") {
    sources = [
        "hello_world.c",
    ]

    include_dirs = [
        "//utils/native/lite/include",
    ]
}
```

### 修改main.c文件

修改OpenHarmony主目录device/rockchip/rk2206/sdk_liteos/board目录下的main.c。该文件为OpenHarmony操作系统的主函数。在main.c文件中添加运行hello_world.c文件的task_example函数。具体内容如下：

```c
#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_compiler.h"
#include "lz_hardware.h"
#include "config_network.h"

#define  MAIN_TAG              "MAIN"
int DeviceManagerStart();
void IotInit(void);
void task_example(); // 申明函数

/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT int Main(void)
{
    int ret;
    LZ_HARDWARE_LOGD(MAIN_TAG, "%s: enter ...", __func__);
  
    HalInit();

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        IotInit();
        task_example(); // 添加hello_world.c文件的函数
        OHOS_SystemInit();
        ClkDevInit();
        /* 开启驱动管理服务 */
        //DeviceManagerStart();
        //ExternalTaskConfigNetwork();
        LZ_HARDWARE_LOGD(MAIN_TAG, "%s: LOS_Start ...", __func__);
        LOS_Start();
    }

    while (1) {
        __asm volatile("wfi");
    }
}
```

## 编译步骤

### 修改Makefile

修改OpenHarmony主目录device/rockchip/rk2206/sdk_liteos目录下的Makefile。该Makefile文件负责将编译好的静态库和可执行文件打包成bin文件。本节将固件库libtask_helloworld.a添加到Makefile中，让其最终编译成bin文件。具体修改如下所示：

```makefile
#######################################
# LDFLAGS
#######################################
LDSCRIPT = board.ld

boot_LIBS = -lbootstrap -lbroadcast
hardware_LIBS = -lhal_iothardware -lhardware -ltask_helloworld
```

注意：最后一行添加 `-ltask_helloworld`。

### 修改BUILD.gn

修改OpenHarmony主目录device/rockchip/rk2206/sdk_liteos目录下的BUILD.gn。该BUILD.gn文件负责编译各个组建，包括静态库task_helloworld。具体内容如下所示：

```gn
import("//build/lite/config/component/lite_component.gni")
import("//build/lite/config/subsystem/lite_subsystem.gni")

declare_args() {
  enable_hos_vendor_wifiiot_xts = false
}

lite_subsystem("wifiiot_sdk") {
  subsystem_components = [ ":sdk" ]
}

build_ext_component("liteos") {
  exec_path = rebase_path(".", root_build_dir)
  outdir = rebase_path(root_out_dir)
  command = "sh ./build.sh $outdir"
  deps = [
    ":sdk",
    "//build/lite:ohos",
  ]
  if (enable_hos_vendor_wifiiot_xts) {
    deps += [ "//build/lite/config/subsystem/xts:xts" ]
  }
}

lite_component("sdk") {
  features = []

  deps = [
    "//device/rockchip/rk2206/sdk_liteos/board:board",
    "//device/rockchip/hardware:hardware",
    # xts
    "//device/rockchip/rk2206/adapter/hals/update:hal_update_static",
    # xts
    "//base/update/ota_lite/frameworks/source:hota",
    "../third_party/littlefs:lzlittlefs",
    "//build/lite/config/component/cJSON:cjson_static",
    "../third_party/lwip:rk2206_lwip",
    "//kernel/liteos_m/components/net/lwip-2.1:lwip",
    "//vendor/lockzhiner/rk2206/samples:app",
    "//third_party/paho_mqtt:pahomqtt_static",
    "//vendor/lockzhiner/rk2206/hdf_config:hdf_config",
    "//vendor/lockzhiner/rk2206/hdf_drivers:hdf_drivers",
    "//drivers/adapter/khdf/liteos_m/test/sample_driver:sample_driver",
    #"//drivers/adapter/uhdf/manager:hdf_manager",
    #"//drivers/adapter/uhdf/posix:hdf_posix",
    "//vendor/lockzhiner/rk2206/samples/a0_hello_world:task_helloworld",
  ]
}
```

注意：倒数第三行内容就是添加task_helloworld。其中，“:”前面为需要编译的路径，“//”表示OpenHarmony源代码主目录；“:”后面为需要编译的选项，即//vendor/lockzhiner/rk2206/samples/a0_hello_world/BUILD.gn文件里的编译选线。

## 烧写程序

请参照[编译环境搭建](/vendor/lockzhiner/rk2206/README_zh.md)

## 实验结果

例程代码编译烧写到开发板后，按下开发板的RESET按键，通过串口软件查看日志，task_helloworld和task_openharmony会交替打印信息，task_helloworld任务每隔1s打印一次Hello World，task_openharmony任务每隔2s打印一次Hello OpenHarmony。

```sh
entering kernel init...
[IOT:D]IotInit: start ....
hilog will init.
[MAIN:D]Main: LOS_Start ...
Entering scheduler
[IOT:D]IotProcess: start ....
Hello World success.
Hello OpenHarmony
Hello World
Hello OpenHarmony
Hello World
Hello World
Hello OpenHarmony
Hello World
Hello World
Hello OpenHarmony
Hello World
Hello World
Hello OpenHarmony
Hello World
Hello World
......
```
