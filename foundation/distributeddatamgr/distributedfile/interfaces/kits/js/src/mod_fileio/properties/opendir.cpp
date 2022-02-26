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

#include "opendir.h"

#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"

#include "../class_dir/dir_entity.h"
#include "../class_dir/dir_n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value Opendir::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> path;
    tie(succ, path, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    std::unique_ptr<DIR, std::function<void(DIR*)>> dir = {opendir(path.get()), closedir};
    if (!dir) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    napi_value objDir = NClass::InstantiateClass(env, DirNExporter::className_, {});
    if (!objDir) {
        UniError(EINVAL).ThrowErr(env, "Cannot instantiate class DirSync");
        return nullptr;
    }

    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, objDir);
    if (!dirEntity) {
        UniError(EINVAL).ThrowErr(env, "Cannot get the entity of objDir");
        return nullptr;
    }
    dirEntity->dir_.swap(dir);
    return objDir;
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS