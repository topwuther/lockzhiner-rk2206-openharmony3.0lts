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

#ifndef PERMISSION_RECORD_MGR_H_
#define PERMISSION_RECORD_MGR_H_
#include <vector>

#include "constant.h"
#include "device_info_manager.h"
#include "distributed_permission_kit.h"
#include "external_deps.h"
#include "ipc_skeleton.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "parameter.h"
#include "permission_log.h"
#include "query_permission_used_request.h"
#include "query_permission_used_result.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct AsyncRecordsCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
    int32_t uid;
    int32_t flag;
    std::string deviceId;
    QueryPermissionUsedRequest request;
    QueryPermissionUsedResult queryResult;
    bool ret = false;
};

struct AsyncGetRecordCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback = 0;
};

napi_value GetPermissionRecord(napi_env env, napi_callback_info info);
napi_value GetPermissionRecordAsync(napi_env env, napi_callback_info info);
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif /* BUNDLE_MGR_H_ */
