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

#include "../../common/napi/n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
class DirentNExporter final : public NExporter {
public:
    inline static const std::string className_ = "Dirent";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info cbinfo);

    static napi_value isBlockDevice(napi_env env, napi_callback_info cbinfo);
    static napi_value isCharacterDevice(napi_env env, napi_callback_info cbinfo);
    static napi_value isDirectory(napi_env env, napi_callback_info cbinfo);
    static napi_value isFIFO(napi_env env, napi_callback_info cbinfo);
    static napi_value isFile(napi_env env, napi_callback_info cbinfo);
    static napi_value isSocket(napi_env env, napi_callback_info cbinfo);
    static napi_value isSymbolicLink(napi_env env, napi_callback_info cbinfo);

    static napi_value GetName(napi_env env, napi_callback_info cbinfo);

    DirentNExporter(napi_env env, napi_value exports);
    ~DirentNExporter() override;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS