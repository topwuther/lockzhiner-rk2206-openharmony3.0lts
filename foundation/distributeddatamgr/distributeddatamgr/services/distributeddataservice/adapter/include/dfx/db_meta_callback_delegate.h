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

#ifndef DISTRIBUTEDDATAMGR_CAL_DATABASE_SIZE_H
#define DISTRIBUTEDDATAMGR_CAL_DATABASE_SIZE_H

#include <vector>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
struct StoreInfo {
    std::string userId;
    std::string appId;
    std::string storeId;
    std::string GetKey()
    {
        return userId + appId + storeId;
    }
};
class DbMetaCallbackDelegate {
public:
    KVSTORE_API virtual ~DbMetaCallbackDelegate() {}
    KVSTORE_API virtual bool GetKvStoreDiskSize(const std::string &storeId, uint64_t &size) = 0;
    KVSTORE_API virtual void GetKvStoreKeys(std::vector<StoreInfo> &entries) = 0;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_CAL_DATABASE_SIZE_H
