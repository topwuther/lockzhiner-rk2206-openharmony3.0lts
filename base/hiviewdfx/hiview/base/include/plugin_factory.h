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
#ifndef HIVIEW_BASE_PLUGIN_FACTORY_H
#define HIVIEW_BASE_PLUGIN_FACTORY_H
#include <map>
#include <memory>

#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
using PluginInstance = std::shared_ptr<Plugin> (*)();
class DllExport PluginFactory {
public:
    static void RegisterPlugin(const std::string& name, PluginInstance func);
    static void UnregisterPlugin(const std::string& name);
    static std::shared_ptr<Plugin> GetPlugin(const std::string& name);

private:
    static std::shared_ptr<std::map<std::string, PluginInstance>> GetGlobalPluginRegistryMap();
};

class PluginRegister {
public:
    PluginRegister(const std::string& name, PluginInstance fp)
    {
        PluginFactory::RegisterPlugin(name, fp);
    };
    ~PluginRegister(){};
};

#define REGISTER(ClassName)                                 \
    class Register##ClassName {                             \
    public:                                                 \
        static std::shared_ptr<Plugin> Instance()           \
        {                                                   \
            return std::make_shared<ClassName>();           \
        }                                                   \
                                                            \
    private:                                                \
        static const PluginRegister g_staticPluginRegister; \
    };                                                      \
    const PluginRegister Register##ClassName::g_staticPluginRegister(#ClassName, Register##ClassName::Instance);
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_PLUGIN_FACTORY_H
