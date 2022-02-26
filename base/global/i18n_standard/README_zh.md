# 国际化组件<a name="ZH-CN_TOPIC_0000001101364976"></a>

-   [简介](#section11660541593)
-   [目录](#section1464106163817)
-   [约束](#section1718733212019)
-   [说明](#section894511013511)
-   [相关仓](#section15583142420413)

## 简介<a name="section11660541593"></a>

**国际化组件**提供时间日期格式化等国际化能力。

## 目录<a name="section1464106163817"></a>

国际化组件源代码目录结构如下所示：

```
/base/global/
├── i18n_standard           # 国际化框架代码仓
│   ├── frameworks          # 国际化框架核心代码
│   ├── interfaces          # 国际化框架接口
│   │   ├── js              # 国际化框架JavaScript接口
│   │   └── native          # 国际化框架native接口
```

## 约束<a name="section1718733212019"></a>

**语言限制**：JavaScript语言

**支持范围限制**：支持的语言符合 ISO 639 标准 2 字母或 3 字母语言码，支持的文本符合 ISO 15924 标准 4 字母文本码，支持的国家符合 ISO 3166 标准 2 字母国家码。

## 说明<a name="section894511013511"></a>

提供时间日期格式化接口，使时间日期格式（如年月日顺序、月份和星期词汇、使用12或24小时制等）跟随系统设置满足不同区域用户的文化习惯。更详细的内容见API文档。示例如下：

```
const date = new Date(2021, 11, 17, 3, 24, 0);  // 创建包含日期和时间信息的Date对象
fmt = new Intl.DateTimeFormat('en-US')  // 创建时间日期格式化实例
console.log(fmt.format(date));  // 使用创建时间日期格式化对象实例进行格式化
```

## 相关仓<a name="section15583142420413"></a>

全球化子系统

**global\_i18n\_standard**

global\_resmgr\_standard

