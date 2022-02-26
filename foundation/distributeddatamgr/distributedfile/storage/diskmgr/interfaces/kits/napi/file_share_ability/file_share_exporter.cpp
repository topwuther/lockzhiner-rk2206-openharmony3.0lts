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

#include "file_share_exporter.h"

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <stack>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../common/ability_helper.h"
#include "../common/common_func.h"
#include "../common/napi/n_class.h"
#include "../common/napi/n_func_arg.h"
#include "../common/napi/n_val.h"
#include "../common/uni_error.h"
#include "file_share_ability.h"
#include "uri.h"

using Uri = OHOS::Uri;
using namespace std;
namespace OHOS {
namespace DistributedFS {
namespace ModuleFMSExpoter {
const int OTHER_ARGUMENT_ERROR = 202;
const int URI_PARAMER_ERROR = 302;
const int FILE_IO_ERROR = 300;
const int FILE_PATH_ERROR = 301;
const int SUCCESS = 0;
const int FAILED = -1;
enum COMMON_NUM {
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
};

void CallBackSuccess(napi_env env, napi_ref successFuncRef, int32_t count, napi_value obj)
{
    napi_value results = nullptr;
    napi_value successFunc = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_get_reference_value(env, successFuncRef, &successFunc);
    if (successFunc == nullptr) {
        return;
    }
    napi_call_function(env, global, successFunc, count, &obj, &results);
}

void CallBackError(napi_env env, napi_ref failFuncRef, string errorProp, int errorCode)
{
    napi_value argvFail[2] = { 0 };
    napi_value results = nullptr;
    napi_value failFunc = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    argvFail[0] = NVal::CreateUTF8String(env, errorProp).val_;
    argvFail[1] = NVal::CreateInt32(env, errorCode).val_;
    napi_get_reference_value(env, failFuncRef, &failFunc);
    if (failFunc == nullptr) {
        return;
    }
    napi_call_function(env, global, failFunc, COMMON_NUM::TWO, argvFail, &results);
}

void CallComplete(napi_env env, napi_ref completeFuncRef)
{
    napi_value completeFunc = nullptr;
    napi_value results = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_get_reference_value(env, completeFuncRef, &completeFunc);
    if (completeFunc == nullptr) {
        return;
    }
    napi_call_function(env, global, completeFunc, COMMON_NUM::ZERO, nullptr, &results);
}

bool CheckUri(AppExecFwk::Ability *ability, napi_env env, string &path)
{
    string pathOrigin = path;
    vector<string> uriSplit;
    string pattern = "/";
    if (path == "") {
        return false;
    }
    string pathTmp = pathOrigin + pattern;
    size_t pos = pathTmp.find(pattern);
    while (pos != pathTmp.npos) {
        string temp = pathTmp.substr(COMMON_NUM::ZERO, pos);
        uriSplit.push_back(temp);
        pathTmp = pathTmp.substr(pos + 1, pathTmp.size());
        pos = pathTmp.find(pattern);
    }
    if (uriSplit[COMMON_NUM::ZERO] != "internal:" || uriSplit[COMMON_NUM::ONE] != "" ||
        uriSplit.size() <= COMMON_NUM::TWO) {
        return false;
    }
    if (uriSplit[COMMON_NUM::TWO] == "app") {
        path = ability->GetDataDir();
    } else if (uriSplit[COMMON_NUM::TWO] == "cache") {
        path = ability->GetCacheDir();
    } else {
        return false;
    }
    for (size_t i = COMMON_NUM::THREE; i < uriSplit.size(); ++i) {
        path = path + "/" + uriSplit[i];
    }
    return true;
}

int GetRealPath(string &path)
{
    unique_ptr<char[]> absPath = make_unique<char[]>(PATH_MAX + 1);
    if (realpath(path.c_str(), absPath.get()) == nullptr) {
        return errno;
    }
    path = absPath.get();
    return SUCCESS;
}

void CallbackUriResult(napi_env env, napi_ref napiFailFun, napi_ref napiSuccFun, string uriRet)
{
    if (uriRet == "") {
        CallBackError(env, napiFailFun, "error: fuzzyFileToUri failed", FILE_IO_ERROR);
    } else if (uriRet == "ERROR_AUTHORITY") {
        CallBackError(env, napiFailFun, "error: authority is not exist", OTHER_ARGUMENT_ERROR);
    } else if (uriRet == "ERROR_JSON_CONFIG") {
        CallBackError(env, napiFailFun, "error: invalid json config", FILE_IO_ERROR);
    } else {
        CallBackSuccess(env, napiSuccFun, COMMON_NUM::ONE, NVal::CreateUTF8String(env, uriRet).val_);
    }
}

int CheckArgumentsError(napi_env env, napi_ref napiFailFun, string deviceIdStr, string authorityStr)
{
    if (deviceIdStr == "" || authorityStr == "") {
        CallBackError(env, napiFailFun, "error: illegal arguments", OTHER_ARGUMENT_ERROR);
        return FAILED;
    }
    return SUCCESS;
}

int CheckUriError(napi_env env, napi_ref napiFailFun, AppExecFwk::Ability *ability, int checkPathResult, string path)
{
    if (!checkPathResult) {
        CallBackError(env, napiFailFun, "error: illegal uri", URI_PARAMER_ERROR);
        return FAILED;
    } else if (path.find(ability->GetDataDir()) != 0 && path.find(ability->GetCacheDir()) != 0) {
        CallBackError(env, napiFailFun, "error: uri cannot out of this package", URI_PARAMER_ERROR);
        return FAILED;
    }
    return SUCCESS;
}

int CheckFilePathError(napi_env env, napi_ref napiFailFun, int realPathResult)
{
    if (realPathResult == ENOENT) {
        CallBackError(env, napiFailFun, "error: file or directory not exist", FILE_PATH_ERROR);
        return FAILED;
    }
    return SUCCESS;
}

int CheckIOError(napi_env env, napi_ref napiFailFun, int realPathResult, string path)
{
    struct stat buf;
    if (realPathResult != SUCCESS) {
        CallBackError(env, napiFailFun, "error: invalid uri", FILE_IO_ERROR);
        return FAILED;
    } else if (stat(path.c_str(), &buf) != SUCCESS || (buf.st_mode & S_IFMT) != S_IFREG) {
        CallBackError(env, napiFailFun, "error: uri cannot be a directory", FILE_IO_ERROR);
        return FAILED;
    }
    return SUCCESS;
}

napi_value FileShareExporter::FuzzyFileToUri(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    napi_ref napiSuccFun, napiCompFun, napiFailFun;
    tie(succ, napiSuccFun, napiFailFun, napiCompFun) =
        CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri = nullptr;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    unique_ptr<char[]> deviceId = nullptr;
    tie(succ, deviceId, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("deviceId").ToUTF8String();

    unique_ptr<char[]> authority = nullptr;
    tie(succ, authority, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("authority").ToUTF8String();

    unique_ptr<char[]> displayname = nullptr;
    tie(succ, displayname, ignore) =
        NVal(env, funcArg[NARG_POS::FIRST]).GetProp("displayName").ToUTF8String();
    string path = (uri == nullptr) ? "" : uri.get();
    string deviceIdStr = (deviceId == nullptr) ? "" : deviceId.get();
    string authorityStr = (authority == nullptr) ? "" : authority.get();
    string displaynameStr = (displayname == nullptr) ? "" : displayname.get();

    AppExecFwk::Ability *ability = AbilityHelper::GetJsAbility(env);
    bool checkPathResult = CheckUri(ability, env, path);
    int realPathResult = GetRealPath(path);

    if (CheckArgumentsError(env, napiFailFun, deviceIdStr, authorityStr) == FAILED ||
        CheckUriError(env, napiFailFun, ability, checkPathResult, path) == FAILED ||
        CheckFilePathError(env, napiFailFun, realPathResult) == FAILED ||
        CheckIOError(env, napiFailFun, realPathResult, path) == FAILED) {
        CallComplete(env, napiCompFun);
        napi_delete_reference(env, napiSuccFun);
        napi_delete_reference(env, napiFailFun);
        napi_delete_reference(env, napiCompFun);
        return nullptr;
    }

    Uri uriObj("");
    uriObj = FileManager::FileShareAbility::FuzzyFileToUri(ability, deviceIdStr, authorityStr, path, displaynameStr);

    string uriRet = uriObj.ToString();
    CallbackUriResult(env, napiFailFun, napiSuccFun, uriRet);

    CallComplete(env, napiCompFun);
    napi_delete_reference(env, napiSuccFun);
    napi_delete_reference(env, napiFailFun);
    napi_delete_reference(env, napiCompFun);
    return NVal::CreateUndefined(env).val_;
}

bool FileShareExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("fuzzyFileToUri", FuzzyFileToUri),
    });
}

string FileShareExporter::GetClassName()
{
    return FileShareExporter::className_;
}

FileShareExporter::FileShareExporter(napi_env env, napi_value exports)
    : NExporter(env, exports)
{}

FileShareExporter::~FileShareExporter() {}
} // namespace ModuleFMSExpoter
} // namespace DistributedFS
} // namespace OHOS