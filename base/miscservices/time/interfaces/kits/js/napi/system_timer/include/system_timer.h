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

#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "time_service_client.h"

namespace OHOS {
namespace MiscServicesNapi {
using namespace OHOS::MiscServices;

constexpr int NONE_PARAMETER = 0;
constexpr int ONE_PARAMETER = 1;
constexpr int TWO_PARAMETERS = 2;
constexpr int THREE_PARAMETERS = 3;

class ITimerInfoInstance : public ITimerInfo {
public:
    ITimerInfoInstance();
    virtual ~ITimerInfoInstance();
    virtual void OnTrigger() override;
    virtual void SetType(const int &type) override;
    virtual void SetRepeat(bool repeat) override;
    virtual void SetInterval(const uint64_t &interval) override;
    virtual void SetWantAgent(std::shared_ptr<OHOS::Notification::WantAgent::WantAgent> wantAgent) override;
    void SetCallbackInfo(const napi_env &env, const napi_ref &ref);

private:
    struct CallbackInfo {
        napi_env env;
        napi_ref ref;
    };

    CallbackInfo callbackInfo_;
};

napi_value SystemtimerInit(napi_env env, napi_value exports);
napi_value CreateTimer(napi_env env, napi_callback_info info);
napi_value StartTimer(napi_env env, napi_callback_info info);
napi_value StopTimer(napi_env env, napi_callback_info info);
napi_value DestroyTimer(napi_env env, napi_callback_info info);
}  // namespace MiscServicesNapi
}  // namespace OHOS
#endif  // SYSTEM_TIMER_H