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
#ifndef NATIVE_HIAPPEVENT_HELPER_H
#define NATIVE_HIAPPEVENT_HELPER_H

#include <memory>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

struct HiAppEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::shared_ptr<AppEventPack> appEventPack;
    int32_t result;
};

void WriteEventFromNapi(const napi_env env, HiAppEventAsyncContext* asyncContext);
} // HiviewDFX
} // OHOS
#endif // NATIVE_HIAPPEVENT_HELPER_H