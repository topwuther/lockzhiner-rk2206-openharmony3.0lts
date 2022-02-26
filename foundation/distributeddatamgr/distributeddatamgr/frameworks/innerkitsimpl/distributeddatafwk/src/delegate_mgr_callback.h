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

#ifndef DISTRIBUTEDDATAMGR_DELEGATE_MGR_CALLBACK_H
#define DISTRIBUTEDDATAMGR_DELEGATE_MGR_CALLBACK_H

#include "db_meta_callback_delegate.h"

namespace OHOS {
namespace AppDistributedKv {
class DelegateMgrCallback : public DistributedKv::DbMetaCallbackDelegate {
public:
    virtual ~DelegateMgrCallback() {}

    explicit DelegateMgrCallback(DistributedDB::KvStoreDelegateManager *delegate)
        : delegate_(delegate) {}

    bool GetKvStoreDiskSize(const std::string &storeId, uint64_t &size) override
    {
        if (IsDestruct()) {
            return false;
        }
        DistributedDB::DBStatus ret = delegate_->GetKvStoreDiskSize(storeId, size);
        if (ret != DistributedDB::DBStatus::OK) {
            return false;
        }
        return true;
    }

    void GetKvStoreKeys(std::vector<DistributedKv::StoreInfo> &entries) override
    {
    }

    bool IsDestruct()
    {
        return delegate_ == nullptr;
    }
private:
    DistributedDB::KvStoreDelegateManager *delegate_ {};
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_DELEGATE_MGR_CALLBACK_H
