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

#include "common_func.h"

#include "../common/napi/n_func_arg.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFile {
using namespace std;

tuple<bool, napi_ref, napi_ref, napi_ref> CommonFunc::GetCallbackHandles(napi_env env, napi_value object)
{
    bool succ = false;
    NVal prop = NVal(env, object);
    napi_value successProp, failProp, completeProp;
    napi_ref successHandle = nullptr;
    napi_ref failHandle = nullptr;
    napi_ref completeHandle = nullptr;
    string success = "success";
    string fail = "fail";
    string complete = "complete";

    successProp = prop.GetProp(success).val_;
    if (successProp != nullptr) {
        napi_create_reference(env, successProp, 1, &successHandle);
        succ = true;
    }

    failProp = prop.GetProp(fail).val_;
    if (succ && failProp != nullptr) {
        napi_create_reference(env, failProp, 1, &failHandle);
        succ = true;
    }

    completeProp = prop.GetProp(complete).val_;
    if (succ && completeProp != nullptr) {
        napi_create_reference(env, completeProp, 1, &completeHandle);
        succ = true;
    }

    return { succ, successHandle, failHandle, completeHandle };
}
} // namespace ModuleFile
} // namespace DistributedFS
} // namespace OHOS
