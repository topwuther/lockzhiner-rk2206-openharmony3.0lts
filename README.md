# 小凌派RK2206非官方维护仓库
福建船政交通职业学院智慧小屋代码共享

这是一个非官方的小凌派RK2206仓库，主要添加一些原有代码没有的新特性以及新案例，还有官方代码仓库中的一些错误修复
## BUG修复:
修复`./device/rockchip/tools/package/mkimage.sh`的一个语法错误，以及在部分Linux发行版中无法正确生成烧录镜像的问题

## 新特性:
 - 允许从局域网中的NTP服务器上获取时间(秒)
 - 能够根据NTP时间生成开发板中的UUID（该UUID只会生成一次并写入到NOR FLASH中，可当做CHIP ID使用）
## 新案例(智能小屋):
 - 门铃系统（x0_doorbell）
 - 智能门系统（x1_smartdoor）
 - 传感器以及LCD屏控制系统（x2_sensor）
 - 语音控制系统（x3_voice）
 - MQTT网络控制系统（x4_netcontrol）
## 环境要求:
 - Python 3.11
 - Git
## 使用方法:
### 下载仓库代码
> git clone https://github.com/topwuther/lockzhiner-rk2206-openharmony3.0lts.git

### 设置环境
> . venv/bin/activate\
hb set -root .

### 使NTP能够使用
修改
`//third_party/ntpclient/src/ntpclient.c`
中的define字段

SERVER_IP为需要连接到的NTP IP地址

### 运行智慧小屋系统
查看例程b7_wifi_tcp，根据“WiFi ssid 和密码设置”来操作以设置WIFI\
修改`//vendor/lockzhiner/rk2206/samples/x4_netcontrol/src/mqttclient.c`
中的define字段

MQTT_SERVER_IP为MQTT的IP地址

MQTT_SERVER_PORT为MQTT的端口

MQTT_USERNAME为MQTT的用户名

MQTT_PASSWORD为MQTT的密码

编译代码
>hb build -f

烧录到开发板中
>./flash.py

