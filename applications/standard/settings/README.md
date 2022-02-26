# Settings 源码开发说明
大纲：
1. 项目介绍
2. 工程结构
   1. 目录结构
   2. 整体架构
3. 代码使用
   1. 代码下载
      1. 从码云clone代码（配置ssh，下载代码）
   2. 环境搭建
      1. 下载DevEco Studio
      2. 安装开发SDK
      3. 更新 ace-loader
   3. 工程导入DevEco Studio
      1. 如何build项目
4. 基础开发说明
   1. NAPI 接口调用
   2. 异步回调的使用
   3. 如何引用资源文件
5. 典型接口的使用
6. 签名打包
   1. 签名
      1. 签名文件的获取
      2. 签名文件的配置
   2. 打包
      1. debug打包
      2. release打包
7. 安装、运行、调试
   1. 应用安装、运行
      1. 重新安装需要清理缓存的内容
   2. 应用调试
      1. log打印
      2. log获取及过滤
8. 发布hap包
   1. 发布到码云
## 1. 项目介绍
Settings是基于Harmony OS平台开发的基于OHOS提供基础设置功能。主要包括页面展示跳转逻辑、基础设置项(wlan设置、亮度设置、应用管理、日期和时间、关于手机)以及本设备其他应用设置项的收集、其他设备设置项收集的实现。项目采用MVVM架构模式，让各个层级之间不直接访问减少相互依赖。

Settings采用纯 JS 语言开发，开发过程中不涉及任何 Java 部分的代码。

## 2. 工程结构
### 目录结构

```
/applications/standard/settings
├── entry/src/main
│   └── js/default
│       ├── common    # 公共代码存在目录
│       ├── pages     # 界面源代码文件存放目录
│       ├── models    # 数据模型源代码存放目录
│       ├── component # UI组件源代码存放目录
│       ├── i18n      # 全球化资源文件存放目录
│       └── app.js
│   └── resources     # 资源存放目录
│   └── config.json   # 应用信息及组件信息清单
```

### 整体架构

![settings_en](./img/settings_en.png)

## 3. 代码使用
### 代码下载
基于L2的 Settings代码可以采用从码云上克隆的方式下载。  
[下载地址](https://gitee.com/OHOS_STD/applications_standard_settings)

#### 从码云克隆代码
##### 1.配置SSH公钥
1. 通过[登录网址](https://gitee.com/login)登录码云
2. 在码云的个人设置中设置SSH公钥
   1. 生成SSH公私钥  
   在命令窗口输入以下命令，邮箱地址替换为自己的邮箱地址，一直回车直至完成为止。
    ```
    ssh-keygen -t rsa -C "xxxx@xxxx.com"
    ```
    > 执行完成后，会在用户目录下的.ssh文件夹内生成 `id_rsa` 与 `id_rsa.pub` 两个文件，其中的 `id_rsa.pub` 即为生成的 SSH公钥  
   2. 复制公钥内容  
   输入`cat .ssh/id_rsa.pub`，复制打印内容。
   ![](./img/3-1.png)
   3. 在设置->安全设置->SSH公钥 中设置 SSH公钥  
   将第二步中复制的内容按照画面提示粘贴到码云上相应的内容区域，并点击确定。
   ![](./img/3-2.png)
   > 注意，在码云中，添加SSH公钥之后，需要进行账号的密码验证，验证通过才能成功添加。
##### 2.下载代码
1. 浏览器打开[下载地址](https://gitee.com/OHOS_STD/applications_standard_settings)。
2. 点击“克隆/下载”按钮，选择 SSH，点击“复制”按钮。
   ![](./img/source_download_1.png)
3. 在本地新建 Launcher 目录，在 Settings 目录中执行如下命令
   ```
   git clone 步骤2中复制的地址
   ```
### 环境搭建
#### 1. 下载安装 DevEco Studio
Settings使用 DevEco Studio 进行开发，开发前需要下载 DevEco Studio 。  

**1）下载**  

在下载页面下载 DevEco Studio 的安装包压缩文件（[下载地址](https://developer.harmonyos.com/cn/develop/deveco-studio#download)）。
下载页面如下：

![](./img/ds_download.png)

> 注：下载 DevEco Studio 需要注册华为账号。
> ![](img/hw_register.png)

**2）安装**

将下载下来的压缩包解压，得到安装文件如图：

![](./img/ds_exe.png)

双击安装文件进行安装，安装过程如图所示：

![](./img/ds_install_1.png)

![](./img/ds_install_2.png)

#### 2. 安装开发 SDK
DevEco Studio 在安装完成之后会自动下载开发 SDK 下载过程如下图所示：

![](./img/sdk_downloading.png) 

#### 3. 更新 ace-loader
> 由于最新的 ace 框架尚未发布到公开版 sdk 中，所以为了让应用能够顺利编译，需要替换 sdk 中的 ace-loader。

替换下面两个路径下的 ace-loader 文件夹。
```
Sdk\js\2.1.1.20\build-tools
Sdk\js\2.0.1.95\build-tools
```
// TODO 这里是否需要npm install？

### 工程导入 DevEco Studio
> 由于 Settings 的开源代码中删除了工程中的 `build.gradle` 文件，所以我们需要在相应的目录创建 `build.gradle` 文件以保证工程可以被正确导入 DevEco Studio 。  

#### 1. 为 Settings工程添加 `build.gradle` 


#### 2. build Settings 工程

## 4. 基础开发说明
## 5. 典型接口的使用


## 6. 签名打包

### **1）签名 ** 

> 打开项目工程，选择File → Project Structure
>
> ![](./img/6-1.png)
>
> 选择Modules → Signing Configs
> 将对应的签名文件配置如下，完成后点击Apply，再点击OK
>
> ![](./img/6-2.png)
>
> 注：签名文件请参考  （[华为开发者官网](https://developer.huawei.com/consumer/cn/)）
>
> 配置完成后，对应的build.gradle文件中会出现如下内容
>
> ![](./img/6-3.png)
>
> 

### 打包

> 编译hap包  
>
> 签名准备完成后，选择Build → Build Haps(s)/APP(s) → Build Hap(s)
>
> ![](./img/6-4.png)
>
> 编译完成后，hap包会生成在entry\build\outputs\hap\debug\phone\路径下（如果没有配置签名，则只会生成未签名的hap包）
>
> ![](./img/6-5.png)



## 7. 安装、运行、调试

### 应用安装、运行
#### 应用安装
获取 root 权限与读写权限：

```
adb root
adb remount
```
将签名好的 hap 包放入设备的 `/system/app` 目录下。

```
adb push settings.hap /system/app
```
> 注意，如果设备不存在 `/system/app` 目录，则需要手动创建该目录。
> ```
> cd system
> mkdir app
> ```
> `/system/app` 目录放置系统应用，例如：Launcher，SystemUI，Settings 等。此目录应用不用安装系统自动拉起。  


#### 应用运行
Launcher属于系统应用，在将签名的 hap 包放入 `/system/app` 目录后，重启系统，应用会自动拉起。
```
adb reboot
```
> 注意，如果设备之前安装过 Launcher 应用，则需要执行如下两条命令清除设备中存储的应用信息才能够在应用重启的时候将我们装入设备的新 hap 包正常拉起。
> ```
> adb shell rm -rf /data/accounts/
> adb shell rm -rf /data/bundlemgr/
> ```
### 应用调试
#### log打印
- 在程序中添加 log
```JS
console.info("Setting log info");
```
可以在DevEco Studio中查看log
![](./img/ds_hilog_window.png)

#### log获取及过滤
- log获取

将log输出至文件  
```
adb shell hilog > 输出文件名称
```

例：
在真实环境查看log，将全log输出到当前目录的hilog.log文件中
```
adb shell hilog > hilog.log
```

- log过滤

在命令行窗口中过滤log
```
hilog | grep 过滤信息
```

例：过滤包含信息 Label 的 hilog
```
hilog | grep Label
```

## 8. 发布hap包
### 重命名hap包
将编译生成的 hap 包按照如下规则重命名：  
|模块|包名|
|:--|-|
|settings|Settings.hap|

### 发布到码云
1. 访问码云的hap包仓库（[仓库地址](https://gitee.com/OHOS_STD/applications_standard_hap)）。

2. 将仓库克隆到本地  
    点击“克隆/下载”按钮，选择SSH标签，点击复制，使用 `git clone` 命令将复制的地址克隆到本地目录。
    ![](./img/hap_repo.png)

  ```
  git clone 复制的地址
  ```

3. 将签名后的 hap 包放入本地目录，并提交。
   ```
   git add .
   git commit -m "提交信息"
   ```

4. 使用 git push 命令将其推送到 hap 包仓库。
   ```
   git push 
   ```

---



