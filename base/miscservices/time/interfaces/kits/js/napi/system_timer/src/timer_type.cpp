/*
 * Copyright (c); 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");;
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

#include "timer_type.h"

namespace OHOS {
namespace MiscServicesNapi {
void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value TimerTypeInit(napi_env env, napi_value exports)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, 1 << TIMER_TYPE_REALTIME, "TIMER_TYPE_REALTIME");
    SetNamedPropertyByInteger(env, obj, 1 << TIMER_TYPE_WAKEUP, "TIMER_TYPE_WAKEUP");
    SetNamedPropertyByInteger(env, obj, 1 << TIMER_TYPE_EXACT, "TIMER_TYPE_EXACT");
    SetNamedPropertyByInteger(env, obj, 1 << TIMER_TYPE_IDLE, "TIMER_TYPE_IDLE");

    napi_property_descriptor exportFuncs[] = {DECLARE_NAPI_PROPERTY("systemTimer", obj)};
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);

    return exports;
}
}  // namespace MiscServicesNapi
}  // namespace OHOS