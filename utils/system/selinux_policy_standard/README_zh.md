# selinux\_policy\_standard组件<a name="ZH-CN_TOPIC_0000001100252616"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [使用说明](#section119744591305)
-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

本组件为OpenHarmony 标准系统上用来放置各业务子系统所需SELinux策略文件的目录仓。

此目录仓下仅存储OpenHarmony 标准系统上各业务子系统需的SELinux策略文件以及用来编译SELinux策略文件的脚本文件。

## 目录<a name="section161941989596"></a>

```
/utils/system/selinux_policy_standard
├── account                     # 帐号子系统的SELinux策略目录，下列各子系统结构相同
│   └── system
│       └── common              # 存放需要编入system镜像的SELinux策略文件
│   └── vendor
│       └── common              # 存放需要编入vendor镜像的SELinux策略文件，如无相关策略，可省略
│   └── public                  # 存放需要编入system和vendor镜像的SELinux策略文件，如无相关策略，可省略
│   └── property_trustlist      # 存放需要编入system镜像的property_trustlist策略文件，如无相关策略，可省略
│   └── policy.mk               # makefile文件，关联业务子系统内部的所有策略文件
├── appexecfwk                  # 应用程序框架子系统的SELinux策略目录
├── communication               # 公共通信子系统的SELinux策略目录
├── distributedschedule         # 分布式任务调度子系统的SELinux策略目录
├── graphic                     # 图形图像子系统的SELinux策略目录
├── hdf                         # 硬件驱动子系统的SELinux策略目录
├── hiviewdfx                   # DFX子系统的SELinux策略目录
├── kernel                      # 内核子系统的SELinux策略目录
├── miscservices                # misc软件服务子系统的SELinux策略目录
├── multimedia                  # 媒体子系统的SELinux策略目录
├── multimodalinput             # 多模输入子系统的SELinux策略目录
├── startup                     # 启动子系统的SELinux策略目录
├── telephony                   # 电话服务子系统的SELinux策略目录
├── udevd                       # 多模输入子系统中udev模块的SELinux策略目录
├── uinput                      # 多模输入子系统中uinput模块的SELinux策略目录
├── updater                     # 升级子系统的SELinux策略目录
├── ...                         # 后续新增业务子系统的SELinux策略目录
├── ohos_policy.mk              # makefile文件，关联当前路径下的所有业务子系统的策略文件
```

## 使用说明<a name="section119744591305"></a>

新增业务子系统可以参考同级其他子系统的目录结构新建SELinux策略文件和编译脚本文件，根据SELinux策略最终编译进去的分区位置来划分不同的策略存放路径，新增SELinux策略时，各业务子系统需要根据实际使用情况将策略放置在对应路径中。SELinux策略的完整编译路径如下所示，具体的编译逻辑可以参考ohos\_policy.mk文件。

```
/utils/system/selinux_policy_standard
├── NEW                         # 新增子系统
│   └── system
│       └── common              # 编入system镜像
│   └── vendor
│       └── common              # 编入vendor镜像
│   └── public                  # 编入system和vendor镜像
│   └── property_trustlist      # 编入system镜像
│   └── policy.mk               # makefile文件
```

## 相关仓<a name="section1371113476307"></a>

安全子系统

**hmf/utils/selinux\_policy\_standard**

