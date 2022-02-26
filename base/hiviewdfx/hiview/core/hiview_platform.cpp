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
#include "hiview_platform.h"
#ifndef _WIN32
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <cinttypes>
#include <csignal>
#include <fstream>
#include <functional>

#include "audit.h"
#include "common_utils.h"
#include "defines.h"
#include "dynamic_module.h"
#include "engine_event_dispatcher.h"
#include "file_util.h"
#include "hiview_global.h"
#include "logger.h"
#include "parameter_ex.h"
#include "plugin_config.h"
#include "plugin_factory.h"
#include "string_util.h"
#include "time_util.h"
#include "file_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t AID_ROOT = 0;
constexpr uint32_t AID_SYSTEM = 1000;
constexpr char ENGINE_EVENT_DISPATCHER[] = "EngineEventDispatcher";
static const char VERSION[] = "xx.x.x.xxx";
static const char SEPARATOR_VERSION[] = " ";
static const char RECORDER_VERSION[] = "01.00";
static const char PLUGIN_CONFIG_NAME[] = "plugin_config";
static const char HIVIEW_PID_FILE_NAME[] = "hiview.pid";
static const char DEFAULT_CONFIG_DIR[] = "/system/etc/hiview/";
static const char CLOUD_UPDATE_CONFIG_DIR[] = "/data/system/hiview/";
static const char DEFAULT_WORK_DIR[] = "/data/log/LogService/";
static const char DEFAULT_COMMERCIAL_WORK_DIR[] = "/log/LogService/";
static const char DEFAULT_PERSIST_DIR[] = "/log/hiview/";
}
DEFINE_LOG_TAG("HiView-HiviewPlatform");
HiviewPlatform::HiviewPlatform()
    : isReady_(false),
      defaultConfigDir_(DEFAULT_CONFIG_DIR),
      cloudUpdateConfigDir_(CLOUD_UPDATE_CONFIG_DIR),
      defaultWorkDir_(DEFAULT_WORK_DIR),
      defaultPersistDir_(DEFAULT_PERSIST_DIR),
      defaultConfigName_(PLUGIN_CONFIG_NAME),
      orderQueue_(nullptr),
      unorderQueue_(nullptr),
      sharedWorkLoop_(nullptr)
{
}

HiviewPlatform::~HiviewPlatform()
{
    if (orderQueue_ != nullptr) {
        orderQueue_->Stop();
    }

    if (unorderQueue_ != nullptr) {
        unorderQueue_->Stop();
    }

    if (sharedWorkLoop_ != nullptr) {
        sharedWorkLoop_->StopLoop();
    }
}

bool HiviewPlatform::InitEnvironment(const std::string& defaultDir, const std::string& cloudUpdateDir,
                                     const std::string& workDir, const std::string& persistDir)
{
    // force create hiview working directories
    ValidateAndCreateDirectories(defaultDir, cloudUpdateDir, workDir, persistDir);

    // update beta config
    UpdateBetaConfigIfNeed();

    // check whether hiview is already started
    ExitHiviewIfNeed();

    std::string cfgPath = GetPluginConfigPath();
    PluginConfig config(cfgPath);
    if (!config.StartParse()) {
        HIVIEW_LOGE("Fail to parse plugin config. exit!");
        return false;
    }
    StartPlatformDispatchQueue();

    // init global context helper, remove in the future
    HiviewGlobal::CreateInstance(static_cast<HiviewContext&>(*this));
    LoadBusinessPlugin(config);

    isReady_ = true;
    NotifyPluginReady();
    return true;
}

void HiviewPlatform::UpdateBetaConfigIfNeed()
{
    int32_t userType = Parameter::GetInteger(KEY_HIVIEW_USER_TYPE, Parameter::UserType::UNKNOWN_TYPE);
    if (userType == Parameter::UserType::BETA) {
        // init audit status
        Audit::GetInstance().Init(true);
    }

    if (userType == Parameter::UserType::COMMERCIAL || userType == Parameter::UserType::OVERSEAS_COMMERCIAL) {
        if (defaultWorkDir_ == std::string(DEFAULT_WORK_DIR)) {
            defaultWorkDir_ = DEFAULT_COMMERCIAL_WORK_DIR;
            FileUtil::CreateDirWithDefaultPerm(defaultWorkDir_, AID_SYSTEM, AID_SYSTEM);
        }
    }
}

void HiviewPlatform::LoadBusinessPlugin(const PluginConfig& config)
{
    // start to load plugin
    // 1. create plugin
    auto const& pluginInfoList = config.GetPluginInfoList();
    for (auto const& pluginInfo : pluginInfoList) {
        HIVIEW_LOGI("Start to create plugin %{public}s delay:%{public}d", pluginInfo.name.c_str(),
                    pluginInfo.loadDelay);
        if (pluginInfo.loadDelay > 0) {
            auto task = std::bind(&HiviewPlatform::ScheduleCreateAndInitPlugin, this, pluginInfo);
            sharedWorkLoop_->AddTimerEvent(nullptr, nullptr, task, pluginInfo.loadDelay, false);
        } else {
            CreatePlugin(pluginInfo);
        }
    }
    // 2. create pipelines
    auto const& pipelineInfoList = config.GetPipelineInfoList();
    for (auto const& pipelineInfo : pipelineInfoList) {
        HIVIEW_LOGI("Start to create pipeline %{public}s", pipelineInfo.name.c_str());
        CreatePipeline(pipelineInfo);
    }

    // 3. bind pipeline and call onload of event source
    for (auto const& pluginInfo : pluginInfoList) {
        HIVIEW_LOGI("Start to Load plugin %{public}s", pluginInfo.name.c_str());
        InitPlugin(config, pluginInfo);
    }

    CleanupUnusedResources();
}

void HiviewPlatform::ProcessArgsRequest(int argc, char* argv[])
{
#ifndef _WIN32
    umask(0002); // 0002 is block other write permissions, -------w-
    signal(SIGPIPE, SIG_IGN);
    int ch = -1;
    while ((ch = getopt(argc, argv, "v")) != -1) {
        if (ch == 'v') {
            HIVIEW_LOGI("hiview version: %s%s%s", VERSION, SEPARATOR_VERSION,
                            RECORDER_VERSION);
            printf("hiview version: %s%s%s\n", VERSION, SEPARATOR_VERSION, RECORDER_VERSION);
            exit(1);
        }
    }
#endif // !_WIN32
}

DynamicModule HiviewPlatform::LoadDynamicPluginIfNeed(const PluginConfig::PluginInfo& pluginInfo) const
{
    if (!pluginInfo.isStatic) {
        // if the plugin Class is AbcPlugin, the so name should be libabcplugin.z.so
        std::string dynamicPluginName = GetDynamicLibName(pluginInfo.name, true);
        auto handle = LoadModule(dynamicPluginName.c_str());
        if (handle == nullptr) {
            // retry load library
            dynamicPluginName = GetDynamicLibName(pluginInfo.name, false);
            handle = LoadModule(dynamicPluginName.c_str());
        }
        return handle;
    }
    return DynamicModuleDefault;
}

std::string HiviewPlatform::GetDynamicLibName(const std::string& name, bool hasOhosSuffix) const
{
#ifdef __HIVIEW_OHOS__
    std::string tmp = "lib" + name;
    if (hasOhosSuffix) {
        tmp.append(".z.so");
    } else {
        tmp.append(".so");
    }

    for (unsigned i = 0; i < tmp.length(); i++) {
        tmp[i] = std::tolower(tmp[i]);
    }
    return tmp;
#elif defined(_WIN32)
    std::string dynamicLibName = "";
    dynamicLibName.append(name);
    dynamicLibName.append(".dll");
    return dynamicLibName;
#else
    // dynamic plugins feature is only enabled in double framework version
    (void)hasOhosSuffix;
    HIVIEW_LOGI("could not load dynamic lib %s, not supported yet.", name.c_str());
    return "";
#endif
}

void HiviewPlatform::CreatePlugin(const PluginConfig::PluginInfo& pluginInfo)
{
    if (pluginInfo.name.empty()) {
        return;
    }
    // the dynamic plugin will register it's constructor to factory automatically after opening the binary
    // if we get null in factory, it means something must go wrong.
    DynamicModule handle = LoadDynamicPluginIfNeed(pluginInfo);
    std::shared_ptr<Plugin> plugin = PluginFactory::GetPlugin(pluginInfo.name);
    if (plugin == nullptr) {
        if (handle != nullptr) {
            UnloadModule(handle);
        }
        return;
    }

    // Initialize plugin parameters
    plugin->SetName(pluginInfo.name);
    plugin->SetHandle(handle);
    plugin->SetHiviewContext(this);

    // call preload, check whether we should release at once
    if (!plugin->ReadyToLoad()) {
        // if the plugin is a dynamic loaded library, the handle will be closed when calling the destructor
        return;
    }
    // hold the global reference of the plugin
    pluginMap_[pluginInfo.name] = std::move(plugin);
}

void HiviewPlatform::CreatePipeline(const PluginConfig::PipelineInfo& pipelineInfo)
{
    std::list<std::weak_ptr<Plugin>> pluginList;
    for (auto& pluginName : pipelineInfo.pluginNameList) {
        auto& plugin = pluginMap_[pluginName];
        if (plugin == nullptr) {
            HIVIEW_LOGI("could not find plugin(%{public}s), skip adding to pipeline(%{public}s).",
                pluginName.c_str(), pipelineInfo.name.c_str());
            continue;
        }
        pluginList.push_back(plugin);
    }
    std::shared_ptr<Pipeline> pipeline = std::make_shared<Pipeline>(pipelineInfo.name, pluginList);
    pipelines_[pipelineInfo.name] = std::move(pipeline);
}

void HiviewPlatform::InitPlugin(const PluginConfig& config __UNUSED, const PluginConfig::PluginInfo& pluginInfo)
{
    auto& plugin = pluginMap_[pluginInfo.name];
    if (plugin == nullptr) {
        return;
    }

    if (pluginInfo.workHandlerType == "thread") {
        auto workLoop = GetAvaliableWorkLoop(pluginInfo.workHandlerName);
        plugin->BindWorkLoop(workLoop);
    }

    auto begin = TimeUtil::GenerateTimestamp();
    plugin->OnLoad();
    if (pluginInfo.isEventSource) {
        auto sharedSource = std::static_pointer_cast<EventSource>(plugin);
        if (sharedSource == nullptr) {
            HIVIEW_LOGE("Fail to cast plugin to event source!");
            return;
        }
        for (auto& pipelineName : pluginInfo.pipelineNameList) {
            sharedSource->AddPipeline(pipelines_[pipelineName]);
        }
        StartEventSource(sharedSource);
    }
    auto end = TimeUtil::GenerateTimestamp();
    HIVIEW_LOGI("Plugin %{public}s loadtime:%{public}" PRIu64 ".", pluginInfo.name.c_str(), end - begin);
}

void HiviewPlatform::NotifyPluginReady()
{
    auto event = std::make_shared<Event>("platform");
    event->messageType_ = Event::MessageType::PLUGIN_MAINTENANCE;
    event->eventId_ = Event::EventId::PLUGIN_LOADED;
    PostUnorderedEvent(nullptr, event);
}

void HiviewPlatform::StartEventSource(std::shared_ptr<EventSource> source)
{
    auto workLoop = source->GetWorkLoop();
    auto name = source->GetName();
    if (workLoop == nullptr) {
        HIVIEW_LOGW("No work loop available, start event source[%s] in current thead!", name.c_str());
        source->StartEventSource();
    } else {
        HIVIEW_LOGI("Start event source[%s] in thead[%s].", name.c_str(), workLoop->GetName().c_str());
        auto task = std::bind(&EventSource::StartEventSource, source.get());
        workLoop->AddEvent(nullptr, nullptr, task);
    }
    HIVIEW_LOGI("Start event source[%s] in current thead done.", name.c_str());
}

// only call from main thread
std::shared_ptr<EventLoop> HiviewPlatform::GetAvaliableWorkLoop(const std::string& name)
{
    auto it = privateWorkLoopMap_.find(name);
    if (it != privateWorkLoopMap_.end()) {
        return it->second;
    }

    auto privateLoop = std::make_shared<EventLoop>(name);
    if (privateLoop != nullptr) {
        privateLoop->StartLoop();
        privateWorkLoopMap_.insert(std::make_pair(name, privateLoop));
    }
    return privateLoop;
}

void HiviewPlatform::CleanupUnusedResources()
{
    auto iter = pluginMap_.begin();
    while (iter != pluginMap_.end()) {
        if (iter->second == nullptr) {
            iter = pluginMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void HiviewPlatform::ScheduleCreateAndInitPlugin(const PluginConfig::PluginInfo& pluginInfo)
{
    // only support thread type
    CreatePlugin(pluginInfo);
    auto& plugin = pluginMap_[pluginInfo.name];
    if (plugin == nullptr) {
        return;
    }

    if (pluginInfo.workHandlerType == "thread") {
        auto workLoop = GetAvaliableWorkLoop(pluginInfo.workHandlerName);
        plugin->BindWorkLoop(workLoop);
    }
    plugin->OnLoad();
}

void HiviewPlatform::StartLoop()
{
    // empty implementation
}

void HiviewPlatform::StartPlatformDispatchQueue()
{
    if (orderQueue_ == nullptr) {
        orderQueue_ = std::make_unique<EventDispatchQueue>("plat_order", Event::ManageType::ORDERED);
        orderQueue_->Start();
    }

    if (unorderQueue_ == nullptr) {
        unorderQueue_ = std::make_unique<EventDispatchQueue>("plat_unorder", Event::ManageType::UNORDERED);
        unorderQueue_->Start();
    }

    if (sharedWorkLoop_ == nullptr) {
        sharedWorkLoop_ = std::make_shared<EventLoop>("plat_shared");
        sharedWorkLoop_->StartLoop();
    }
}

std::list<std::weak_ptr<Plugin>> HiviewPlatform::GetPipelineSequenceByName(const std::string& name)
{
    if (!isReady_) {
        return std::list<std::weak_ptr<Plugin>>();
    }

    auto& pipeline = pipelines_[name];
    if (pipeline != nullptr) {
        return pipeline->GetProcessSequence();
    }
    return std::list<std::weak_ptr<Plugin>>(0);
}

void HiviewPlatform::PostOrderedEvent(std::shared_ptr<Plugin> plugin, std::shared_ptr<Event> event)
{
    if (!isReady_) {
        return;
    }

    if (plugin != nullptr && orderQueue_ != nullptr && event != nullptr) {
        event->processType_ = Event::ManageType::ORDERED;
        orderQueue_->Enqueue(event);
    }
}

void HiviewPlatform::PostUnorderedEvent(std::shared_ptr<Plugin> plugin, std::shared_ptr<Event> event)
{
    if (!isReady_) {
        return;
    }

    if (plugin == nullptr) {
        HIVIEW_LOGI("maybe platform send event");
    }

    if (unorderQueue_ != nullptr && event != nullptr) {
        event->processType_ = Event::ManageType::UNORDERED;
        unorderQueue_->Enqueue(event);
    }
}

void HiviewPlatform::RegisterOrderedEventListener(std::weak_ptr<EventListener> listener)
{
    if (orderQueue_ != nullptr) {
        orderQueue_->RegisterListener(listener);
    }
}

void HiviewPlatform::RegisterUnorderedEventListener(std::weak_ptr<EventListener> listener)
{
    if (unorderQueue_ != nullptr) {
        unorderQueue_->RegisterListener(listener);
    }

    // dispatch engine event
    std::map<std::string, std::shared_ptr<Plugin>>::iterator it = pluginMap_.find(ENGINE_EVENT_DISPATCHER);
    if ((it != pluginMap_.end()) && (it->second != nullptr)) {
        auto dispatcher = std::static_pointer_cast<EngineEventDispatcher>(it->second);
        dispatcher->RegisterListener(listener);
    }
}

bool HiviewPlatform::PostSyncEventToTarget(std::shared_ptr<Plugin> caller, const std::string& calleeName,
                                           std::shared_ptr<Event> event)
{
    if (!isReady_) {
        return false;
    }

    auto it = pluginMap_.find(calleeName);
    if (it == pluginMap_.end()) {
        return false;
    }

    auto callee = it->second;
    if (callee == nullptr) {
        return false;
    }

    auto workLoop = callee->GetWorkLoop();
    std::future<bool> ret;
    if (workLoop == nullptr) {
        ret = sharedWorkLoop_->AddEventForResult(callee, event);
    } else {
        ret = workLoop->AddEventForResult(callee, event);
    }
    return ret.get();
}

void HiviewPlatform::PostAsyncEventToTarget(std::shared_ptr<Plugin> caller, const std::string& calleeName,
                                            std::shared_ptr<Event> event)
{
    if (!isReady_) {
        return;
    }

    auto it = pluginMap_.find(calleeName);
    if (it == pluginMap_.end()) {
        return;
    }

    auto callee = it->second;
    if (callee == nullptr) {
        return;
    }

    auto workLoop = callee->GetWorkLoop();
    if (workLoop == nullptr) {
        sharedWorkLoop_->AddEvent(callee, event);
    } else {
        workLoop->AddEvent(callee, event);
    }
}

std::shared_ptr<EventLoop> HiviewPlatform::GetSharedWorkLoop()
{
    return sharedWorkLoop_;
}

void HiviewPlatform::RequestUnloadPlugin(std::shared_ptr<Plugin> caller)
{
    if (caller == nullptr) {
        return;
    }

    std::string name = caller->GetName();
    auto task = std::bind(&HiviewPlatform::UnloadPlugin, this, name);
    // delay 1s to unload target plugin
    const int unloadDelay = 1;
    sharedWorkLoop_->AddTimerEvent(nullptr, nullptr, task, unloadDelay, false);
}

void HiviewPlatform::UnloadPlugin(const std::string& name)
{
    auto it = pluginMap_.find(name);
    if (it == pluginMap_.end()) {
        return;
    }

    auto target = it->second;
    if (target == nullptr) {
        return;
    }

    auto count = target.use_count();
    // two counts for 1.current ref 2.map holder ref
    if (count > 2) {
        HIVIEW_LOGW("Plugin %s has more refs(%ld), may caused by unfinished task. unload delay.", name.c_str(),
                    count);
        RequestUnloadPlugin(target);
        return;
    }

    pluginMap_.erase(name);
    target->OnUnload();
    auto looper = target->GetWorkLoop();
    if (looper == nullptr) {
        return;
    }

    auto looperName = looper->GetName();
    // three counts for 1.current ref 2.plugin ref 3.map holder ref
    if (looper.use_count() <= 3) {
        HIVIEW_LOGI("%s has refs(%ld).", looperName.c_str(), looper.use_count());
        looper->StopLoop();
        privateWorkLoopMap_.erase(looperName);
        HIVIEW_LOGI("Stop %s done.", looperName.c_str());
    }

    if (target->IsDynamic()) {
        // remove static register before closing the dynamic library handle
        PluginFactory::UnregisterPlugin(target->GetName());
    }
}

std::string HiviewPlatform::GetHiViewDirectory(HiviewContext::DirectoryType type)
{
    switch (type) {
        case HiviewContext::DirectoryType::CONFIG_DIRECTORY:
            return defaultConfigDir_;
        case HiviewContext::DirectoryType::CLOUD_UPDATE_DIRECTORY:
            return cloudUpdateConfigDir_;
        case HiviewContext::DirectoryType::WORK_DIRECTORY:
            return defaultWorkDir_;
        case HiviewContext::DirectoryType::PERSIST_DIR:
            return defaultPersistDir_;
        default:
            break;
    }
    return "";
}

void HiviewPlatform::ValidateAndCreateDirectories(const std::string& localPath, const std::string& cloudUpdatePath,
                                                  const std::string& workPath, const std::string& persistPath)
{
    if (!localPath.empty()) {
        defaultConfigDir_ = localPath;
        FileUtil::CreateDirWithDefaultPerm(defaultConfigDir_, AID_ROOT, AID_SYSTEM);
    }

    if (!cloudUpdatePath.empty()) {
        cloudUpdateConfigDir_ = cloudUpdatePath;
        FileUtil::CreateDirWithDefaultPerm(cloudUpdateConfigDir_, AID_ROOT, AID_SYSTEM);
    }

    if (!workPath.empty()) {
        defaultWorkDir_ = workPath;
        FileUtil::CreateDirWithDefaultPerm(defaultWorkDir_, AID_ROOT, AID_SYSTEM);
    }

    if (!persistPath.empty()) {
        defaultPersistDir_ = persistPath;
        FileUtil::CreateDirWithDefaultPerm(defaultPersistDir_, AID_ROOT, AID_SYSTEM);
    }
}

#ifndef _WIN32
void HiviewPlatform::ExitHiviewIfNeed()
{
    int selfPid = getpid();
    std::string selfProcName = CommonUtils::GetProcNameByPid(selfPid);
    if (selfProcName != "hiview") {
        return;
    }

    std::string pidFile = defaultWorkDir_ + "/" + std::string(HIVIEW_PID_FILE_NAME);
    if (!FileUtil::FileExists(pidFile)) {
        return;
    }

    std::string content;
    FileUtil::LoadStringFromFile(pidFile, content);
    int32_t pid = -1;
    if (!StringUtil::StrToInt(content, pid)) {
        return;
    }

    std::string procName = CommonUtils::GetProcNameByPid(pid);
    if (procName == "hiview") {
        printf("Hiview is already started, exit! \n");
        exit(1);
    }
    FileUtil::SaveStringToFile(pidFile, std::to_string(selfPid));
}
#else
void HiviewPlatform::ExitHiviewIfNeed()
{
}
#endif

std::string HiviewPlatform::GetPluginConfigPath()
{
    return defaultConfigDir_ + defaultConfigName_;
}

void HiviewPlatform::PublishPluginCapacity(PluginCapacityInfo& pluginCapacityInfo)
{
    auto it = pluginMap_.find(DISTRIBUTED_COMMUNICATOR_PLUGIN);
    if (it == pluginMap_.end()) {
        return;
    }
    auto callee = it->second;
    if (callee == nullptr) {
        return;
    }
    auto event = std::make_shared<CapacityPublishEvent>(pluginCapacityInfo.name_, pluginCapacityInfo);
    event->messageType_ = Event::MessageType::CROSS_PLATFORM;
    event->eventId_ = CapacityEventId::CAP_PUBLISH;
    auto eventParent = std::dynamic_pointer_cast<Event>(event);
    callee->OnEvent(eventParent);
}

void HiviewPlatform::GetRemoteByCapacity(const std::string& plugin, const std::string& capacity,
                                         std::list<std::string>& deviceIdList)
{
    auto it = pluginMap_.find(DISTRIBUTED_COMMUNICATOR_PLUGIN);
    if (it == pluginMap_.end()) {
        return;
    }
    auto callee = it->second;
    if (callee == nullptr) {
        return;
    }
    auto event = std::make_shared<CapacityObtainEvent>(plugin, capacity);
    event->messageType_ = Event::MessageType::CROSS_PLATFORM;
    event->eventId_ = CapacityEventId::CAP_OBTAIN;
    auto eventParent = std::dynamic_pointer_cast<Event>(event);
    if (!callee->OnEvent(eventParent)) {
        return;
    }
    deviceIdList = event->deviceList_;
}

int32_t HiviewPlatform::PostEventToRemote(std::shared_ptr<Plugin> caller, const std::string& deviceId,
                                          const std::string& targetPlugin, std::shared_ptr<Event> event)
{
    if (event == nullptr) {
        return -1;
    }
    event->SetValue("targetPlugin", targetPlugin);
    event->SetValue("deviceId", deviceId);
    if (PostSyncEventToTarget(caller, DISTRIBUTED_COMMUNICATOR_PLUGIN, event)) {
        return event->GetIntValue("result");
    }
    return -1;
}

std::shared_ptr<Plugin> HiviewPlatform::GetPluginByName(const std::string& name)
{
    auto it = pluginMap_.find(name);
    if (it == pluginMap_.end()) {
        return nullptr;
    }
    return it->second;
}

std::string HiviewPlatform::GetHiviewProperty(const std::string& key, const std::string& defaultValue)
{
    auto propPair = hiviewProperty_.find(key);
    if (propPair != hiviewProperty_.end()) {
        return propPair->second;
    }
    return Parameter::GetString(key, defaultValue);
}

bool HiviewPlatform::SetHiviewProperty(const std::string& key, const std::string& value, bool forceUpdate)
{
    auto propPair = hiviewProperty_.find(key);
    if (forceUpdate || (propPair == hiviewProperty_.end())) {
        hiviewProperty_[key] = value;
        return true;
    }
    return Parameter::SetProperty(key, value);
}
} // namespace HiviewDFX
} // namespace OHOS
