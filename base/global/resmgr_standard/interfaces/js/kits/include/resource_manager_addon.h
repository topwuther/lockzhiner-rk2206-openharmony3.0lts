/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RESOURCE_MANAGER_ADDON_H
#define RESOURCE_MANAGER_ADDON_H

#include <memory>
#include <string>

#include "ability.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "resource_manager.h"

namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerAddon {
public:
    static napi_value Init(napi_env env, napi_value exports);

    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

    ResourceManagerAddon();

    ~ResourceManagerAddon();

    bool InitContext(napi_env env, const std::string bundleName, AppExecFwk::Ability *ability);

    inline std::shared_ptr<ResourceManager> GetResMgr();

    std::string GetLocale(std::unique_ptr<ResConfig> &cfg);

private:
    static napi_async_execute_callback GetResMgrExecute();

    static napi_value GetResourceManager(napi_env env, napi_callback_info info);

    static int GetResId(napi_env env, size_t argc, napi_value *argv);

    static napi_value ProcessOnlyIdParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetString(napi_env env, napi_callback_info info);

    static napi_value GetStringArray(napi_env env, napi_callback_info info);

    static napi_value GetMedia(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64(napi_env env, napi_callback_info info);

    static napi_value ProcessNoParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetConfiguration(napi_env env, napi_callback_info info);

    static napi_value GetDeviceCapability(napi_env env, napi_callback_info info);

    static napi_value GetPluralString(napi_env env, napi_callback_info info);

    static napi_value New(napi_env env, napi_callback_info info);

    std::string bundleName_;
    napi_env env_;
    napi_ref wrapper_;
    std::shared_ptr<ResourceManager> resMgr_;
};

struct ResMgrAsyncContext {
    napi_async_work work_;

    std::string bundleName_;
    int32_t resId_;
    int32_t param_;

    typedef napi_value (*CreateNapiValue)(napi_env env, ResMgrAsyncContext &context);
    CreateNapiValue createValueFunc_;
    std::string value_;
    std::vector<std::string> arrayValue_;

    std::unique_ptr<char[]> mediaData;
    int len_;

    napi_deferred deferred_;
    napi_ref callbackRef_;

    std::string errMsg_;
    int success_;

    std::shared_ptr<ResourceManagerAddon> addon_;
    AppExecFwk::Ability *ability_;

    ResMgrAsyncContext() : work_(nullptr), resId_(0), param_(0), createValueFunc_(nullptr), len_(0), deferred_(nullptr),
        callbackRef_(nullptr), success_(true), addon_(nullptr), ability_(nullptr) {}

    void SetErrorMsg(const std::string &msg, bool withResId = false);
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif