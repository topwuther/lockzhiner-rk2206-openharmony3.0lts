# 分布式调度组件<a name="ZH-CN_TOPIC_0000001162308327"></a>

-   [简介](#section11660541593)
-   [系统架构](#section13587185873516)
-   [目录](#section161941989596)
-   [约束](#section119744591305)
-   [使用](#section10729231131110)
-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

分布式任务调度模块负责跨设备组件管理，提供访问和控制远程组件的能力，支持分布式场景下的应用协同。主要功能如下：

-   远程启动元能力：跨设备拉起远端设备上指定元能力。
-   远程迁移元能力：将元能力跨设备迁移到远端。
-   远程绑定元能力：跨设备绑定远端设备上指定元能力。


## 系统架构<a name="section13587185873516"></a>

![](figures/dms-architecture_zh.png)

**图 1**  分布式调度组件架构图<a name="fig4460722185514"></a> 

## 目录<a name="section161941989596"></a>

分布式调度组件主要代码目录结构如下：

```
/foundation/distributedschedule/dmsfwk
├── interfaces                              # 接口定义
├── services                                # 核心功能
│   └── dtbschedmgr
│       ├── include
│       │   ├── ability_connection_wrapper_stub.h  # 绑定回调封装类
│       │   ├── adapter
│       │   │   └── dnetwork_adapter.h             # 软总线适配层
│       │   ├── bundle
│       │   │   └── bundle_manager_internal.h      # bms功能封装接口
│       │   ├── caller_info.h                      # 调用方基本信息
│       │   ├── connect_death_recipient.h          # 绑定回调死亡监听接口
│       │   ├── continuation_callback_death_recipient.h # 迁移回调死亡监听接口
│       │   ├── deviceManager
│       │   │   └── dms_device_info.h              # 设备信息定义和获取接口
│       │   ├── distributed_device_node_listener.h # 设备上下线监听接口
│       │   ├── distributed_sched_ability_shell.h  # 迁移回调管理接口
│       │   ├── distributed_sched_adapter.h        # dms适配层
│       │   ├── distributed_sched_continuation.h   # 迁移token管理接口
│       │   ├── distributed_sched_dumper.h         # dump接口
│       │   ├── distributed_sched_interface.h      # 对外接口
│       │   ├── distributed_sched_permission.h     # 权限校验接口
│       │   ├── distributed_sched_proxy.h          # proxy端接口
│       │   ├── distributed_sched_service.h        # 服务端接口
│       │   ├── distributed_sched_stub.h           # stub端接口
│       │   ├── dtbschedmgr_device_info_storage.h  # 设备信息存储管理
│       │   ├── dtbschedmgr_log.h                  # 日志模块
│       │   ├── parcel_helper.h                    # 序列化/反序列化辅助宏定义
│       │   └── uri.h
│       ├── src
│       │   ├── ability_connection_wrapper_stub.cpp  # 绑定回调封装类实现
│       │   ├── adapter
│       │   │   └── dnetwork_adapter.cpp             # 软总线适配层
│       │   ├── bundle
│       │   │   └── bundle_manager_internal.cpp      # bms功能封装实现
│       │   ├── connect_death_recipient.cpp          # 绑定回调死亡监听实现
│       │   ├── continuation_callback_death_recipient.cpp # 迁移回调死亡监听实现
│       │   ├── deviceManager
│       │   │   └── dms_device_info.cpp              # 设备信息获取接口实现
│       │   ├── distributed_device_node_listener.cpp # 设备上下线监听实现
│       │   ├── distributed_sched_ability_shell.cpp  # 迁移回调管理实现
│       │   ├── distributed_sched_adapter.cpp        # dms适配层实现
│       │   ├── distributed_sched_continuation.cpp   # 迁移token管理实现
│       │   ├── distributed_sched_dumper.cpp         # dump实现
│       │   ├── distributed_sched_permission.cpp     # 权限校验实现
│       │   ├── distributed_sched_proxy.cpp          # proxy端实现
│       │   ├── distributed_sched_service.cpp        # 服务端实现
│       │   ├── distributed_sched_stub.cpp           # stub端实现
│       │   └── dtbschedmgr_device_info_storage.cpp  # 设备信息存储管理实现
│       └──── BUILD.gn
├── sa_profile
├── utils
├── LICENSE
```

## 约束<a name="section119744591305"></a>

-   组网设备需在同一局域网中。
-   组网之前，需先完成设备绑定，绑定流程参见安全子系统中说明。

## 使用<a name="section10729231131110"></a>

-   **分布式任务调度模块编译**

分布式任务调度模块，其代码所在目录如下：

```
foundation/distributedschedule/dmsfwk
```


## 相关仓<a name="section1371113476307"></a>

分布式任务调度子系统

[distributedschedule\_dms\_fwk](https://gitee.com/openharmony/distributedschedule_dms_fwk)