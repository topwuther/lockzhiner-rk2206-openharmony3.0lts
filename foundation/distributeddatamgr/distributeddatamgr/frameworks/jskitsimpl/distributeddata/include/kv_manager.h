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
#ifndef OHOS_KV_MANAGER_H
#define OHOS_KV_MANAGER_H

#include <string>
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "distributed_kv_data_manager.h"
namespace OHOS::DistributedData {
class KVManager {
public:
    static napi_value CreateKVManager(napi_env env, napi_callback_info info);
    static napi_value GetKVStore(napi_env env, napi_callback_info info);
private:
    static napi_value GetCtor(napi_env env);
    static napi_value Initialize(napi_env env, napi_callback_info info);

    DistributedKv::DistributedKvDataManager kvDataManager_ {};
    std::string bundleName_ {};
    static napi_ref ctor_;
};
}
#endif // OHOS_KV_MANAGER_H
