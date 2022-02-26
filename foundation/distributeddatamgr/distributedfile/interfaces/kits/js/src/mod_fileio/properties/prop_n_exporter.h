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
class PropNExporter final : public NExporter {
public:
    inline static const std::string className_ = "__properities__";

    static napi_value OpenSync(napi_env env, napi_callback_info info);
    static napi_value AccessSync(napi_env env, napi_callback_info info);
    static napi_value ChmodSync(napi_env env, napi_callback_info info);
    static napi_value ChownSync(napi_env env, napi_callback_info info);
    static napi_value CloseSync(napi_env env, napi_callback_info info);
    static napi_value CopyFileSync(napi_env env, napi_callback_info info);
    static napi_value FchmodSync(napi_env env, napi_callback_info info);
    static napi_value FchownSync(napi_env env, napi_callback_info info);
    static napi_value FstatSync(napi_env env, napi_callback_info info);

    static napi_value FtruncateSync(napi_env env, napi_callback_info info);
    static napi_value MkdirSync(napi_env env, napi_callback_info info);
    static napi_value ReadSync(napi_env env, napi_callback_info info);
    static napi_value RenameSync(napi_env env, napi_callback_info info);
    static napi_value RmdirSync(napi_env env, napi_callback_info info);
    static napi_value UnlinkSync(napi_env env, napi_callback_info info);
    static napi_value FsyncSync(napi_env env, napi_callback_info info);
    static napi_value TruncateSync(napi_env env, napi_callback_info info);
    static napi_value WriteSync(napi_env env, napi_callback_info info);

    static napi_value OpenDirSync(napi_env env, napi_callback_info info);

    bool Export() override;
    std::string GetClassName() override;

    PropNExporter(napi_env env, napi_value exports);
    ~PropNExporter() override;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS