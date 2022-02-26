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

#ifndef RDB_JSKIT_NAPI_RDB_STORE_H
#define RDB_JSKIT_NAPI_RDB_STORE_H
#include <mutex>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "rdb_helper.h"

namespace OHOS {
namespace RdbJsKit {
class RdbStoreProxy {
public:
    static void Init(napi_env env, napi_value exports);
    static napi_value NewInstance(napi_env env, std::shared_ptr<OHOS::NativeRdb::RdbStore> value);
    static RdbStoreProxy *GetNativeInstance(napi_env env, napi_value self);
    void Release(napi_env env);
    RdbStoreProxy();
    ~RdbStoreProxy();

private:
    static napi_value Initialize(napi_env env, napi_callback_info info);
    static napi_value Delete(napi_env env, napi_callback_info info);
    static napi_value Update(napi_env env, napi_callback_info info);
    static napi_value Insert(napi_env env, napi_callback_info info);
    static napi_value Query(napi_env env, napi_callback_info info);
    static napi_value QuerySql(napi_env env, napi_callback_info info);
    static napi_value ExecuteSql(napi_env env, napi_callback_info info);
    static napi_ref constructor_;
    std::mutex mutex_;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore_;
    napi_ref ref_ = nullptr;
};
} // namespace RdbJsKit
} // namespace OHOS

#endif // RDB_JSKIT_NAPI_RDB_STORE_H
