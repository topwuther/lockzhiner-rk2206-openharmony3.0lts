# Update Service<a name="EN-US_TOPIC_0000001102254666"></a>

-   [Introduction](#section184mcpsimp)
-   [Directory Structure](#section193mcpsimp)
-   [Description](#section208mcpsimp)
    -   [JS APIs](#section210mcpsimp)
    -   [Usage](#section253mcpsimp)

-   [Repositories Involved](#section366mcpsimp)

## Introduction<a name="section184mcpsimp"></a>

The update service is a system ability \(SA\) started by the init process of OHOS to implement an update.

The update service provides the following functions:

1. Searching for available update packages

2. Downloading update packages

3. Setting and obtaining the update policy

4. Triggering an update

## Directory Structure<a name="section193mcpsimp"></a>

```
base/update/updateservice  # Update service code
├── client                 # NAPI-based update client
├── engine                 # Update client engine
│   ├── etc                # rc configuration files for the update client engine
│   ├── include            # Header files for the update client engine
│   ├── sa_profile         # SA profiles
│   └── src                # Source code of the update client engine
├── interfaces             # Update client APIs
│   └── innerkits          # SA APIs 
├── kits                   # External APIs
│   └── js                 # JS APIs for the update app
└── tests                  # Test code
    └── unittest           # Unit test code for the update client
```

## Description<a name="section208mcpsimp"></a>

### JS APIs<a name="section210mcpsimp"></a>

<a name="table212mcpsimp"></a>
<table><tbody><tr id="row217mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p219mcpsimp"><a name="p219mcpsimp"></a><a name="p219mcpsimp"></a><strong id="b6143153974418"><a name="b6143153974418"></a><a name="b6143153974418"></a>API</strong></p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p222mcpsimp"><a name="p222mcpsimp"></a><a name="p222mcpsimp"></a><strong id="b156019475446"><a name="b156019475446"></a><a name="b156019475446"></a>Description</strong></p>
</td>
</tr>
<tr id="row223mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p16387178102716"><a name="p16387178102716"></a><a name="p16387178102716"></a>checkNewVersion</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p227mcpsimp"><a name="p227mcpsimp"></a><a name="p227mcpsimp"></a>Checks whether a new update package is available.</p>
</td>
</tr>
<tr id="row228mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p1884710150275"><a name="p1884710150275"></a><a name="p1884710150275"></a>download()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p232mcpsimp"><a name="p232mcpsimp"></a><a name="p232mcpsimp"></a>Downloads the update package. </p>
</td>
</tr>
<tr id="row233mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p7326722162717"><a name="p7326722162717"></a><a name="p7326722162717"></a>upgrade()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p237mcpsimp"><a name="p237mcpsimp"></a><a name="p237mcpsimp"></a>Writes the update command to the misc partition and runs the <strong id="b1069864618574"><a name="b1069864618574"></a><a name="b1069864618574"></a>reboot</strong> command to access the updater.</p>
</td>
</tr>
<tr id="row238mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p4981103002720"><a name="p4981103002720"></a><a name="p4981103002720"></a>getNewVersionInfo()</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p242mcpsimp"><a name="p242mcpsimp"></a><a name="p242mcpsimp"></a>Obtains the version information after a version update.</p>
</td>
</tr>
<tr id="row243mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p568117524271"><a name="p568117524271"></a><a name="p568117524271"></a>setUpdatePolicy</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p247mcpsimp"><a name="p247mcpsimp"></a><a name="p247mcpsimp"></a>Sets the update policy.</p>
</td>
</tr>
<tr id="row248mcpsimp"><td class="cellrowborder" valign="top" width="52%"><p id="p19534844192712"><a name="p19534844192712"></a><a name="p19534844192712"></a>getUpdatePolicy</p>
</td>
<td class="cellrowborder" valign="top" width="48%"><p id="p252mcpsimp"><a name="p252mcpsimp"></a><a name="p252mcpsimp"></a>Obtains the update policy.</p>
</td>
</tr>
</tbody>
</table>

### Usage<a name="section253mcpsimp"></a>

1. Import  **libupdateclient**.

```
import client from 'libupdateclient.z.so'
```

2. Obtain the  **Updater**  object.

```
let updater = client.getUpdater('OTA');
```

3. Obtain the new version information.

```
updater.getNewVersionInfo(info => {
	info "New version information"
});
```

4. Checks for a new version.

```
updater.checkNewVersion(info => {
	info "New version information"
});
```

5. Download the new version and monitor the download process.

```
updater.download();
updater.on("downloadProgress", progress => {
	progress "Download progress information"
});
```

6. Start the update.

```
updater.upgrade();
updater.on("upgradeProgress", progress => {
	progress "Update progress information"
});
```

7. Set the update policy.

```
updater.setUpdatePolicy(result => {
	result "Update policy setting result"
});
```

8. Check the update policy.

```
updater.getUpdatePolicy(policy => {
	policy "Update policy"
});
```

## Repositories Involved<a name="section366mcpsimp"></a>

Update subsystem

update\_app

**update\_updateservice**

update\_updater

