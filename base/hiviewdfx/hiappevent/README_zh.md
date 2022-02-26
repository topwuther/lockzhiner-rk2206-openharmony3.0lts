# HiAppEvent组件<a name="ZH-CN_TOPIC_0000001162014029"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [编译构建](#section137768191623)
-   [说明](#section1312121216216)
    -   [接口说明](#section1551164914237)
    -   [使用说明](#section129654513264)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

HiAppEvent为OpenHarmony应用提供事件打点接口，用于帮助应用记录在运行过程中发生的故障信息、统计信息、安全信息、用户行为信息，以支撑开发者分析应用的运行情况。

**图 1**  HiAppEvent架构图<a name="fig32154874419"></a>  
![](figures/HiAppEvent架构图.png "HiAppEvent架构图")

## 目录<a name="section161941989596"></a>

```
/base/hiviewdfx/hiappevent   # hiappevent部件代码
├── frameworks               # 框架代码
│   └── native               # 打点接口的native实现代码
├── interfaces               # 对外接口存放目录
│   └── js                   # js接口
│       └── innerkits        # js接口内部实现代码
│           └── napi         # 基于napi实现的js接口代码
```

## 编译构建<a name="section137768191623"></a>

依赖 Clang 编译器\(Clang 8.0.0 \)及以上，依赖C++11版本及以上。

## 说明<a name="section1312121216216"></a>

### 接口说明<a name="section1551164914237"></a>

**表 1**  Js打点接口介绍

<a name="table107919166559"></a>
<table><thead align="left"><tr id="row1880201655520"><th class="cellrowborder" valign="top" width="15.981598159815983%" id="mcps1.2.4.1.1"><p id="p5801164558"><a name="p5801164558"></a><a name="p5801164558"></a>模块</p>
</th>
<th class="cellrowborder" valign="top" width="50.68506850685068%" id="mcps1.2.4.1.2"><p id="p168019163559"><a name="p168019163559"></a><a name="p168019163559"></a>方法</p>
</th>
<th class="cellrowborder" valign="top" width="33.33333333333333%" id="mcps1.2.4.1.3"><p id="p780101685516"><a name="p780101685516"></a><a name="p780101685516"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row148011162552"><td class="cellrowborder" valign="top" width="15.981598159815983%" headers="mcps1.2.4.1.1 "><p id="p188061611553"><a name="p188061611553"></a><a name="p188061611553"></a>hiAppEvent</p>
</td>
<td class="cellrowborder" valign="top" width="50.68506850685068%" headers="mcps1.2.4.1.2 "><p id="p1880171695519"><a name="p1880171695519"></a><a name="p1880171695519"></a>write(string eventName, EventType type, object keyValues, AsyncCallback&lt;void&gt; callback): void</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p983410810310"><a name="p983410810310"></a><a name="p983410810310"></a>接口功能：应用事件异步打点方法，使用callback方式作为异步回调。</p>
<p id="p683519817319"><a name="p683519817319"></a><a name="p683519817319"></a>输入参数：</p>
<a name="ul108351681336"></a><a name="ul108351681336"></a><ul id="ul108351681336"><li>eventName：事件名称。</li><li>type：事件类型。</li><li>keyValues：事件参数键值对，为Json对象类型。</li><li>callback：回调函数，可以在回调函数中处理接口返回值。返回值为0表示事件参数校验成功，事件正常异步写入事件文件；大于0表示事件存在异常参数，事件在忽略异常参数后再异步写入事件文件；小于0表示事件校验失败，不执行事件异步打点操作。</li></ul>
</td>
</tr>
<tr id="row78021665512"><td class="cellrowborder" valign="top" width="15.981598159815983%" headers="mcps1.2.4.1.1 "><p id="p1380916165510"><a name="p1380916165510"></a><a name="p1380916165510"></a>hiAppEvent</p>
</td>
<td class="cellrowborder" valign="top" width="50.68506850685068%" headers="mcps1.2.4.1.2 "><p id="p1380161665518"><a name="p1380161665518"></a><a name="p1380161665518"></a>write(string eventName, EventType type, object keyValues): Promise&lt;void&gt;</p>
</td>
<td class="cellrowborder" valign="top" width="33.33333333333333%" headers="mcps1.2.4.1.3 "><p id="p12532811415"><a name="p12532811415"></a><a name="p12532811415"></a>接口功能：应用事件异步打点方法，使用promise方式作为异步回调。</p>
<p id="p75313814417"><a name="p75313814417"></a><a name="p75313814417"></a>输入参数：同上。</p>
</td>
</tr>
</tbody>
</table>

### 使用说明<a name="section129654513264"></a>

Js接口实例

1.  源代码开发

    引入模块：

    ```
    import hiAppEvent from '@ohos.hiAppEvent'
    ```

2.  应用执行事件打点

    ```
    // callback方式
    hiAppEvent.write("test_event", hiAppEvent.EventType.FAULT, {"int_data":100, "str_data":"strValue"}, (err, value) => {
        if (err) {
            // 事件写入异常：事件存在异常参数或者事件校验失败不执行写入
            console.error(`failed to write event because ${err.code}`);
            return;
        }

        // 事件写入正常
        console.log(`success to write event: ${value}`);
    });

    // Promise方式
    hiAppEvent.write("test_event", hiAppEvent.EventType.FAULT, {"int_data":100, "str_data":"strValue"})
        .then((value) => {
            // 事件写入正常
            console.log(`success to write event: ${value}`);
        }).catch((err) => {
            // 事件写入异常：事件存在异常参数或者事件校验失败不执行写入
            console.error(`failed to write event because ${err.code}`);
        });
    ```


## 相关仓<a name="section1371113476307"></a>

[DFX子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/DFX%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

[hiviewdfx\_hiview](https://gitee.com/openharmony/hiviewdfx_hiview/blob/master/README_zh.md)

[hiviewdfx\_hilog](https://gitee.com/openharmony/hiviewdfx_hilog/blob/master/README_zh.md)

**hiviewdfx\_hiappevent**

[hiviewdfx\_hisysevent](https://gitee.com/openharmony/hiviewdfx_hisysevent/blob/master/README_zh.md)

[hiviewdfx\_faultloggerd](https://gitee.com/openharmony/hiviewdfx_faultloggerd/blob/master/README_zh.md)

[hiviewdfx\_hilog\_lite](https://gitee.com/openharmony/hiviewdfx_hilog_lite/blob/master/README_zh.md)

[hiviewdfx\_hievent\_lite](https://gitee.com/openharmony/hiviewdfx_hievent_lite/blob/master/README_zh.md)

[hiviewdfx\_hiview\_lite](https://gitee.com/openharmony/hiviewdfx_hiview_lite/blob/master/README_zh.md)

