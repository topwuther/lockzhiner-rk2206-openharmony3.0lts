# safwk\_lite<a name="EN-US_TOPIC_0000001081445008"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section1464106163817)
-   [Usage](#section10729231131110)
-   [Repositories Involved](#section176111311166)

## Introduction<a name="section11660541593"></a>

The  **safwk\_lite**  module provides an empty process for running basic services.

## Directory Structure<a name="section1464106163817"></a>

The following table describes the directory structure of the Distributed Scheduler.

**Table 1**  Directory structure of the major source code

<a name="table43531856201716"></a>
<table><thead align="left"><tr id="row20416556201718"><th class="cellrowborder" valign="top" width="50%" id="mcps1.1.3.1.1"><p id="p10416456121716"><a name="p10416456121716"></a><a name="p10416456121716"></a>Directory</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.1.3.1.2"><p id="p1841645631717"><a name="p1841645631717"></a><a name="p1841645631717"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row104169564177"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.1.3.1.1 "><p id="p17416125614179"><a name="p17416125614179"></a><a name="p17416125614179"></a>safwk_lite</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.1.3.1.2 "><p id="p04163569170"><a name="p04163569170"></a><a name="p04163569170"></a>Implementation of the foundation process</p>
</td>
</tr>
</tbody>
</table>

The source code directory structure of the  **safwk\_lite**  module is as follows:

```
├── BUILD.gn
├── readme.md
├── LICENSE
├── src
    └── main.c
```

## Usage<a name="section10729231131110"></a>

Add a service to the foundation process.

After writing the service based on the service template, add the dependencies to the  **BUILD.gn**  file.

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

## Repositories Involved<a name="section176111311166"></a>

[Distributed Scheduler subsystem](en-us_topic_0000001115719369.md)

**[safwk\_lite](https://gitee.com/openharmony/distributedschedule_services_safwk_lite)**

