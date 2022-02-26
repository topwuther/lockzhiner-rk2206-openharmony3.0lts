# 快速入门

## 环境配置

1. 预安装

   ```
   ./prebuilts_download.sh
   pip3 install --user ohos-build
   ```

2. 设置环境变量

   ```
   source build/envsetup.sh
   ```

## 编译

1. 编译L0

   ```
   hb set   //选择lockzhiner-rk2206
   hb clean
   hb build
   ```

## 固件烧写

1. 烧写固件

   ```
   ./flash.py
   ```

3. 查看开发板烧写状态

   ```
   ./flash.py -q
   ```

   
