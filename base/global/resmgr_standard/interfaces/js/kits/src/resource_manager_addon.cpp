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

#include "resource_manager_addon.h"

#include <fstream>
#include <memory>
#include <vector>

#include "hilog/log.h"
#include "node_api.h"

namespace OHOS {
namespace Global {
namespace Resource {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "ResourceManagerJs" };
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;
static napi_ref* g_constructor = nullptr;
std::vector<char> g_codes = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

ResourceManagerAddon::ResourceManagerAddon() : env_(nullptr), wrapper_(nullptr) {}

ResourceManagerAddon::~ResourceManagerAddon()
{
    napi_delete_reference(env_, wrapper_);
    HiLog::Info(LABEL, "~ResourceManagerAddon %{public}s", bundleName_.c_str());
}

std::shared_ptr<ResourceManager> ResourceManagerAddon::GetResMgr()
{
    return resMgr_;
}

void ResourceManagerAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    std::shared_ptr<ResourceManagerAddon> *addonPtr =
        static_cast<std::shared_ptr<ResourceManagerAddon> *>(nativeObject);
    delete addonPtr;
}

void ResMgrAsyncContext::SetErrorMsg(const std::string &msg, bool withResId)
{
    errMsg_ = msg;
    success_ = false;
    if (withResId) {
        HiLog::Error(LABEL, "%{public}s id = %{public}d", msg.c_str(), resId_);
    } else {
        HiLog::Error(LABEL, "%{public}s", msg.c_str());
    }
}

napi_value ResourceManagerAddon::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor creatorProp[] = {
        DECLARE_NAPI_FUNCTION("getResourceManager", GetResourceManager),
    };
    napi_status status = napi_define_properties(env, exports, 1, creatorProp);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set getResourceManager at init");
        return nullptr;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getString", GetString),
        DECLARE_NAPI_FUNCTION("getStringArray", GetStringArray),
        DECLARE_NAPI_FUNCTION("getMedia", GetMedia),
        DECLARE_NAPI_FUNCTION("getMediaBase64", GetMediaBase64),
        DECLARE_NAPI_FUNCTION("getConfiguration", GetConfiguration),
        DECLARE_NAPI_FUNCTION("getDeviceCapability", GetDeviceCapability),
        DECLARE_NAPI_FUNCTION("getPluralString", GetPluralString)
    };

    napi_value constructor;
    status = napi_define_class(env, "ResourceManager", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to define class at Init");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "ResourceManager", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set property at init");
        return nullptr;
    }

    g_constructor = new (std::nothrow) napi_ref;
    if (g_constructor == nullptr) {
        HiLog::Error(LABEL, "Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_async_execute_callback ResourceManagerAddon::GetResMgrExecute()
{
    return [](napi_env env, void* data) {
        ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
        std::shared_ptr<ResourceManagerAddon> obj = std::make_shared<ResourceManagerAddon>();
        if (obj == nullptr) {
            asyncContext->SetErrorMsg("Failed to create ResourceManagerAddon");
            return;
        }
        if (!obj->InitContext(env, asyncContext->bundleName_, asyncContext->ability_)) {
            asyncContext->SetErrorMsg("InitContext failed");
            return;
        }

        asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) -> napi_value {
            napi_value constructor;
            napi_status status = napi_get_reference_value(env, *g_constructor, &constructor);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to get ResourceManagerAddon");
                return nullptr;
            }
            napi_value result = nullptr;
            status = napi_new_instance(env, constructor, 0, nullptr, &result);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to create ResourceManagerAddon");
                return nullptr;
            }
            std::shared_ptr<ResourceManagerAddon> *addonPtr = new std::shared_ptr<ResourceManagerAddon>(context.addon_);
            status = napi_wrap(env, result, reinterpret_cast<void *>(addonPtr), ResourceManagerAddon::Destructor,
                nullptr, &context.addon_->wrapper_);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to wrap ResourceManagerAddon");
                delete addonPtr;
                return nullptr;
            }
            return result;
        };
        asyncContext->addon_ = obj;
    };
}


auto completeFunc = [](napi_env env, napi_status status, void* data) {
    ResMgrAsyncContext* asyncContext = static_cast<ResMgrAsyncContext*>(data);

    napi_value finalResult = nullptr;
    if (asyncContext->createValueFunc_ != nullptr) {
        finalResult = asyncContext->createValueFunc_(env, *asyncContext);
    }

    napi_value result[] = { nullptr, nullptr };
    if (asyncContext->success_) {
        napi_get_undefined(env, &result[0]);
        result[1] = finalResult;
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, asyncContext->errMsg_.c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[0]);
        napi_get_undefined(env, &result[1]);
    }
    do {
        if (asyncContext->deferred_) {
            if (asyncContext->success_) {
                if (napi_resolve_deferred(env, asyncContext->deferred_, result[1]) != napi_ok) {
                    HiLog::Error(LABEL, "napi_resolve_deferred failed");
                }
            } else {
                if (napi_reject_deferred(env, asyncContext->deferred_, result[0]) != napi_ok) {
                    HiLog::Error(LABEL, "napi_reject_deferred failed");
                }
            }
        } else {
            napi_value callback = nullptr;
            napi_status status = napi_get_reference_value(env, asyncContext->callbackRef_, &callback);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_get_reference_value failed status=%{public}d", status);
                break;
            }
            napi_value userRet = nullptr;
            status = napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(napi_value), result, &userRet);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_call_function failed status=%{public}d", status);
                break;
            }
            status = napi_delete_reference(env, asyncContext->callbackRef_);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_call_function failed status=%{public}d", status);
                break;
            }
        }
    } while (false);
    napi_delete_async_work(env, asyncContext->work_);
    delete asyncContext;
};

bool GetGlobalAbility(napi_env env, ResMgrAsyncContext &context)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get global");
        return false;
    }

    napi_value abilityObj;
    status = napi_get_named_property(env, global, "ability", &abilityObj);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get ability property");
        return false;
    }

    Ability* ability = nullptr;
    status = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get native ability");
        return false;
    }
    context.ability_ = ability;
    return true;
}

napi_value ResourceManagerAddon::GetResourceManager(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_string) {
            size_t len = 0;
            napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to get bundle name length");
                return nullptr;
            }
            std::vector<char> buf(len + 1);
            status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to get bundle name");
                return nullptr;
            }
            asyncContext->bundleName_ = buf.data();
        } else if ((i == 0 || i == 1) && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    if (!GetGlobalAbility(env, *asyncContext)) {
        HiLog::Error(LABEL, "get global ability failed");
        return nullptr;
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "getResourceManager", NAPI_AUTO_LENGTH, &resource);
    napi_status status = napi_create_async_work(env, nullptr, resource, GetResMgrExecute(), completeFunc,
        static_cast<void*>(asyncContext.get()), &asyncContext->work_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for getResourceManager %{public}d", status);
        return result;
    }
    status = napi_queue_async_work(env, asyncContext->work_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for getResourceManager %{public}d", status);
        return result;
    }

    asyncContext.release();
    return result;
}

napi_value ResourceManagerAddon::New(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 1);

    napi_value target = nullptr;
    napi_get_new_target(env, info, &target);
    if (target != nullptr) {
        return thisVar;
    }

    return nullptr;
}

bool ResourceManagerAddon::InitContext(napi_env env, const std::string bundleName, Ability* ability)
{
    env_ = env;
    if (bundleName.length() == 0) {
        resMgr_ = ability->GetResourceManager();
    } else {
        std::shared_ptr<Context> bundleConext = ability->CreateBundleContext(bundleName, 0);
        if (bundleConext != nullptr) {
            resMgr_ = bundleConext->GetResourceManager();
        }
    }

    bundleName_ = bundleName;
    return resMgr_ != nullptr;
}

int ResourceManagerAddon::GetResId(napi_env env, size_t argc, napi_value *argv)
{
    if (argc == 0 || argv == nullptr) {
        return 0;
    }

    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, argv[0], &valuetype);
    if (valuetype != napi_number) {
        HiLog::Error(LABEL, "Invalid param, not number");
        return 0;
    }
    int resId = 0;
    status = napi_get_value_int32(env, argv[0], &resId);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get id number");
        return 0;
    }

    return resId;
}

napi_value ResourceManagerAddon::ProcessOnlyIdParam(napi_env env, napi_callback_info info, const std::string &name,
    napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to unwrap %{public}s", name.c_str());
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_number) {
            asyncContext->resId_ =  GetResId(env, argc, argv);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, execute, completeFunc,
        static_cast<void*>(asyncContext.get()), &asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for %{public}s", name.c_str());
        return result;
    }
    if (napi_queue_async_work(env, asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for %{public}s", name.c_str());
        return result;
    }

    asyncContext.release();
    return result;
}

auto getStringFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state = asyncContext->addon_->GetResMgr()->GetStringById(asyncContext->resId_, asyncContext->value_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetString failed state", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) {
        napi_value jsValue = nullptr;
        if (napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &jsValue) != napi_ok) {
            context.SetErrorMsg("Failed to create result");
            return jsValue;
        }
        return jsValue;
    };
};

napi_value ResourceManagerAddon::GetString(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getString", getStringFunc);
}

auto getStringArrayFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state = asyncContext->addon_->GetResMgr()->GetStringArrayById(asyncContext->resId_,
        asyncContext->arrayValue_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetStringArray failed state", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) -> napi_value {
        napi_value result;
        napi_status status = napi_create_array_with_length(env, context.arrayValue_.size(), &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create array");
            return nullptr;
        }
        for (size_t i = 0; i < context.arrayValue_.size(); i++) {
            napi_value value;
            status = napi_create_string_utf8(env, context.arrayValue_[i].c_str(), NAPI_AUTO_LENGTH, &value);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to create string item");
                return nullptr;
            }
            status = napi_set_element(env, result, i, value);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to set array item");
                return nullptr;
            }
        }

        return result;
    };
};

napi_value ResourceManagerAddon::GetStringArray(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getStringArray", getStringArrayFunc);
}

std::unique_ptr<char[]> EncodeBase64(std::unique_ptr<char[]> &data, int srcLen)
{
    int len = (srcLen / 3) * 4; // Split 3 bytes to 4 parts, each containing 6 bits.
    int outLen = ((srcLen % 3) != 0) ? (len + 4) : len;
    const char *srcData = data.get();
    std::unique_ptr<char[]>  result = std::make_unique<char[]>(outLen + 1);
    char *dstData = result.get();
    int j = 0;
    int i = 0;
    for (; i < srcLen - 3; i += 3) {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        unsigned char byte2 = static_cast<unsigned char>(srcData[i + 1]);
        unsigned char byte3 = static_cast<unsigned char>(srcData[i + 2]);
        dstData[j++] = g_codes[byte1 >> 2];
        dstData[j++] = g_codes[((byte1 & 0x3) << 4) | (byte2 >> 4)];
        dstData[j++] = g_codes[((byte2 & 0xF) << 2) | (byte3 >> 6)];
        dstData[j++] = g_codes[byte3 & 0x3F];
    }
    if (srcLen % 3 == 1) {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        dstData[j++] = g_codes[byte1 >> 2];
        dstData[j++] = g_codes[(byte1 & 0x3) << 4];
        dstData[j++] = '=';
        dstData[j++] = '=';
    } else {
        unsigned char byte1 = static_cast<unsigned char>(srcData[i]);
        unsigned char byte2 = static_cast<unsigned char>(srcData[i + 1]);
        dstData[j++] = g_codes[byte1 >> 2];
        dstData[j++] = g_codes[((byte1 & 0x3) << 4) | (byte2 >> 4)];
        dstData[j++] = g_codes[(byte2 & 0xF) << 2];
        dstData[j++] = '=';
    }
    dstData[outLen] = '\0';

    return result;
}

std::unique_ptr<char[]> LoadResourceFile(std::string &path, ResMgrAsyncContext &asyncContext, int &len)
{
    std::ifstream mediaStream(path, std::ios::binary);
    if (!mediaStream.is_open()) {
        asyncContext.SetErrorMsg("Failed to open media");
        return nullptr;
    }
    mediaStream.seekg(0, std::ios::end);
    len = mediaStream.tellg();
    std::unique_ptr<char[]> tempData = std::make_unique<char[]>(len);
    if (tempData == nullptr) {
        asyncContext.SetErrorMsg("Failed to create media buffer");
        return nullptr;
    }
    mediaStream.seekg(0, std::ios::beg);
    mediaStream.read((tempData.get()), len);
    return tempData;
}

auto getMediaFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    std::string path;
    RState state = asyncContext->addon_->GetResMgr()->GetMediaById(asyncContext->resId_, path);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetMedia path failed", true);
        return;
    }
    asyncContext->mediaData = LoadResourceFile(path, *asyncContext, asyncContext->len_);
    if (asyncContext->mediaData == nullptr) {
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        napi_value buffer;
        napi_status status = napi_create_external_arraybuffer(env, context.mediaData.get(), context.len_,
            [](napi_env env, void *data, void *hint) {
                HiLog::Error(LABEL, "Media buffer finalized");
                delete[] static_cast<char*>(data);
            }, nullptr, &buffer);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create media external array buffer");
            return nullptr;
        }

        napi_value result = nullptr;
        status = napi_create_typedarray(env, napi_uint8_array, context.len_, buffer, 0, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create media typed array");
            return nullptr;
        }
        context.mediaData.release();
        return result;
    };
};

napi_value ResourceManagerAddon::GetMedia(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getMedia", getMediaFunc);
}

auto getMediaBase64Func = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    int len = 0;
    std::string path;
    RState state = asyncContext->addon_->GetResMgr()->GetMediaById(asyncContext->resId_, path);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetMedia path failed", true);
        return;
    }
    std::unique_ptr<char[]> tempData = LoadResourceFile(path, *asyncContext, len);
    if (tempData == nullptr) {
        return;
    }
    auto pos = path.find_last_of('.');
    std::string imgType;
    if (pos != std::string::npos) {
        imgType = path.substr(pos + 1);
    }
    std::unique_ptr<char[]> base64Data = EncodeBase64(tempData, len);
    asyncContext->value_ = "data:image/" + imgType + ";base64," + base64Data.get();
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) {
        napi_value result;
        if (napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &result) != napi_ok) {
            context.SetErrorMsg("Failed to create result");
            return result;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetMediaBase64(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "GetMediaBase64", getMediaBase64Func);
}

std::string ResourceManagerAddon::GetLocale(std::unique_ptr<ResConfig> &cfg)
{
    std::string result;
    const icu::Locale *localeInfo = cfg->GetLocaleInfo();
    if (localeInfo == nullptr) {
        return result;
    }
    const char *lang = localeInfo->getLanguage();
    if (lang == nullptr) {
        return result;
    }
    result = lang;

    const char *script = localeInfo->getScript();
    if (script != nullptr) {
        result += std::string("_") + script;
    }

    const char *region = localeInfo->getCountry();
    if (region != nullptr) {
        result += std::string("_") + region;
    }
    return result;
}

napi_value ResourceManagerAddon::ProcessNoParam(napi_env env, napi_callback_info info, const std::string &name,
    napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 1);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to unwrap ProcessNoParam");
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (valueType == napi_function) {
        napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef_);
    }

    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, execute, completeFunc,
        static_cast<void*>(asyncContext.get()), &asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for %{public}s", name.c_str());
        return result;
    }
    if (napi_queue_async_work(env, asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for %{public}s", name.c_str());
        return result;
    }
    asyncContext.release();
    return result;
}

auto getConfigFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        std::unique_ptr<ResConfig> cfg(CreateResConfig());
        context.addon_->GetResMgr()->GetResConfig(*cfg);

        napi_value result;
        napi_status status = napi_create_object(env, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create Configuration object");
            return nullptr;
        }

        napi_value direction;
        status = napi_create_int32(env, static_cast<int>(cfg->GetDirection()), &direction);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create direction");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "direction", direction);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set direction property");
            return nullptr;
        }

        napi_value locale;
        status = napi_create_string_utf8(env, context.addon_->GetLocale(cfg).c_str(), NAPI_AUTO_LENGTH, &locale);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create locale");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "locale", locale);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set locale property");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetConfiguration(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetConfiguration", getConfigFunc);
}

auto getDeviceCapFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        std::unique_ptr<ResConfig> cfg(CreateResConfig());
        context.addon_->GetResMgr()->GetResConfig(*cfg);

        napi_value result;
        napi_status status = napi_create_object(env, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create GetDeviceCapability object");
            return nullptr;
        }

        napi_value deviceType;
        status = napi_create_int32(env, static_cast<int>(cfg->GetDeviceType()), &deviceType);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create deviceType");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "deviceType", deviceType);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set deviceType property");
            return nullptr;
        }

        napi_value screenDensity;
        status = napi_create_int32(env, static_cast<int>(cfg->GetScreenDensity()), &screenDensity);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create screenDensity");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "screenDensity", screenDensity);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set screenDensity property");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetDeviceCapability(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetDeviceCapability", getDeviceCapFunc);
}

auto getPluralCapFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state = asyncContext->addon_->GetResMgr()->GetPluralStringByIdFormat(asyncContext->value_,
        asyncContext->resId_, asyncContext->param_, asyncContext->param_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetPluralString failed", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        napi_value result;
        napi_status status = napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create plural string");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetPluralString(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to unwrap GetPluralString");
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_number) {
            asyncContext->resId_ =  GetResId(env, argc, argv);
        } else if (i == 1 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->param_);
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "getPluralString", NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, getPluralCapFunc, completeFunc,
        static_cast<void*>(asyncContext.get()), &asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for GetPluralString");
        return result;
    }
    if (napi_queue_async_work(env, asyncContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for GetPluralString");
        return result;
    }
    asyncContext.release();
    return result;
}

static napi_value ResMgrInit(napi_env env, napi_value exports)
{
    return ResourceManagerAddon::Init(env, exports);
}

static napi_module g_resourceManagerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ResMgrInit,
    .nm_modname = "resourceManager",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void ResMgrRegister()
{
    napi_module_register(&g_resourceManagerModule);
}
} // namespace Resource
} // namespace Global
} // namespace OHOS