# 数据分级保护<a name="ZH-CN_TOPIC_0000001154987675"></a>

-   [简介](#section11660541593)
-   [系统架构](#section342962219551)
-   [接口说明](#section92711824195113)
-   [相关仓](#section155556361910)

## 简介<a name="section11660541593"></a>

在OpenHarmony中，数据分级保护模块负责提供数据分级的保护策略。数据分级保护模块提供了数据分级相关的接口定义。（OpenHarmony当前不提供实际的功能实现。依赖设备厂商实现接口对应的功能，对搭载OpenHarmony的设备上的数据提供安全保护）。

数据分级保护模块当前提供如下接口定义：

-   数据分级标签设置和查询接口：对业务生成的文件数据提供设置和查询风险等级标签的接口，业务可使用该接口设定和查询落盘文件数据的风险等级，使该文件在系统中具有对应的数据风险分级标识。
-   基于设备安全等级的数据跨设备访问控制接口：提供基于设备安全等级的数据跨设备访问控制的接口，分布式跨设备数据传输业务可使用该接口获得对端设备可支持的数据风险等级。

为实现上述接口定义，数据分级保护模块当前包含数据分级标签设置查询接口和基于设备安全等级的数据跨设备访问控制接口两个子模块，模块中仅包括接口定义，而不包含实际的功能实现，其部署逻辑如下图：

## 系统架构<a name="section342962219551"></a>

**图 1**  数据分级保护子系统架构图<a name="fig4460722185514"></a>  


![](figures/dataclassification_zh.png)

## 接口说明<a name="section92711824195113"></a>

**表 1**  数据分级保护提供的API接口功能介绍

<a name="table1741910115412"></a>
<table><thead align="left"><tr id="row84116107545"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p19410105546"><a name="p19410105546"></a><a name="p19410105546"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p20411510105417"><a name="p20411510105417"></a><a name="p20411510105417"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row1411110205418"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p7664989558"><a name="p7664989558"></a><a name="p7664989558"></a>int SetLabel(int userId, const char *filePath, const char *labelName, const char *labelValue, int flag);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p1941010175414"><a name="p1941010175414"></a><a name="p1941010175414"></a>设置风险等级标签能力，当前返回成功，设备厂商需自行实现标签风险等级设置能力。建议设置在文件的扩展属性中，数据风险等级更详细的定义描述参考开发者文档。</p>
</td>
</tr>
<tr id="row10411710145415"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p552394945518"><a name="p552394945518"></a><a name="p552394945518"></a>int GetLabel(int userId, const char *filePath, const char *labelName, char *labelValue, const int valueLen);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p241101012548"><a name="p241101012548"></a><a name="p241101012548"></a>查询风险等级标签能力，当前返回S3，设备厂商自行实现标签风险等级查询能力。数据风险等级更详细的定义描述参考开发者文档。</p>
</td>
</tr>
<tr id="row1142121095419"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p14759321205620"><a name="p14759321205620"></a><a name="p14759321205620"></a>int GetFlag(int userId, const char *filePath, const char *labelName);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p14211020544"><a name="p14211020544"></a><a name="p14211020544"></a>查询风险等级的辅助信息，当前返回FLAG_FILE_PROTECTION_COMPLETE_UNLESS_OPEN，设备厂商自行实现标风险等级的辅助信息查询能力。数据风险等级更详细的定义描述参考开发者文档。</p>
</td>
</tr>
<tr id="row10264187175820"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1526447155814"><a name="p1526447155814"></a><a name="p1526447155814"></a>int32_t DEVSL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p172641072584"><a name="p172641072584"></a><a name="p172641072584"></a>获取对应设备可支持的数据风险等级，当前返回S3，设备厂商需自行实现该功能，数据风险等级更详细的定义描述参考开发者文档。</p>
</td>
</tr>
<tr id="row18882199125920"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p6882169185915"><a name="p6882169185915"></a><a name="p6882169185915"></a>int32_t DEVSL_OnStart(int32_t maxDevNum);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p6882119135918"><a name="p6882119135918"></a><a name="p6882119135918"></a>设备数据安全等级模块初始化，设备厂商需自行实现该功能。</p>
</td>
</tr>
<tr id="row316118198591"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1216221920596"><a name="p1216221920596"></a><a name="p1216221920596"></a>void DEVSL_ToFinish(void);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p11162171911596"><a name="p11162171911596"></a><a name="p11162171911596"></a>设备数据安全等级模块去初始化，设备厂商需自行实现该功能。</p>
</td>
</tr>
</tbody>
</table>

## 相关仓<a name="section155556361910"></a>

安全子系统

base/security/dataclassification

