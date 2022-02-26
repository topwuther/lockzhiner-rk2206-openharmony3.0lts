# 相机<a name="ZH-CN_TOPIC_0000001103554544"></a>

-   [简介](#section11660541593)
    -   [架构图](#section78574815486)

-   [目录](#section161941989596)
-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

相机应用是OpenHarmony标准系统中预置的系统应用，为用户提供基础的相机拍摄功能，包括预览、拍照、缩略图显示、跳转相册、分布式多端协同。

### 架构图<a name="section78574815486"></a>

![](figures/camera.png)

## 目录<a name="section161941989596"></a>

```
/applications/standard/camera/
├── figures                                             # 架构图目录
├── entry                                               # 相机主Ability
│   └── src
│       └── main
│           ├── config.json                             # 应用配置文件
│           └── js                                      # js代码目录
│                └──default
│                       ├──common                       # 公共资源文件目录
│                           ├──component                # 自定义组件目录
│                                   └──DeviceListDialog # 分布式设备列表弹框组件                
│                           ├──media                    # 图片和音频资源目录
│                           └──utils                    # 公共工具类目录
│                       ├──i18n                         # 全球化字符串资源目录
│                       ├──model                        # Model层
│                           ├──KvStoreModel             # 分布式数据Model
│                           ├──PreviewModel             # 相机主界面Model
│                           └──RemoteDeviceModel        # 分布式设备列表Model
│                       ├──pages                        # View层
│                           ├──DistributedView          # 分布式协同界面
│                           └──PreviewView              # 相机主界面
│                       ├──presenter                    # Presenter层
│                           ├──DistributedPresenter     # 分布式协同界面与数据的交互逻辑封装
│                           └──PreviewPresenter         # 相机主界面与数据的交互逻辑封装
│                       └──app.js                       # 文件用于全局JavaScript逻辑和应用生命周期管理
│           └── resources                               # 资源文件目录
├── signature                                           # 证书文件目录
├── LICENSE                                             # 许可文件
```

## 相关仓<a name="section1371113476307"></a>

系统应用

**applications\_camera**