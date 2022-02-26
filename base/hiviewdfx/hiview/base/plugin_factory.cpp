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
#include "plugin_factory.h"
#include "logger.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-PluginFactory");
std::shared_ptr<std::map<std::string, PluginInstance>> PluginFactory::GetGlobalPluginRegistryMap()
{
    static std::shared_ptr<std::map<std::string, PluginInstance>> pluginMap;
    if (pluginMap == nullptr) {
        pluginMap = std::make_shared<std::map<std::string, PluginInstance>>();
    }
    return pluginMap;
}

void PluginFactory::RegisterPlugin(const std::string& name, PluginInstance func)
{
    if (func == nullptr) {
        HIVIEW_LOGW("Register null plugin constructor from %{public}s.", name.c_str());
        return;
    }
    // force update plugin constructor
    auto pluginMap = GetGlobalPluginRegistryMap();
    pluginMap->insert(std::make_pair(name, func));
#ifdef _WIN32
    // When PluginFactory is loading, the logger is not loaded.
#else
    HIVIEW_LOGD("Register plugin constructor from %{public}s.", name.c_str());
#endif // _WIN32
}

void PluginFactory::UnregisterPlugin(const std::string& name)
{
    auto pluginMap = GetGlobalPluginRegistryMap();
    pluginMap->erase(name);
}

std::shared_ptr<Plugin> PluginFactory::GetPlugin(const std::string& name)
{
    auto pluginMap = GetGlobalPluginRegistryMap();
    auto it = pluginMap->find(name);
    if (it == pluginMap->end()) {
        HIVIEW_LOGW("Could not find plugin with name:%{public}s.", name.c_str());
        return nullptr;
    }
    return it->second();
}
} // namespace HiviewDFX
} // namespace OHOS
