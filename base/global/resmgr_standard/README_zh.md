# 资源管理组件<a name="ZH-CN_TOPIC_0000001162518223"></a>

-   [简介](#section11660541593)
-   [目录](#section1464106163817)
-   [约束](#section1718733212019)
-   [说明](#section894511013511)
-   [相关仓](#section15583142420413)

## 简介<a name="section11660541593"></a>

**资源管理组件**为应用提供加载多语言界面资源的能力，如不同语言下提供不同的应用名称或图标等。

## 目录<a name="section1464106163817"></a>

资源管理组件源代码目录结构如下所示：

```
/base/global/
├── resmgr_standard         # 资源管理代码仓
│   ├── frameworks          # 资源管理核心代码
│   │   ├── resmgr          # 资源解析核心代码
│   │   │   ├── include     # 资源管理头文件
│   │   │   ├── src         # 资源管理实现代码
│   │   │   └── test        # 资源管理测试代码
│   ├── interfaces          # 资源管理接口
│   │   ├── innerkits       # 资源管理对子系统间接口
│   │   └── js              # 资源管理JavaScript接口
```

## 约束<a name="section1718733212019"></a>

**语言限制**：JavaScript语言

## 说明<a name="section894511013511"></a>

调用  **getString **接口获取应用的资源信息 。

```
import resmgr from '@ohos.resmgr'
.....
    resmgr.getResourceManager((error,mgr) => {
        // callback
        mgr.getString(0x1000000, (error, value) => {
            if (error != null) {
                console.log(error);
            } else {
                console.log(value);
            }
        });

        // promise
        mgr.getString(0x1000000).then(value => {
            console.log(value);
        }).catch(error => {
            console.log("getstring promise " + error);
        });
    });
```

## 相关仓<a name="section15583142420413"></a>

全球化子系统

global\_i18n\_standard

**global\_resmgr\_standard**

