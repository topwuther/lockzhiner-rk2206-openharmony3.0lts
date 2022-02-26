# dataclassification<a name="EN-US_TOPIC_0000001154987675"></a>

-   [Introduction](#section11660541593)
-   [Architecture](#section342962219551)
-   [Available APIs](#section92711824195113)
-   [Repositories Involved](#section155556361910)

## Introduction<a name="section11660541593"></a>

The data classification module of OpenHarmony provides hierarchical data protection policies and related APIs. \(Currently, OpenHarmony does not provide implementations for specific APIs. These APIs must be implemented by the device vendors to protect the security of data on OpenHarmony devices.\)

The data classification module provides the following APIs \(into two submodules\):

-   APIs for setting and obtaining the data label: With these APIs, you can set and obtain the security level of a file to be written to the disk.
-   APIs for controlling cross-device data access based on the device security level: The distributed cross-device data transmission service can use these APIs to obtain the highest data security level supported by the peer device.

The two submodules only contain API definitions, but do not implement these APIs. The following figure shows the architecture of the data classification module.

## Architecture<a name="section342962219551"></a>

**Figure  1**  Architecture of the data classification module<a name="fig4460722185514"></a>  


![](figures/dataclassification.png)

## Available APIs<a name="section92711824195113"></a>

**Table  1**  APIs provided by the data classification module

<a name="table1741910115412"></a>
<table><thead align="left"><tr id="row84116107545"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p19410105546"><a name="p19410105546"></a><a name="p19410105546"></a>API</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p20411510105417"><a name="p20411510105417"></a><a name="p20411510105417"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row1411110205418"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p7664989558"><a name="p7664989558"></a><a name="p7664989558"></a>int SetLabel(int userId, const char *filePath, const char *labelName, const char *labelValue, int flag);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p1941010175414"><a name="p1941010175414"></a><a name="p1941010175414"></a>Sets a specified label. Currently, this API returns success. You need to implement this function by yourself. You are advised to set the label in the extended attribute of a file. For details about the data security levels, see the developer documentation.</p>
</td>
</tr>
<tr id="row10411710145415"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p552394945518"><a name="p552394945518"></a><a name="p552394945518"></a>int GetLabel(int userId, const char *filePath, const char *labelName, char *labelValue, const int valueLen);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p241101012548"><a name="p241101012548"></a><a name="p241101012548"></a>Obtains the label. Currently, this API returns <strong id="b499513695911"><a name="b499513695911"></a><a name="b499513695911"></a>S3</strong>. You need to implement this function by yourself. For details about the data security levels, see the developer documentation.</p>
</td>
</tr>
<tr id="row1142121095419"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p14759321205620"><a name="p14759321205620"></a><a name="p14759321205620"></a>int GetFlag(int userId, const char *filePath, const char *labelName);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p14211020544"><a name="p14211020544"></a><a name="p14211020544"></a>Obtains the flag of a data security level. Currently, this API returns <strong id="b1887015618017"><a name="b1887015618017"></a><a name="b1887015618017"></a>FLAG_FILE_PROTECTION_COMPLETE_UNLESS_OPEN</strong>. You need to implement this function by yourself. For details about the data security levels, see the developer documentation.</p>
</td>
</tr>
<tr id="row10264187175820"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1526447155814"><a name="p1526447155814"></a><a name="p1526447155814"></a>int32_t DEVSL_GetHighestSecLevel(DEVSLQueryParams *queryParams, uint32_t *levelInfo);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p172641072584"><a name="p172641072584"></a><a name="p172641072584"></a>Obtains the highest security level supported by the peer device. Currently, this API returns <strong id="b76381220139"><a name="b76381220139"></a><a name="b76381220139"></a>S3</strong>. You need to implement this function by yourself. For details about the data security levels, see the developer documentation.</p>
</td>
</tr>
<tr id="row18882199125920"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p6882169185915"><a name="p6882169185915"></a><a name="p6882169185915"></a>int32_t DEVSL_OnStart(int32_t maxDevNum);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p6882119135918"><a name="p6882119135918"></a><a name="p6882119135918"></a>Initializes the data classification module. You need to implement this function by yourself.</p>
</td>
</tr>
<tr id="row316118198591"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1216221920596"><a name="p1216221920596"></a><a name="p1216221920596"></a>void DEVSL_ToFinish(void);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p11162171911596"><a name="p11162171911596"></a><a name="p11162171911596"></a>Deinitializes the data classification module. You need to implement this function by yourself.</p>
</td>
</tr>
</tbody>
</table>

## Repositories Involved<a name="section155556361910"></a>

Security subsystem

**base/security/dataclassification**

