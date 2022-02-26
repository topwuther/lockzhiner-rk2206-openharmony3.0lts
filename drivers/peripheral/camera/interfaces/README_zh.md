# Camera驱动子系统HDI使用说明

## 简介
该仓下主要包含相机驱动框架模型 HDI（Hardware Driver Interface）接口定义及其实现，对上实现相机HDI接口，对下实现相机Pipeline模型，管理相机各个硬件设备，HDI接口主要提供如下功能:

* ICameraHost：设备管理接口，camera_host服务注册到系统的入口；
* ICameraDevice：设备控制接口，设备参数的下发回传及使能等管理；
* IStreamOperator：流控制管理器，管理流与捕获动作的关系，控制buffer轮转线程；
* IOfflineStreamOperator：离线流管理器，控制离线流buffer正常回传；

图1 相关模块逻辑视图

![](C:\Users\grj\Pictures\子系统架构图.png)



## 目录
该仓下源代码目录结构如下所示:
drivers/peripheral/camera/interfaces

    .
    ├── include
    │    ├── callback #框架涉及的所有callback接口目录
    │    ├── device #ICameraDeviceCallback接口Remote实现
    │    │   │   ├── camera_device_callback.cpp #device回调响应在此处实现或参考自定义
    │    │   │   ├── camera_device_callback.h
    │    │   │   ├── camera_device_callback_proxy.cpp    #client端实现，编译在hdi_impl中
    │    │   │   ├── camera_device_callback_proxy.h
    │    │   │   ├── camera_device_callback_stub.cpp #server端实现，编译在client/BUILD.gn中
    │    │   │   └── camera_device_callback_stub.h
    │    │   ├── host
    │    │   │   ├── camera_host_callback.cpp #host回调响应在此处实现或参考自定义
    │    │   │   ├── camera_host_callback.h
    │    │   │   ├── camera_host_callback_proxy.cpp #client端实现，编译在hdi_impl中
    │    │   │   ├── camera_host_callback_proxy.h
    │    │   │   ├── camera_host_callback_stub.cpp #server端实现，编译在client/BUILD.gn中
    │    │   │   └── camera_host_callback_stub.h
    │    │   └── operator
    │    │       ├── stream_operator_callback.cpp #StreamOperator回调响应在此处实现或参考自定义
    │    │       ├── stream_operator_callback.h
    │    │       ├── stream_operator_callback_proxy.cpp #client端实现，编译在hdi_impl中
    │    │       ├── stream_operator_callback_proxy.h
    │    │       ├── stream_operator_callback_stub.cpp #server端实现，编译在client/BUILD.gn中
    │    │       └── stream_operator_callback_stub.h
    │    ├── client
    │    │   ├── BUILD.gn                          #编译为libcamera_client，作为远端调用库
    │    │   ├── camera_device_proxy.cpp           #ICameraDevice接口远端client代理实现
    │    │   ├── camera_device_proxy.h
    │    │   ├── camera_host_proxy.cpp             #ICameraHost接口远端client代理实现
    │    │   ├── camera_host_proxy.h
    │    │   ├── offline_stream_operator_proxy.cpp #IOfflineStreamOperator接口远端client代理实现
    │    │   ├── offline_stream_operator_proxy.h
    │    │   ├── stream_operator_proxy.cpp  #IStreamOperator接口远端client代理实现
    │    │   └── stream_operator_proxy.h
    │    ├── icamera_device_callback.h      #IcameraDeviceCallback回调接口定义
    │    ├── icamera_device.h               #ICameraDevice接口定义
    │    ├── icamera_host_callback.h        #ICameraHostCallback回调接口定义
    │    ├── icamera_host.h                 #ICameraHost接口定义
    │    ├── ioffline_stream_operator.h     #IOfflineSteamOperator接口定义
    │    ├── istream_operator_callback.h    #IStreamOperatorCallback回调接口定义
    │    ├── istream_operator.h             #IStreamOperator接口定义
    │    ├── server
    │    │   ├── camera_device_service_stub.cpp  #ICameraDevice服务端序列化实现
    │    │   ├── camera_device_service_stub.h
    │    │   ├── camera_host_driver.cpp          #camera_host服务注册到IServiceManager
    │    │   ├── camera_host_service_stub.cpp    #ICameraHost服务端序列化实现
    │    │   ├── camera_host_service_stub.h
    │    │   ├── offline_stream_operator_service_stub.cpp    #IOfflineStreamOperator离线流服务端序列化实现
    │    │   ├── offline_stream_operator_service_stub.h
    │    │   ├── stream_operator_service_stub.cpp       #IStreamOperator服务端序列化实现
    │    │   └── stream_operator_service_stub.h
    │    ├── types.h #整个框架对外类型定义
    │    └── utils_data_stub.h #camera_metadata & StreamInfo序列化实现
    └── README_zh.md



## 说明
### 接口说明
相机驱动提供给上层层可直接调用的能力接口，提供的部分接口说明如表1 HDI接口列表所示：

表 1 HDI接口列表

<a name="table1513255710559"></a>
<table><thead align="left"><tr id="row171321857155517"><th class="cellrowborder" align="center" valign="top" width="12.121212121212123%" id="mcps1.2.4.1.1"><p id="p6132957115511"><a name="p6132957115511"></a><a name="p6132957115511"></a>
头文件
</p>
</th>
<th class="cellrowborder" align="center" valign="top" width="64.95649564956496%" id="mcps1.2.4.1.2"><p id="p14132125715552"><a name="p14132125715552"></a><a name="p14132125715552"></a>
接口名称
</p>
</th>
<th class="cellrowborder" align="center" valign="top" width="22.922292229222922%" id="mcps1.2.4.1.3"><p id="p18132205755516"><a name="p18132205755516"></a><a name="p18132205755516"></a>
功能描述
</p>

</th>
</tr>
</thead>
<tbody><tr id="row13132357165514"><td class="cellrowborder" rowspan="5" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p829618389386"><a name="p829618389386"></a><a name="p829618389386"></a></p>
<p id="p35261387384"><a name="p35261387384"></a><a name="p35261387384"></a></p>
<p id="p776383812388"><a name="p776383812388"></a><a name="p776383812388"></a></p>
<p id="p11950123812382"><a name="p11950123812382"></a><a name="p11950123812382"></a></p>
<p id="p13168103915381"><a name="p13168103915381"></a><a name="p13168103915381"></a></p>
<p id="p825185015460"><a name="p825185015460"></a><a name="p825185015460"></a>icamera_host.h</p>
<p id="p2133757135510"><a name="p2133757135510"></a><a name="p2133757135510"></a></p>
<p id="p1476175815372"><a name="p1476175815372"></a><a name="p1476175815372"></a></p>
</td>

<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p1365411515117"><a name="p1365411515117"></a><a name="p1365411515117"></a>
CamRetCode SetCallback(const OHOS::sptr&lt;ICameraHostCallback&gt &callback);
</p>
</td>

<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p041814588404"><a name="p041814588404"></a><a name="p041814588404"></a>
设置ICameraHostCallback回调接口
</p>
</td>

</tr>
<tr id="row9132135715515"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1398804043612"><a name="p1398804043612"></a><a name="p1398804043612"></a>
CamRetCode GetCameraIds(std::vector&lt;std::string> &cameraIds);
</p>

</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1341845816404"><a name="p1341845816404"></a><a name="p1341845816404"></a>
获取当前可用的Camera设备列表，数据来源于camera_host_config.hcs配置文件，由开发者配置
</p>
</td>

</tr>
<tr id="row171330575555"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1974125024812"><a name="p1974125024812"></a><a name="p1974125024812"></a>
CamRetCode GetCameraAbility(const std::string &cameraId,<br>
    std::shared_ptr&lt;CameraAbility> &ability);<br>
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1041815816403"><a name="p1041815816403"></a><a name="p1041815816403"></a>
获取相应camera的能力集，CameraAbility定义为CameraMetadata
</p>
</td>

</tr>
<tr id="row576145883720"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p8761658183714"><a name="p8761658183714"></a><a name="p8761658183714"></a>
CamRetCode OpenCamera(const std::string &cameraId,<br>
        const OHOS::sptr&lt;ICameraDeviceCallback> &callback,<br>
        OHOS::sptr&lt;ICameraDevice> &pDevice);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p15418165812409"><a name="p15418165812409"></a><a name="p15418165812409"></a>
camera设备上电，cameraId由GetCameraIds接口获取，返回相应的CameraDevice指针
</p>
</td>

</tr>
<tr id="row1957862120383"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p55781217381"><a name="p55781217381"></a><a name="p55781217381"></a>
CamRetCode SetFlashlight(const std::string &cameraId, bool &isEnable);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1641875834010"><a name="p1641875834010"></a><a name="p1641875834010"></a>
打开或关闭手电筒，isEnable = true打开手电筒；isEnable = false关闭手电筒
</p>
</td>

</tr>
<tr id="row1513316577554"><td class="cellrowborder" rowspan="7" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p14171441118"><a name="p14171441118"></a><a name="p14171441118"></a></p>
<p id="p154814318410"><a name="p154814318410"></a><a name="p154814318410"></a></p>
<p id="p3481154311418"><a name="p3481154311418"></a><a name="p3481154311418"></a></p>
<p id="p57063567463"><a name="p57063567463"></a><a name="p57063567463"></a>icamera_device.h</p>
<p id="p7909447418"><a name="p7909447418"></a><a name="p7909447418"></a></p>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p228510326414"><a name="p228510326414"></a><a name="p228510326414"></a>
CamRetCode GetStreamOperator(<br>
const OHOS::sptr&lt;IStreamOperatorCallback> &callback,<br>
        OHOS::sptr&lt;IStreamOperator> &streamOperator);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p17421321134612"><a name="p17421321134612"></a><a name="p17421321134612"></a>
获取流控制器，同步设置流控制回调接口给服务
</p>
</td>
</tr>
<tr id="row171331657185514"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p12841932114117"><a name="p12841932114117"></a><a name="p12841932114117"></a>
CamRetCode UpdateSettings(const std::shared_ptr&lt;CameraSetting> &settings);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1874202174615"><a name="p1874202174615"></a><a name="p1874202174615"></a>
设置设备控制参数，CameraSetting定义为CameraMetadata
</p>
</td>
</tr>
<tr id="row41331557165518"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p92831132184119"><a name="p92831132184119"></a><a name="p92831132184119"></a>
CamRetCode SetResultMode(const ResultCallbackMode &mode);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p474262184610"><a name="p474262184610"></a><a name="p474262184610"></a>
设置Result回调模式和回调函数，详细参照types.h文件注释
</p>
</td>
</tr>
<tr id="row77021769584"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p8283123284110"><a name="p8283123284110"></a><a name="p8283123284110"></a>
CamRetCode GetEnabledResults(std::vector&lt;MetaType> &results);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p107422021204615"><a name="p107422021204615"></a><a name="p107422021204615"></a>
获取sensor使能上报项，即，camera_metadata的tags
</p>
</td>
</tr>
<tr id="row71857914585"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p4282032114118"><a name="p4282032114118"></a><a name="p4282032114118"></a>
CamRetCode EnableResult(const std::vector&lt;MetaType> &results);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p374210219468"><a name="p374210219468"></a><a name="p374210219468"></a>
使能具体的tags上报
</p>
</td>
</tr>
<tr id="row884115357415"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p68421035114115"><a name="p68421035114115"></a><a name="p68421035114115"></a>
CamRetCode DisableResult(const std::vector&lt;MetaType&gt;  &results);
</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p1674212113467"><a name="p1674212113467"></a><a name="p1674212113467"></a>
禁止具体的tags上报
</p>
</td>
</tr>
<tr id="row18831119115815"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p172641732134117"><a name="p172641732134117"></a><a name="p172641732134117"></a>
void Close();
</p>
</td>
<td class="cellrowborder" valign="top" headers="mcps1.2.4.1.2 "><p id="p10742182114462"><a name="p10742182114462"></a><a name="p10742182114462"></a>
关闭camera设备
</p>
</td>

</tr>
<tr id="row1452521025813"><td class="cellrowborder" rowspan="10" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p033128174618"><a name="p033128174618"></a><a name="p033128174618"></a></p>
<p id="p4252162854616"><a name="p4252162854616"></a><a name="p4252162854616"></a></p>
<p id="p10421192894615"><a name="p10421192894615"></a><a name="p10421192894615"></a></p>
<p id="p12525910165811"><a name="p12525910165811"></a><a name="p12525910165811"></a>istream_operator.h</p>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode IsStreamsSupported(<br>
        OperationMode mode,<br>
        const std::shared_ptr&lt;CameraStandard::CameraMetadata> &modeSetting,<br>
        const std::shared_ptr&lt;StreamInfo> &pInfo,<br>
        StreamSupportType &pType);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p1675964994818"><a name="p1675964994818"></a><a name="p1675964994818"></a>
查询模式和流的配置组合是否支持OperationMode操作模式,现阶段只支持普通模式<br>
modeSetting定义为CameraMetadata，判断硬件是否支持对应的tag，pInfo携带需求流属性，判断硬件是否支持，pType出参返回支持方式.<br>
详见types.h注释
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode CreateStreams(const std::vector&lt;std::shared_ptr&lt;StreamInfo>> &streamInfos);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
批量创建流，streamInfo指定流的各项参数
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode ReleaseStreams(const std::vector&lt;int> &streamIds);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
通过streamId批量释放流资源
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode CommitStreams(OperationMode mode,<br>
        const std::shared_ptr&lt;CameraStandard::CameraMetadata> &modeSetting);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
使能相机流的配置，并配置工作模式
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode GetStreamAttributes(<br>
        std::vector&lt;std::shared_ptr&lt;StreamAttribute>> &attributes);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
获取所有流属性
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode AttachBufferQueue(int streamId,<br>
        const OHOS::sptr&lt;OHOS::IBufferProducer> &producer);</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
绑定BufferQueue到streamId对应的流，通过BufferQueue上传图像数据
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode DetachBufferQueue(int streamId);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
解绑streamId对应流的BufferQueue
</p>
</td>



</tr>
<tr id="row1948179195"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p5783143154816"><a name="p5783143154816"></a><a name="p5783143154816"></a>
CamRetCode Capture(int captureId,<br>
        const std::shared_ptr&lt;CaptureInfo> &pInfo,  bool isStreaming);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1759749134818"><a name="p1759749134818"></a><a name="p1759749134818"></a>
图像捕获，captureId 标志此次捕获请求的id，info 捕获图像的参数信息，如果数组中的info存在多个，
则表明是batch模式，连续捕获多帧，isStreaming 是否连续捕获
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode CancelCapture(int captureId);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
取消图像捕获，针对连续捕获，单帧捕获框架自动结束
</p>
</td>
</tr>
<tr id="row1948179195"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p5783143154816"><a name="p5783143154816"></a><a name="p5783143154816"></a>
CamRetCode ChangeToOfflineStream(const std::vector&lt;int> &streamIds,<br>
        OHOS::sptr&lt;IStreamOperatorCallback> &callback,<br>
        OHOS::sptr&lt;IOfflineStreamOperator> &offlineOperator);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1759749134818"><a name="p1759749134818"></a><a name="p1759749134818"></a>
将普通流转换成离线流，截取pipeline一段作为离线流控制器，保证后续buffer顺利回传
</p>
</td>

</tr>
</tr>
<tr id="row1452521025813"><td class="cellrowborder" rowspan="4" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p033128174618"><a name="p033128174618"></a><a name="p033128174618"></a></p>
<p id="p4252162854616"><a name="p4252162854616"></a><a name="p4252162854616"></a></p>
<p id="p10421192894615"><a name="p10421192894615"></a><a name="p10421192894615"></a></p>
<p id="p12525910165811"><a name="p12525910165811"></a><a name="p12525910165811"></a>istream_operator_callback.h</p>
</td>

<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
void OnCaptureStarted(int32_t captureId, const std::vector&lt;int32_t> &streamId);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p1675964994818"><a name="p1675964994818"></a><a name="p1675964994818"></a>
开始捕获图像状态回调
</p>
</td>

</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
void OnCaptureEnded(int32_t captureId,<br>
        const std::vector&lt;std::shared_ptr&lt;CaptureEndedInfo>> &info);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
结束捕获状态回调
</p>
</td>
</tr>
<tr id="row1948179195"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p5783143154816"><a name="p5783143154816"></a><a name="p5783143154816"></a>
void OnCaptureError(int32_t captureId,<br>
        const std::vector&lt;std::shared_ptr&lt;CaptureErrorInfo>> &info);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1759749134818"><a name="p1759749134818"></a><a name="p1759749134818"></a>
捕获错误回调
</p>
</td>
</tr>
<tr id="row1331121813197"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p2728173711481"><a name="p2728173711481"></a><a name="p2728173711481"></a>
void OnFrameShutter(int32_t captureId,<br>
        const std::vector&lt;int32_t> &streamId, uint64_t timestamp);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p107591749104810"><a name="p107591749104810"></a><a name="p107591749104810"></a>
单帧捕获完成回调，与CaptureInfo中enableShutterCallback_配合使用
</p>
</td>
</tr>
</tr>
<tr id="row1452521025813"><td class="cellrowborder" rowspan="3" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p033128174618"><a name="p033128174618"></a><a name="p033128174618"></a></p>
<p id="p4252162854616"><a name="p4252162854616"></a><a name="p4252162854616"></a></p>
<p id="p10421192894615"><a name="p10421192894615"></a><a name="p10421192894615"></a></p>
<p id="p12525910165811"><a name="p12525910165811"></a><a name="p12525910165811"></a>ioffline_stream_operator.h</p>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode CancelCapture(int captureId);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p1675964994818"><a name="p1675964994818"></a><a name="p1675964994818"></a>
取消离线流的连续捕获动作
</p>
</td>
</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
CamRetCode ReleaseStreams(const std::vector&lt;int> &streamIds);
</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
释放streamId对应流资源
</p>
</td>
</tr>
<tr id="row1948179195"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p5783143154816"><a name="p5783143154816"></a><a name="p5783143154816"></a>
CamRetCode Release();</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p1759749134818"><a name="p1759749134818"></a><a name="p1759749134818"></a>
释放IOfflineStreamOperator句柄</p>
</td>
</tr>
</tr>
<tr id="row1452521025813"><td class="cellrowborder" rowspan="2" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p033128174618"><a name="p033128174618"></a><a name="p033128174618"></a></p>
<p id="p4252162854616"><a name="p4252162854616"></a><a name="p4252162854616"></a></p>
<p id="p10421192894615"><a name="p10421192894615"></a><a name="p10421192894615"></a></p>
<p id="p12525910165811"><a name="p12525910165811"></a><a name="p12525910165811"></a>icamera_host_callback.h</p>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
void OnCameraStatus(const std::string &cameraId, CameraStatus status);</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p1675964994818"><a name="p1675964994818"></a><a name="p1675964994818"></a>
上报camera状态，针对UVC设备，状态定义详见types.h</p>
</td>
</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
void OnFlashlightStatus(const std::string &cameraId, FlashlightStatus status);</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
上报手电筒状态</p>
</td>

</tr>
</tr>
<tr id="row1452521025813"><td class="cellrowborder" rowspan="2" valign="top" width="12.121212121212123%" headers="mcps1.2.4.1.1 "><p id="p033128174618"><a name="p033128174618"></a><a name="p033128174618"></a></p>
<p id="p4252162854616"><a name="p4252162854616"></a><a name="p4252162854616"></a></p>
<p id="p10421192894615"><a name="p10421192894615"></a><a name="p10421192894615"></a></p>
<p id="p12525910165811"><a name="p12525910165811"></a><a name="p12525910165811"></a>icamera_device_callback.h</p>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
void OnError(ErrorType type, int32_t errorMsg);</p>
</td>
<td class="cellrowborder" align="center" valign="top" width="22.922292229222922%" headers="mcps1.2.4.1.3 "><p id="p1675964994818"><a name="p1675964994818"></a><a name="p1675964994818"></a>
Camera设备错误回调</p>
</td>
</tr>
<tr id="row172902161193"><td class="cellrowborder" valign="top" headers="mcps1.2.4.1.1 "><p id="p1249042564815"><a name="p1249042564815"></a><a name="p1249042564815"></a>
void OnResult(uint64_t timestamp, const std::shared_ptr&lt;CameraStandard::CameraMetadata> &result);</p>
</td>
<td class="cellrowborder" align="center" valign="top" headers="mcps1.2.4.1.2 "><p id="p17591149104819"><a name="p17591149104819"></a><a name="p17591149104819"></a>
Camera设备Meta回调</p>
</td>


</tbody>
</table>


### 使用说明

该仓核心功能是提供相机驱动能力接口供上层系统服务调用，提供的驱动能力接口统一归属为HDI接口层。



<a name="table1513255710559"></a>
<table><thead align="left"><tr id="row171321857155517"> 

</th>
<th class="cellrowborder" align="center" valign="top" width="50%" id="mcps1.2.4.1.2"><p id="p14132125715552"><a name="p14132125715552"></a><a name="p14132125715552"></a>接口名称</p>
</th>
</th>
<th class="cellrowborder" align="center" valign="top" width="50%" id="mcps1.2.4.1.2"><p id="p14132125715552"><a name="p14132125715552"></a><a name="p14132125715552"></a>使用说明</p>
</th>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode SetCallback(const OHOS::sptr&lt;ICameraHostCallback> &callback);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务<br>
constexpr const char *TEST_SERVICE_NAME = "camera_service";<br>
sptr&lt;ICameraHost> cameraHost = ICameraHost::Get(TEST_SERVICE_NAME);<br>
判断cameraHost服务指针对象不为空继续以下调用<br>
OHOS::sptr&lt;CameraHostCallback> callback = new CameraHostCallback();<br>
sampleObj->SetCallback(callback);<br>
判断返回值</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode GetCameraIds(std::vector&lt;std::string> &cameraIds) ;
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务<br>
判断cameraHost服务指针对象不为空<br>
获取cameraIds<br>
std::vector&lt;std::string> cameraIds;<br>
sampleObj->GetCameraIds(cameraIds);<br>
判断返回值<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode GetCameraAbility(const std::string &cameraId,<br>
        std::shared_ptr&lt;CameraAbility> &ability);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务<br>
判断cameraHost服务指针对象不为空<br>
获取cameraIds<br>
cameraId中选取一个获取其对应的属性<br>
std::string cameraId = cameraIds.front();<br>
sampleObj->GetCameraAbility(cameraId, ability);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode OpenCamera(const std::string &cameraId,<br>
        const OHOS::sptr&lt;ICameraDeviceCallback> &callback,<br>
        OHOS::sptr&lt;ICameraDevice> &pDevice);<br>
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务<br>
判断cameraHost服务指针对象不为空<br>
打开cameraId对应设备<br>
OHOS::sptr&lt;CameraDeviceCallback> deviceCallback = new CameraDeviceCallback();<br>
OHOS::sptr&lt;ICameraDevice> cameraDevice = nullptr;<br>
sampleObj->OpenCamera(cameraId, deviceCallback, cameraDevice);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode SetFlashlight(const std::string &cameraId, bool &isEnable);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务<br>
判断cameraHost服务指针对象不为空<br>
打开手电筒<br>
bool isEnable = true;<br>
sampleObj->SetFlashlight(cameraIds.front(), isEnable);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode GetStreamOperator(<br>
        const OHOS::sptr&lt;IStreamOperatorCallback> &callback,<br>
        OHOS::sptr&lt;IStreamOperator> &streamOperator);<br>
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice后做以下调用<br>
OHOS::sptr&lt;StreamOperatorCallback> streamOperatorCallback = new StreamOperatorCallback();<br>
OHOS::sptr&lt;IStreamOperator> streamOperator = nullptr;<br>
cameraDevice->GetStreamOperator(streamOperatorCallback, streamOperator);<br>
判断返回值，并判断streamOperator是否为空<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode UpdateSettings(const std::shared_ptr&lt;CameraSetting> &settings);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice后做以下调用<br>
std::shared_ptr&lt;CameraSetting> cameraSetting = new CameraSetting(entryCapacity, dataCapacity);<br>
entryCapacity: 需要设置的tag数量，dataCapacity: 所有tag对应的数据项数量<br>
cameraDevice-> UpdateSettings(cameraSetting);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode SetResultMode(const ResultCallbackMode &mode);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice后做以下调用<br>
ResultCallbackMode resultCallbackMode = ON_CHANGED;<br>
cameraDevice->SetResultMode(resultCallbackMode);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode GetEnabledResults(std::vector&lt;MetaType> &results);</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice后做以下调用<br>
std::vector&lt;MetaType> results;<br>
cameraDevice->GetEnabledResults(results);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode EnableResult(const std::vector&lt;MetaType> &results);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用GetEnabledResults接口获取支持的使能后，调用使能接口<br>
std::vector&lt;MetaType> results;<br>
cameraDevice->EnableResult(results);;<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode DisableResult(const std::vector&lt;MetaType> &results);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用GetEnabledResults接口获取支持的使能后，调用取消使能接口<br>
std::vector&lt;MetaType> results;<br>
cameraDevice->EnableResult(results);;<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
void Close();
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用Close关闭当前设备
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode IsStreamsSupported(<br>
        OperationMode mode,<br>
        const std::shared_ptr&lt;CameraStandard::CameraMetadata> &modeSetting,<br>
        const std::shared_ptr&lt;StreamInfo> &pInfo,<br>
        StreamSupportType &pType);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
OperationMode operationMode = NORMAL;<br>
StreamSupportType supportType;<br>
std::shared_ptr&lt;StreamInfo> streamInfo = std::make_shared&lt;StreamInfo>();<br>
streamInfo->streamId_ = 1001;<br>
streamInfo->width_ = 720;<br>
streamInfo->height_ = 480;<br>
streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;<br>
streamInfo->datasapce_ = 8;<br>
streamInfo->intent_ = PREVIEW;<br>
StreamConsumer previewConsumer;<br>
streamInfo->bufferQueue_ = previewConsumer.CreateProducer(<br>
    [](void* addr, uint32_t size) { }); // StreamConsumer参见UT，或详细了解BufferProducer的使用<br>
streamInfo->tunneledMode_ = 5;<br>
streamOperator->IsStreamsSupported(NORMAL, ability, streamInfo, supportType);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode CreateStreams(const std::vector&lt;std::shared_ptr&lt;StreamInfo>> &streamInfos);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
按照IsStreamsSupported步骤的StreamInfo定义方式，创建一个或多个对象放入std::vector&lt;std::shared_ptr&lt;Camera::StreamInfo>> streamInfos;容器中做如下调用<br>
streamOperator->CreateStreams(streamInfos);<br>
判断返回值
</p>
</td>
</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode ReleaseStreams(const std::vector&lt;int> &streamIds);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
std::vector&lt;int> streamIds; // 一个货多个需要释放掉的流id<br>
streamOperator->ReleaseStreams(streamIds);<br>
判断返回值<br>
</p>
</td>
</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode CommitStreams(OperationMode mode,<br>
        const std::shared_ptr&lt;CameraStandard::CameraMetadata> &modeSetting);</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
与CreateStreams联合使用，先做CreateStreams调用
std::shared_ptr&lt;CameraStandard::CameraMetadata> modeSetting = new CameraSetting(entryCapacity, dataCapacity);<br>
streamOperator->CommitStreams(NORMAL, modeSetting);<br>
判断返回值<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode GetStreamAttributes(<br>
        std::vector&lt;std::shared_ptr&lt;StreamAttribute>> &attributes);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
std::vector&lt;std::shared_ptr&lt;StreamAttribute>> attributes;<br>
streamOperator->GetStreamAttributes(attributes);<br>
判断返回值<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode AttachBufferQueue(int streamId,<br>
        const OHOS::sptr&lt;OHOS::IBufferProducer> &producer);<br>
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
注意：如果CreateStreams时，streamInfo里面bufferQueue_不为空，则这个不用设置，否则视为替换<br>
OHOS::sptr&lt;OHOS::IBufferProducer> producer; // 创建方式见CreateStreams<br>
streamOperator->AttachBufferQueue(producer);<br>
判断返回值<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode DetachBufferQueue(int streamId);<br>
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
注意：如果需要调用AttachBufferQueue，最好调用DetachBufferQueue先解绑<br>
OHOS::sptr&lt;OHOS::IBufferProducer> producer; // 创建方式见CreateStreams<br>
streamOperator->AttachBufferQueue( DetachBufferQueue);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode Capture(int captureId,<br>
        const std::shared_ptr&lt;CaptureInfo> &pInfo,  bool isStreaming);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
调用CreateStreams创建流，调用CommitStreams配置流<br>
int captureId = 2001; // 调用端提供捕获动作的唯一标识<br>
std::shared_ptr&lt;CaptureInfo> captureInfo = std::make_shared<:CaptureInfo>();<br>
captureInfo->streamIds_ = {streamInfo->streamId_};<br>
captureInfo->captureSetting_ = new CameraMetadata(entryCapacity, dataCapacity); // 需要做数据填充<br>
captureInfo->enableShutterCallback_ = false;<br>
streamOperator->Capture(captureId, captureInfo, true);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode CancelCapture(int captureId);</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
与Capture捕获动作配套使用，并且本次捕获为连续捕获，否则无需调用<br>
int captureId = 2001; // 调用端提供捕获动作的唯一标识<br>
streamOperator->CancelCapture(captureId, captureInfo, true);<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode ChangeToOfflineStream(const std::vector&lt;int> &streamIds,<br>
        OHOS::sptr&lt;IStreamOperatorCallback> &callback,<br>
        OHOS::sptr&lt;IOfflineStreamOperator> &offlineOperator);<br></p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
获取camera_host服务，调用OpenCamera接口获取cameraDevice，调用ICameraDevice的GetStreamOperator接口获取流控制器代理对象<br>
判断流控制器对象streamOperator不为空<br>
与Capture捕获动作配套使用，如果不希望捕获中断，则调用此接口使得未完成的buffer顺利回传<br>
std::vector&lt;int> streamIds = {1001};<br>
OHOS::sptr&lt;IStreamOperatorCallback> offlineStreamOperatorCallback = new StreamOperatorCallback();<br>
OHOS::sptr&lt;IOfflineStreamOperator> offlineOperator;<br>
streamOperator->ChangeToOfflineStream(streamIds, offlineStreamOperatorCallback, offlineOperator);<br>
判断返回值
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode CancelCapture(int captureId);
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
调用完ChangeToOfflineStream后，会获取到离线流代理对象offlineStreamOperator<br>
int captureId = 2001; // 调用端提供捕获动作的唯一标识，切换到离线流的捕获动作标识<br>
offlineStreamOperator->CancelCapture(captureId);<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode ReleaseStreams(const std::vector&lt;int> &streamIds);</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
调用完ChangeToOfflineStream后，会获取到离线流代理对象offlineStreamOperator<br>
int captureId = 2001; // 调用端提供捕获动作的唯一标识，切换到离线流的捕获动作标识<br>
std::vector&lt;int> streamIds = {captureId};<br>
offlineStreamOperator->ReleaseStreams(streamIds);<br>
释放离线流资源后判断返回值<br>
</p>
</td>

</tr>
</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
CamRetCode Release();
</p>
</td>


</td>
<td class="cellrowborder" valign="top" width="64.95649564956496%" headers="mcps1.2.4.1.2 "><p id="p16761419154811"><a name="p16761419154811"></a><a name="p16761419154811"></a>
调用完ChangeToOfflineStream后，会获取到离线流代理对象offlineStreamOperator<br>
offlineStreamOperator->Release();<br>
释放IOfflineStreamOperator控制的流资源，并且释放IOfflineStreamOprator句柄<br>
</p>
</td>


</tbody>
</table>



