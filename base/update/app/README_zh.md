# 升级客户端应用<a name="ZH-CN_TOPIC_0000001148414479"></a>

-   [简介](#section182mcpsimp)
-   [目录](#section190mcpsimp)
-   [说明](#section198mcpsimp)
-   [相关仓](#section206mcpsimp)

## 简介<a name="section182mcpsimp"></a>

升级客户端应用运行于OHOS 上，提供与用户进行交互的界面，并进行升级操作。

主要功能包括：

1、触发升级服务组件检查可用的升级包，显示升级包检查的结果

2、下载升级包，显示下载的进度和状态

3、触发升级

4、升级完成后，显示升级后版本信息

## 目录<a name="section190mcpsimp"></a>

```
base/update/app    # 升级客户端应用代码仓目录
├── entry          # 升级客户端应用代码目录
│ └── src          # 升级客户端源码目录
└── gradle         # 配置文件目录目录
    └── wrapper    # gradle配置文件目录目录
```

## 说明<a name="section198mcpsimp"></a>

升级客户端应用由JavaScript实现，业务逻辑由C++实现，JS应用与业务逻辑通过NAPI进行交互。

NAPI的实现，请参考

base/update/updateservice/client

## 相关仓<a name="section206mcpsimp"></a>

升级子系统

**update\_app**

update\_updateservice

