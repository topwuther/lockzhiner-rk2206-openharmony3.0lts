# 开发者测试组件<a name="ZH-CN_TOPIC_0000001122338419"></a>

-   [简介](#section7375710115617)
-   [目录](#section102031353175317)
-   [约束](#section87444710110)
-   [安装](#section1347156474)
-   [使用测试框架](#section75882026185016)
-   [测试结果与日志](#section414715805819)
-   [涉及仓](#section6299103515474)

## 简介<a name="section7375710115617"></a>

开发者基于系统新增特性可以通过开发者自己开发用例保证，对于系统已有特性的修改，也可通过修改项目中原有的测试用例保证，开发者测试旨在帮助开发者在开发阶段就能开发出高质量代码

## 目录<a name="section102031353175317"></a>

```
developertest/
├── aw                            # 测试框架的静态库
│   ├── cxx                      # 支持C++
│   └── python                   # 支持Python
├── config                        # 测试框架配置
│   ├── build_config.xml         # 用例编译配置
│   ├── filter_config.xml        # 用例筛选配置
│   ├── framework_config.xml     # 测试类型配置
│   └── user_config.xml          # 用户使用配置
├── examples                      # 测试用例示例
│   ├── calculator               # 计算器示例
│   └── test                     # 测试资源示例
├── src                           # 测试框架源码
│   ├── main                     # 入口函数
│   └── core                     # 测试框架核心代码
├── third_party                   # 测试框架依赖第三方组件适配
│   └── lib                      # 静态库编译配置
├── BUILD.gn                      # 测试框架编译入口
├── start.bat                     # 开发者测试入口（Windows）
└── start.sh                      # 开发者测试入口（Linux）
```

## 约束<a name="section87444710110"></a>

测试工具环境依赖

1.  python版本\>=3.7.5
2.  paramiko版本\>=2.7.1
3.  setuptools版本\>=40.8.0
4.  rsa版本\>=4.0
5.  NFS版本\>=V4，设备不支持hdc连接，支持串口时使用。
6.  pyserial版本\>=3.3，设备不支持hdc连接，支持串口时使用。
7.  运行操作系统：Windows版本\>=Win10，Linux为Ubuntu18.04。

## 安装<a name="section1347156474"></a>

-   依赖python环境：
    1.  安装Linux扩展组件readline。

        执行如下命令如下：

        ```
        sudo apt-get install libreadline-dev
        ```

        安装成功提示

        ```
        Reading package lists... Done
        Building dependency tree
        Reading state information... Done
        libreadline-dev is already the newest version (7.0-3).
        0 upgraded, 0 newly installed, 0 to remove and 11 not upgraded.
        ```

    2.  安装setuptools插件，安装命令如下：

        ```
        pip3 install setuptools
        ```

        安装成功提示如下：

        ```
        Requirement already satisfied: setuptools in d:\programs\python37\lib\site-packages (41.2.0)
        ```

    3.  安装paramiko插件，安装命令如下：

        ```
        pip3 install paramiko
        ```

        安装成功提示如下：

        ```
        Installing collected packages: pycparser, cffi, pynacl, bcrypt, cryptography, paramiko
        Successfully installed bcrypt-3.2.0 cffi-1.14.4 cryptography-3.3.1 paramiko-2.7.2 pycparser-2.20 pynacl-1.4.0
        ```

    4.  安装python的rsa插件，安装命令如下：

        ```
        pip3 install rsa
        ```

        安装成功截图如下：

        ```
        Installing collected packages: pyasn1, rsa
        Successfully installed pyasn1-0.4.8 rsa-4.7
        ```

    5.  需要本地的python安装串口插件pyserial，安装命令如下：

        ```
        pip3 install pyserial
        ```

        安装成功提示如下：

        ```
        Requirement already satisfied: pyserial in d:\programs\python37\lib\site-packages\pyserial-3.4-py3.7.egg (3.4)
        ```

    6.  如果设备仅支持串口输出测试结果，则需要安装NFS Server

        windows环境下安装，例如安装haneWIN NFS Server1.2.50，下载地址：https://www.hanewin.net/nfs-e.htm。

        Linux环境下安装，安装命令如下：

        ```
        sudo apt install nfs-kernel-server
        ```

        安装成功提示如下：

        ```
        Reading package lists... Done
        Building dependency tree
        Reading state information... Done
        nfs-kernel-server is already the newest version (1:1.3.4-2.1ubuntu5.3).
        0 upgraded, 0 newly installed, 0 to remove and 11 not upgraded.
        ```



## 使用测试框架<a name="section75882026185016"></a>

-   可选，安装xdevice组件。
    1.  打开xdevice安装目录：test/xdevice。
    2.  打开控制台窗口，执行如下命令：

        ```
        python setup.py install
        ```

        安装成功提示如下：

        ```
        Installed d:\programs\python37\lib\site-packages\xdevice-0.0.0-py3.7.egg
        Processing dependencies for xdevice==0.0.0
        Finished processing dependencies for xdevice==0.0.0
        ```


-   developertest组件配置。

    文件：developertest/config/user\_config.xml

    1.  测试框架通用配置。

        \[build\]    \# 配置测试用例的编译参数，例如：

        ```
        <build>
            <example>false</example>
            <version>false</version>
            <testcase>true</testcase>
            ... ...
        </build>
        ```

        >![](public_sys-resources/icon-note.gif) **说明：** 
        >测试用例的编译参数说明如下：
        >example：是否编译测试用例示例，默认false。
        >version：是否编译测试版本，默认false。
        >testcase：是否编译测试用例，默认true。

    2.  支持hdc连接的被测设备。

        \[device\]    \# 配置标签为usb-hdc的环境信息，测试设备的IP地址和hdc映射的端口号，例如：

        ```
        <device type="usb-hdc">
            <ip>192.168.1.1</ip>
            <port>9111</port>
            <sn></sn>
        </device>
        ```

    3.  仅支持串口的被测设备。

        \[device\]    \# 配置标签为ipcamera的串口信息，COM口和波特率，例如：

        ```
        <device type="com" label="ipcamera">
            <serial>
                <com>COM1</com>
                <type>cmd</type>
                <baud_rate>115200</baud_rate>
                <data_bits>8</data_bits>
                <stop_bits>1</stop_bits>
                <timeout>1</timeout>
            </serial>
        </device>
        ```


-   修改developertest组件配置。可选，如果测试用例已完成编译，可以直接指定测试用例的编译输出路径，测试平台执行测试用例时即不会重新编译测试用例。

    文件：config/user\_config.xml。

    1.  \[test\_cases\]    \# 指定测试用例的输出路径，编译输出目录，例如：

        ```
        <test_cases>
            <dir>/home/opencode/out/release/tests</dir>
        </test_cases>
        ```

    2.  \[NFS\]    \# 被测设备仅支持串口时配置，指定NFS的映射路径，host\_dir为PC侧的NFS目录，board\_dir为板侧创建的目录，例如：

        ```
        <NFS>
            <host_dir>D:\nfs</host_dir>
            <board_dir>user</board_dir>
        </NFS>
        ```


-   测试环境准备（当被测设备仅支持串口时，需要检查）。
    -   系统镜像与文件系统已烧录进开发板，开发板上系统正常运行，在系统模式下，如shell登录时设备提示符OHOS\#。
    -   开发主机和开发板串口连接正常，网口连接正常。
    -   开发主机IP与开发板IP处在同一小网网段，相互可以ping通。
    -   开发主机侧创建空目录用于开发板通过NFS挂载测试用例，并且NFS服务启动正常。

-   运行测试套。
    -   启动测试框架，打开test/developertest目录。
        1.  Windows环境启动测试框架。

            ```
            start.bat
            ```

        2.  Linux环境启动测试框架。

            ```
            ./strat.sh
            ```


    -   设备形态选择。

        根据实际的开发板选择，设备形态配置：developertest/config/framework\_config.xml。

    -   执行测试指令。
        1.  查询测试用例支持的子系统，模块，产品形态以及测试类型，使用show命令。

            ```
            usage: 
                show productlist      Querying Supported Product Forms
                show typelist         Querying the Supported Test Type
                show subsystemlist    Querying Supported Subsystems
                show modulelist       Querying Supported Modules
            ```

        2.  执行测试指令示例，其中-t为必选，-ss和-tm为可选字段

            ```
            run -t ut -ss test -tm example
            ```

        3.  参数说明：指定参数可以执行特定特性、模块对应的测试套

            ```
            usage: run [-h] [-p PRODUCTFORM] [-t [TESTTYPE [TESTTYPE ...]]]
                [-ss SUBSYSTEM] [-tm TESTMODULE] [-ts TESTSUIT]
                [-tc TESTCASE] [-tl TESTLEVEL] 
            
            optional arguments:
                -h, --help            show this help message and exit
                -p PRODUCTFORM, --productform PRODUCTFORM    Specified product form
                -t [TESTTYPE [TESTTYPE ...]], --testtype [TESTTYPE [TESTTYPE ...]]
                    Specify test type(UT,MST,ST,PERF,ALL)
                -ss SUBSYSTEM, --subsystem SUBSYSTEM    Specify test subsystem
                -tm TESTMODULE, --testmodule TESTMODULE    Specified test module
                -ts TESTSUIT, --testsuite TESTSUIT    Specify test suite
                -tc TESTCASE, --testcase TESTCASE    Specify test case
                -tl TESTLEVEL, --testlevel TESTLEVEL    Specify test level
            ```



-   测试框架帮助。
    -   帮助指令，用于查询测试平台支持哪些测试指令。

        ```
        help
        ```



-   退出自测试平台。
    -   退出自测试平台，使用如下命令退出测试平台。

        ```
        quit
        ```



## 测试结果与日志<a name="section414715805819"></a>

-   通过在测试框架中执行测试指令，即可以生成测试日志和测试报告。
-   测试结果
    -   测试用例的结果会直接显示在控制台上，执行一次的测试结果根路径如下：

        ```
        reports/xxxx-xx-xx-xx-xx-xx
        ```

    -   测试用例格式化结果。

        ```
        result/
        ```

    -   测试用例日志。

        ```
        log/plan_log_xxxx-xx-xx-xx-xx-xx.log
        ```

    -   测试报告汇总。

        ```
        summary_report.html
        ```

    -   测试报告详情。

        ```
        details_report.html
        ```


-   测试框架日志

    ```
    reports/platform_log_xxxx-xx-xx-xx-xx-xx.log
    ```

-   最新测试报告

    ```
    reports/latest
    ```


## 涉及仓<a name="section6299103515474"></a>

[测试子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E6%B5%8B%E8%AF%95%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

**test\_developertest**

[test\_xdevice](https://gitee.com/openharmony/test_xdevice/blob/master/README_zh.md)
