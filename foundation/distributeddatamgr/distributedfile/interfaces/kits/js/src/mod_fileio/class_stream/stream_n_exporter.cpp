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

#include "stream_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

#include "securec.h"

#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "stream_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value StreamNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    /* To get entity */
    bool succ = false;
    FILE *filp = nullptr;
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may have been closed");
        return nullptr;
    } else {
        filp = streamEntity->fp.get();
    }

    void *buf = nullptr;
    int64_t len;
    bool hasPos = false;
    int64_t pos;
    tie(succ, buf, len, hasPos, pos) = CommonFunc::GetReadArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }

    if (hasPos && (fseek(filp, pos, SEEK_SET) == -1)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    size_t actLen = fread(buf, 1, len, filp);
    if (actLen != static_cast<size_t>(len) && ferror(filp)) {
        UniError(errno).ThrowErr(env);
    }

    return NVal::CreateInt64(env, actLen).val_;
}

napi_value StreamNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EINVAL).ThrowErr(env, "Stream may have been closed yet");
        return nullptr;
    }
    streamEntity->fp.reset();
    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    FILE *filp = nullptr;
    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may has been closed");
        return nullptr;
    } else {
        filp = streamEntity->fp.get();
    }

    void *buf = nullptr;
    size_t len;
    size_t position;
    unique_ptr<char[]> bufGuard;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]);
    if (!succ) {
        return nullptr;
    }
    if (hasPos && (fseek(filp, position, SEEK_SET) == -1)) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    ssize_t writeLen = fwrite(buf, 1, len, filp);
    if (writeLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

napi_value StreamNExporter::FlushSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto streamEntity = NClass::GetEntityOf<StreamEntity>(env, funcArg.GetThisVar());
    if (!streamEntity || !streamEntity->fp) {
        UniError(EBADF).ThrowErr(env, "Stream may has been closed");
        return nullptr;
    }

    int ret = fflush(streamEntity->fp.get());
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value StreamNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    unique_ptr<StreamEntity> streamEntity = make_unique<StreamEntity>();
    if (!NClass::SetEntityFor<StreamEntity>(env, funcArg.GetThisVar(), move(streamEntity))) {
        stringstream ss;
        ss << "INNER BUG. Failed to wrap entity for obj stat";
        UniError(EIO).ThrowErr(env, ss.str());
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool StreamNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("writeSync", WriteSync),
        NVal::DeclareNapiFunction("flushSync", FlushSync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value cls = nullptr;
    tie(succ, cls) = NClass::DefineClass(exports_.env_, className, StreamNExporter::Constructor, move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, cls);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
        return false;
    }

    return exports_.AddProp(className, cls);
}

string StreamNExporter::GetClassName()
{
    return StreamNExporter::className_;
}

StreamNExporter::StreamNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

StreamNExporter::~StreamNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS