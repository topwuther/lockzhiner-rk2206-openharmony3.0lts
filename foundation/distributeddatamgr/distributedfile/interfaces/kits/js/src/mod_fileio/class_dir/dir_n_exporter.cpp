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

#include "dir_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "securec.h"

#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../class_dirent/dirent_entity.h"
#include "../class_dirent/dirent_n_exporter.h"
#include "../common_func.h"
#include "dir_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static DirEntity *GetDirEntity(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = NClass::GetEntityOf<DirEntity>(env, funcArg.GetThisVar());
    if (!dirEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dir");
        return nullptr;
    }
    return dirEntity;
}

napi_value DirNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    DirEntity *dirEntity = GetDirEntity(env, info);
    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }

    dirEntity->dir_.reset();
    return nullptr;
}

napi_value DirNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    DirEntity *dirEntity = GetDirEntity(env, info);
    if (!dirEntity || !dirEntity->dir_) {
        UniError(EBADF).ThrowErr(env, "Dir has been closed yet");
        return nullptr;
    }

    struct dirent tmpDirent;
    {
        lock_guard(dirEntity->lock_);
        errno = 0;
        dirent *res = nullptr;
        do {
            res = readdir(dirEntity->dir_.get());
            if (res == nullptr && errno) {
                UniError(errno).ThrowErr(env);
                return nullptr;
            } else if (res == nullptr) {
                return NVal::CreateUndefined(env).val_;
            } else if (string(res->d_name) == "." || string(res->d_name) == "..") {
                continue;
            } else {
                tmpDirent = *res;
                break;
            }
        } while (true);
    }

    napi_value objDirent = NClass::InstantiateClass(env, DirentNExporter::className_, {});
    if (!objDirent) {
        return nullptr;
    }

    auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, objDirent);
    if (!direntEntity) {
        return nullptr;
    }
    direntEntity->dirent_ = tmpDirent;

    return objDirent;
}

napi_value DirNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto dirEntity = make_unique<DirEntity>();
    if (!NClass::SetEntityFor<DirEntity>(env, funcArg.GetThisVar(), move(dirEntity))) {
        stringstream ss;
        ss << "INNER BUG. Failed to wrap entity for obj dir";
        UniError(EIO).ThrowErr(env, ss.str());
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool DirNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
    };

    string className = GetClassName();

    bool succ = false;
    napi_value clas = nullptr;
    tie(succ, clas) = NClass::DefineClass(exports_.env_, className, DirNExporter::Constructor, std::move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class Dirent");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, clas);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class Dirent");
        return false;
    }

    return exports_.AddProp(className, clas);
}

string DirNExporter::GetClassName()
{
    return DirNExporter::className_;
}

DirNExporter::DirNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

DirNExporter::~DirNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS