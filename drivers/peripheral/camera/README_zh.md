# 代码仓
OHOS_STD/drivers_peripheral

# 目录结构
```
    .
    ├── hal
    │   ├── adapter
    │   │   ├── chipset
    │   │   │   ├── gni
    │   │   │   ├── hispark_taurus
    │   │   │   │   ├── include
    │   │   │   │   │   ├── device_manager
    │   │   │   │   │   └── driver_adapter
    │   │   │   │   └── src
    │   │   │   │       ├── device_manager
    │   │   │   │       ├── driver_adapter
    │   │   │   │       └── pipeline_core
    │   │   │   │           ├── ipp_algo_example
    │   │   │   │           └── nodes
    │   │   │   │               ├── mpi_node
    │   │   │   │               ├── venc_node
    │   │   │   │               ├── vi_node
    │   │   │   │               ├── vo_node
    │   │   │   │               └── vpss_node
    │   │   │   └── rpi3
    │   │   │       ├── include
    │   │   │       │   └── device_manager
    │   │   │       └── src
    │   │   │           ├── device_manager
    │   │   │           └── driver_adapter
    │   │   │               ├── main_test
    │   │   │               └── test
    │   │   │                   └── unittest
    │   │   │                       └── include
    │   │   └── platform
    │   │       └── v4l2
    │   │           └── src
    │   │               ├── driver_adapter
    │   │               │   ├── include
    │   │               │   └── src
    │   │               └── pipeline_core
    │   │                   ├── ipp_algo_example
    │   │                   └── nodes
    │   │                       ├── uvc_node
    │   │                       └── v4l2_source_node
    │   ├── buffer_manager
    │   │   ├── include
    │   │   ├── src
    │   │   │   ├── gralloc_buffer_allocator
    │   │   │   └── heap_buffer_allocator
    │   │   └── test
    │   │       └── unittest
    │   ├── device_manager
    │   │   ├── include
    │   │   ├── src
    │   │   └── test
    │   │       └── unittest
    │   │           ├── mpi
    │   │           └── v4l2
    │   ├── hdi_impl
    │   │   ├── include
    │   │   │   ├── camera_device
    │   │   │   ├── camera_host
    │   │   │   ├── offline_stream_operator
    │   │   │   └── stream_operator
    │   │   ├── src
    │   │   │   ├── camera_device
    │   │   │   ├── camera_host
    │   │   │   ├── offline_stream_operator
    │   │   │   └── stream_operator
    │   │   └── test
    │   │       └── unittest
    │   ├── include
    │   ├── init
    │   ├── pipeline_core
    │   │   ├── host_stream
    │   │   │   ├── include
    │   │   │   └── src
    │   │   ├── include
    │   │   ├── ipp
    │   │   │   ├── include
    │   │   │   └── src
    │   │   ├── nodes
    │   │   │   ├── include
    │   │   │   └── src
    │   │   │       ├── dummy_node
    │   │   │       ├── fork_node
    │   │   │       ├── merge_node
    │   │   │       ├── node_base
    │   │   │       ├── sensor_node
    │   │   │       ├── sink_node
    │   │   │       ├── source_node
    │   │   │       └── transform_node
    │   │   ├── pipeline_impl
    │   │   │   ├── include
    │   │   │   └── src
    │   │   │       ├── builder
    │   │   │       ├── dispatcher
    │   │   │       ├── parser
    │   │   │       └── strategy
    │   │   │           └── config
    │   │   ├── src
    │   │   ├── test
    │   │   │   └── unittest
    │   │   └── utils
    │   ├── test
    │   │   ├── mpi
    │   │   │   ├── include
    │   │   │   └── src
    │   │   └── v4l2
    │   │       ├── include
    │   │       └── src
    │   └── utils
    │       ├── event
    │       └── watchdog
    ├── hal_c
    │   ├── hdi_cif
    │   │   ├── include
    │   │   └── src
    │   └── include
    └── interfaces
        └── include
            ├── callback
            │   ├── device
            │   ├── host
            │   └── operator
            ├── client
            └── server

```

# 接口说明

## 接口数量：25

### 包：

    package ohos.hdi.camera.host@1.0;

### 路径：

    interfaces/include/icamera_host.h

### 类：

    CameraHost：提供设备个数查询、能力查询和打开设备等接口；

### 函数：

```

/**
 * @brief 设置Host回调函数
 *
 * @param callback Host回调函数
 * @return CamRetCode
 */
CamRetCode SetCallback([in] ICameraHostCallback callback);

/**
 * @brief 获取当前可用的Camera设备列表
 *
 * @param ids 返回的当前可用的设备列表
 */
CamRetCode GetCameraIds([out] String[] ids);

/**
 * @brief 获取Camera的能力集
 *
 * @param cameraId 要获取的Camera设备id
 * @param ability Camera设备的能力集
 * @return CamRetCode
 * @see CameraAbility
 */
CamRetCode GetCameraAbility([in] String cameraId, [out] CameraAbility ability);

/**
 * @brief 打开Camera设备
 *
 * @param cameraId 要打开的Camera设备id
 * @param callback Camera设备的回调函数
 * @param camera 返回的Camera设备接口
 * @return CamRetCode
 * @see ICameraDeviceCallback
 * @see ICameraDevice
 */
CamRetCode OpenCamera([in] String cameraId, [in] ICameraDeviceCallback callback, [out] ICameraDevice camera);

/**
 * @brief 打开或者关闭手电筒
 *
 * @param cameraId 手电筒与之关联的Camera设备id
 * @param isEnable 闪光灯打开或者关闭
 * @return CamRetCode
 */
CamRetCode SetFlashlight[in] String cameraId, [out] boolean isEnable);

```


### 包：
    package ohos.hdi.camera.device@1.0;
### 路径：
    interfaces/include/icamera_device.h
### 类：
    CameraDevice: Camera设备控制接口;
### 函数：

    /**
     * @brief 获取流控制器
     *
     * @param callback 流回调函数
     * @param operator 返回的流控制器
     * @return CamRetCode
     */
    CamRetCode GetStreamOperator([in] IStreamOperatorCallback callback, [out] IStreamOperator operator);

    /**
     * @brief 更新设备控制参数
     *
     * @param setting Camera设置参数
     * @return CamRetCode
     */
    CamRetCode UpdateSettings(CameraSetting setting);

    /**
     * @brief 设置Result回调模式和回调函数
     *
     * @param mode Result的上报模式
     * @param callback Result的上报callback函数
     * @return CamRetCode
     */
    CamRetCode SetResultMode([in] ResultCallbackMode mode);

    /**
     * @brief 获取使能的ResultMeta
     *
     * @param results 使能的ResultMeta数组
     * @return CamRetCode
     */
    CamRetCode GetEnabledResults([out] MetaType[] results);

    /**
     * @brief 使能具体的ResultMeta
     *
     * @param results 需要使能的ResultMeta数组
     * @return CamRetCode
     */
    CamRetCode EnableResult([in] MetaType[] results);

    /**
     * @brief 禁止具体的ResultMeta
     *
     * @param results 需要禁止的ResultMeta数组
     * @return CamRetCode
     */
    CamRetCode DisableResult(in] MetaType[] results);

    /**
     * @brief 关闭Camera设备
     */
    void Close();



### 包：
    package ohos.hdi.camera.stream@1.0;
### 路径：
    interfaces/include/istream_operator.h
### 类：
    SteamOperator: 提供设备个数查询、能力查询和打开设备等接口;
### 函数：


    /**
     * @brief 查询模式和流的配置组合是否支持
     *
     * @param mode 要配置的模式
     * @param modeSettings 模式对应的参数
     * @param info 流的信息
     * @param type 配置流支持的方式
     * @return RetCode
     */
    RetCode IsStreamsSupported([in] OperationMode mode, [in] CameraMetadata modeSetting, [in] StreamInfo[] info, [out] StreamSupportType type);

     /**
     * @brief 创建流
     *
     * @param streamInfo 需要创建的流信息列表
     * @return RetCode
     */
    RetCode CreateStreams([in] StreamInfo[] streamInfo);

    /**
     * @brief 释放流
     *
     * @param streamIds 需要释放的流Id列表
     * @return RetCode
     */
    RetCode ReleaseStreams([in] int[] streamIds);

    /**
     * @brief 使能相机流的配置，并配置工作模式
     *
     * @param mode 要配置的模式
     * @param modeSetting 模式对应的参数
     * @return RetCode
     * @see OperationMode
     */
    RetCode CommitStreams([in] OperationMode mode, [in] CameraMetadata modeSetting);

    /**
     * @brief 获取所有流特征
     *
     * @param streamId  需要获取额的流id
     * @param attribute 流的属性
     * @return RetCode
     */
    RetCode GetStreamAttributes([out] StreamAttribute[] attributes);

    /**
     * @brief 关联BufferQueue
     *
     * @param streamId 需要关联的流id
     * @param producer 要关联的BufferQueue对应的producer接口
     * @return RetCode
     */
    RetCode AttachBufferQueue([in] streamId, [in] IBufferClientProducer producer);

    /**
     * @brief 取消关联BufferQueue
     *
     * @param streamId 取消关联的流id
     * @return RetCode
     */
    RetCode DetachBufferQueue([in] int streamId);

    /**
     * @brief 捕获图像
     *
     * @param captureId 标志此次捕获请求的id
    * @param info 捕获图像的参数信息，如果数组中的info存在多个，则表明是batch模式，连续捕获多帧
     * @param isStreaming 是否连续捕获
     * @see StreamAttribute
     * @return RetCode
     */
    RetCode Capture([in] int captureId, [in] CaptureInfo[] info, [in] bool isStreaming);

    /**
     * @brief 取消图像捕获
     *
     * @return RetCode
     */
    RetCode CancelCapture([in] int captureId);

    /**
     * @brief 将普通流转换成离线流
     *
     * @param streams 需要转换的普通流Id列表
     * @param callback 离线流的callback
     * @param offlineOperator 离线流的控制器
     * @return RetCode
     */
    RetCode ChangeToOfflineStream([in] int[] streamIds, [in]IStreamOperatorCallback callback, [out]IOfflineStreamOperator offlineOperator);`

### 包：
    package ohos.hdi.camera.stream@1.0;
### 路径：
    interfaces/include/istream_operator.h
### 类：
    IOfflineStreamOperator: 离线流控制类;
### 函数：

    /**
     * @brief 取消图像捕获
     *
     * @param captureId 取消捕获的captureId
     * @return RetCode
     */
    RetCode CancelCapture([in] int captureId);

    /**
     * @brief 释放离线流资源
     *
     * @param streamIds 释放的流id列表
     * @return RetCode
     */
    RetCode ReleaseStreams([in] int[] streamIds);

    /**
     * @brief 释放IOfflineStreamOperator句柄
     *
     * @return RetCode
     */
    RetCode Release();`

## 数据结构定义

    interfaces/hdi/types.h

## 使用方法

    using namespace Camera;

### 获取CameraHost

    std::shared_ptr<Camera::CameraHost> cameraHost = Camera::CameraHost::CreateCameraHost();

### 获取配置的cameraId

    std::vector<std::string> cameraIds;
cameraHost->GetCameraIds(cameraIds); #

### 打开camera设备并获取到device

    const std::shared_ptr<Camera::ICameraDeviceCallback> callback = std::make_shared<Camera::ICameraDeviceCallback>();

    std::shared_ptr<Camera::CameraDevice> device;

    std::cout << "cameraIds.front() = " << cameraIds.front() << std::endl;

    Camera::CamRetCode rc = cameraHost->OpenCamera(cameraIds.front(), callback, device);


### 调用device的GetStreamOperator函数获取streamOperator

    std::make_shared<Camera::IStreamOperatorCallback>();
    std::shared_ptr<Camera::StreamOperator> streamOperator = nullptr;
    rc = device->GetStreamOperator(streamOperatorCallback, streamOperator);`
### 批量创建数据流
    std::vector<std::shared_ptr<Camera::StreamInfo>> streamInfos;
    std::shared_ptr<Camera::StreamInfo> streamInfo = std::make_shared<Camera::StreamInfo>();
    streamInfo->streamId_ = 1001;
    streamInfo->width_ = 1280;
    streamInfo->height_ = 720;
    streamInfo->format_ = 2;
    streamInfo->datasapce_ = 10;
    streamInfo->intent_ = Camera::PREVIEW;
    streamInfo->tunneledMode_ = 5;
    streamInfos.push_back(streamInfo);
    rc = streamOperator->CreateStreams(streamInfos);
### 配流起流
    rc = streamOperator->CommitStreams(Camera::NORMAL, nullptr);

