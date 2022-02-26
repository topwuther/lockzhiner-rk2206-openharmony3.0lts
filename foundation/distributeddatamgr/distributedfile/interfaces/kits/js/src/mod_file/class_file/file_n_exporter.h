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

#pragma once

#include "ability.h"

#include "../../common/napi/n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFile {
enum COMMON_NUM {
    ZERO = 0,
    ONE = 1,
    TWO = 2,
    THOUSAND = 1000,
    MILLION = 1000000,
};

struct FileInfo {
    int32_t length = 0;
    int64_t lastModifiedTime = 0;
    std::string type = "";
    std::string uri = "";
};

struct AsyncAccessCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    int errorType = -1;
    int result = -100;
};

struct AsyncMkdirCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    bool recursive = false;
    std::string url = "";
    int result = -100;
    int errorType = -1;
};

struct AsyncRmdirCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    bool recursive = false;
    std::string url = "";
    int result = -100;
    int errorType = -1;
};

struct AsyncGetCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    bool recursive = false;
    std::string url = "";
    std::string originUri = "";
    int result = -100;
    int errorType = -1;
    int32_t length = 0;
    int64_t lastMT = 0;
    std::string type = "";
    std::vector<std::string> subFiles;
};

struct AsyncListCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    bool recursive = false;
    std::string url = "";
    std::string originUri = "";
    int result = -100;
    int errorType = -1;
    std::vector<FileInfo> fileList;
};

struct AsyncCopyCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    std::string urlDst = "";
    std::string originDst = "";
    int result = -100;
    int errorType = -1;
};

struct AsyncMoveCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    std::string urlDst = "";
    std::string originDst = "";
    int result = -100;
    int errorType = -1;
};

struct AsyncDeleteCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    int result = -100;
    int errorType = -1;
};

struct AsyncWriteCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    std::string text = "";
    bool append = false;
    int result = -100;
    int errorType = -1;
};

struct AsyncWriteBufferCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    bool append = false;
    int result = -100;
    int errorType = -1;
    int32_t length = 0;
    int32_t position = 0;
    void* buf = nullptr;
    napi_ref bufferAddress = nullptr;
};

struct AsyncReadCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    int result = -100;
    int errorType = -1;
    std::string contents = "";
};

struct AsyncReadBufferCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback[3] = { 0 };
    std::string url = "";
    int length = 0;
    int position = 0;
    int result = -100;
    int errorType = -1;
    int32_t len = 0;
    std::string contents = "";
};

class FileNExporter final : public NExporter {
public:
    inline static const std::string className_ = "File";
    static napi_value Mkdir(napi_env env, napi_callback_info info);
    static napi_value Rmdir(napi_env env, napi_callback_info info);
    static napi_value Get(napi_env env, napi_callback_info info);
    static napi_value List(napi_env env, napi_callback_info info);
    static napi_value Copy(napi_env env, napi_callback_info info);
    static napi_value Move(napi_env env, napi_callback_info info);
    static napi_value Delete(napi_env env, napi_callback_info info);
    static napi_value Access(napi_env env, napi_callback_info info);
    static napi_value WriteText(napi_env env, napi_callback_info info);
    static napi_value WriteArrayBuffer(napi_env env, napi_callback_info info);
    static napi_value ReadText(napi_env env, napi_callback_info info);
    static napi_value ReadArrayBuffer(napi_env env, napi_callback_info info);

    bool Export() override;

    std::string GetClassName() override;

    FileNExporter(napi_env env, napi_value exports);
    ~FileNExporter() override;
};
}  // namespace ModuleFile
}  // namespace DistributedFS
}  // namespace OHOS