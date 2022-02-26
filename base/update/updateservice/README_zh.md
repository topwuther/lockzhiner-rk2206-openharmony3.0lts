# 升级服务组件<a name="ZH-CN_TOPIC_0000001102254666"></a>

-   [简介](#section184mcpsimp)
-   [目录](#section193mcpsimp)
-   [说明](#section208mcpsimp)
    -   [JS接口说明](#section210mcpsimp)
    -   [使用说明](#section253mcpsimp)

-   [相关仓](#section366mcpsimp)

## 简介<a name="section184mcpsimp"></a>

升级服务组件是一个SA\(System Ability\),  由OHOS 的init 进程负责启动。

升级服务器引擎主要功能包括：

1、查找可用的升级包

2、下载升级包

3、设置/获取升级策略

4、触发升级

## 目录<a name="section193mcpsimp"></a>

```
base/update/updateservice  # 升级服务代码仓目录
├── client                 # 升级客户端napi 接口目录
├── engine                 # 升级客户端引擎服务目录
│   ├── etc                # 升级客户端引擎rc配置文件目录
│   ├── include            # 升级客户端引擎头文件目录
│   ├── sa_profile         # SA 配置文件目录
│   └── src                # 升级客户端引擎源码目录
├── interfaces             # 升级客户端接口目录
│   └── innerkits          # SA 接口定义和封装目录
├── kits                   # 对外接口封装目录
│   └── js                 # 提供给升级客户端应用的JS 接口目录
└── tests                  # 测试代码目录
    └── unittest           # 升级客户端UT代码目录
```

## 说明<a name="section208mcpsimp"></a>

### JS接口说明<a name="section210mcpsimp"></a>

<a name="table212mcpsimp"></a>
<table><tbody><tr id="row217mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p219mcpsimp"><a name="p219mcpsimp"></a><a name="p219mcpsimp"></a><strong id="b220mcpsimp"><a name="b220mcpsimp"></a><a name="b220mcpsimp"></a>接口</strong></p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p222mcpsimp"><a name="p222mcpsimp"></a><a name="p222mcpsimp"></a>说明</p>
</td>
</tr>
<tr id="row223mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p16387178102716"><a name="p16387178102716"></a><a name="p16387178102716"></a>checkNewVersion</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p227mcpsimp"><a name="p227mcpsimp"></a><a name="p227mcpsimp"></a>检查是否有可用的升级包版本</p>
</td>
</tr>
<tr id="row228mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p1884710150275"><a name="p1884710150275"></a><a name="p1884710150275"></a>download()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p232mcpsimp"><a name="p232mcpsimp"></a><a name="p232mcpsimp"></a>下载升级包</p>
</td>
</tr>
<tr id="row233mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p7326722162717"><a name="p7326722162717"></a><a name="p7326722162717"></a>upgrade()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p237mcpsimp"><a name="p237mcpsimp"></a><a name="p237mcpsimp"></a>将升级命令写入到misc分区，最终调用reboot命令，进入到updater 子系统中。</p>
</td>
</tr>
<tr id="row238mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p4981103002720"><a name="p4981103002720"></a><a name="p4981103002720"></a>getNewVersionInfo()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p242mcpsimp"><a name="p242mcpsimp"></a><a name="p242mcpsimp"></a>升级完成后，获取升级后的版本信息</p>
</td>
</tr>
<tr id="row243mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p568117524271"><a name="p568117524271"></a><a name="p568117524271"></a>setUpdatePolicy</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p247mcpsimp"><a name="p247mcpsimp"></a><a name="p247mcpsimp"></a>设置升级策略</p>
</td>
</tr>
<tr id="row248mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p19534844192712"><a name="p19534844192712"></a><a name="p19534844192712"></a>getUpdatePolicy</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p252mcpsimp"><a name="p252mcpsimp"></a><a name="p252mcpsimp"></a>获取升级策略</p>
</td>
</tr>
</tbody>
</table>

### 使用说明<a name="section253mcpsimp"></a>

1，导入updateclient lib

```
import client from 'libupdateclient.z.so'
```

2，获取update对象

```
let updater = client.getUpdater('OTA');
```

3，获取新版本信息

```
updater.getNewVersionInfo(info => {
	info "新版本信息"
});
```

4，检查新版本

```
updater.checkNewVersion(info => {
	info "新版本信息"
});
```

5，下载新版本，并监听下载进程

```
updater.download();
updater.on("downloadProgress", progress => {
	progress "下载进度信息"
});
```

6，启动升级

```
updater.upgrade();
updater.on("upgradeProgress", progress => {
	progress "升级进度信息"
});
```

7，设置升级策略

```
updater.setUpdatePolicy(result => {
	result "设置升级策略结果"
});
```

8，查看升级策略

```
updater.getUpdatePolicy(policy => {
	policy "升级策略"
});
```

## 相关仓<a name="section366mcpsimp"></a>

升级子系统

update\_app

**update\_updateservice**

update\_updater

