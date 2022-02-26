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

#include "dirent_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>

#include "securec.h"

#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "dirent_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static DirentEntity *GetDirentEntity(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto direntEntity = NClass::GetEntityOf<DirentEntity>(env, funcArg.GetThisVar());
    if (!direntEntity) {
        UniError(EIO).ThrowErr(env, "Cannot get entity of Dirent");
        return nullptr;
    }
    return direntEntity;
}

static napi_value CheckDirentDType(napi_env env, napi_callback_info info, unsigned char dType)
{
    DirentEntity *direntEntity = GetDirentEntity(env, info);
    if (!direntEntity) {
        return nullptr;
    }

    return NVal::CreateBool(env, direntEntity->dirent_.d_type == dType).val_;
}

napi_value DirentNExporter::isBlockDevice(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_BLK);
}

napi_value DirentNExporter::isCharacterDevice(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_CHR);
}

napi_value DirentNExporter::isDirectory(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_DIR);
}

napi_value DirentNExporter::isFIFO(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_FIFO);
}

napi_value DirentNExporter::isFile(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_REG);
}

napi_value DirentNExporter::isSocket(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_SOCK);
}

napi_value DirentNExporter::isSymbolicLink(napi_env env, napi_callback_info info)
{
    return CheckDirentDType(env, info, DT_LNK);
}

napi_value DirentNExporter::GetName(napi_env env, napi_callback_info info)
{
    DirentEntity *direntEntity = GetDirentEntity(env, info);
    if (!direntEntity) {
        return nullptr;
    }
    return NVal::CreateUTF8String(env, direntEntity->dirent_.d_name).val_;
}

napi_value DirentNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto direntEntity = make_unique<DirentEntity>();
    if (!NClass::SetEntityFor<DirentEntity>(env, funcArg.GetThisVar(), move(direntEntity))) {
        stringstream ss;
        ss << "INNER BUG. Failed to wrap entity for obj dirent";
        UniError(EIO).ThrowErr(env, ss.str());
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool DirentNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("isBlockDevice", isBlockDevice),
        NVal::DeclareNapiFunction("isCharacterDevice", isCharacterDevice),
        NVal::DeclareNapiFunction("isDirectory", isDirectory),
        NVal::DeclareNapiFunction("isFIFO", isFIFO),
        NVal::DeclareNapiFunction("isFile", isFile),
        NVal::DeclareNapiFunction("isSocket", isSocket),
        NVal::DeclareNapiFunction("isSymbolicLink", isSymbolicLink),

        NVal::DeclareNapiGetter("name", GetName),
    };

    string className = GetClassName();

    bool succ = false;
    napi_value clas = nullptr;
    tie(succ, clas) = NClass::DefineClass(exports_.env_, className, DirentNExporter::Constructor, std::move(props));
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

string DirentNExporter::GetClassName()
{
    return DirentNExporter::className_;
}

DirentNExporter::DirentNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

DirentNExporter::~DirentNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS