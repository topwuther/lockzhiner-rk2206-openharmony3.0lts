/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KV_STORE_TASK_H
#define KV_STORE_TASK_H

#include <functional>
#include <string>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreTask {
public:
    KVSTORE_API ~KvStoreTask() {};
    KVSTORE_API KvStoreTask(std::function<void()> lambda);
    KVSTORE_API KvStoreTask(std::function<void()> lambda, const std::string &taskName);
    KVSTORE_API void operator()();

private:
    std::function<void()> task_;
    std::string name_;
};
} // namespace DistributedKv
} // namespace OHOS

#endif // TASK_H
