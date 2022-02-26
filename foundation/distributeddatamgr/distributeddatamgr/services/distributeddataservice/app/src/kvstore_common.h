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

#ifndef KVSTORE_COMMON_H
#define KVSTORE_COMMON_H

#include "types.h"

namespace OHOS::DistributedKv {
struct KvStoreParams {
    Options options;
    bool deviceCoordinate;
    std::string deviceAccountId;
    std::string appId;
    std::string storeId;
    std::string appDirectory;
};
}
#endif // KVSTORE_COMMON_H
