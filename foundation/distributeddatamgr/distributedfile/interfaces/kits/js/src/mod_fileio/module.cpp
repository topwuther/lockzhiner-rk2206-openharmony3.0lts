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

#include <memory>
#include <vector>

#include "../common/log.h"
#include "class_dir/dir_n_exporter.h"
#include "class_dirent/dirent_n_exporter.h"
#include "class_stat/stat_n_exporter.h"
#include "class_stream/stream_n_exporter.h"
#include "properties/prop_n_exporter.h"

using namespace std;

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
static napi_value Export(napi_env env, napi_value exports)
{
    std::vector<unique_ptr<NExporter>> products;
    products.emplace_back(make_unique<PropNExporter>(env, exports));
    products.emplace_back(make_unique<DirentNExporter>(env, exports));
    products.emplace_back(make_unique<DirNExporter>(env, exports));
    products.emplace_back(make_unique<StatNExporter>(env, exports));
    products.emplace_back(make_unique<StreamNExporter>(env, exports));

    for (auto &&product : products) {
        if (!product->Export()) {
            HILOGE("INNER BUG. Failed to export class %{public}s for module fileio", product->GetClassName().c_str());
            return nullptr;
        } else {
            HILOGE("Class %{public}s for module fileio has been exported", product->GetClassName().c_str());
        }
    }
    return exports;
}

NAPI_MODULE(fileio, Export)
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS