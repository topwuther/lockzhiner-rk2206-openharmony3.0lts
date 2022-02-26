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

#include "../common/napi/uni_header.h"

#include <limits>
#include <memory>
#include <tuple>

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
constexpr int64_t INVALID_POSITION = std::numeric_limits<decltype(INVALID_POSITION)>::max();

struct CommonFunc {
    static std::tuple<bool, void *, int64_t, bool, int64_t> GetReadArg(napi_env env,
                                                                       napi_value readBuf,
                                                                       napi_value option);
    static std::tuple<bool, std::unique_ptr<char[]>, void *, int64_t, bool, int64_t> GetWriteArg(napi_env env,
                                                                                                 napi_value argWBuf,
                                                                                                 napi_value argOption);
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS