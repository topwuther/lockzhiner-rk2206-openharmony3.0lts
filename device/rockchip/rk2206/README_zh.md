# device_rockchip

#### 介绍

##### 目录

```
device/rockchip/rk2206
├── adapter                      # 
|   └── hals                     # hals适配目录
|   └── communication            # 连接类接口适配目录
|   └── iot_hardware             # 连接类iothardware适配目录
|   └── utils                    # 连接类通用工具
|   └── include                  # 芯片固件库头文件
|── sdk_liteos                   # RK2206芯片liteos
|── third_party                  # 第三方软件
```

仓库包含编译构建脚本和打包镜像工具。

系统要求： Ubuntu 20.04.3 LTS 64位系统版本。

编译环境搭建包含如下几步：

1. 准备工作
2. 安装VBox虚拟机
3. 安装Ubuntu操作系统
4. 获取源码
5. 安装的库和工具
6. 安装python3
7. 安装hb
8. 安装arm-none-eabi-gcc
9. 编译流程
10. 烧录打印

## 准备工作

准备一台电脑，安装Windows系统

## 安装VBox虚拟机

下载网址（百度云）：
提取码：

## 下载Ubuntu操作系统

下载网址（百度云）：
提取码：

## 获取源码

```shell
git clone https://gitee.com/Lockzhiner-Electronics/lockzhiner-rk2206-openharmony3.0lts.git
cd lockzhiner-rk2206-openharmony3.0lts
```

## 安装的库和工具

> - 通常系统默认安装samba、vim等常用软件。

> - 使用如下apt-get命令安装下面的库和工具：

```
sudo apt-get install build-essential gcc g++ make zlib* libffi-dev e2fsprogs pkg-config flex bison perl bc openssl libssl-dev libelf-dev libc6-dev-amd64 binutils binutils-dev libdwarf-dev u-boot-tools mtd-utils gcc-arm-linux-gnueabi
```

## 安装Python3

1. 打开Linux编译服务器终端。
2. 输入如下命令，查看python版本号：

   ```
   python3 --version
   ```

   1. 运行如下命令，查看Ubuntu版本：

   ```
   cat /etc/issue
   ```

   2. ubuntu 20安装python。

   ```
   sudo apt-get install python3
   ```
3. 设置python和python3软链接为python3。

   ```
   sudo update-alternatives --install /usr/bin/python python /usr/bin/python3.8 1
   sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1
   ```
4. 安装并升级Python包管理工具（pip3），任选如下一种方式。

   - **命令行方式：**

     ```
     sudo apt-get install python3-setuptools python3-pip -y
     sudo pip3 install --upgrade pip
     ```
   - **安装包方式：**

     ```
     curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
     python get-pip.py
     ```

## 安装hb

### 安装方法

1. 运行如下命令安装hb

   ```
   pip3 uninstall ohos-build # 如果安装了hb,先卸载
   pip3 install ohos-build # 安装hb
   pip3 install build/lite
   ```
2. 设置环境变量

   ```
   vim ~/.bashrc
   ```

   将以下命令拷贝到.bashrc文件的最后一行，保存并退出。

   ```
   export PATH=~/.local/bin:$PATH
   ```

   执行如下命令更新环境变量。

   ```
   source ~/.bashrc
   ```
3. 执行"hb -h"，有打印以下信息即表示安装成功：

   ```
   usage: hb

   OHOS build system

   positional arguments:
     {build,set,env,clean}
       build               Build source code
       set                 OHOS build settings
       env                 Show OHOS build env
       clean               Clean output

   optional arguments:
     -h, --help            show this help message and exit
   ```

## 安装arm-none-eabi-gcc

1. Ubuntu自动安装arm-none-eabi-gcc

   ```shell
   sudo apt-get install gcc-arm-none-eabi
   ```
2. 手动安装arm-none-eabi-gcc

   下载[arm-none-eabi-gcc 编译工具下载](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2)

   解压 [gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2) 安装包至\~/toolchain/路径下。

   ```shell
   mkdir -p ~/toolchain/
   tar -jxvf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 -C ~/toolchain/
   ```

   设置环境变量。

   ```
   vim ~/.bashrc
   ```

   将以下命令拷贝到.bashrc文件的最后一行，保存并退出。

   ```
   export PATH=~/toolchain/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH
   ```

   生效环境变量。

   ```
   source ~/.bashrc
   ```

## 编译流程

编译步骤如下所示：

```shell
hb set -root .
hb set
lockzhiner
   lockzhiner-rk2206

选择lockzhiner-rk2206

hb build -f
```

## 烧录打印

### windows烧录打印

1. 在Windows上，进入samba服务器
2. 进入 `//device/rockchip/tools/windows/DriverAssitant`目录，点击 `DriverInstall.exe`，安装驱动文件。
3. 进入 `//device/rockchip/tools/windows`目录，点击 `RKDevTool.exe`
4. 进入烧写工具主界面，选择“下载镜像”界面
5. 使用USB线，连接小凌派-RK2206开发板的USB烧写口
6. 在小凌派-RK2206开发板上，长按MaskRom按键（详见板子的按钮），点击ReSet按键（详见板子的按钮），烧写工具出现：“发现一个MASKROM设备”
7. 点击“执行”按钮，下载烧写
8. 使用USB串口线，连接USB_UART口，打开串口工具(波特率:115200)，reset（详见板子的按钮） 启动板子，查看log。

### linux烧录打印

```shell
cd device/rockchip/tools/linux   
./flash.py                          # 烧写固件
./flash.py -q                       # 查看开发板烧写状态
```

# 相关仓

* [device/rockchip](https://gitee.com/openharmony/device_soc_rockchip)
