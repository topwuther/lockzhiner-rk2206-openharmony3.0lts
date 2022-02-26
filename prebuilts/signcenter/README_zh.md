# 签名工具仓<a name="ZH-CN_TOPIC_0000001086718894"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [约束](#section119744591305)
-   [说明](#section1312121216216)

## 简介<a name="section11660541593"></a>

在OpenHarmony构建中需要对应用进行签名，以此保证应用完整性和来源可靠。本仓提供二进制签名工具（hapsigntoolv2.jar），用于OpenHarmony应用签名。

## 目录<a name="section161941989596"></a>

```
/prebuilts/signcenter
├── NOTICE                     # 开源NOTICE
├── hapsigntool                # 签名工具存放目录
│   └── hapsigntoolv2.jar     # OpenHarmony应用签名工具
```

## 约束<a name="section119744591305"></a>

运行环境约束：JDK8

## 说明<a name="section1312121216216"></a>

签名命令示例：

```
java -jar hapsigntoolv2.jar sign -mode localjks -privatekey "OpenHarmony Software Signature" -inputFile camera.hap -outputFile signed_camera.hap -signAlg SHA256withECDSA -keystore OpenHarmony.jks -keystorepasswd 123456 -keyaliaspasswd 123456 -profile camera_release.p7b -certpath OpenHarmony.cer -profileSigned 1
```

关键字段说明：

```
hapsigntoolv2.jar ：OpenHarmony签名工具
-mode ：签名模式。OpenHarmony签名密钥存放于本地keystore文件，因此签名模式选择localjks
-privatekey：密钥对别名
-inputFile ：待签名的应用
-outputFile：签名后的应用
-signAlg : 签名算法
-keystore：keystore文件路径
-keystorepasswd：keystore的密码，OpenHarmony.jks的默认密码为123456
-keyaliaspasswd：签名密钥的密码，密钥（OpenHarmony Software Signature）的默认密码为123456
-profile ：应用能力授权文件
-certpath：签名证书文件路径
-profileSigned：指示profile文件是否带有签名，1表示有签名，0表示没有签名，缺省值为1。
```

