# appspawn应用孵化器组件<a name="ZH-CN_TOPIC_0000001081995748"></a>

-   [简介](#section469617221261)
-   [目录](#section15884114210197)
-   [约束](#section12212842173518)
-   [对应仓库](#section641143415335)

## 简介<a name="section469617221261"></a>

应用孵化器，负责接受应用程序框架的命令孵化应用进程，设置其对应权限，并调用应用程序框架的入口。

## 目录<a name="section15884114210197"></a>

```
base/startup/appspawn_lite/     # 应用孵化器组件
├── LICENSE
└── services
    ├── include              # 应用孵化器组件头文件目录
    ├── src                  # 应用孵化器组件源文件目录
    └── test                 # 应用孵化器组件测试用例源文件目录
        └── unittest
```

## 约束<a name="section12212842173518"></a>

目前支持小型系统设备（参考内存≥1MB），如Hi3516DV300 、Hi3518EV300。

## 对应仓库<a name="section641143415335"></a>

[启动恢复子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/%E5%90%AF%E5%8A%A8%E6%81%A2%E5%A4%8D%E5%AD%90%E7%B3%BB%E7%BB%9F.md)

[startup\_syspara\_lite](https://gitee.com/openharmony/startup_syspara_lite/blob/master/README_zh.md)

**[startup\_appspawn\_lite](https://gitee.com/openharmony/startup_appspawn_lite/blob/master/README_zh.md)**

[startup\_bootstrap\_lite](https://gitee.com/openharmony/startup_bootstrap_lite/blob/master/README_zh.md)

[startup\_init\_lite](https://gitee.com/openharmony/startup_init_lite/blob/master/README_zh.md)
