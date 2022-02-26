# safwk组件<a name="ZH-CN_TOPIC_0000001115588558"></a>

-   [简介](#section11660541593)
-   [目录](#section161941989596)
-   [说明](#section1312121216216)
    -   [接口说明](#section1551164914237)
    -   [使用说明](#section129654513264)

-   [相关仓](#section1371113476307)

## 简介<a name="section11660541593"></a>

在分布式任务调度子系统中safwk组件定义OpenHarmony中SystemAbility的实现方法，并提供启动、注册等接口实现。

## 目录<a name="section161941989596"></a>

```
/foundation/distributedschedule
│── safwk               # 组件目录
│  ├── ohos.build       # 组件编译脚本
│  ├── interfaces       # 对外接口目录
│  ├── services         # 组件服务实现
```

## 说明<a name="section1312121216216"></a>

### 接口说明<a name="section1551164914237"></a>

<a name="table775715438253"></a>
<table><thead align="left"><tr id="row12757154342519"><th class="cellrowborder" valign="top" width="43.19%" id="mcps1.1.3.1.1"><p id="p1075794372512"><a name="p1075794372512"></a><a name="p1075794372512"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="56.81%" id="mcps1.1.3.1.2"><p id="p375844342518"><a name="p375844342518"></a><a name="p375844342518"></a>接口描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row1975804332517"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p5758174313255"><a name="p5758174313255"></a><a name="p5758174313255"></a>sptr&lt;IRemoteObject&gt; GetSystemAbility(int32_t systemAbilityId);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p14758743192519"><a name="p14758743192519"></a><a name="p14758743192519"></a>获取指定系统服务的RPC对象。</p>
</td>
</tr>
<tr id="row2758943102514"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p107581438250"><a name="p107581438250"></a><a name="p107581438250"></a>bool Publish(sptr&lt;IRemoteObject&gt; systemAbility);</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p8758743202512"><a name="p8758743202512"></a><a name="p8758743202512"></a>发布系统服务。</p>
</td>
</tr>
<tr id="row09311240175710"><td class="cellrowborder" valign="top" width="43.19%" headers="mcps1.1.3.1.1 "><p id="p159328405571"><a name="p159328405571"></a><a name="p159328405571"></a>virtual void DoStartSAProcess(const std::string&amp; profilePath) = 0;</p>
</td>
<td class="cellrowborder" valign="top" width="56.81%" headers="mcps1.1.3.1.2 "><p id="p493294018574"><a name="p493294018574"></a><a name="p493294018574"></a>根据SA profile配置启动System Ability。</p>
</td>
</tr>
</tbody>
</table>

### 使用说明<a name="section129654513264"></a>

SystemAbility实现一般采用XXX.rc + profile.xml + libXXX.z.so的方式由init进程执行对应的XXX.rc文件拉起相关SystemAbility进程；

**C++实现SystemAbility**

-   **1.1 定义IPC对外接口IXXX**

定义该服务对外提供的能力集合函数，统一继承HOSP提供IPC接口类IRemoteBroker；同时实现该IPC对外接口唯一标识符DECLARE\_INTERFACE\_DESCRIPTOR\(XXX\);该标识符用于IPC通信的校验等目的。

以分布式调度子系统的测试程序IListenAbility服务为例接口定义如下：

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

-   **1.2 定义客户端通信代码XXXProxy**

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

-   **1.3** **定义服务端通信代码XXXStub**

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

-   **1.4 SystemAbility的实现类**

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

-   **1.5 SystemAbility配置**

以c++实现的SA必须配置相关System Ability的profile配置文件才会完成SA的加载注册逻辑，否则没有编写profile配置的System Ability不会完成注册。配置方法如下：

在子系统的根目录新建一个以sa\_profile为名的文件夹；然后在此文件夹中新建两个文件：一个以serviceId为前缀的xml文件；另外一个为BUILD.gn文件

serviceid.xml：

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

BUILD.gn：

```
import("//build/ohos/sa_profile/sa_profile.gni")
ohos_sa_profile("xxx_sa_profile") {
    sources = [
        "serviceid.xml"
    ]
    subsystem_name = "distributedschedule"
}
```

>![](public_sys-resources/icon-note.gif) **说明：** 
>1.  进程名字即该SystemAbility要运行的进程空间，此字段是必填选项；
>2.  一个SystemAbility配置文件只能配置一个SystemAbility节点；配置多个会导致编译失败，切记
>3.  SystemAbility的name为对应的serviceId必须与代码中注册的serviceId保持一致；必配项
>4.  libpath为SystemAbility的加载路径，必配项；
>5.  run-on-create：true表示进程启动后即向samgr组件注册该SystemAbility；false表示按需启动，即在其他模块访问到该SystemAbility时启动；必配项；
>6.  distributed：true表示该SystemAbility为分布式SystemAbility，支持跨设备访问；false表示只有本地跨IPC访问；
>7.  def-permission：可不设置，含义为当distributed为true时（即支持分布式访问方式时）；设备A进程想与设备B进程的该SystemAbility进行跨RPC通信时必须申请的权限，才能正常进行通信
>8.  bootphase：可不设置；可以设置的值有三种：BootStartPhase、CoreStartPhase、OtherStartPhase（默认类型），三种优先级依次降低，当同一个进程中，会优先拉起注册配置BootStartPhase的SystemAbility，然后是配置了CoreStartPhase的SystemAbility，最后是OtherStartPhase；当高优先级的SystemAbility全部启动注册完毕才会启动下一级的SystemAbility的注册启动；
>9.  dump-level：表示systemdumper支持的level等级，默认配置1就OK；
>10. BUILD.gn中subsystem\_name为相应部件名称；sources表示当前子系统需要配置的SystemAbility列表，可支持配置多个SystemAbility

以上步骤完成后，全量编译代码后会在out路径向生成一个以进程名为前缀的xml文件listen\_test.xml；路径为：out\\phone-release\\system\\profile\\listen\_test.xml

-   **1.6 rc配置文件**

rc配置文件为linux提供的native进程拉起策略，为手机在开机启动阶段由init进程解析配置的rc文件进行拉起

```
service listen_test /system/bin/sa_main /system/profile/listen_test.xml
    class z_core
    user system
    group system shell
    seclabel u:r:xxxx:s0
```

>![](public_sys-resources/icon-note.gif) **说明：** 
>listen\_ability的实现可以参考:test/unittest/common/listen\_ability

## 相关仓<a name="section1371113476307"></a>

分布式任务调度子系统

distributedschedule\_dms\_fwk

**distributedschedule\_safwk**

distributedschedule\_samgr

distributedschedule\_safwk\_lite

hdistributedschedule\_samgr\_lite

distributedschedule\_dms\_fwk\_lite

