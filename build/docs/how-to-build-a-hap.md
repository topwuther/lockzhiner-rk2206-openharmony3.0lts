# hap的编译

## 术语

gn 目标： 

## hap包的构成

L2上的hap包由资源，raw assets，js assets，native库，config.json等部分构成。


## 编译系统提供的模板

编译系统提供了4个模板，用来编译hap包。

模板集成在ohos.gni中，使用之前需要引用build/ohos.gni

### ohos_resources

- 声明一个资源目标。资源目标被restool编译之后会生成index文件，hap中会打包资源源文件和index文件。
- 该目标会同时生成资源编译后的ResourceTable.h，直接依赖该目标就可以引用该头文件

- 资源目标的目标名必须以"resources"或"resource"或"res"结尾，否则编译检查时会报错
- 支持的变量:
  1. sources: 资源的路径，变量类型是list，可以写多个路径
  2. hap_profile: 编译资源时需要提供对应hap包的config.json
  3. deps: 当前目标的依赖，可选

### ohos_assets

- 声明一个资产目标
- 注意拼写：assets不是assert
- assets目标的目标名必须以"assets"或"asset"结尾
- 支持的变量：
  1. sources：raw assets所在路径，变量类型是list，可以写多个路径
  2. deps： 当前目标的依赖，可选

## ohos_js_assets

- 声明一个JS 资源目标，JS资源是L2 hap包的可执行部分
- JS assets目标的目标名必须以"assets"或"asset"结尾
- 支持的变量：
  1. source_dir: JS 资源的路径，变量类型是string，只能写一个
  2. deps: 当前目标的依赖，可选

### ohos_hap

- 声明一个hap目标，该目标会生成一个hap包，最终将会打包到system镜像中

- 支持的变量：

  1.  hap_profile: hap包的config.json

  2. deps: 当前目标的依赖

  3. shared_libraries: 当前目标依赖的native库

  4. hap_name: hap包的名字，可选，默认为目标名

  5. final_hap_path: 用户可以制定生成的hap的位置，可选，final_hap_path中会覆盖hap_name

  6. subsystem_name: hap包从属的子系统名，需要和ohos.build中的名字对应，否则将导致无法安装到system镜像中

  7. part_name: hap包从属的部件名，同subsystem_name

  8. js2abc: 是否需要将该hap包转换为ARK的字节码

     签名篇见：https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/quick-start/%E9%85%8D%E7%BD%AEOpenHarmony%E5%BA%94%E7%94%A8%E7%AD%BE%E5%90%8D%E4%BF%A1%E6%81%AF.md

  9. certificate_profile: hap对应的授权文件，用于签名

  10. certificate_file: 证书文件，证书文件和授权文件，应用开发者需要去openharmony官网申请

  11. keystore_path: keystore文件，用于签名

  12. keystore_password: keystore的密码，用于签名

  13. key_alias: key的别名 

  14. module_install_name:安装时的hap包名称

  15. module_install_dir: 安装到system中的位置，默认安装在system/app目录下

## 一个例子

```json
import("//build/ohos.gni") # 引用ohos.gni

ohos_hap("clock") {
  hap_profile = "./src/main/config.json" # config.json
  deps = [
    ":clock_js_assets", # JS assets
    ":clock_resources", # 资源
  ]
  shared_libraries = [
    "//third_party/libpng:libpng", # native库
  ]
  certificate_profile = "../signature/systemui.p7b" # Cer文件
  hap_name = "SystemUI-NavigationBar" # 名字
  part_name = "prebuilt_hap"
  subsystem_name = "applications"
}
ohos_js_assets("clock_js_assets") {
  source_dir = "./src/main/js/default"
}
ohos_resources("clock_resources") {
  sources = [ "./src/main/resources" ]
  hap_profile = "./src/main/config.json"
}
```

