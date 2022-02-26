# safwk<a name="EN-US_TOPIC_0000001115588558"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Usage](#section1312121216216)
    -   [Available APIs](#section1551164914237)
    -   [Usage Guidelines](#section129654513264)

-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

The  **safwk**  module of the Distributed Scheduler subsystem defines how to implement a  **SystemAbility**  in OpenHarmony and provides APIs for system ability startup and registration.

## Directory Structure<a name="section161941989596"></a>

```
/foundation/distributedschedule
│── safwk                # Directory for the safwk module
│  ├── ohos.build       # Compilation script for safwk
│  ├── interfaces       # APIs exposed externally
│  ├── services         # Service implementation
```

## Usage<a name="section1312121216216"></a>

### Available APIs<a name="section1551164914237"></a>

<a name="table775715438253"></a>
<table><thead align="left"><tr id="row12757154342519"><th class="cellrowborder" valign="top" width="43.19%" id="mcps1.1.3.1.1"><p id="p1075794372512"><a name="p1075794372512"></a><a name="p1075794372512"></a>API</p>
</th>
<th class="cellrowborder" valign="top" width="56.81%" id="mcps1.1.3.1.2"><p id="p375844342518"><a name="p375844342518"></a><a name="p375844342518"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row1975804332517"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p5758174313255"><a name="p5758174313255"></a><a name="p5758174313255"></a>sptr&lt;IRemoteObject&gt; GetSystemAbility(int32_t systemAbilityId);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p14758743192519"><a name="p14758743192519"></a><a name="p14758743192519"></a>Obtains the remote procedure call (RPC) object of a specified system ability.</p>
</td>
</tr>
<tr id="row2758943102514"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p107581438250"><a name="p107581438250"></a><a name="p107581438250"></a>bool Publish(sptr&lt;IRemoteObject&gt; systemAbility);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p8758743202512"><a name="p8758743202512"></a><a name="p8758743202512"></a>Publishes a specified system ability.</p>
</td>
</tr>
<tr id="row09311240175710"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p159328405571"><a name="p159328405571"></a><a name="p159328405571"></a>virtual void DoStartSAProcess(const std::string&amp; profilePath) = 0;</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p493294018574"><a name="p493294018574"></a><a name="p493294018574"></a>Enables a system ability based on the SA profile information.</p>
</td>
</tr>
<tr id="row159634125718"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p10596134105710"><a name="p10596134105710"></a><a name="p10596134105710"></a>void OnConnectedSystemAbility(const sptr&lt;IRemoteObject&gt;&amp; connectionCallback);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p105961241125713"><a name="p105961241125713"></a><a name="p105961241125713"></a>Called when a system ability is connected.</p>
</td>
</tr>
<tr id="row611715428577"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p10118242155716"><a name="p10118242155716"></a><a name="p10118242155716"></a>void OnDisConnectedSystemAbility(int32_t systemAbilityId);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p81189429578"><a name="p81189429578"></a><a name="p81189429578"></a>Called when a system ability is disconnected.</p>
</td>
</tr>
</tbody>
</table>

### Usage Guidelines<a name="section129654513264"></a>

System abilities can be implemented in both C++ and Java languages. In C++, you must define the  _XXX_**.rc**,  **profile.xml**, and  **lib**_XXX_**.z.so**  files to declare the system ability, and the init process executes the specified  _XXX_**.rc**  file to start the process of the particular system ability. Similar to the implementation in C++, the system ability process is started by  [the foundationserver module](en-us_topic_0000001078878484.md)  in Java.

**Implementing a System Ability in C++**

-   **Define the  _IXXX_  class for IPC.**

This  _IXXX_  class is used to define the functions for the system ability to provide specific capabilities. To define this class, implement the  **IRemoteBroker**  interface provided by OpenHarmony for inter-process communication \(IPC\). In addition, implement the  **DECLARE\_INTERFACE\_DESCRIPTOR\(_XXX_\)**  that uniquely identifies this class. The identifier is used for purposes such as IPC communication verification.

The following example shows how to define the  **IListenAbility**  class for testing in the Distributed Scheduler subsystem:

```
namespace OHOS {
class IListenAbility : public IRemoteBroker {
public:
    virtual int AddVolume(int volume) = 0;

public:
    enum {
        ADD_VOLUME = 1,
    };
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.test.IListenAbility");
};
}
```

-   **Define the  _XXX_Proxy class for client communication.**

```
namespace OHOS {
class ListenAbilityProxy : public IRemoteProxy<IListenAbility> {
public:
    int AddVolume(int volume);

    explicit ListenAbilityProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IListenAbility>(impl)
    {
    }

private:
    static inline BrokerDelegator<ListenAbilityProxy> delegator_;
};
} // namespace OHOS
```

-   **Define the  _XXX_Stub class for server communication.**

```
namespace OHOS {
int32_t ListenAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case ADD_VOLUME: {
            return reply.WriteInt32(AddVolume(data.ReadInt32()));
        }

        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
}
```

-   **Implement a system ability.**

```
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD001800, "SA_TST"};
}

REGISTER_SYSTEM_ABILITY_BY_ID(ListenAbility, DISTRIBUTED_SCHED_TEST_LISTEN_ID, true);

ListenAbility::ListenAbility(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    HiLog::Info(LABEL, ":%s called", __func__);
    HiLog::Info(LABEL, "ListenAbility()");
}

ListenAbility::~ListenAbility()
{
    HiLog::Info(LABEL, "~ListenAbility()");
}

int ListenAbility::AddVolume(int volume)
{
    pid_t current = getpid();
    HiLog::Info(LABEL, "ListenAbility::AddVolume volume = %d, pid = %d.", volume, current);
    return (volume + 1);
}

void ListenAbility::OnDump()
{
}

void ListenAbility::OnStart()
{
    HiLog::Info(LABEL, "ListenAbility::OnStart()");
    HiLog::Info(LABEL, "ListenAbility:%s called:-----Publish------", __func__);
    bool res = Publish(this);
    if (res) {
        HiLog::Error(LABEL, "ListenAbility: res == false");
    }
    HiLog::Info(LABEL, "ListenAbility:%s called:AddAbilityListener_OS_TST----beg-----", __func__);
    AddSystemAbilityListener(DISTRIBUTED_SCHED_TEST_OS_ID);
    HiLog::Info(LABEL, "ListenAbility:%s called:AddAbilityListener_OS_TST----end-----", __func__);

    HiLog::Info(LABEL, "ListenAbility:%s called:StopAbility_OS_TST----beg-----", __func__);
    StopAbility(DISTRIBUTED_SCHED_TEST_OS_ID);
    HiLog::Info(LABEL, "ListenAbility:%s called:StopAbility_OS_TST----end-----", __func__);
    return;
}

void ListenAbility::OnStop()
{
}
```

-   **Configure the system ability.**

If the system ability is implemented in C++, you must define a profile for it so that it can be loaded and registered. The configuration procedure is as follows:

Create a folder named  **sa\_profile**  in the root directory of the subsystem. Then, create two files in this folder, including an XML file prefixed with the service ID of the system ability and a  **BUILD.gn**  file.

Sample  _serviceid_**.xml**  file:

```
<?xml version="1.0" encoding="UTF-8"?>
<info>
    <process>listen_test</process>
    <systemability>
    <name>serviceid</name>
    <libpath>/system/lib64/liblistentest.z.so</libpath>
    <run-on-create>true</run-on-create>
    <distributed>false</distributed>
    <dump-level>1</dump-level>
</systemability>
</info>
```

Sample  **BUILD.gn**  file:

```
import("//build/ohos/sa_profile/sa_profile.gni")
ohos_sa_profile("xxx_sa_profile") {
    sources = [
        "serviceid.xml"
    ]
    subsystem_name = "distributedschedule"
}
```

>![](public_sys-resources/icon-note.gif) **NOTE:** 
>1.  Set the  **process**  tag to the name of the process where the system ability will run. This tag is mandatory.
>2.  Add only one  **systemability**  node in the profile of a system ability. If multiple  **systemability**  nodes are added, the building fails.
>3.  Set the  **name**  tag to the service ID registered in the code for the system ability. This tag is mandatory.
>4.  Set the  **libpath**  tag to the path for loading the system ability. This tag is mandatory.
>5.  Set the  **run-on-create**  tag to  **true**  if you want to register this system ability with the  **samgr**  module immediately after the process is started. Set this tag to  **false**  if you want the system ability to start only when other modules access the system ability. This tag is mandatory.
>6.  Set the  **distributed**  tag to  **true**  if this system ability allows cross-device access; set it to  **false**  if it allows IPC only on the local device.
>7.  Set the  **def-permission**  tag to define the permissions, if any, required for the process on another device to access this system ability during cross-device RPC communication when  **distributed**  is set to  **true**. This tag is optional.
>8.  Set the  **bootphase**  tag to define the startup priority of the system ability, which \(from high to low\) can be  **BootStartPhase**,  **CoreStartPhase**, or  **OtherStartPhase**  \(default value\). In the same process, the system ability of the  **BootStartPhase**  priority will be started first, then that of the  **CoreStartPhase**  priority, and finally the  **OtherStartPhase**  priority. System abilities of a lower priority can be started and registered only after those of a high priority have all been started and registered. This tag is optional.
>9.  Set  **dump-level**  to  **1**, which indicates that the level is supported by the system dumper.
>10. In the  **BUILD.gn**  file, set  **subsystem\_name**  to the name of the corresponding subsystem, and add the list of system abilities that need to be configured for the specified subsystem in  **sources**. Multiple system abilities can be configured.

After the preceding files are configured and full code building is complete, a  **listen\_test.xml**  prefixed with the process name will be generated in the  **out**  directory. The path is  **out\\phone-release\\system\\profile\\listen\_test.xml**  in this example.

-   **Configure the  _XXX_.rc file.**

The  **_XXX_.rc**  configuration file defines the native process startup policy provided by Linux. The  **init**  process parses the configured  **_XXX_.rc**  file during device startup.

```
service listen_test /system/bin/sa_main /system/profile/listen_test.xml
    class z_core
    user system
    group system shell
    seclabel u:r:xxxx:s0
```

>![](public_sys-resources/icon-note.gif) **NOTE:** 
>For details about the implementation of  **listen\_ability**, see the code in  **test/unittest/common/listen\_ability**.

## Repositories Involved<a name="section1371113476307"></a>

[Distributed Scheduler subsystem](en-us_topic_0000001115719369.md)

[dmsadapter](en-us_topic_0000001124134145.md)

[dmsfwk](en-us_topic_0000001078718754.md)

[foundationserver](en-us_topic_0000001078878484.md)

**[safwk](safwk.md)**

[samgr](en-us_topic_0000001124076649.md)

