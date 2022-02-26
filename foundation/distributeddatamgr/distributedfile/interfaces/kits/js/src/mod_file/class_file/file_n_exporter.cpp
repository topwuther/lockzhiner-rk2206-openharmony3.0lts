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

#include "file_n_exporter.h"

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../../common/ability_helper.h"
#include "../../common/fd_guard.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"
#include "../common_func.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFile {
using namespace std;

constexpr int SUCCESS = 0;
constexpr int FAILD = -1;
constexpr int URI_PARAMER_ERROR = 202;
constexpr int FILE_IO_ERROR = 300;
constexpr int FILE_PATH_ERROR = 301;
constexpr int DIR_FAULT_PERM = 0775;
constexpr int SPLITE_ZERO = 0;
const string TYPE_FILE = "file";
const string TYPE_DIR = "dir";
const string ENCODING_UTF8 = "utf-8";

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

bool CheckUri(napi_env env, string &path)
{
    constexpr int spilteOne = 1;
    constexpr int spilteTwo = 2;
    constexpr int spilteThree = 3;
    string pathOrigin = path;
    vector<string> uriSplit;
    string pattern = "/";
    if (path == "") {
        return false;
    }
    string pathTmp = pathOrigin + pattern;
    size_t pos = pathTmp.find(pattern);
    while (pos != pathTmp.npos) {
        string temp = pathTmp.substr(SPLITE_ZERO, pos);
        uriSplit.push_back(temp);
        pathTmp = pathTmp.substr(pos + 1, pathTmp.size());
        pos = pathTmp.find(pattern);
    }
    if (uriSplit[SPLITE_ZERO] != "internal:" || uriSplit[spilteOne] != "" || uriSplit.size() <= spilteThree) {
        return false;
    }
    AppExecFwk::Ability *ability = AbilityHelper::GetJsAbility(env);
    if (!ability) {
        return false;
    }
    if (uriSplit[spilteTwo] == "app") {
        path = ability->GetDataDir();
    } else if (uriSplit[spilteTwo] == "cache") {
        path = ability->GetCacheDir();
    } else {
        return false;
    }
    for (size_t i = spilteThree; i < uriSplit.size(); ++i) {
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
    return 0;
}

string UriToAbsolute(string path)
{
    stack<string> uriResult;
    vector<string> uriSplit;
    string pattern = "/";

    string pathTmp = path + pattern;
    size_t pos = pathTmp.find(pattern);
    while (pos != pathTmp.npos) {
        string temp = pathTmp.substr(SPLITE_ZERO, pos);
        uriSplit.push_back(temp);
        pathTmp = pathTmp.substr(pos + 1, pathTmp.size());
        pos = pathTmp.find(pattern);
    }
    for (auto urisp : uriSplit) {
        if (urisp == "." || urisp == "") {
            continue;
        } else if (urisp == ".." && !uriResult.empty()) {
            uriResult.pop();
        } else {
            uriResult.push(urisp);
        }
    }
    path = "";
    while (!uriResult.empty()) {
        path = "/" + uriResult.top() + path;
        uriResult.pop();
    }
    return path;
}

bool GetFileNames(string path, vector<string> &filenames, bool rec, bool isList)
{
    DIR *pDir;
    struct dirent *ptr = nullptr;
    if (!(pDir = opendir(path.c_str()))) {
        return false;
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            if (isList) {
                filenames.push_back(path + "/" + ptr->d_name);
            } else if (ptr->d_type == DT_DIR && rec) {
                GetFileNames(path + "/" + ptr->d_name, filenames, rec, isList);
            } else if (ptr->d_type == DT_REG) {
                filenames.push_back(path + "/" + ptr->d_name);
            }
        }
    }
    closedir(pDir);
    return true;
}

bool Mkdirs(string path)
{
    for (size_t i = 1; i < path.length(); ++i) {
        if (path[i] == '/') {
            path[i] = '\0';
            if (access(path.c_str(), 0) != 0) {
                if (mkdir(path.c_str(), DIR_FAULT_PERM) == FAILD) {
                    return false;
                }
            }
            path[i] = '/';
        }
    }
    if (path.length() <= 0 || access(path.c_str(), 0) == 0 || mkdir(path.c_str(), DIR_FAULT_PERM) == FAILD) {
        return false;
    }
    return true;
}

bool Rmdirs(string path)
{
    DIR *pDir;
    struct dirent *ptr = nullptr;
    if (!(pDir = opendir(path.c_str()))) {
        return false;
    }
    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            if ((ptr->d_type == DT_DIR && Rmdirs(path + "/" + ptr->d_name)) ||
                remove((path + "/" + ptr->d_name).c_str()) == 0) {
            } else {
                closedir(pDir);
                return false;
            }
        }
    }
    closedir(pDir);
    if (rmdir(path.c_str()) != 0) {
        return false;
    }
    return true;
}

string ConvertUri(string path, string originPath, string originUri)
{
    if (path.find(originPath) != path.npos) {
        if (originUri[originUri.length() - 1] == '/') {
            originUri = originUri.substr(0, originUri.length() - 1);
        }
        path.replace(0, originPath.length(), originUri);
    } else {
        return "error";
    }
    return path;
}

void MkdirExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncMkdirCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    if (GetRealPath(path) == ENOENT) {
        path = UriToAbsolute(path);
        if (asyncCallbackInfo->recursive && Mkdirs(path)) {
            asyncCallbackInfo->result = SUCCESS;
        } else if (mkdir((char *)path.c_str(), DIR_FAULT_PERM) != FAILD) {
            asyncCallbackInfo->result = SUCCESS;
        }
    }
}

void MkdirComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncMkdirCallbackInfo *)data;

    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->result == FAILD) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "make directory failed", FILE_IO_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void RmdirExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncRmdirCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO) {
        if (asyncCallbackInfo->recursive && Rmdirs(path)) {
            asyncCallbackInfo->result = SUCCESS;
        } else if (remove((char *)path.c_str()) != FAILD) {
            asyncCallbackInfo->result = SUCCESS;
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void RmdirComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncRmdirCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "remove directory failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void GetExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncGetCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    struct stat buf;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO && stat((char *)path.c_str(), &buf) == COMMON_NUM::ZERO) {
        asyncCallbackInfo->length = buf.st_size;
        asyncCallbackInfo->lastMT = buf.st_mtime * COMMON_NUM::THOUSAND +
            (int64_t)((buf.st_mtim).tv_nsec / COMMON_NUM::MILLION);
        asyncCallbackInfo->url = path;
        std::vector<string> subFiles;
        bool rec = asyncCallbackInfo->recursive;
        if ((buf.st_mode & S_IFMT) == S_IFDIR && GetFileNames(path, subFiles, rec, false)) {
            (asyncCallbackInfo->subFiles).assign(subFiles.begin(), subFiles.end());
            asyncCallbackInfo->type = TYPE_DIR;
            asyncCallbackInfo->result = SUCCESS;
        } else if ((buf.st_mode & S_IFMT) == S_IFREG) {
            asyncCallbackInfo->type = TYPE_FILE;
            asyncCallbackInfo->result = SUCCESS;
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void GetComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncGetCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        napi_value subFilesNapi = nullptr;
        napi_create_array(env, &subFilesNapi);
        int32_t i = 0;
        for (auto filename : asyncCallbackInfo->subFiles) {
            napi_set_property(
                env, subFilesNapi, NVal::CreateInt32(env, i).val_,
                NVal::CreateUTF8String(
                    env, ConvertUri(filename, asyncCallbackInfo->url, asyncCallbackInfo->originUri).c_str())
                    .val_);
            i = i + 1;
        }
        NVal objn = NVal::CreateObject(env);
        objn.AddProp("lastModifiedTime", NVal::CreateInt64(env, asyncCallbackInfo->lastMT).val_);
        objn.AddProp("length", NVal::CreateInt32(env, asyncCallbackInfo->length).val_);
        objn.AddProp("uri", NVal::CreateUTF8String(env, asyncCallbackInfo->originUri).val_);
        objn.AddProp("type", NVal::CreateUTF8String(env, asyncCallbackInfo->type).val_);
        objn.AddProp("subFiles", subFilesNapi);

        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE, objn.val_);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "get file failed", FILE_IO_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void ListExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncListCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    std::vector<string> fileNames;
    struct stat buf;
    int statPath = GetRealPath(path);
    if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    } else if (statPath != COMMON_NUM::ZERO || stat((char *)path.c_str(), &buf) != COMMON_NUM::ZERO) {
        asyncCallbackInfo->errorType = FILE_IO_ERROR;
    } else if ((buf.st_mode & S_IFMT) == S_IFREG) {
        asyncCallbackInfo->result = SUCCESS;
    } else {
        asyncCallbackInfo->url = path;
        bool getStat = GetFileNames(path, fileNames, false, true);
        if (!getStat) {
            asyncCallbackInfo->errorType = FILE_IO_ERROR;
        } else {
            vector<FileInfo> fileList;
            for (auto ph : fileNames) {
                struct stat tmp;
                int r = stat(ph.c_str(), &tmp);
                FileInfo fi;
                if (r == 0 && S_ISDIR(tmp.st_mode)) {
                    fi.type = TYPE_DIR;
                } else if (r == 0 && (tmp.st_mode & S_IFMT) == S_IFREG) {
                    fi.type = TYPE_FILE;
                }
                fi.length = tmp.st_size;
                fi.lastModifiedTime = tmp.st_mtime * COMMON_NUM::THOUSAND +
                    (int64_t)((tmp.st_mtim).tv_nsec / COMMON_NUM::MILLION);
                fi.uri = ph;
                fileList.push_back(fi);
            }
            (asyncCallbackInfo->fileList).assign(fileList.begin(), fileList.end());
            asyncCallbackInfo->result = SUCCESS;
        }
    }
}

void ListComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncListCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        napi_value fileListNapi;
        napi_create_array(env, &fileListNapi);
        int32_t i = 0;
        for (auto fileInfo : asyncCallbackInfo->fileList) {
            NVal objt = NVal::CreateObject(env);
            objt.AddProp("lastModifiedTime", NVal::CreateInt64(env, fileInfo.lastModifiedTime).val_);
            objt.AddProp("length", NVal::CreateInt32(env, fileInfo.length).val_);
            string uriTojs = ConvertUri(fileInfo.uri, asyncCallbackInfo->url, asyncCallbackInfo->originUri);
            objt.AddProp("uri", NVal::CreateUTF8String(env, uriTojs).val_);
            objt.AddProp("type", NVal::CreateUTF8String(env, fileInfo.type).val_);

            napi_set_property(env, fileListNapi, NVal::CreateInt32(env, i).val_, objt.val_);
            i = i + 1;
        }
        NVal objn = NVal::CreateObject(env);
        objn.AddProp("fileList", fileListNapi);
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE, objn.val_);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "list file failed", FILE_IO_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void CopyExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncCopyCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    string pathDst = asyncCallbackInfo->urlDst;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int statPath = GetRealPath(path);
    int statDst = GetRealPath(pathDst);
    if (statPath == COMMON_NUM::ZERO && statDst == ENOENT) {
        pathDst = UriToAbsolute(pathDst);
        struct stat statbf;
        FDGuard sfd;
        FDGuard ofd;
        sfd.SetFD(open((char *)path.c_str(), O_RDONLY));
        int res = stat((char *)path.c_str(), &statbf);
        ofd.SetFD(open((char *)pathDst.c_str(), O_WRONLY | O_CREAT, statbf.st_mode));
        if (sfd.GetFD() != FAILD && ofd.GetFD() != FAILD && res != FAILD &&
            sendfile(ofd.GetFD(), sfd.GetFD(), nullptr, statbf.st_size) != FAILD) {
            asyncCallbackInfo->result = SUCCESS;
        }
        if (asyncCallbackInfo->result == FAILD) {
            remove((char *)pathDst.c_str());
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void CopyComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncCopyCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE,
                        NVal::CreateUTF8String(env, asyncCallbackInfo->originDst).val_);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "copy file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void MoveExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncMoveCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    string pathDst = asyncCallbackInfo->urlDst;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int statPath = GetRealPath(path);
    int statDst = GetRealPath(pathDst);
    if (statPath == COMMON_NUM::ZERO && statDst == ENOENT) {
        pathDst = UriToAbsolute(pathDst);
        struct stat statbf;
        FDGuard sfd;
        FDGuard ofd;
        sfd.SetFD(open((char *)path.c_str(), O_RDONLY));
        int res = stat((char *)path.c_str(), &statbf);
        ofd.SetFD(open((char *)pathDst.c_str(), O_WRONLY | O_CREAT, statbf.st_mode));
        if (sfd.GetFD() != FAILD && ofd.GetFD() != FAILD && res != FAILD &&
            sendfile(ofd.GetFD(), sfd.GetFD(), nullptr, statbf.st_size) != FAILD) {
            asyncCallbackInfo->result = SUCCESS;
            remove((char *)path.c_str());
        }
        if (asyncCallbackInfo->result == FAILD) {
            asyncCallbackInfo->errorType = FILE_IO_ERROR;
            remove((char *)pathDst.c_str());
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void MoveComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncMoveCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE,
                        NVal::CreateUTF8String(env, asyncCallbackInfo->originDst).val_);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "move file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void DeleteExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncDeleteCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    int statPath = GetRealPath(path);
    if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    } else if (statPath == COMMON_NUM::ZERO && remove((char *)path.c_str()) != FAILD) {
        asyncCallbackInfo->result = SUCCESS;
    } else {
        asyncCallbackInfo->errorType = FILE_IO_ERROR;
    }
}

void DeleteComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncDeleteCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "delete file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void AccessExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncAccessCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    int statPath = GetRealPath(path);
    if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    } else if (statPath == COMMON_NUM::ZERO) {
        asyncCallbackInfo->result = SUCCESS;
    } else {
        asyncCallbackInfo->errorType = FILE_IO_ERROR;
    }
}

void AccessComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncAccessCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "access file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void WriteTextExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncWriteCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    string text = asyncCallbackInfo->text;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int fd = -1;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO || statPath == ENOENT) {
        if (asyncCallbackInfo->append) {
            fd = open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
        } else {
            fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        }
        if (fd != FAILD) {
            if (write(fd, text.c_str(), text.length()) != FAILD) {
                asyncCallbackInfo->result = SUCCESS;
            }
            close(fd);
        }
    }
}

void WriteTextComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncWriteCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "write file failed", FILE_IO_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void WriteArrayBufferExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncWriteBufferCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int fd = -1;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO || statPath == ENOENT) {
        if (asyncCallbackInfo->append) {
            fd = open(path.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd == FAILD) {
                return;
            }
        } else {
            fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd == FAILD) {
                return;
            }
            lseek(fd, asyncCallbackInfo->position, SEEK_CUR);
        }
        if (write(fd, asyncCallbackInfo->buf, asyncCallbackInfo->length) != FAILD) {
            asyncCallbackInfo->result = SUCCESS;
        }
        close(fd);
    }
}

void WriteArrayBufferComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncWriteBufferCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ZERO, nullptr);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "write file failed", FILE_IO_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->bufferAddress);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void ReadTextExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncReadCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO) {
        FDGuard fdg;
        fdg.SetFD(open(path.c_str(), O_RDONLY));
        struct stat buf;
        int result = stat((char *)path.c_str(), &buf);
        if (fdg.GetFD() != FAILD && result != FAILD) {
            char *buffer = new char[buf.st_size];
            if (read(fdg.GetFD(), buffer, buf.st_size) != FAILD) {
                asyncCallbackInfo->result = SUCCESS;
                asyncCallbackInfo->contents = buffer;
            }
            delete[] buffer;
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void ReadTextComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncReadCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        NVal objn = NVal::CreateObject(env);
        objn.AddProp("text", NVal::CreateUTF8String(env, asyncCallbackInfo->contents).val_);
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE, objn.val_);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "read file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

void ReadArrayBufferExec(napi_env env, void *data)
{
    auto *asyncCallbackInfo = (AsyncReadBufferCallbackInfo *)data;
    string path = asyncCallbackInfo->url;
    asyncCallbackInfo->result = FAILD;
    asyncCallbackInfo->errorType = FILE_IO_ERROR;
    int statPath = GetRealPath(path);
    if (statPath == COMMON_NUM::ZERO) {
        FDGuard fdg;
        fdg.SetFD(open(path.c_str(), O_RDONLY));
        struct stat buf;
        int result = stat((char *)path.c_str(), &buf);
        if (fdg.GetFD() != FAILD && result != FAILD) {
            int32_t begin = (buf.st_size < asyncCallbackInfo->position) ? buf.st_size : asyncCallbackInfo->position;
            int32_t len =
                (asyncCallbackInfo->length == COMMON_NUM::ZERO) ? (buf.st_size - begin) : asyncCallbackInfo->length;
            char *buffer = new char[len + 1];
            lseek(fdg.GetFD(), begin, SEEK_CUR);
            if (read(fdg.GetFD(), buffer, len) != FAILD) {
                asyncCallbackInfo->result = SUCCESS;
                asyncCallbackInfo->len = len;
                asyncCallbackInfo->contents = buffer;
            }
            delete[] buffer;
        }
    } else if (statPath == ENOENT) {
        asyncCallbackInfo->errorType = FILE_PATH_ERROR;
    }
}

void ReadArrayBufferComp(napi_env env, napi_status status, void *data)
{
    auto *asyncCallbackInfo = (AsyncReadBufferCallbackInfo *)data;
    if (asyncCallbackInfo->result == SUCCESS) {
        napi_value typeArr = nullptr;
        napi_create_array(env, &typeArr);
        for (int32_t i = 0; i < asyncCallbackInfo->len; ++i) {
            napi_set_property(env, typeArr, NVal::CreateInt32(env, i).val_,
                              NVal::CreateInt32(env, (int32_t)(asyncCallbackInfo->contents)[i]).val_);
        }
        NVal objn = NVal::CreateObject(env);
        objn.AddProp("buffer", typeArr);
        CallBackSuccess(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO], COMMON_NUM::ONE, objn.val_);
    } else if (asyncCallbackInfo->errorType == FILE_IO_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "read file failed", FILE_IO_ERROR);
    } else if (asyncCallbackInfo->errorType == FILE_PATH_ERROR) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "file not exist", FILE_PATH_ERROR);
    }
    CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ZERO]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::ONE]);
    napi_delete_reference(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

napi_value FileNExporter::Mkdir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncMkdirCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    bool recursive = false;
    tie(succ, recursive) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("recursive").ToBool();

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->recursive = recursive;
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, MkdirExec, MkdirComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Rmdir(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncRmdirCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    bool recursive = false;
    tie(succ, recursive) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("recursive").ToBool();

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->recursive = recursive;
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, RmdirExec, RmdirComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Get(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto *asyncCallbackInfo = new AsyncGetCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    bool succ = false;
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);
    unique_ptr<char[]> uri = nullptr;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    bool recursive = false;
    tie(succ, recursive) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("recursive").ToBool();
    string path = (uri == nullptr) ? "" : uri.get();
    asyncCallbackInfo->originUri = path;
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->recursive = recursive;
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, GetExec, GetComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::List(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto *asyncCallbackInfo = new AsyncListCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    bool succ = false;
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri = nullptr;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    string path = (uri == nullptr) ? "" : uri.get();
    asyncCallbackInfo->originUri = path;
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, ListExec, ListComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Copy(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncCopyCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> srcUri, dstUri;
    tie(succ, srcUri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("srcUri").ToUTF8String();
    tie(succ, dstUri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("dstUri").ToUTF8String();
    string srcPath = ((srcUri == nullptr) ? "" : (srcUri.get()));
    string dstPath = ((dstUri == nullptr) ? "" : (dstUri.get()));
    asyncCallbackInfo->originDst = dstPath;
    if (!CheckUri(env, srcPath) || !CheckUri(env, dstPath)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = srcPath;
    asyncCallbackInfo->urlDst = dstPath;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, CopyExec, CopyComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Move(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncMoveCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> srcUri, dstUri;
    tie(succ, srcUri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("srcUri").ToUTF8String();
    tie(succ, dstUri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("dstUri").ToUTF8String();

    string srcPath = ((srcUri == nullptr) ? "" : (srcUri.get()));
    string dstPath = ((dstUri == nullptr) ? "" : (dstUri.get()));
    asyncCallbackInfo->originDst = dstPath;
    if (!CheckUri(env, srcPath) || !CheckUri(env, dstPath)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = srcPath;
    asyncCallbackInfo->urlDst = dstPath;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, MoveExec, MoveComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Delete(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncDeleteCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, DeleteExec, DeleteComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);

    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::Access(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    AsyncAccessCallbackInfo *asyncCallbackInfo = new AsyncAccessCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, AccessExec, AccessComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::WriteText(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto *asyncCallbackInfo = new AsyncWriteCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    bool succ = false;
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri, text, encoding;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();
    tie(succ, text, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("text").ToUTF8String();
    tie(succ, encoding, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("encoding").ToUTF8String();

    bool append = false;
    tie(succ, append) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("append").ToBool();

    string path = (uri == nullptr) ? "" : uri.get();
    string encode = (encoding == nullptr) ? ENCODING_UTF8 : encoding.get();
    transform(encode.begin(), encode.end(), encode.begin(), ::tolower);
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    if (encode != ENCODING_UTF8) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "write file failed", FILE_IO_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    string content = text.get();
    asyncCallbackInfo->url = path;
    asyncCallbackInfo->text = content;
    asyncCallbackInfo->append = append;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, WriteTextExec, WriteTextComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::WriteArrayBuffer(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncWriteBufferCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    int32_t position = 0;
    tie(succ, position) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("position").ToInt32();

    bool append = false;
    tie(succ, append) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("append").ToBool();

    void *buffer = nullptr;
    size_t bufLength = 0;
    napi_ref bufferRef = nullptr;
    NVal bufNapi = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("buffer");
    tie(succ, buffer, bufLength) = bufNapi.ToTypedArray();
    napi_create_reference(env, bufNapi.val_, 1, &bufferRef);

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;
    asyncCallbackInfo->position = position;
    asyncCallbackInfo->append = append;
    asyncCallbackInfo->buf = buffer;
    asyncCallbackInfo->length = bufLength;
    asyncCallbackInfo->bufferAddress = bufferRef;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, WriteArrayBufferExec,
                           WriteArrayBufferComp, (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::ReadText(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    bool succ = false;
    auto *asyncCallbackInfo = new AsyncReadCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri, encoding;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();
    tie(succ, encoding, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("encoding").ToUTF8String();

    string path = (uri == nullptr) ? "" : uri.get();
    string encode = (encoding == nullptr) ? ENCODING_UTF8 : encoding.get();
    transform(encode.begin(), encode.end(), encode.begin(), ::tolower);
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    if (encode != ENCODING_UTF8) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "read file failed", FILE_IO_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, ReadTextExec, ReadTextComp,
                           (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

napi_value FileNExporter::ReadArrayBuffer(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    auto *asyncCallbackInfo = new AsyncReadBufferCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };
    bool succ = false;
    tie(succ, asyncCallbackInfo->callback[COMMON_NUM::ZERO], asyncCallbackInfo->callback[COMMON_NUM::ONE],
        asyncCallbackInfo->callback[COMMON_NUM::TWO]) = CommonFunc::GetCallbackHandles(env, funcArg[NARG_POS::FIRST]);

    unique_ptr<char[]> uri;
    tie(succ, uri, ignore) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("uri").ToUTF8String();

    int position = 0;
    tie(succ, position) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("position").ToInt32();

    int length = 0;
    tie(succ, length) = NVal(env, funcArg[NARG_POS::FIRST]).GetProp("length").ToInt32();

    string path = (uri == nullptr) ? "" : uri.get();
    if (!CheckUri(env, path)) {
        CallBackError(env, asyncCallbackInfo->callback[COMMON_NUM::ONE], "illegal uri", URI_PARAMER_ERROR);
        CallComplete(env, asyncCallbackInfo->callback[COMMON_NUM::TWO]);
        delete asyncCallbackInfo;
        return nullptr;
    }
    asyncCallbackInfo->url = path;
    asyncCallbackInfo->length = length;
    asyncCallbackInfo->position = position;

    napi_create_async_work(env, nullptr, NVal::CreateUTF8String(env, "ResourceName").val_, ReadArrayBufferExec,
                           ReadArrayBufferComp, (void *)asyncCallbackInfo, &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    return NVal::CreateUndefined(env).val_;
}

bool FileNExporter::Export()
{
    return exports_.AddProp({
        NVal::DeclareNapiFunction("mkdir", Mkdir),
        NVal::DeclareNapiFunction("rmdir", Rmdir),
        NVal::DeclareNapiFunction("get", Get),
        NVal::DeclareNapiFunction("list", List),
        NVal::DeclareNapiFunction("copy", Copy),
        NVal::DeclareNapiFunction("move", Move),
        NVal::DeclareNapiFunction("delete", Delete),
        NVal::DeclareNapiFunction("access", Access),
        NVal::DeclareNapiFunction("writeText", WriteText),
        NVal::DeclareNapiFunction("writeArrayBuffer", WriteArrayBuffer),
        NVal::DeclareNapiFunction("readText", ReadText),
        NVal::DeclareNapiFunction("readArrayBuffer", ReadArrayBuffer),
    });
}

string FileNExporter::GetClassName()
{
    return FileNExporter::className_;
}

FileNExporter::FileNExporter(napi_env env, napi_value exports)
    : NExporter(env, exports)
{}

FileNExporter::~FileNExporter() {}
} // namespace ModuleFile
} // namespace DistributedFS
} // namespace OHOS