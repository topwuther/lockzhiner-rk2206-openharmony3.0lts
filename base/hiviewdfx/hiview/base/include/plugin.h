/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HIVIEW_BASE_PLUGIN_H
#define HIVIEW_BASE_PLUGIN_H

#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "defines.h"
#include "dynamic_module.h"
#include "event.h"
#include "event_loop.h"
#include "plugin_extra_info.h"

class HiEvent;
namespace OHOS {
namespace HiviewDFX {
class HiviewContext;
class DllExport Plugin : public EventHandler, public std::enable_shared_from_this<Plugin> {
public:
    Plugin() : handle_(DynamicModuleDefault), context_(nullptr){};
    virtual ~Plugin();
    // do not store the event in the callback
    // create a new one through copy constructor is preferred
    virtual bool OnEvent(std::shared_ptr<Event>& event) override;
    virtual bool CanProcessEvent(std::shared_ptr<Event> event) override;
    virtual bool CanProcessMoreEvents() override;
    bool OnEventProxy(std::shared_ptr<Event> event) override;
    virtual std::string GetHandlerInfo() override;

    // check whether the plugin should be loaded in current environment
    // the plugin will not be load if return false
    // called by plugin framework in main thread
    virtual bool ReadyToLoad()
    {
        return true;
    };

    // the plugin instance will be stored in platform after calling this function
    // and other loaded plugin can pass event to this plugin
    // called by plugin framework in main thread
    virtual void OnLoad(){};

    // release resource and clear pending workloads
    // after calling this function ,the reference of this plugin will be deleted
    // called by plugin framework in main thread
    virtual void OnUnload(){};

    // dump plugin info from dump command line
    virtual void Dump(int fd __UNUSED, const std::vector<std::string>& cmds __UNUSED) {};

    // create an event with specific type
    virtual std::shared_ptr<Event> GetEvent(Event::MessageType type)
    {
        auto event = std::make_shared<Event>(name_);
        event->messageType_ = type;
        return event;
    };

    // called by audit module
    virtual std::string GetPluginInfo();

    // reinsert the event into the workloop
    // delay in seconds
    void DelayProcessEvent(std::shared_ptr<Event> event, uint64_t delay);

    const std::string& GetName();
    const std::string& GetVersion();
    void SetName(const std::string& name);
    void SetVersion(const std::string& version);
    void BindWorkLoop(std::shared_ptr<EventLoop> loop);
    std::shared_ptr<EventLoop> GetWorkLoop();

    bool IsDynamic() const
    {
        return (handle_ != DynamicModuleDefault);
    };

    HiviewContext* GetHiviewContext()
    {
        return context_;
    };

    void SetHiviewContext(HiviewContext* context)
    {
        std::call_once(contextSetFlag_, [&]() { context_ = context; });
    };

    void SetHandle(DynamicModule handle)
    {
        handle_ = handle;
    };

protected:
    std::string name_;
    std::string version_;
    std::shared_ptr<EventLoop> workLoop_;

private:
    DynamicModule handle_;
    // the reference of the plugin platform
    // always available in framework callbacks
    HiviewContext* context_;
    std::once_flag contextSetFlag_;
};
class HiviewContext {
public:
    virtual ~HiviewContext(){};
    // post event to broadcast queue, the event will be terminated delivering if consumed by one plugin
    virtual void PostOrderedEvent(std::shared_ptr<Plugin> plugin __UNUSED, std::shared_ptr<Event> event __UNUSED) {};

    // post event to broadcast queue, the event will be delivered to all plugin that concern this event
    virtual void PostUnorderedEvent(std::shared_ptr<Plugin> plugin __UNUSED, std::shared_ptr<Event> event __UNUSED) {};

    // register listener to ordered broadcast queue
    // for the reason that no business use this interface, mark it as deprecated
    virtual void RegisterOrderedEventListener(std::weak_ptr<EventListener> listener __UNUSED) {};

    // register listener to unordered broadcast queue
    virtual void RegisterUnorderedEventListener(std::weak_ptr<EventListener> listener __UNUSED) {};

    // send a event to a specific plugin and wait the return of the OnEvent.
    virtual bool PostSyncEventToTarget(std::shared_ptr<Plugin> caller __UNUSED, const std::string& callee __UNUSED,
                                       std::shared_ptr<Event> event __UNUSED)
    {
        return true;
    }

    // send a event to a specific plugin and return at once
    virtual void PostAsyncEventToTarget(std::shared_ptr<Plugin> caller __UNUSED, const std::string& callee __UNUSED,
                                        std::shared_ptr<Event> event __UNUSED) {};

    // post event to distributed communicator plugin, to send to remote device
    virtual int32_t PostEventToRemote(std::shared_ptr<Plugin> caller __UNUSED, const std::string& deviceId __UNUSED,
        const std::string& targetPlugin __UNUSED, std::shared_ptr<Event> event __UNUSED)
    {
        return 0;
    }
    // request plugin platform to release plugin instance
    // do not recommend unload an plugin in pipeline scheme
    // platform will check the reference count if other module still holding the reference of this module
    virtual void RequestUnloadPlugin(std::shared_ptr<Plugin> caller __UNUSED) {};

    // publish plugin capacity and get remote capacity
    virtual void PublishPluginCapacity(PluginCapacityInfo &pluginCapacityInfo __UNUSED) {};
    virtual void GetRemoteByCapacity(const std::string& plugin __UNUSED, const std::string& capacity __UNUSED,
        std::list<std::string> &deviceIdList __UNUSED) {};

    // get the shared event loop reference
    virtual std::shared_ptr<EventLoop> GetSharedWorkLoop()
    {
        return nullptr;
    }

    // get predefined pipeline list
    virtual std::list<std::weak_ptr<Plugin>> GetPipelineSequenceByName(const std::string& name __UNUSED)
    {
        return std::list<std::weak_ptr<Plugin>>(0);
    }

    // check if all non-pending loaded plugins are loaded
    virtual bool IsReady()
    {
        return false;
    }

    enum class DirectoryType {
        CONFIG_DIRECTORY,
        CLOUD_UPDATE_DIRECTORY,
        WORK_DIRECTORY,
        PERSIST_DIR,
    };
    // get hiview available directory
    virtual std::string GetHiViewDirectory(DirectoryType type __UNUSED)
    {
        return "";
    }
    virtual std::string GetHiviewProperty(const std::string& key __UNUSED, const std::string& defaultValue)
    {
        return defaultValue;
    }

    virtual bool SetHiviewProperty(const std::string& key __UNUSED, const std::string& value __UNUSED,
        bool forceUpdate __UNUSED)
    {
        return true;
    }
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_PLUGIN_H
