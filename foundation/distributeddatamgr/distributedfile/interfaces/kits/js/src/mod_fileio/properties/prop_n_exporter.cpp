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

#include "prop_n_exporter.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../../common/fd_guard.h"
#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"
#include "../class_dir/dir_entity.h"
#include "../class_dir/dir_n_exporter.h"
#include "../class_stat/stat_entity.h"
#include "../class_stat/stat_n_exporter.h"
#include "../class_stream/stream_entity.h"
#include "../class_stream/stream_n_exporter.h"
#include "../common_func.h"

#include "create_stream.h"
#include "fdopen_stream.h"
#include "opendir.h"
#include "stat.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value PropNExporter::AccessSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
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

    int argc = funcArg.GetArgc();
    int ret = -1;
    if (argc == NARG_CNT::ONE) {
        ret = access(path.get(), 0);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        ret = access(path.get(), mode);
    }
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::CopyFileSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> src;
    tie(succ, src, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid src");
        return nullptr;
    }

    unique_ptr<char[]> dest;
    tie(succ, dest, ignore) = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid dest");
        return nullptr;
    }

    FDGuard sfd;
    FDGuard ofd;
    struct stat statbf;
    sfd.SetFD(open(src.get(), O_RDONLY));
    if (sfd.GetFD() == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    int res = fstat(sfd.GetFD(), &statbf);
    if (res == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    // Note that mode means if we should force to save instead of the mode of newly create file
    ofd.SetFD(open(dest.get(), O_WRONLY | O_CREAT, statbf.st_mode));
    if (ofd.GetFD() == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    int ret = sendfile(ofd.GetFD(), sfd.GetFD(), nullptr, statbf.st_size);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::MkdirSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
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

    int ret = -1;
    int argc = funcArg.GetArgc();
    if (argc == NARG_CNT::ONE) {
        ret = mkdir(path.get(), 0775);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        ret = mkdir(path.get(), mode);
    }

    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::OpenSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
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

    int flags = O_RDONLY;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        tie(succ, flags) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid flags");
            return nullptr;
        }
    }

    int fd = -1;
    int argc = funcArg.GetArgc();
    if (argc != NARG_CNT::THREE) {
        if ((flags & O_CREAT) || (flags & O_TMPFILE)) {
            UniError(EINVAL).ThrowErr(env, "called with O_CREAT/O_TMPFILE but no mode");
            return nullptr;
        }
        fd = open(path.get(), flags);
    } else {
        int mode;
        tie(succ, mode) = NVal(env, funcArg.GetArg(NARG_POS::THIRD)).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid mode");
            return nullptr;
        }
        fd = open(path.get(), flags, mode);
    }

    if (fd == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, fd).val_;
}

napi_value PropNExporter::ChmodSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
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

    int mode;
    tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    if (chmod(path.get(), mode) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::ChownSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
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

    int owner;
    tie(succ, owner) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid owner");
    }

    int group;
    tie(succ, group) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid group");
    }

    if (chown(path.get(), owner, group) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::CloseSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    if (close(fd) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FchmodSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int mode;
    tie(succ, mode) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid mode");
        return nullptr;
    }

    int ret = fchmod(fd, mode);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FchownSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int owner;
    tie(succ, owner) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid owner");
    }

    int group;
    tie(succ, group) = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid group");
    }

    int ret = fchown(fd, owner, group);
    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FstatSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    struct stat buf;
    if (fstat(fd, &buf) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    napi_value objStat = NClass::InstantiateClass(env, StatNExporter::className_, {});
    if (!objStat) {
        UniError(EINVAL).ThrowErr(env, "Cannot instantiate class");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, objStat);
    if (!statEntity) {
        UniError(EINVAL).ThrowErr(env, "Cannot get the entity of objStat");
        return nullptr;
    }
    statEntity->stat_ = buf;

    return objStat;
}

napi_value PropNExporter::FtruncateSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    int argc = funcArg.GetArgc();
    int ret = -1;
    if (argc == NARG_CNT::ONE) {
        ret = ftruncate(fd, 0);
    } else {
        int len;
        tie(succ, len) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid len");
            return nullptr;
        }
        ret = ftruncate(fd, len);
    }

    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::ReadSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;

    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    int64_t len;
    bool hasPos = false;
    int64_t pos;
    tie(succ, buf, len, hasPos, pos) = CommonFunc::GetReadArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t actLen;
    if (hasPos) {
        actLen = pread(fd, buf, len, pos);
    } else {
        actLen = read(fd, buf, len);
    }
    if (actLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, actLen).val_;
}

napi_value PropNExporter::RenameSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    unique_ptr<char[]> src;
    tie(succ, src, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid src");
        return nullptr;
    }

    unique_ptr<char[]> dest;
    tie(succ, dest, ignore) = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid dest");
        return nullptr;
    }

    if (rename(src.get(), dest.get()) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::RmdirSync(napi_env env, napi_callback_info info)
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

    if (rmdir(path.get()) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::UnlinkSync(napi_env env, napi_callback_info info)
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

    if (unlink(path.get()) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::FsyncSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    if (fsync(fd) == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::TruncateSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
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

    int ret = -1;
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        ret = truncate(path.get(), 0);
    } else {
        int len;
        tie(succ, len) = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid len");
        }
        ret = truncate(path.get(), len);
    }

    if (ret == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value PropNExporter::WriteSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    int fd;
    tie(succ, fd) = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fd");
        return nullptr;
    }

    void *buf = nullptr;
    size_t len;
    size_t position;
    unique_ptr<char[]> bufGuard;
    bool hasPos = false;
    tie(succ, bufGuard, buf, len, hasPos, position) =
        CommonFunc::GetWriteArg(env, funcArg[NARG_POS::SECOND], funcArg[NARG_POS::THIRD]);
    if (!succ) {
        return nullptr;
    }

    ssize_t writeLen;
    if (position == (size_t)INVALID_POSITION) {
        writeLen = write(fd, buf, len);
    } else {
        writeLen = pwrite(fd, buf, len, position);
    }

    if (writeLen == -1) {
        UniError(errno).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, writeLen).val_;
}

bool PropNExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("accessSync", AccessSync),
        NVal::DeclareNapiFunction("chmodSync", ChmodSync),
        NVal::DeclareNapiFunction("chownSync", ChownSync),
        NVal::DeclareNapiFunction("closeSync", CloseSync),
        NVal::DeclareNapiFunction("copyFileSync", CopyFileSync),
        NVal::DeclareNapiFunction("createStreamSync", CreateStream::Sync),
        NVal::DeclareNapiFunction("fchmodSync", FchmodSync),
        NVal::DeclareNapiFunction("fchownSync", FchownSync),
        NVal::DeclareNapiFunction("fdopenStreamSync", FdopenStream::Sync),
        NVal::DeclareNapiFunction("fstatSync", FstatSync),
        NVal::DeclareNapiFunction("fsyncSync", FsyncSync),
        NVal::DeclareNapiFunction("ftruncateSync", FtruncateSync),
        NVal::DeclareNapiFunction("mkdirSync", MkdirSync),
        NVal::DeclareNapiFunction("opendirSync", Opendir::Sync),
        NVal::DeclareNapiFunction("openSync", OpenSync),
        NVal::DeclareNapiFunction("readSync", ReadSync),
        NVal::DeclareNapiFunction("renameSync", RenameSync),
        NVal::DeclareNapiFunction("rmdirSync", RmdirSync),
        NVal::DeclareNapiFunction("statSync", Stat::Sync),
        NVal::DeclareNapiFunction("truncateSync", TruncateSync),
        NVal::DeclareNapiFunction("unlinkSync", UnlinkSync),
        NVal::DeclareNapiFunction("writeSync", WriteSync),
    });
}

string PropNExporter::GetClassName()
{
    return PropNExporter::className_;
}

PropNExporter::PropNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

PropNExporter::~PropNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS