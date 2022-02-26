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

#ifndef SYSTEM_TIMER_TYPE_H
#define SYSTEM_TIMER_TYPE_H

#include <string>
#include "napi/native_api.h"

namespace OHOS {
namespace MiscServicesNapi {
constexpr int TIMER_TYPE_REALTIME = 0;
constexpr int TIMER_TYPE_WAKEUP = 1;
constexpr int TIMER_TYPE_EXACT = 2;
constexpr int TIMER_TYPE_IDLE = 3;

napi_value TimerTypeInit(napi_env env, napi_value exports);
}  // namespace MiscServicesNapi
}  // namespace OHOS

#endif  // SYSTEM_TIMER_TYPE_H