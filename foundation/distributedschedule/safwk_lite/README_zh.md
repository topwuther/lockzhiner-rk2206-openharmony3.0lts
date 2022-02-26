# 介绍<a name="ZH-CN_TOPIC_0000001081445008"></a>

-   [简介](#section11660541593)
-   [目录](#section1464106163817)
-   [使用](#section10729231131110)
-   [涉及仓](#section176111311166)

## 简介<a name="section11660541593"></a>

safwklite模块负责提供基础服务运行的空进程。

## 目录<a name="section1464106163817"></a>

分布式任务调度源代码目录结构如下表所示：

**表1 **主要源代码目录结构

<a name="table43531856201716"></a>
<table><thead align="left"><tr id="row20416556201718"><th class="cellrowborder" valign="top" width="50%" id="mcps1.1.3.1.1"><p id="p10416456121716"><a name="p10416456121716"></a><a name="p10416456121716"></a>名称</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.1.3.1.2"><p id="p1841645631717"><a name="p1841645631717"></a><a name="p1841645631717"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row104169564177"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.1.3.1.1 "><p id="p17416125614179"><a name="p17416125614179"></a><a name="p17416125614179"></a>safwk_lite</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.1.3.1.2 "><p id="p04163569170"><a name="p04163569170"></a><a name="p04163569170"></a>foundation进程实现</p>
</td>
</tr>
</tbody>
</table>

其中分布式任务调度safwk\_lite组件的源代码目录结构如下：

```
├── BUILD.gn
├── readme.md
├── LICENSE
├── src
    └── main.c
```

## 使用<a name="section10729231131110"></a>

在foundation进程中添加服务

按照服务的模板写完服务后在BUILD.gn中添加依赖即可：

```
deps = [
  "${aafwk_lite_path}/services/abilitymgr_lite:abilityms",
  "${appexecfwk_lite_path}/services/bundlemgr_lite:bundlems",
  "//base/hiviewdfx/hilog_lite/frameworks/featured:hilog_shared",
  "//base/security/permission/services/permission_lite/ipc_auth:ipc_auth_target",
  "//base/security/permission/services/permission_lite/pms:pms_target",
  "//foundation/distributedschedule/dmsfwk_lite:dtbschedmgr",
  "//foundation/distributedschedule/samgr_lite/samgr_server:server",
]
```

## 涉及仓<a name="section176111311166"></a>

**[分布式任务调度子系统](zh-cn_topic_0000001115719369.md)**

[safwk\_lite](https://gitee.com/openharmony/distributedschedule_services_safwk_lite)

