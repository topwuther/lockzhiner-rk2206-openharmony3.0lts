# 小凌派RK2206非官方维护仓库
福建船政交通职业学院智慧小屋代码共享

这是一个非官方的小凌派RK2206仓库，主要添加一些原有代码没有的新特性以及新案例，还有官方代码仓库中的一些错误修复
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
下载仓库代码
> git clone https://github.com/topwuther/lockzhiner-rk2206-openharmony3.0lts.git

设置环境
> . venv/bin/activate\
hb set -root .

编译代码
>hb build -f

烧录到开发板中
>./flash.py

