# HiCollie组件<a name="ZH-CN_TOPIC_0000001077200880"></a>

-   [简介](#section11660541593)
-   [系统架构](#section342962219551)
-   [目录](#section55125489224)
-   [约束](#section161941989596)
-   [编译构建](#section20568163942320)
-   [说明](#section12699104113233)
-   [接口说明](#section8725142134818)
-   [使用说明](#section19959125052315)
-   [相关仓](#section1134984213235)

## 简介<a name="section11660541593"></a>

HiCollie提供了软件看门狗功能。针对系统服务死锁、应用主线程阻塞，用户业务流程超时等故障，HiCollie提供了一套统一的用于故障检测和故障日志生成的框架，提供软件超时故障日志，辅助定位软件超时问题。

## 系统架构<a name="section342962219551"></a>

## 目录<a name="section55125489224"></a>

```
/base/hiviewdfx/hicollie                       #  HiCollie目录
├── frameworks/native/                       # HiCollie native代码
├── interfaces/native/innerkits/include      # HiCollie native头文件
```

## 约束<a name="section161941989596"></a>

HiCollie接口单个进程最多可以注册128个定时器。超出上限的定时器注册操作无效，无法完成设定的逻辑超时检测功能。

## 编译构建<a name="section20568163942320"></a>

请参考HiCollie开发指南。

## 说明<a name="section12699104113233"></a>

## 接口说明<a name="section8725142134818"></a>

## 相关仓<a name="section1134984213235"></a>

DFX子系统：

hmf/hiviwdfx

DFX组件：

hmf/hiviwdfx/hilog

hmf/hiviwdfx/hitrace

**hmf/hiviwdfx/hicollie**

hmf/hiviwdfx/hidumper

hmf/hiviwdfx/hiappevent

hmf/hiviwdfx/hisysevent

hmf/hiviwdfx/debug

